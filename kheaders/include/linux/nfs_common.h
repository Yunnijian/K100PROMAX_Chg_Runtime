/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_NFS_COMMON_H
#define _LINUX_NFS_COMMON_H

#include <linux/errno.h>
#include <uapi/linux/nfs.h>



int nfs_stat_to_errno(enum nfs_stat status);
int nfs4_stat_to_errno(int stat);

__u32 nfs_localio_errno_to_nfs4_stat(int errno);

#endif 
