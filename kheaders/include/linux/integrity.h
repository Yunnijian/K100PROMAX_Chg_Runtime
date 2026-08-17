/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef _LINUX_INTEGRITY_H
#define _LINUX_INTEGRITY_H

#include <linux/fs.h>
#include <linux/iversion.h>

enum integrity_status {
	INTEGRITY_PASS = 0,
	INTEGRITY_PASS_IMMUTABLE,
	INTEGRITY_FAIL,
	INTEGRITY_FAIL_IMMUTABLE,
	INTEGRITY_NOLABEL,
	INTEGRITY_NOXATTRS,
	INTEGRITY_UNKNOWN,
};

#ifdef CONFIG_INTEGRITY
extern void __init integrity_load_keys(void);

#else
static inline void integrity_load_keys(void)
{
}
#endif 


struct integrity_inode_attributes {
	u64 version;		
	unsigned long ino;
	dev_t dev;
};


static inline void
integrity_inode_attrs_store(struct integrity_inode_attributes *attrs,
			    u64 i_version, const struct inode *inode)
{
	attrs->version = i_version;
	attrs->dev = inode->i_sb->s_dev;
	attrs->ino = inode->i_ino;
}


static inline bool
integrity_inode_attrs_changed(const struct integrity_inode_attributes *attrs,
			      const struct inode *inode)
{
	return (inode->i_sb->s_dev != attrs->dev ||
		inode->i_ino != attrs->ino ||
		!inode_eq_iversion(inode, attrs->version));
}


#endif 
