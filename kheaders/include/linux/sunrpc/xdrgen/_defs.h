/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _SUNRPC_XDRGEN__DEFS_H_
#define _SUNRPC_XDRGEN__DEFS_H_

#define TRUE	(true)
#define FALSE	(false)

typedef struct {
	u32 len;
	unsigned char *data;
} string;

typedef struct {
	u32 len;
	u8 *data;
} opaque;

#endif 
