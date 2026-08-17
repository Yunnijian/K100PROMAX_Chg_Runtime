/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __PSP_SEV_H__
#define __PSP_SEV_H__

#include <uapi/linux/psp-sev.h>

#define SEV_FW_BLOB_MAX_SIZE	0x4000	


enum sev_state {
	SEV_STATE_UNINIT		= 0x0,
	SEV_STATE_INIT			= 0x1,
	SEV_STATE_WORKING		= 0x2,

	SEV_STATE_MAX
};


enum sev_cmd {
	
	SEV_CMD_INIT			= 0x001,
	SEV_CMD_SHUTDOWN		= 0x002,
	SEV_CMD_FACTORY_RESET		= 0x003,
	SEV_CMD_PLATFORM_STATUS		= 0x004,
	SEV_CMD_PEK_GEN			= 0x005,
	SEV_CMD_PEK_CSR			= 0x006,
	SEV_CMD_PEK_CERT_IMPORT		= 0x007,
	SEV_CMD_PDH_CERT_EXPORT		= 0x008,
	SEV_CMD_PDH_GEN			= 0x009,
	SEV_CMD_DF_FLUSH		= 0x00A,
	SEV_CMD_DOWNLOAD_FIRMWARE	= 0x00B,
	SEV_CMD_GET_ID			= 0x00C,
	SEV_CMD_INIT_EX                 = 0x00D,

	
	SEV_CMD_DECOMMISSION		= 0x020,
	SEV_CMD_ACTIVATE		= 0x021,
	SEV_CMD_DEACTIVATE		= 0x022,
	SEV_CMD_GUEST_STATUS		= 0x023,

	
	SEV_CMD_LAUNCH_START		= 0x030,
	SEV_CMD_LAUNCH_UPDATE_DATA	= 0x031,
	SEV_CMD_LAUNCH_UPDATE_VMSA	= 0x032,
	SEV_CMD_LAUNCH_MEASURE		= 0x033,
	SEV_CMD_LAUNCH_UPDATE_SECRET	= 0x034,
	SEV_CMD_LAUNCH_FINISH		= 0x035,
	SEV_CMD_ATTESTATION_REPORT	= 0x036,

	
	SEV_CMD_SEND_START		= 0x040,
	SEV_CMD_SEND_UPDATE_DATA	= 0x041,
	SEV_CMD_SEND_UPDATE_VMSA	= 0x042,
	SEV_CMD_SEND_FINISH		= 0x043,
	SEV_CMD_SEND_CANCEL		= 0x044,

	
	SEV_CMD_RECEIVE_START		= 0x050,
	SEV_CMD_RECEIVE_UPDATE_DATA	= 0x051,
	SEV_CMD_RECEIVE_UPDATE_VMSA	= 0x052,
	SEV_CMD_RECEIVE_FINISH		= 0x053,

	
	SEV_CMD_DBG_DECRYPT		= 0x060,
	SEV_CMD_DBG_ENCRYPT		= 0x061,

	
	SEV_CMD_SNP_INIT		= 0x081,
	SEV_CMD_SNP_SHUTDOWN		= 0x082,
	SEV_CMD_SNP_PLATFORM_STATUS	= 0x083,
	SEV_CMD_SNP_DF_FLUSH		= 0x084,
	SEV_CMD_SNP_INIT_EX		= 0x085,
	SEV_CMD_SNP_SHUTDOWN_EX		= 0x086,
	SEV_CMD_SNP_DECOMMISSION	= 0x090,
	SEV_CMD_SNP_ACTIVATE		= 0x091,
	SEV_CMD_SNP_GUEST_STATUS	= 0x092,
	SEV_CMD_SNP_GCTX_CREATE		= 0x093,
	SEV_CMD_SNP_GUEST_REQUEST	= 0x094,
	SEV_CMD_SNP_ACTIVATE_EX		= 0x095,
	SEV_CMD_SNP_LAUNCH_START	= 0x0A0,
	SEV_CMD_SNP_LAUNCH_UPDATE	= 0x0A1,
	SEV_CMD_SNP_LAUNCH_FINISH	= 0x0A2,
	SEV_CMD_SNP_DBG_DECRYPT		= 0x0B0,
	SEV_CMD_SNP_DBG_ENCRYPT		= 0x0B1,
	SEV_CMD_SNP_PAGE_SWAP_OUT	= 0x0C0,
	SEV_CMD_SNP_PAGE_SWAP_IN	= 0x0C1,
	SEV_CMD_SNP_PAGE_MOVE		= 0x0C2,
	SEV_CMD_SNP_PAGE_MD_INIT	= 0x0C3,
	SEV_CMD_SNP_PAGE_SET_STATE	= 0x0C6,
	SEV_CMD_SNP_PAGE_RECLAIM	= 0x0C7,
	SEV_CMD_SNP_PAGE_UNSMASH	= 0x0C8,
	SEV_CMD_SNP_CONFIG		= 0x0C9,
	SEV_CMD_SNP_DOWNLOAD_FIRMWARE_EX = 0x0CA,
	SEV_CMD_SNP_COMMIT		= 0x0CB,
	SEV_CMD_SNP_VLEK_LOAD		= 0x0CD,

	SEV_CMD_MAX,
};


struct sev_data_init {
	u32 flags;			
	u32 reserved;			
	u64 tmr_address;		
	u32 tmr_len;			
} __packed;


struct sev_data_init_ex {
	u32 length;                     
	u32 flags;                      
	u64 tmr_address;                
	u32 tmr_len;                    
	u32 reserved;                   
	u64 nv_address;                 
	u32 nv_len;                     
} __packed;

#define SEV_INIT_FLAGS_SEV_ES	0x01


struct sev_data_pek_csr {
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_pek_cert_import {
	u64 pek_cert_address;			
	u32 pek_cert_len;			
	u32 reserved;				
	u64 oca_cert_address;			
	u32 oca_cert_len;			
} __packed;


struct sev_data_download_firmware {
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_get_id {
	u64 address;				
	u32 len;				
} __packed;

struct sev_data_pdh_cert_export {
	u64 pdh_cert_address;			
	u32 pdh_cert_len;			
	u32 reserved;				
	u64 cert_chain_address;			
	u32 cert_chain_len;			
} __packed;


struct sev_data_decommission {
	u32 handle;				
} __packed;


struct sev_data_activate {
	u32 handle;				
	u32 asid;				
} __packed;


struct sev_data_deactivate {
	u32 handle;				
} __packed;


struct sev_data_guest_status {
	u32 handle;				
	u32 policy;				
	u32 asid;				
	u8 state;				
} __packed;


struct sev_data_launch_start {
	u32 handle;				
	u32 policy;				
	u64 dh_cert_address;			
	u32 dh_cert_len;			
	u32 reserved;				
	u64 session_address;			
	u32 session_len;			
} __packed;


struct sev_data_launch_update_data {
	u32 handle;				
	u32 reserved;
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_launch_update_vmsa {
	u32 handle;				
	u32 reserved;
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_launch_measure {
	u32 handle;				
	u32 reserved;
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_launch_secret {
	u32 handle;				
	u32 reserved1;
	u64 hdr_address;			
	u32 hdr_len;				
	u32 reserved2;
	u64 guest_address;			
	u32 guest_len;				
	u32 reserved3;
	u64 trans_address;			
	u32 trans_len;				
} __packed;


struct sev_data_launch_finish {
	u32 handle;				
} __packed;


struct sev_data_send_start {
	u32 handle;				
	u32 policy;				
	u64 pdh_cert_address;			
	u32 pdh_cert_len;			
	u32 reserved1;
	u64 plat_certs_address;			
	u32 plat_certs_len;			
	u32 reserved2;
	u64 amd_certs_address;			
	u32 amd_certs_len;			
	u32 reserved3;
	u64 session_address;			
	u32 session_len;			
} __packed;


struct sev_data_send_update_data {
	u32 handle;				
	u32 reserved1;
	u64 hdr_address;			
	u32 hdr_len;				
	u32 reserved2;
	u64 guest_address;			
	u32 guest_len;				
	u32 reserved3;
	u64 trans_address;			
	u32 trans_len;				
} __packed;


struct sev_data_send_update_vmsa {
	u32 handle;				
	u64 hdr_address;			
	u32 hdr_len;				
	u32 reserved2;
	u64 guest_address;			
	u32 guest_len;				
	u32 reserved3;
	u64 trans_address;			
	u32 trans_len;				
} __packed;


struct sev_data_send_finish {
	u32 handle;				
} __packed;


struct sev_data_send_cancel {
	u32 handle;				
} __packed;


struct sev_data_receive_start {
	u32 handle;				
	u32 policy;				
	u64 pdh_cert_address;			
	u32 pdh_cert_len;			
	u32 reserved1;
	u64 session_address;			
	u32 session_len;			
} __packed;


struct sev_data_receive_update_data {
	u32 handle;				
	u32 reserved1;
	u64 hdr_address;			
	u32 hdr_len;				
	u32 reserved2;
	u64 guest_address;			
	u32 guest_len;				
	u32 reserved3;
	u64 trans_address;			
	u32 trans_len;				
} __packed;


struct sev_data_receive_update_vmsa {
	u32 handle;				
	u32 reserved1;
	u64 hdr_address;			
	u32 hdr_len;				
	u32 reserved2;
	u64 guest_address;			
	u32 guest_len;				
	u32 reserved3;
	u64 trans_address;			
	u32 trans_len;				
} __packed;


struct sev_data_receive_finish {
	u32 handle;				
} __packed;


struct sev_data_dbg {
	u32 handle;				
	u32 reserved;
	u64 src_addr;				
	u64 dst_addr;				
	u32 len;				
} __packed;


struct sev_data_attestation_report {
	u32 handle;				
	u32 reserved;
	u64 address;				
	u8 mnonce[16];				
	u32 len;				
} __packed;


struct sev_data_snp_download_firmware {
	u64 address;				
	u32 len;				
} __packed;


struct sev_data_snp_activate {
	u64 gctx_paddr;				
	u32 asid;				
} __packed;


struct sev_data_snp_addr {
	u64 address;				
} __packed;


struct sev_data_snp_launch_start {
	u64 gctx_paddr;				
	u64 policy;				
	u64 ma_gctx_paddr;			
	u32 ma_en:1;				
	u32 imi_en:1;				
	u32 rsvd:30;
	u32 desired_tsc_khz;			
	u8 gosvw[16];				
} __packed;


enum {
	SNP_PAGE_TYPE_NORMAL		= 0x1,
	SNP_PAGE_TYPE_VMSA		= 0x2,
	SNP_PAGE_TYPE_ZERO		= 0x3,
	SNP_PAGE_TYPE_UNMEASURED	= 0x4,
	SNP_PAGE_TYPE_SECRET		= 0x5,
	SNP_PAGE_TYPE_CPUID		= 0x6,

	SNP_PAGE_TYPE_MAX
};


struct sev_data_snp_launch_update {
	u64 gctx_paddr;				
	u32 page_size:1;			
	u32 page_type:3;			
	u32 imi_page:1;				
	u32 rsvd:27;
	u32 rsvd2;
	u64 address;				
	u32 rsvd3:8;
	u32 vmpl1_perms:8;			
	u32 vmpl2_perms:8;			
	u32 vmpl3_perms:8;			
	u32 rsvd4;
} __packed;


struct sev_data_snp_launch_finish {
	u64 gctx_paddr;
	u64 id_block_paddr;
	u64 id_auth_paddr;
	u8 id_block_en:1;
	u8 auth_key_en:1;
	u8 vcek_disabled:1;
	u64 rsvd:61;
	u8 host_data[32];
} __packed;


struct sev_data_snp_guest_status {
	u64 gctx_paddr;
	u64 address;
} __packed;


struct sev_data_snp_page_reclaim {
	u64 paddr;
} __packed;


struct sev_data_snp_page_unsmash {
	u64 paddr;
} __packed;


struct sev_data_snp_dbg {
	u64 gctx_paddr;				
	u64 src_addr;				
	u64 dst_addr;				
} __packed;


struct sev_data_snp_guest_request {
	u64 gctx_paddr;				
	u64 req_paddr;				
	u64 res_paddr;				
} __packed;


struct sev_data_snp_init_ex {
	u32 init_rmp:1;
	u32 list_paddr_en:1;
	u32 rsvd:30;
	u32 rsvd1;
	u64 list_paddr;
	u8  rsvd2[48];
} __packed;


struct sev_data_range {
	u64 base;
	u32 page_count;
	u32 rsvd;
} __packed;


struct sev_data_range_list {
	u32 num_elements;
	u32 rsvd;
	struct sev_data_range ranges[];
} __packed;


struct sev_data_snp_shutdown_ex {
	u32 len;
	u32 iommu_snp_shutdown:1;
	u32 rsvd1:31;
} __packed;


struct sev_platform_init_args {
	int error;
	bool probe;
};


struct sev_data_snp_commit {
	u32 len;
} __packed;

#ifdef CONFIG_CRYPTO_DEV_SP_PSP


int sev_platform_init(struct sev_platform_init_args *args);


int sev_platform_status(struct sev_user_data_status *status, int *error);


int sev_issue_cmd_external_user(struct file *filep, unsigned int id,
				void *data, int *error);


int sev_guest_deactivate(struct sev_data_deactivate *data, int *error);


int sev_guest_activate(struct sev_data_activate *data, int *error);


int sev_guest_df_flush(int *error);


int sev_guest_decommission(struct sev_data_decommission *data, int *error);


int sev_do_cmd(int cmd, void *data, int *psp_ret);

void *psp_copy_user_blob(u64 uaddr, u32 len);
void *snp_alloc_firmware_page(gfp_t mask);
void snp_free_firmware_page(void *addr);

#else	

static inline int
sev_platform_status(struct sev_user_data_status *status, int *error) { return -ENODEV; }

static inline int sev_platform_init(struct sev_platform_init_args *args) { return -ENODEV; }

static inline int
sev_guest_deactivate(struct sev_data_deactivate *data, int *error) { return -ENODEV; }

static inline int
sev_guest_decommission(struct sev_data_decommission *data, int *error) { return -ENODEV; }

static inline int
sev_do_cmd(int cmd, void *data, int *psp_ret) { return -ENODEV; }

static inline int
sev_guest_activate(struct sev_data_activate *data, int *error) { return -ENODEV; }

static inline int sev_guest_df_flush(int *error) { return -ENODEV; }

static inline int
sev_issue_cmd_external_user(struct file *filep, unsigned int id, void *data, int *error) { return -ENODEV; }

static inline void *psp_copy_user_blob(u64 __user uaddr, u32 len) { return ERR_PTR(-EINVAL); }

static inline void *snp_alloc_firmware_page(gfp_t mask)
{
	return NULL;
}

static inline void snp_free_firmware_page(void *addr) { }

#endif	

#endif	
