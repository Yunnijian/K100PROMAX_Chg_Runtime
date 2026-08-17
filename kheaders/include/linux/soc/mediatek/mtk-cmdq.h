/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __MTK_CMDQ_H__
#define __MTK_CMDQ_H__

#include <linux/mailbox_client.h>
#include <linux/mailbox/mtk-cmdq-mailbox.h>
#include <linux/timer.h>

#define CMDQ_ADDR_HIGH(addr)	((u32)(((addr) >> 16) & GENMASK(31, 0)))
#define CMDQ_ADDR_LOW(addr)	((u16)(addr) | BIT(1))


#define CMDQ_THR_SPR_IDX0	(0)
#define CMDQ_THR_SPR_IDX1	(1)
#define CMDQ_THR_SPR_IDX2	(2)
#define CMDQ_THR_SPR_IDX3	(3)

struct cmdq_pkt;

enum cmdq_logic_op {
	CMDQ_LOGIC_ASSIGN = 0,
	CMDQ_LOGIC_ADD = 1,
	CMDQ_LOGIC_SUBTRACT = 2,
	CMDQ_LOGIC_MULTIPLY = 3,
	CMDQ_LOGIC_XOR = 8,
	CMDQ_LOGIC_NOT = 9,
	CMDQ_LOGIC_OR = 10,
	CMDQ_LOGIC_AND = 11,
	CMDQ_LOGIC_LEFT_SHIFT = 12,
	CMDQ_LOGIC_RIGHT_SHIFT = 13,
	CMDQ_LOGIC_MAX,
};

struct cmdq_operand {
	
	bool reg;
	union {
		
		u16 idx;
		
		u16 value;
	};
};

struct cmdq_client_reg {
	u8 subsys;
	u16 offset;
	u16 size;
};

struct cmdq_client {
	struct mbox_client client;
	struct mbox_chan *chan;
};

#if IS_ENABLED(CONFIG_MTK_CMDQ)


int cmdq_dev_get_client_reg(struct device *dev,
			    struct cmdq_client_reg *client_reg, int idx);


struct cmdq_client *cmdq_mbox_create(struct device *dev, int index);


void cmdq_mbox_destroy(struct cmdq_client *client);


int cmdq_pkt_create(struct cmdq_client *client, struct cmdq_pkt *pkt, size_t size);


void cmdq_pkt_destroy(struct cmdq_client *client, struct cmdq_pkt *pkt);


int cmdq_pkt_write(struct cmdq_pkt *pkt, u8 subsys, u16 offset, u32 value);


int cmdq_pkt_write_mask(struct cmdq_pkt *pkt, u8 subsys,
			u16 offset, u32 value, u32 mask);


int cmdq_pkt_read_s(struct cmdq_pkt *pkt, u16 high_addr_reg_idx, u16 addr_low,
		    u16 reg_idx);


int cmdq_pkt_write_s(struct cmdq_pkt *pkt, u16 high_addr_reg_idx,
		     u16 addr_low, u16 src_reg_idx);


int cmdq_pkt_write_s_mask(struct cmdq_pkt *pkt, u16 high_addr_reg_idx,
			  u16 addr_low, u16 src_reg_idx, u32 mask);


int cmdq_pkt_write_s_value(struct cmdq_pkt *pkt, u8 high_addr_reg_idx,
			   u16 addr_low, u32 value);


int cmdq_pkt_write_s_mask_value(struct cmdq_pkt *pkt, u8 high_addr_reg_idx,
				u16 addr_low, u32 value, u32 mask);


int cmdq_pkt_mem_move(struct cmdq_pkt *pkt, dma_addr_t src_addr, dma_addr_t dst_addr);


int cmdq_pkt_wfe(struct cmdq_pkt *pkt, u16 event, bool clear);


int cmdq_pkt_acquire_event(struct cmdq_pkt *pkt, u16 event);


int cmdq_pkt_clear_event(struct cmdq_pkt *pkt, u16 event);


int cmdq_pkt_set_event(struct cmdq_pkt *pkt, u16 event);


int cmdq_pkt_poll(struct cmdq_pkt *pkt, u8 subsys,
		  u16 offset, u32 value);


int cmdq_pkt_poll_mask(struct cmdq_pkt *pkt, u8 subsys,
		       u16 offset, u32 value, u32 mask);


int cmdq_pkt_logic_command(struct cmdq_pkt *pkt, u16 result_reg_idx,
			   struct cmdq_operand *left_operand,
			   enum cmdq_logic_op s_op,
			   struct cmdq_operand *right_operand);


int cmdq_pkt_assign(struct cmdq_pkt *pkt, u16 reg_idx, u32 value);


int cmdq_pkt_poll_addr(struct cmdq_pkt *pkt, dma_addr_t addr, u32 value, u32 mask);


int cmdq_pkt_jump_abs(struct cmdq_pkt *pkt, dma_addr_t addr, u8 shift_pa);


static inline int cmdq_pkt_jump(struct cmdq_pkt *pkt, dma_addr_t addr, u8 shift_pa)
{
	return cmdq_pkt_jump_abs(pkt, addr, shift_pa);
}


int cmdq_pkt_jump_rel(struct cmdq_pkt *pkt, s32 offset, u8 shift_pa);


int cmdq_pkt_eoc(struct cmdq_pkt *pkt);


int cmdq_pkt_finalize(struct cmdq_pkt *pkt);

#else 

static inline int cmdq_dev_get_client_reg(struct device *dev,
					  struct cmdq_client_reg *client_reg, int idx)
{
	return -ENODEV;
}

static inline struct cmdq_client *cmdq_mbox_create(struct device *dev, int index)
{
	return ERR_PTR(-EINVAL);
}

static inline void cmdq_mbox_destroy(struct cmdq_client *client) { }

static inline int cmdq_pkt_create(struct cmdq_client *client, struct cmdq_pkt *pkt, size_t size)
{
	return -EINVAL;
}

static inline void cmdq_pkt_destroy(struct cmdq_client *client, struct cmdq_pkt *pkt) { }

static inline int cmdq_pkt_write(struct cmdq_pkt *pkt, u8 subsys, u16 offset, u32 value)
{
	return -ENOENT;
}

static inline int cmdq_pkt_write_mask(struct cmdq_pkt *pkt, u8 subsys,
				      u16 offset, u32 value, u32 mask)
{
	return -ENOENT;
}

static inline int cmdq_pkt_read_s(struct cmdq_pkt *pkt, u16 high_addr_reg_idx,
				  u16 addr_low, u16 reg_idx)
{
	return -ENOENT;
}

static inline int cmdq_pkt_write_s(struct cmdq_pkt *pkt, u16 high_addr_reg_idx,
				   u16 addr_low, u16 src_reg_idx)
{
	return -ENOENT;
}

static inline int cmdq_pkt_write_s_mask(struct cmdq_pkt *pkt, u16 high_addr_reg_idx,
					u16 addr_low, u16 src_reg_idx, u32 mask)
{
	return -ENOENT;
}

static inline int cmdq_pkt_write_s_value(struct cmdq_pkt *pkt, u8 high_addr_reg_idx,
					 u16 addr_low, u32 value)
{
	return -ENOENT;
}

static inline int cmdq_pkt_write_s_mask_value(struct cmdq_pkt *pkt, u8 high_addr_reg_idx,
					      u16 addr_low, u32 value, u32 mask)
{
	return -ENOENT;
}

static inline int cmdq_pkt_wfe(struct cmdq_pkt *pkt, u16 event, bool clear)
{
	return -EINVAL;
}

static inline int cmdq_pkt_clear_event(struct cmdq_pkt *pkt, u16 event)
{
	return -EINVAL;
}

static inline int cmdq_pkt_set_event(struct cmdq_pkt *pkt, u16 event)
{
	return -EINVAL;
}

static inline int cmdq_pkt_poll(struct cmdq_pkt *pkt, u8 subsys,
				u16 offset, u32 value)
{
	return -EINVAL;
}

static inline int cmdq_pkt_poll_mask(struct cmdq_pkt *pkt, u8 subsys,
				     u16 offset, u32 value, u32 mask)
{
	return -EINVAL;
}

static inline int cmdq_pkt_assign(struct cmdq_pkt *pkt, u16 reg_idx, u32 value)
{
	return -EINVAL;
}

static inline int cmdq_pkt_poll_addr(struct cmdq_pkt *pkt, dma_addr_t addr, u32 value, u32 mask)
{
	return -EINVAL;
}

static inline int cmdq_pkt_jump_abs(struct cmdq_pkt *pkt, dma_addr_t addr, u8 shift_pa)
{
	return -EINVAL;
}

static inline int cmdq_pkt_jump(struct cmdq_pkt *pkt, dma_addr_t addr, u8 shift_pa)
{
	return -EINVAL;
}

static inline int cmdq_pkt_jump_rel(struct cmdq_pkt *pkt, s32 offset, u8 shift_pa)
{
	return -EINVAL;
}

static inline int cmdq_pkt_eoc(struct cmdq_pkt *pkt)
{
	return -EINVAL;
}

static inline int cmdq_pkt_finalize(struct cmdq_pkt *pkt)
{
	return -EINVAL;
}

#endif 

#endif	
