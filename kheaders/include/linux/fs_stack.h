/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FS_STACK_H
#define _LINUX_FS_STACK_H



#include <linux/fs.h>


extern void fsstack_copy_attr_all(struct inode *dest, const struct inode *src);
extern void fsstack_copy_inode_size(struct inode *dst, struct inode *src);


static inline void fsstack_copy_attr_atime(struct inode *dest,
					   const struct inode *src)
{
	inode_set_atime_to_ts(dest, inode_get_atime(src));
}

static inline void fsstack_copy_attr_times(struct inode *dest,
					   const struct inode *src)
{
	inode_set_atime_to_ts(dest, inode_get_atime(src));
	inode_set_mtime_to_ts(dest, inode_get_mtime(src));
	inode_set_ctime_to_ts(dest, inode_get_ctime(src));
}

#endif 
