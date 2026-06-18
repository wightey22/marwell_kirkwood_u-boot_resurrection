/*
 * File Quota VFS module for Samba 3.6.x
 * Copyright (c) TAKEDA Yasuma, 2008
 * Copyright (c) SATOH Fumiyasu @ OSS Technology, Inc., 2009-2011
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

/*
 * smb.conf format is here.
 *   vfs objects = file_quota
 *   file_quota:quota = *.jpg:500k *.png:20k *.pdf:2M
 *   file_quota:exclude_users = administrator @admins
 *
 * These settings restricts a user can't create files over these limits.
 * And you can use "*" and "?" as wild card.
 */

#define VFS_FILE_QUOTA_VER 2011081801

#include "includes.h"
#include "smbd/smbd.h"
#include "auth.h"

extern struct current_user current_user;

static int vfs_file_quota_debug_level = DBGC_VFS;
#undef DBGC_CLASS
#define DBGC_CLASS DBGC_VFS

/* Function prototypes */
static ssize_t file_quota_read(vfs_handle_struct *handle, files_struct *fsp, void *data, size_t n);

static ssize_t file_quota_pread(vfs_handle_struct *handle, files_struct *fsp, void *data, size_t n, SMB_OFF_T offset);

static ssize_t file_quota_write(vfs_handle_struct *handle, files_struct *fsp, const void *data, size_t n);

static ssize_t file_quota_pwrite(vfs_handle_struct *handle, files_struct *fsp, const void *data, size_t n, SMB_OFF_T offset);

static int file_quota_aio_read(struct vfs_handle_struct *handle, struct files_struct *fsp, SMB_STRUCT_AIOCB *aiocb);

static int file_quota_aio_write(struct vfs_handle_struct *handle, struct files_struct *fsp, SMB_STRUCT_AIOCB *aiocb);

static int file_quota_ftruncate(vfs_handle_struct *handle, files_struct *fsp, SMB_OFF_T len);

static int file_quota_rename(vfs_handle_struct *handle, const struct smb_filename *smb_fname_src, const struct smb_filename *smb_fname_dst);

static int64 match_pattern_files(vfs_handle_struct *handle, const struct smb_filename *smb_fname);
static bool has_exceeded_filesize(vfs_handle_struct *handle, const struct smb_filename *smb_fname, int64 limit);
static bool will_exceed_filesize(vfs_handle_struct *handle, const struct smb_filename *smb_fname, int64 limit, SMB_OFF_T len);
static int64 xstrtoi64(const char *ptr);

#define VFS_FILE_QUOTA_NOLIMIT (-1)

/* VFS operations structure */
static struct vfs_fn_pointers vfs_file_quota_fns = {
#ifdef FILE_QUOTA_READ_OP
	.vfs_read = file_quota_read,
	.pread = file_quota_pread,
#endif
	.write = file_quota_write,
	.pwrite = file_quota_pwrite,
#ifdef FILE_QUOTA_READ_OP
	.aio_read = file_quota_aio_read,
#endif
	.aio_write = file_quota_aio_write,
	.ftruncate = file_quota_ftruncate,
	.rename = file_quota_rename,
};


#ifdef FILE_QUOTA_READ_OP
/* Implementation of vfs_ops. */
static ssize_t file_quota_read(vfs_handle_struct *handle, files_struct *fsp, void *data, size_t n)
{
	ssize_t result;
	int64 limit;

	DEBUG(5, ("file_quota: read check [%s]\n", fsp_str_dbg(fsp)));
	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not read [%s] because exceeded file size\n", fsp_str_dbg(fsp)));
				return -1;
			}
		}
	}
	result = SMB_VFS_NEXT_READ(handle, fsp, data, n);
	return result;
}


static ssize_t file_quota_pread(vfs_handle_struct *handle, files_struct *fsp, void *data, size_t n, SMB_OFF_T offset)
{
	int result;
	int64 limit, f_last;
	
	DEBUG(5, ("file_quota: pread check [%s]\n", fsp_str_dbg(fsp)));

	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not pread [%s] because exceeded file size\n", fsp_str_dbg(fsp)));
				return -1;
			}

			f_last = offset + n;
			if (will_exceed_filesize(handle, fsp->fsp_name, limit, f_last) == true){
				DEBUG(3, ("file_quota: can not pread [%s] because file size will exceed[%lld]\n", fsp_str_dbg(fsp), f_last));
				return -1;
			}
		}

	}
	result = SMB_VFS_NEXT_PREAD(handle, fsp, data, n, offset);
	return result;
}
#endif

static ssize_t file_quota_write(vfs_handle_struct *handle, files_struct *fsp, const void *data, size_t n)
{
	ssize_t result;
	int64 limit;

	DEBUG(5, ("file_quota: write check [%s]\n", fsp_str_dbg(fsp)));

	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not write [%s] because exceeded file size\n", fsp_str_dbg(fsp)));
				return -1;
			}
		}
	}
	result = SMB_VFS_NEXT_WRITE(handle, fsp, data, n);
	return result;
}


static ssize_t file_quota_pwrite(vfs_handle_struct *handle, files_struct *fsp, const void *data, size_t n, SMB_OFF_T offset)
{
	int result;
        int64 limit, f_last;

	DEBUG(5, ("file_quota: pwrite check [%s]\n", fsp_str_dbg(fsp)));
	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not pwrite [%s] because file size has exceeded.\n", fsp_str_dbg(fsp)));
				return -1;
			}

			f_last = offset + n;
			if (will_exceed_filesize(handle, fsp->fsp_name, limit, f_last) == true){
				DEBUG(3, ("file_quota: can not pwrite [%s] because file size will exceed[%lld]\n", fsp_str_dbg(fsp), f_last));
				return -1;
			}

		}
	}
	result = SMB_VFS_NEXT_PWRITE(handle, fsp, data, n, offset);
	return result;
}

#ifdef FILE_QUOTA_READ_OP
static int file_quota_aio_read(struct vfs_handle_struct *handle, struct files_struct *fsp, SMB_STRUCT_AIOCB *aiocb)
{
	int result = 0;
	int64 limit, f_last;
	DEBUG(5, ("file_quota: aio read check [%s]\n", fsp_str_dbg(fsp)));
	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not aio read [%s] because exceeded file size\n", fsp_str_dbg(fsp)));
				return -1;
			}
		}
	}
	result = SMB_VFS_NEXT_AIO_READ(handle, fsp, aiocb);
	return result;
}
#endif

static int file_quota_aio_write(struct vfs_handle_struct *handle, struct files_struct *fsp, SMB_STRUCT_AIOCB *aiocb)
{
	int result = 0;
	int64 limit;

	DEBUG(5, ("file_quota: aio write check [%s]\n", fsp_str_dbg(fsp)));

	if (!fsp->is_directory){
		limit = match_pattern_files(handle, fsp->fsp_name);
		if (limit != VFS_FILE_QUOTA_NOLIMIT){
			if (has_exceeded_filesize(handle, fsp->fsp_name, limit) == true){
				DEBUG(3, ("file_quota: can not aio write [%s] because exceeded file size\n", fsp_str_dbg(fsp)));
				return -1;
			}
		}
	}
	result = SMB_VFS_NEXT_AIO_WRITE(handle, fsp, aiocb);
	return result;
}

static int file_quota_ftruncate(vfs_handle_struct *handle, files_struct *fsp, SMB_OFF_T len)
{
	int result = 0;
	int64 limit;

	DEBUG(5, ("file_quota: ftruncate check [%s]\n", fsp_str_dbg(fsp)));
	limit = match_pattern_files(handle, fsp->fsp_name);
	if (limit != VFS_FILE_QUOTA_NOLIMIT){
		if (will_exceed_filesize(handle, fsp->fsp_name, limit, len) == true){
			DEBUG(3, ("file_quota: can not ftruncate [%s] because file size will exceed[%lld]\n", fsp_str_dbg(fsp), len));
			return -1;
		}
	}

	result = SMB_VFS_NEXT_FTRUNCATE(handle, fsp, len);
	return result;
}

static int file_quota_rename(vfs_handle_struct *handle, const struct smb_filename *smb_fname_src, const struct smb_filename *smb_fname_dst)
{
	int result = 0;
	int64 limit;
	
	DEBUG(5, ("file_quota: rename check [%s to %s]\n", smb_fname_str_dbg(smb_fname_src), smb_fname_str_dbg(smb_fname_dst)));

	limit = match_pattern_files(handle, smb_fname_src);
	if (limit != VFS_FILE_QUOTA_NOLIMIT){
		if (has_exceeded_filesize(handle, smb_fname_src, limit) == true){
			DEBUG(3, ("file_quota: can not rename [%s(old)] because file size has exceeded.\n", smb_fname_str_dbg(smb_fname_src)));
			errno = EACCES; /* permission denied */
			return -1;
		}
	}
	limit = match_pattern_files(handle, smb_fname_dst);
	if (limit != VFS_FILE_QUOTA_NOLIMIT){
		if (has_exceeded_filesize(handle, smb_fname_src, limit) == true){
			DEBUG(3, ("file_quota: can not rename [%s(new)] because file size has exceeded.\n", smb_fname_str_dbg(smb_fname_dst)));
			errno = EACCES; /* permission denied */
			return -1;
		}
	}
	result = SMB_VFS_NEXT_RENAME(handle, smb_fname_src, smb_fname_dst);
	return result;
}

NTSTATUS vfs_file_quota_init(void)
{
	NTSTATUS ret = smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "file_quota", &vfs_file_quota_fns);
	
	if (!NT_STATUS_IS_OK(ret))
		return ret;

	vfs_file_quota_debug_level == debug_add_class("file_quota");
	if (vfs_file_quota_debug_level == -1){
		vfs_file_quota_debug_level = DBGC_VFS;
		DEBUG(0, ("%s: Couldn't register custom debugging class!\n", "vfs_file_quota_init"));
	} else {
		DEBUG(10, ("%s: Debug class number of '%s': %d\n", "vfs_file_quota_init","file_quota", vfs_file_quota_debug_level));
	}
	return ret;
}

/*
 * Parse and check the filename whether match or not
 * You can use "*" and "?" as wild card in the parameter. 
 * Sample format is here.
 * file_quota:quota=*.jpg:100k *.png:200k *.*:1m *:1g
 *
 * @return The size limit(includes 0) if matched.
 *         Negative value(-1) is not matched.
 */

static int64 match_pattern_files(vfs_handle_struct *handle, const struct smb_filename *smb_fname){
	bool ret = false;
	const char **lp_list, **user_list;
	char *pattern, *pos;
	int64 limit = VFS_FILE_QUOTA_NOLIMIT;

	lp_list = lp_parm_string_list(SNUM(handle->conn), "file_quota", "quota", NULL);
	if (lp_list == NULL){
	  return limit;
	}

	/* check pattern match */
	for (;*lp_list;*lp_list++){
		pattern = SMB_STRDUP(*lp_list);

		pos = strchr_m(pattern, ':');

		if (!pos){
			/* wrong format because ':' is not included. */
			SAFE_FREE(pattern);
			continue;
		}

		*pos = '\0'; /* separate pattern and limit by NULL terminate */

		if (unix_wild_match(pattern, smb_fname->base_name)){
			/* pattern matched */
		  
			/* check exuclude_users */
			user_list = lp_parm_string_list(SNUM(handle->conn), "file_quota", "exclude_users", NULL);
			if (user_list){
				if (token_contains_name_in_list(handle->conn->session_info->unix_name, NULL, NULL, current_user.nt_user_token, user_list)){
					DEBUG(5, ("file_quota: exclude user :%s\n", handle->conn->session_info->unix_name));
					return limit; /* limit is NOLIMIT at here */
				}
			}
			/* convert from string to integer and set the limit */
			limit = xstrtoi64(++pos);
			DEBUG(5, ("file_quota: filename [%s] is matched. pattern:[%s] limit %lld\n", smb_fname_str_dbg(smb_fname), pattern, limit));
			SAFE_FREE(pattern);
			return limit;
		}
		SAFE_FREE(pattern);
	}
	return limit;
}

static bool has_exceeded_filesize(vfs_handle_struct *handle, const struct smb_filename *smb_fname, int64 limit){
	off_t size;

	DEBUG(5, ("file_quota: filename [%s]/limit [%lld]\n", smb_fname_str_dbg(smb_fname), limit));

	if (VALID_STAT(smb_fname->st)) {
		size = smb_fname->st.st_ex_size;
	} else {
		struct smb_filename *smb_fname_tmp;
		NTSTATUS status = create_synthetic_smb_fname(talloc_tos(),
			smb_fname->base_name, NULL, NULL, &smb_fname_tmp);
		if (!NT_STATUS_IS_OK(status)) {
			return false;
		}
		if (SMB_VFS_NEXT_STAT(handle, smb_fname_tmp) != 0) {
			return false;
		}
		size = smb_fname_tmp->st.st_ex_size;
	}

	if (limit < size) {
		DEBUG(3, ("file_quota: %s [size:%lld] has exceeded the limit [%lld].\n",smb_fname_str_dbg(smb_fname), size, limit));
		return true;
	}

	return false;
}

static bool will_exceed_filesize(vfs_handle_struct *handle, const struct smb_filename *smb_fname, int64 limit, SMB_OFF_T len){
	DEBUG(5, ("file_quota: filename [%s]/limit [%lld]/len %lld\n", smb_fname_str_dbg(smb_fname), limit, len));
	if (limit < len) {
		DEBUG(3, ("file_quota: %s will exceed the limit [%lld].\n", smb_fname_str_dbg(smb_fname), limit));
		return true;
	}
	return false;
}


/* 
 * more friendly strtoll function.
 * 1k/1K -> 1 * 1024
 * 1m/1M -> 1 * 1024 * 1024
 * 1g/1G -> 1 * 1024 * 1024 * 1024
 * 1t/1T, 1p/1P and so on.
 * @return int64 integer from string
 */
static int64 xstrtoi64 (const char *ptr)
{
	char *retptr;
	int64 ret = -1;
	/* bit width of 'signed' int64 */
	int ll_width = sizeof(int64) * 8 - 1;

	ret = (int64)strtoll (ptr, &retptr, 10);

	switch (*retptr) {
	case 'Y':
	case 'y':
		if ((ll_width > 80) &&
		    ((long long) ret < (long long)1<<(ll_width - 80)))
		{
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'Z':
	case 'z':
		if ((ll_width > 70) &&
		    ((long long) ret < (long long)1<<(ll_width - 70)))
		{
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'E':
	case 'e':
		if ((long long) ret < (long long)1<<(ll_width - 60)){
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'P':
	case 'p':
		if ((long long) ret < (long long)1<<(ll_width - 50)){
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'T':
	case 't':
		if ((long long) ret < (long long)1<<(ll_width - 40)){
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'G':
	case 'g':
		if ((long long) ret < (long long)1<<(ll_width - 30)){
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'M':
	case 'm':
		if ((long long) ret < (long long)1<<(ll_width - 20)){
			ret <<= 10;
		} else {
			ret = -1;
			break;
		}
	case 'K':
	case 'k':
		if ((long long) ret < (long long)1<<(ll_width - 10)){
			ret <<= 10;

		} else {
			ret = -1;
			break;
		}
		retptr++;
	default:
		break;
	}

	if (retptr && strlen(retptr)){
		/* wrong format */
		DEBUG(3, ("convert failed from %s to int64\n", ptr));
		ret = -1;
	}
	DEBUG(3, ("converted %s to %lld\n", ptr, ret));
	return ret;
}
