/* SPDX-License-Identifier: MIT */


#ifndef __XEN_PUBLIC_HVM_IOREQ_H__
#define __XEN_PUBLIC_HVM_IOREQ_H__

#define IOREQ_READ      1
#define IOREQ_WRITE     0

#define STATE_IOREQ_NONE        0
#define STATE_IOREQ_READY       1
#define STATE_IOREQ_INPROCESS   2
#define STATE_IORESP_READY      3

#define IOREQ_TYPE_PIO          0 
#define IOREQ_TYPE_COPY         1 
#define IOREQ_TYPE_PCI_CONFIG   2
#define IOREQ_TYPE_TIMEOFFSET   7
#define IOREQ_TYPE_INVALIDATE   8 


struct ioreq {
	uint64_t addr;          
	uint64_t data;          
	uint32_t count;         
	uint32_t size;          
	uint32_t vp_eport;      
	uint16_t _pad0;
	uint8_t state:4;
	uint8_t data_is_ptr:1;  
	uint8_t dir:1;          
	uint8_t df:1;
	uint8_t _pad1:1;
	uint8_t type;           
};

#endif 
