/*
 Unix SMB/Netbios implementation.
 Version 3.2.x
 recvfile implementations.
 Copyright (C) Jeremy Allison 2007.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

/*
 * This file handles the OS dependent recvfile implementations.
 * The API is such that it returns -1 on error, else returns the
 * number of bytes written.
 */

#include "includes.h"
#include "system/filesys.h"

/* Do this on our own in TRANSFER_BUF_SIZE chunks.
 * It's safe to make direct syscalls to lseek/write here
 * as we're below the Samba vfs layer.
 *
 * Returns -1 on short reads from fromfd (read error)
 * and sets errno.
 *
 * Returns number of bytes written to 'tofd'
 * return != count then sets errno.
 * Returns count if complete success.
 */

#ifndef TRANSFER_BUF_SIZE
#define TRANSFER_BUF_SIZE (128*1024)
#endif

/* 2012/06/25 t.saito add for recvfile */
#if 0
#define CONFIG_BUFFALO_ADD_RECVFILE
#include <linux/unistd.h>
#endif
/* 2012/06/25 t.saito add end */

/* 2012/06/18 t.saito add for recvfile */
#if 0
#define SAITO_RECVFILE_DBG
#endif
#if 0
#define SPLICE_RD_64K
#endif
#if 0
#define SPLICE_RD_NONBLOCK
#endif
#if 0
#define SPLICE_WR_NONBLOCK
#endif
/* 2012/06/18 t.saito add end */

static ssize_t default_sys_recvfile(int fromfd,
			int tofd,
			SMB_OFF_T offset,
			size_t count)
{
	int saved_errno = 0;
	size_t total = 0;
	size_t bufsize = MIN(TRANSFER_BUF_SIZE,count);
	size_t total_written = 0;
	char *buffer = NULL;

	DEBUG(10,("default_sys_recvfile: from = %d, to = %d, "
		"offset=%.0f, count = %lu\n",
		fromfd, tofd, (double)offset,
		(unsigned long)count));

	if (count == 0) {
		return 0;
	}

	if (tofd != -1 && offset != (SMB_OFF_T)-1) {
		if (sys_lseek(tofd, offset, SEEK_SET) == -1) {
			if (errno != ESPIPE) {
				return -1;
			}
		}
	}

	buffer = SMB_MALLOC_ARRAY(char, bufsize);
	if (buffer == NULL) {
		return -1;
	}

	while (total < count) {
		size_t num_written = 0;
		ssize_t read_ret;
		size_t toread = MIN(bufsize,count - total);

		/* Read from socket - ignore EINTR. */
		read_ret = sys_read(fromfd, buffer, toread);
		if (read_ret <= 0) {
			/* EOF or socket error. */
			free(buffer);
			return -1;
		}

		num_written = 0;

		/* Don't write any more after a write error. */
		while (tofd != -1 && (num_written < read_ret)) {
			ssize_t write_ret;

			/* Write to file - ignore EINTR. */
			write_ret = sys_write(tofd,
					buffer + num_written,
					read_ret - num_written);

			if (write_ret <= 0) {
				/* write error - stop writing. */
				tofd = -1;
                                if (total_written == 0) {
					/* Ensure we return
					   -1 if the first
					   write failed. */
                                        total_written = -1;
                                }
				saved_errno = errno;
				break;
			}

			num_written += (size_t)write_ret;
			total_written += (size_t)write_ret;
		}

		total += read_ret;
	}

	free(buffer);
	if (saved_errno) {
		/* Return the correct write error. */
		errno = saved_errno;
	}
	return (ssize_t)total_written;
}

#if defined(HAVE_LINUX_SPLICE)

/*
 * Try and use the Linux system call to do this.
 * Remember we only return -1 if the socket read
 * failed. Else we return the number of bytes
 * actually written. We always read count bytes
 * from the network in the case of return != -1.
 */

#ifdef CONFIG_BUFFALO_ADD_RECVFILE
ssize_t sys_recvfile(int fromfd,
			int tofd,
			SMB_OFF_T offset,
			size_t count)
{
	ssize_t rwbytes[2];
	ssize_t total_written = 0;
	ssize_t total_receive = 0;
	int retRecv = 0;
	sigset_t set, old_set;
	int saved_errno;

	bzero(rwbytes, sizeof(rwbytes));

#ifdef SAITO_RECVFILE_DBG
	DEBUG(2,("real syscall sys_recvfile(): from = %d, to = %d, "
		"offset=%.0f, count = %lu\n",
		fromfd, tofd, (double)offset,
		(unsigned long)count));
#else
	DEBUG(10,("real syscall sys_recvfile(): from = %d, to = %d, "
		"offset=%.0f, count = %lu\n",
		fromfd, tofd, (double)offset,
		(unsigned long)count));
#endif

	/* our linux recvfile() fixed the timeout value to 32sec */
	do {
		retRecv = 0;

		sigfillset(&set);
		sigdelset(&set, SIGQUIT);
		sigdelset(&set, SIGABRT);
		sigdelset(&set, SIGKILL);
		sigdelset(&set, SIGTERM);
		sigdelset(&set, SIGSTOP);
		sigprocmask(SIG_BLOCK, &set, &old_set);

		retRecv = recvfile(tofd, fromfd, &offset, count-total_receive, rwbytes);

		sigprocmask(SIG_SETMASK, &old_set, NULL);

		total_receive += rwbytes[0];
		total_written += rwbytes[1];
		saved_errno = errno;
#ifdef SAITO_RECVFILE_DBG
		
		DEBUG(2,("real syscall sys_recvfile(): errno=%d, total_receive=%lu, total_written=%lu\n",
							saved_errno, (unsigned long)total_receive, (unsigned long)total_written));
#else
		DEBUG(10,("real syscall sys_recvfile(): errno=%d, total_receive=%lu, total_written=%lu\n",
							saved_errno, (unsigned long)total_receive, (unsigned long)total_written));
#endif

		if (0 < retRecv) {
			if (total_receive == count) {
				break;
			}
			continue;
		} else if (0 == retRecv) {
			DEBUG(2,("real syscall sys_recvfile(): no data received, "
							"count=[%lu], total_received/written=[%lu/%lu] rwbytes=[%lu/%lu]\n",
							(unsigned long)count,
							(unsigned long)total_receive, (unsigned long)total_written,
							(unsigned long)rwbytes[0], (unsigned long)rwbytes[1]));
			break;
		}

		if ( saved_errno == EINTR) {
			continue;
		}
		switch(saved_errno) {
			case EINTR:
				continue;
			case ENOSPC:
			case EDQUOT:
				DEBUG(2,("real syscall sys_recvfile(): failed ENOSPC or EDQUOT\n"));
				break;
			case EPIPE:
			default:
				DEBUG(2,("real syscall sys_recvfile(): failed EPIPE or else, "
								"count=[%lu], total_received/written=[%lu/%lu], rwbytes=[%lu/%lu]\n",
								(unsigned long)count,
								(unsigned long)total_receive, (unsigned long)total_written,
								(unsigned long)rwbytes[0], (unsigned long)rwbytes[1]));
				break;
		}
		DEBUG(1,("real_syscall sys_recvfile(): failed errno=%d\n", saved_errno));
		break;
	} while (total_written < count);

	if (total_receive < count) {
		if (drain_socket(fromfd, count-total_receive) != (count-total_receive)) {
			DEBUG(1,("real_syscall sys_recvfile(): faild drain_socket\n"));
			return -1;
		}
		errno = saved_errno;
	}

	/*
	 * for ext3, the total_written will always be 0, which is inconsistent with ecryptfs.
	 * This would make samba have various bahaviors on differnt fs.
	 */
	if (total_written < count && (ENOSPC == errno || EDQUOT == errno))
		total_written = 0;

	return total_written;
}

#else
ssize_t sys_recvfile(int fromfd,
			int tofd,
			SMB_OFF_T offset,
			size_t count)
{
	static int pipefd[2] = { -1, -1 };
#ifdef SAITO_RECVFILE_DBG
  int err;
#endif
#if defined(SPLICE_RD_NONBLOCK) || defined(SPLICE_WR_NONBLOCK)
	static bool try_splice_call = true;
#else
	static bool try_splice_call = false;
#endif
	size_t total_written = 0;
	loff_t splice_offset = offset;

/* 2012/06/18 t.saito change */
#ifdef SAITO_RECVFILE_DBG
	DEBUG(2,("sys_recvfile: from = %d, to = %d, "
		"offset=%.0f, count = %lu\n",
		fromfd, tofd, (double)offset,
		(unsigned long)count));
#else
	DEBUG(10,("sys_recvfile: from = %d, to = %d, "
		"offset=%.0f, count = %lu\n",
		fromfd, tofd, (double)offset,
		(unsigned long)count));
#endif

	if (count == 0) {
		return 0;
	}

	/*
	 * Older Linux kernels have splice for sendfile,
	 * but it fails for recvfile. Ensure we only try
	 * this once and always fall back to the userspace
	 * implementation if recvfile splice fails. JRA.
	 */

	if (!try_splice_call) {
#ifdef SAITO_RECVFILE_DBG
		DEBUG(2,("sys_recvfile: 1st try_splice_call = false\n"));
#endif
		return default_sys_recvfile(fromfd,
				tofd,
				offset,
				count);
	}
	  
	if ((pipefd[0] == -1) && (pipe(pipefd) == -1)) {
		try_splice_call = false;
#ifdef SAITO_RECVFILE_DBG
		DEBUG(2,("sys_recvfile: pipe create error\n"));
#endif
		return default_sys_recvfile(fromfd, tofd, offset, count);
	}

	while (count > 0) {
		int nread, to_write;

#ifdef SAITO_RECVFILE_DBG
		DEBUG(2,("sys_recvfile: splice count = %lu\n", (unsigned long)count));
#endif

#ifdef SPLICE_RD_NONBLOCK
		nread = splice(fromfd, NULL, pipefd[1], NULL,
			       MIN(count, 65536), SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
#elif defined SPLICE_RD_64K
		nread = splice(fromfd, NULL, pipefd[1], NULL,
			       MIN(count, 65536), SPLICE_F_MOVE);
#else
		nread = splice(fromfd, NULL, pipefd[1], NULL,
			       MIN(count, 16384), SPLICE_F_MOVE);
#endif
		
		if (nread == -1) {
#ifdef SPLICE_RD_NONBLOCK
			if (errno == EINTR || errno == EAGAIN) {
#ifdef SAITO_RECVFILE_DBG
				err = errno;
				DEBUG(2,("sys_recvfile: splice socket2pipe continue errno = %d\n", err));
#endif
#else
			if (errno == EINTR) {
#endif
				continue;
			}
			
			if (total_written == 0 &&
			    (errno == EBADF || errno == EINVAL)) {
				try_splice_call = false;
#ifdef SAITO_RECVFILE_DBG
				err = errno;
				DEBUG(2,("sys_recvfile: splice socket2pipe error errno = %d\n", err));
#endif
#if defined(SPLICE_RD_NONBLOCK) || defined(SPLICE_WR_NONBLOCK)
				close(pipefd[0]);
				close(pipefd[1]);
#endif
				return default_sys_recvfile(fromfd, tofd,
							    offset, count);
			}
			break;
		}

		to_write = nread;
		while (to_write > 0) {
			int thistime;
#ifdef SPLICE_WR_NONBLOCK
			thistime = splice(pipefd[0], NULL, tofd,
					  &splice_offset, to_write,
					  SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
#else
			thistime = splice(pipefd[0], NULL, tofd,
					  &splice_offset, to_write,
					  SPLICE_F_MOVE);
#endif
			if (thistime == -1) {
#ifdef SPLICE_WR_NONBLOCK
				if(errno == EAGAIN){
#ifdef SAITO_RECVFILE_DBG
					DEBUG(2,("sys_recvfile: splice pipe2file continue errno = EAGAIN\n"));
#endif
					continue;
				}
#endif

#ifdef SAITO_RECVFILE_DBG
				err = errno;
				DEBUG(1,("sys_recvfile: splice pipe2file error errno = %d\n", err));
#endif
				goto done;
			}
			to_write -= thistime;
		}
		total_written += nread;
		count -= nread;
	}

 done:
	if (count) {
		int saved_errno = errno;
		if (drain_socket(fromfd, count) != count) {
			/* socket is dead. */
#if defined(SPLICE_RD_NONBLOCK) || defined(SPLICE_WR_NONBLOCK)
#if 0
				close(pipefd[0]);
				close(pipefd[1]);
#endif
#endif
				return -1;
		}
		errno = saved_errno;
	}

#if defined(SPLICE_RD_NONBLOCK) || defined(SPLICE_WR_NONBLOCK)
#if 0
	close(pipefd[0]);
	close(pipefd[1]);
#endif
#endif
	
	return total_written;
}
#endif	/* CONFIG_BUFFALO_ADD_RECVFILE */
#else

/*****************************************************************
 No recvfile system call - use the default 128 chunk implementation.
*****************************************************************/

ssize_t sys_recvfile(int fromfd,
			int tofd,
			SMB_OFF_T offset,
			size_t count)
{
	return default_sys_recvfile(fromfd, tofd, offset, count);
}
#endif

/*****************************************************************
 Throw away "count" bytes from the client socket.
 Returns count or -1 on error.
*****************************************************************/

ssize_t drain_socket(int sockfd, size_t count)
{
	size_t total = 0;
	size_t bufsize = MIN(TRANSFER_BUF_SIZE,count);
	char *buffer = NULL;

	if (count == 0) {
		return 0;
	}

	buffer = SMB_MALLOC_ARRAY(char, bufsize);
	if (buffer == NULL) {
		return -1;
	}

	while (total < count) {
		ssize_t read_ret;
		size_t toread = MIN(bufsize,count - total);

		/* Read from socket - ignore EINTR. */
		read_ret = sys_read(sockfd, buffer, toread);
		if (read_ret <= 0) {
			/* EOF or socket error. */
			free(buffer);
			return -1;
		}
		total += read_ret;
	}

	free(buffer);
	return count;
}
