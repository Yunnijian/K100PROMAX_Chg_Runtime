/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */

#ifndef _UAPI__LINUX_UIO_H
#define _UAPI__LINUX_UIO_H

#include <linux/compiler.h>
#include <linux/types.h>


struct iovec
{
	void __user *iov_base;	
	__kernel_size_t iov_len; 
};

struct dmabuf_cmsg {
	__u64 frag_offset;	
	__u32 frag_size;	
	__u32 frag_token;	
	__u32  dmabuf_id;	
	__u32 flags;		
};

struct dmabuf_token {
	__u32 token_start;
	__u32 token_count;
};


 
#define UIO_FASTIOV	8
#define UIO_MAXIOV	1024


#endif 
