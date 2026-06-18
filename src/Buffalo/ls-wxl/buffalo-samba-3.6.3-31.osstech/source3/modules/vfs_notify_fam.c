/*
 * FAM file notification support.
 *
 * Copyright (c) James Peach 2005
 * Copyright (c) Volker Lendecke 2007
 * Copyright (c) SATOH Fumiyasu 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"
#include "smbd/smbd.h"
#include "librpc/gen_ndr/notify.h"

#include <fam.h>

static int vfs_notify_fam_debug_level = DBGC_VFS;

#undef DBGC_CLASS
#define DBGC_CLASS vfs_notify_fam_debug_level

#if !defined(HAVE_FAM_H_FAMCODES_TYPEDEF)
/* Gamin provides this typedef which means we can't use 'enum FAMCodes' as per
 * every other FAM implementation. Phooey.
 */
typedef enum FAMCodes FAMCodes;
#endif

/* NOTE: There are multiple versions of FAM floating around the net, each with
 * slight differences from the original SGI FAM implementation. In this file,
 * we rely only on the SGI features and do not assume any extensions. For
 * example, we do not look at FAMErrno, because it is not set by the original
 * implementation.
 *
 * Random FAM links:
 *	http://oss.sgi.com/projects/fam/
 *	http://savannah.nongnu.org/projects/fam/
 *	http://sourceforge.net/projects/bsdfam/
 */

/* ------------------------------------------------------------------------- */

struct fam_connection_context {
	struct connection_struct *conn;
	FAMConnection fam_conn;
	struct fam_watch_context *watch_list;
	struct fd_event *fd_event;
};

struct fam_watch_context {
	struct fam_watch_context *prev, *next;
	struct fam_connection_context *fam_conn_ctx;
	struct FAMRequest fr;
	struct sys_notify_context *sys_ctx;
	void (*callback)(struct sys_notify_context *ctx, 
			 void *private_data,
			 struct notify_event *ev);
	void *private_data;
	uint32_t mask; /* the inotify mask */
	uint32_t filter; /* the windows completion filter */
	const char *path;
};


static void fam_connection_context_free_data(void **p_data)
{
	struct fam_connection_context *fam_conn_ctx =
		*(struct fam_connection_context **)p_data;
	FAMConnection *fam_conn = &fam_conn_ctx->fam_conn;

	DEBUG(5, ("Destruct fam_connection_context for service[%s]\n",
		lp_servicename(SNUM(fam_conn_ctx->conn))));

	if (FAMCONNECTION_GETFD(fam_conn) != -1) {
		DEBUG(10, ("FAMClose\n"));
		FAMClose(fam_conn);
	}

	TALLOC_FREE(fam_conn_ctx->fd_event);
	TALLOC_FREE(fam_conn_ctx);

	*p_data = NULL;
}

static int fam_connect(vfs_handle_struct *vfs_handle,  const char *service, const char *user)
{
	struct fam_connection_context *fam_conn_ctx;

	DEBUG(5, ("Initializing fam_connection_context for service[%s]\n",
		lp_servicename(SNUM(vfs_handle->conn))));

	fam_conn_ctx = TALLOC_ZERO_P(vfs_handle->conn, struct fam_connection_context);
	if (!fam_conn_ctx) {
		return -1;
	}

	fam_conn_ctx->conn = vfs_handle->conn;
	FAMCONNECTION_GETFD(&fam_conn_ctx->fam_conn) = -1;

	SMB_VFS_HANDLE_SET_DATA(vfs_handle, fam_conn_ctx,
				fam_connection_context_free_data,
				struct fam_connection_context,
				return -1);

	return SMB_VFS_NEXT_CONNECT(vfs_handle, service, user);
}

static void fam_handler(struct event_context *event_ctx,
			struct fd_event *fd_event,
			uint16 flags,
			void *private_data);

static NTSTATUS fam_open_fam(struct fam_connection_context *fam_conn_ctx,
				    struct event_context *event_ctx)
{
	FAMConnection *fam_conn = &fam_conn_ctx->fam_conn;
	int res;
	char *name;

	DEBUG(5, ("Opening FAM connection for service[%s]\n",
		lp_servicename(SNUM(fam_conn_ctx->conn))));

	ZERO_STRUCTP(fam_conn);
	FAMCONNECTION_GETFD(fam_conn) = -1;


#ifdef HAVE_FAMNOEXISTS
	/* We should honor outside setting of the GAM_CLIENT_ID. */
	setenv("GAM_CLIENT_ID","SAMBA",0);
	/* Disable auto-reconnect on failure feature (OSSTech Gamin only) */
	setenv("GAM_NO_RECONNECT","1",0);
	/* Don't read $HOME/.gaminrc (OSSTech Gamin only) */
	setenv("GAM_USER_CONF_FILE","",0);
#endif

	if (asprintf(&name, "smbd (%lu)", (unsigned long)sys_getpid()) == -1) {
		DEBUG(0, ("No memory\n"));
		return NT_STATUS_NO_MEMORY;
	}

	res = FAMOpen2(fam_conn, name);
	SAFE_FREE(name);

	if (res < 0) {
		DEBUG(0, ("FAM file change notifications not available\n"));
		/*
		 * No idea how to get NT_STATUS from a FAM result
		 */
		FAMCONNECTION_GETFD(fam_conn) = -1;
		return NT_STATUS_UNEXPECTED_IO_ERROR;
	}

#ifdef HAVE_FAMNOEXISTS
	/*
	 * This reduces the chatter between GAMIN and samba making the pair
	 * much more reliable.
	 */
	FAMNoExists(fam_conn);
#endif

	fam_conn_ctx->fd_event = event_add_fd(event_ctx, fam_conn_ctx,
			 FAMCONNECTION_GETFD(fam_conn),
			 EVENT_FD_READ, fam_handler,
			 (void *)fam_conn_ctx);
	if (fam_conn_ctx->fd_event == NULL) {
		DEBUG(0, ("event_add_fd failed\n"));
		FAMClose(fam_conn);
		FAMCONNECTION_GETFD(fam_conn) = -1;
		return NT_STATUS_NO_MEMORY;
	}

	return NT_STATUS_OK;
}

static void fam_reopen_fam(struct fam_connection_context *fam_conn_ctx,
		       struct event_context *event_ctx)
{
	FAMConnection *fam_conn = &fam_conn_ctx->fam_conn;
	struct fam_watch_context *ctx;

	DEBUG(5, ("Re-opening FAM connection for service[%s]\n",
		lp_servicename(SNUM(fam_conn_ctx->conn))));

	TALLOC_FREE(fam_conn_ctx->fd_event);
	FAMClose(fam_conn);
	FAMCONNECTION_GETFD(fam_conn) = -1;

	become_user(fam_conn_ctx->conn, fam_conn_ctx->conn->vuid);

	if (!NT_STATUS_IS_OK(fam_open_fam(fam_conn_ctx, event_ctx))) {
		unbecome_user();
		DEBUG(5, ("Re-opening FAM connection failed\n"));
		return;
	}

	for (ctx = fam_conn_ctx->watch_list; ctx; ctx = ctx->next) {
		DEBUG(10, ("FAMMonitorDirectory: %s\n",ctx->path));
		FAMMonitorDirectory(fam_conn, ctx->path, &ctx->fr, NULL);
	}

	unbecome_user();
}

static void fam_handler(struct event_context *event_ctx,
			struct fd_event *fd_event,
			uint16 flags,
			void *private_data)
{
	struct fam_connection_context *fam_conn_ctx =
		(struct fam_connection_context *)private_data;
	FAMConnection *fam_conn = &fam_conn_ctx->fam_conn;
	int fam_pending;
	FAMEvent fam_event;
	struct fam_watch_context *ctx;
	struct notify_event ne;

	fam_pending = FAMPending(fam_conn);
	if (fam_pending == 0) {
		DEBUG(10, ("fam_handler called but nothing pending\n"));
		return;
	}
	if (fam_pending == -1) {
		DEBUG(3, ("FAMPending returned an error\n"));
		fam_reopen_fam(fam_conn_ctx, event_ctx);
		return;
	}

	if (FAMNextEvent(fam_conn, &fam_event) != 1) {
		DEBUG(3, ("FAMNextEvent returned an error\n"));
		fam_reopen_fam(fam_conn_ctx, event_ctx);
		return;
	}

	DEBUG(10, ("Got FAMCode %d for %s\n", fam_event.code,
		   fam_event.filename));

	switch (fam_event.code) {
	case FAMChanged:
		ne.action = NOTIFY_ACTION_MODIFIED;
		break;
	case FAMCreated:
		ne.action = NOTIFY_ACTION_ADDED;
		break;
	case FAMDeleted:
		ne.action = NOTIFY_ACTION_REMOVED;
		break;
	default:
		DEBUG(10, ("Ignoring code FAMCode %d for file %s\n",
			   (int)fam_event.code, fam_event.filename));
		return;
	}

	for (ctx = fam_conn_ctx->watch_list; ctx; ctx = ctx->next) {
		if (memcmp(&fam_event.fr, &ctx->fr, sizeof(FAMRequest)) == 0) {
			break;
		}
	}

	if (ctx == NULL) {
		DEBUG(5, ("Discarding event for file %s\n",
			  fam_event.filename));
		return;
	}

	if ((ne.path = strrchr_m(fam_event.filename, '\\')) == NULL) {
		ne.path = fam_event.filename;
	}

	ctx->callback(ctx->sys_ctx, ctx->private_data, &ne);
}

static int fam_watch_context_destructor(struct fam_watch_context *ctx)
{
	FAMConnection *fam_conn = &(ctx->fam_conn_ctx->fam_conn);

	DEBUG(5, ("Destruct fam_watch_context for %s on service[%s]\n",
		ctx->path,
		lp_servicename(SNUM(ctx->fam_conn_ctx->conn))));

	if (FAMCONNECTION_GETFD(fam_conn) != -1) {
		DEBUG(10, ("FAMCancelMonitor: %s\n",ctx->path));
		FAMCancelMonitor(fam_conn, &ctx->fr);
	}
	DLIST_REMOVE(ctx->fam_conn_ctx->watch_list, ctx);
	return 0;
}

/*
  add a watch. The watch is removed when the caller calls
  talloc_free() on *handle
*/
static NTSTATUS fam_watch(vfs_handle_struct *vfs_handle,
			  struct sys_notify_context *ctx,
			  struct notify_entry *e,
			  void (*callback)(struct sys_notify_context *ctx, 
					   void *private_data,
					   struct notify_event *ev),
			  void *private_data, 
			  void *handle_p)
{
	struct fam_connection_context *fam_conn_ctx;
	FAMConnection *fam_conn;
	const uint32 fam_mask = (FILE_NOTIFY_CHANGE_FILE_NAME|
				 FILE_NOTIFY_CHANGE_DIR_NAME);
	struct fam_watch_context *watch;
	void **handle = (void **)handle_p;

	if ((e->filter & fam_mask) == 0) {
		DEBUG(10, ("filter = %u, ignoring in FAM\n", e->filter));
		return NT_STATUS_OK;
	}

	SMB_VFS_HANDLE_GET_DATA(vfs_handle,
				fam_conn_ctx,
				struct fam_connection_context,
				return NT_STATUS_INVALID_HANDLE);
	fam_conn = &fam_conn_ctx->fam_conn;

	if (FAMCONNECTION_GETFD(fam_conn) == -1) {
		if (!NT_STATUS_IS_OK(fam_open_fam(fam_conn_ctx, ctx->ev))) {
			/*
			 * Just let smbd do all the work itself
			 */
			return NT_STATUS_OK;
		}
	}

	if (!(watch = TALLOC_P(fam_conn_ctx, struct fam_watch_context))) {
		DEBUG(0, ("TALLOC_P failed\n"));
		return NT_STATUS_NO_MEMORY;
	}

	watch->fam_conn_ctx = fam_conn_ctx;
	watch->callback = callback;
	watch->private_data = private_data;
	watch->sys_ctx = ctx;

	if (!(watch->path = talloc_strdup(watch, e->path))) {
		DEBUG(0, ("talloc_asprintf failed\n"));
		TALLOC_FREE(watch);
		return NT_STATUS_NO_MEMORY;
	}

	/*
	 * The FAM module in this early state will only take care of
	 * FAMCreated and FAMDeleted events, Leave the rest to
	 * notify_internal.c
	 */

	watch->filter = fam_mask;
	e->filter &= ~fam_mask;

	DLIST_ADD(fam_conn_ctx->watch_list, watch);
	talloc_set_destructor(watch, fam_watch_context_destructor);

	/*
	 * Only directories monitored so far
	 */

	DEBUG(10, ("FAMMonitorDirectory: %s\n",watch->path));
	FAMMonitorDirectory(fam_conn, watch->path,
				    &watch->fr, NULL);

	*handle = (void *)watch;

	return NT_STATUS_OK;
}

/* VFS operations structure */

static struct vfs_fn_pointers notify_fam_fns = {
	.connect_fn = fam_connect,
	.notify_watch = fam_watch,
};


NTSTATUS vfs_notify_fam_init(void);
NTSTATUS vfs_notify_fam_init(void)
{
	NTSTATUS ret;

	ret = smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "notify_fam",
				&notify_fam_fns);

	vfs_notify_fam_debug_level = debug_add_class("notify_fam");
	if (vfs_notify_fam_debug_level == -1) {
		vfs_notify_fam_debug_level = DBGC_VFS;
		DEBUG(0, ("%s: Couldn't register custom debugging class!\n",
			"vfs_notify_fam_init"));
	} else {
		DEBUG(10, ("%s: Debug class number of '%s': %d\n",
			"vfs_notify_fam_init","notify_fam",vfs_notify_fam_debug_level));
	}

	return ret;
}
