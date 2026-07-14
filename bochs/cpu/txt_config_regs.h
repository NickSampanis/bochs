/*
 * config_regs.h: Intel(r) TXT configuration register -related definitions
 *
 * Copyright (c) 2003-2011, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __TXT_CONFIG_REGS_H__
#define __TXT_CONFIG_REGS_H__

#define IA32_GETSEC_CAPABILITIES	0
#define IA32_GETSEC_ENTERACCS		2
#define IA32_GETSEC_SENTER		4
#define IA32_GETSEC_SEXIT		5
#define IA32_GETSEC_PARAMETERS		6
#define IA32_GETSEC_SMCTRL		7
#define IA32_GETSEC_WAKEUP		8

/*
 * GETSEC[] leaf functions
 */

typedef union {
    uint32_t _raw;
    struct {
        uint32_t chipset_present  : 1;
        uint32_t undefined1	  : 1;
        uint32_t enteraccs	  : 1;
        uint32_t exitac	          : 1;
        uint32_t senter	          : 1;
        uint32_t sexit	          : 1;
        uint32_t parameters	  : 1;
        uint32_t smctrl	          : 1;
        uint32_t wakeup	          : 1;
        uint32_t undefined9	  : 22;
        uint32_t extended_leafs   : 1;
    };
} capabilities_t;


#define TXT_AREA_PRIVATE  0
#define TXT_AREA_PUBLIC   1


/*
 * TXT configuration registers (offsets from TXT_{PUB, PRIV}_CONFIG_REGS_BASE)
 */

#define TXT_PUB_CONFIG_REGS_BASE       0xfed30000
#define TXT_PRIV_CONFIG_REGS_BASE      0xfed20000

#define TXT_CONFIG_REGS_SIZE           (TXT_PUB_CONFIG_REGS_BASE - \
                                        TXT_PRIV_CONFIG_REGS_BASE)

/* offsets to config regs (from either public or private _BASE) */
#define TXTCR_STS                   0x0000
#define TXTCR_ESTS                  0x0008
#define TXTCR_ERRORCODE             0x0030
#define TXTCR_CMD_RESET             0x0038
#define TXTCR_CMD_CLOSE_PRIVATE     0x0048
#define TXTCR_SACM_STATUS           0x00A0
#define TXTCR_VER_FSBIF             0x0100
#define TXTCR_DIDVID                0x0110
#define TXTCR_VER_QPIIF             0x0200
#define TXTCR_CMD_UNLOCK_MEM_CONFIG 0x0218
#define TXTCR_SINIT_BASE            0x0270
#define TXTCR_SINIT_SIZE            0x0278
#define TXTCR_MLE_JOIN              0x0290
#define TXTCR_HEAP_BASE             0x0300
#define TXTCR_HEAP_SIZE             0x0308
#define TXTCR_MSEG_BASE             0x0310
#define TXTCR_MSEG_SIZE             0x0318
#define TXTCR_DPR                   0x0330
#define TXTCR_CMD_OPEN_LOCALITY1    0x0380
#define TXTCR_CMD_CLOSE_LOCALITY1   0x0388
#define TXTCR_CMD_OPEN_LOCALITY2    0x0390
#define TXTCR_CMD_CLOSE_LOCALITY2   0x0398
#define TXTCR_PUBLIC_KEY            0x0400
#define TXTCR_TPM_SUPPORTED         0x0800
#define TXTCR_CMD_SECRETS           0x08e0
#define TXTCR_CMD_NO_SECRETS        0x08e8
#define TXTCR_E2STS                 0x08f0

/*
 * format of ERRORCODE register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t   type       : 30;    /* external-specific error code */
        uint64_t   external   : 1;     /* 0=from proc, 1=from external SW */
        uint64_t   valid      : 1;     /* 1=valid */
    };
} txt_errorcode_t;

/*
 * format of ESTS register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t   txt_reset_sts      : 1;
    };
} txt_ests_t;

/*
 * format of E2STS register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t   reserved             : 1;
        uint64_t   secrets_sts          : 1;
    };
} txt_e2sts_t;

/*
 * format of STS register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t   senter_done_sts         : 1;
        uint64_t   sexit_done_sts          : 1;
        uint64_t   reserved1               : 4;
        uint64_t   mem_config_lock_sts     : 1;
        uint64_t   private_open_sts        : 1;
        uint64_t   reserved2               : 7;
        uint64_t   locality_1_open_sts     : 1;
        uint64_t   locality_2_open_sts     : 1;
    };
} txt_sts_t;

/*
 * format of DIDVID register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint16_t  vendor_id;
        uint16_t  device_id;
        uint16_t  revision_id;
        uint16_t  reserved;
    };
} txt_didvid_t;

/*
 * format of VER.FSBIF and VER.QPIIF registers
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t  reserved       : 31;
        uint64_t  prod_fused     : 1;
    };
} txt_ver_fsbif_qpiif_t;

/*
 * format of DPR register
 */
typedef union {
    uint64_t _raw;
    struct {
        uint64_t  lock           : 1;
        uint64_t  reserved1      : 3;
        uint64_t  size           : 8;
        uint64_t  reserved2      : 8;
        uint64_t  top            : 12;
        uint64_t  reserved3      : 32;
    };
} txt_dpr_t;

/*
 * RLP JOIN structure for GETSEC[WAKEUP] and MLE_JOIN register
 */
typedef struct {
    uint32_t	gdt_limit;
    uint32_t	gdt_base;
    uint32_t	seg_sel;               /* cs (ds, es, ss are seg_sel+8) */
    uint32_t	entry_point;           /* phys addr */
} mle_join_t;

/*
 * format of MSEG header
 */
typedef struct {
    uint32_t    revision_id;
    uint32_t    smm_mon_feat;
    uint32_t    gdtr_limit;
    uint32_t    gdtr_base_offset;
    uint32_t    cs_sel;
    uint32_t    eip_offset;
    uint32_t    esp_offset;
    uint32_t    cr3_offset;
} mseg_hdr_t;

typedef void   txt_heap_t;

/*
 * authenticated code (AC) module header (ver 0.0)
 */

typedef union {
    uint16_t _raw;
    struct {
        uint16_t  reserved          : 14;
        uint16_t  pre_production    : 1;
        uint16_t  debug_signed      : 1;
    };
} acm_flags_t;


//extern acm_hdr_t *g_sinit;

/* value of module_type field */
#define ACM_TYPE_CHIPSET        0x02

/* value of module_subtype field */
#define ACM_SUBTYPE_RESET       0x01

/* value of module_vendor field */
#define ACM_VENDOR_INTEL        0x8086

/* ranges of padding present in TXTCR_SINIT_SIZE reg */
#define ACM_SIZE_MIN_PADDING    0x10000
#define ACM_SIZE_MAX_PADDING    0x40000

typedef union {
    uint32_t _raw;
    struct {
        uint32_t  ext_policy        : 2;
        uint32_t  tpm_family        : 4;
        uint32_t  tpm_nv_index_set  : 1;
        uint32_t  reserved          : 25;
    };
} tpm_cap_t;

/* ext_policy field values */
#define TPM_EXT_POLICY_ILLEGAL          0x00
#define TPM_EXT_POLICY_ALG_AGILE_CMD    0x01
#define TPM_EXT_POLICY_EMBEDED_ALGS     0x10
#define TPM_EXT_POLICY_BOTH             0x11

/* tpm_family field values */
#define TPM_FAMILY_ILLEGAL      0x0000
#define TPM_FAMILY_DTPM_12      0x0001
#define TPM_FAMILY_DTPM_20      0x0010
#define TPM_FAMILY_DTPM_BOTH    0x0011
#define TPM_FAMILY_PTT_20       0x1000

typedef struct {
    tpm_cap_t   capabilities;
    uint16_t    count;
    uint16_t    alg_id[];
} tpm_info_list_t;

typedef union {
    uint32_t  _raw;
    struct {
        uint32_t  rlp_wake_getsec     : 1;
        uint32_t  rlp_wake_monitor    : 1;
        uint32_t  ecx_pgtbl           : 1;
        uint32_t  stm                 : 1;
        uint32_t  pcr_map_no_legacy   : 1;
        uint32_t  pcr_map_da          : 1;
        uint32_t  platform_type       : 2;
        uint32_t  max_phy_addr        : 1;
        uint32_t  tcg_event_log_format: 1;
        uint32_t  cbnt_supported      : 1;
        uint32_t  reserved1           : 3;
        uint32_t  tpr_support         : 1;
        uint32_t  reserved2           : 17;
    };
} txt_caps_t;

#ifndef __packed
#ifdef WIN32
#define __packed
#else  //linux
#define __packed   __attribute__ ((packed))
#endif //end WIN32
#endif //end __packed

#define PLATFORM_TYPE_CLIENT 0x01
#define PLATFORM_TYPE_SERVER 0x02


#ifdef WIN32
#pragma pack(push, 1)
#endif

typedef struct {
    uint16_t     module_type;
    uint16_t     module_subtype;
    uint32_t     header_len;
    uint32_t     header_ver;          /* currently 0.0 */
    uint16_t     chipset_id;
    acm_flags_t  flags;
    uint32_t     module_vendor;
    uint32_t     date;
    uint32_t     size;
    uint16_t     txt_svn;
    uint16_t     se_svn;
    uint32_t     code_control;
    uint32_t     error_entry_point;
    uint32_t     gdt_limit;
    uint32_t     gdt_base;
    uint32_t     seg_sel;
    uint32_t     entry_point;
    uint8_t      reserved2[64];
    uint32_t     key_size;
    uint32_t     scratch_size;
    uint8_t      rsa2048_pubkey[256];
    uint32_t     pub_exp;
    uint8_t      rsa2048_sig[256];
    uint32_t     scratch[143];//
    uint8_t      user_area[0];
} acm_hdr_t;

typedef struct __packed {
  uint32_t    data1;
  uint16_t    data2;
  uint16_t    data3;
  uint16_t    data4;
  uint8_t     data5[6];
} uuid_t;

typedef struct __packed {
    uuid_t      uuid;
    uint8_t     chipset_acm_type;
    uint8_t     version;             /* currently 7 */
    uint16_t    length;
    uint32_t    chipset_id_list;
    uint32_t    os_sinit_data_ver;
    uint32_t    min_mle_hdr_ver;
    txt_caps_t  capabilities;
    uint8_t     acm_ver;
    uint8_t     acm_revision[3];
    /* versions >= 4 */
    uint32_t    processor_id_list;
    /* versions >= 5 */
    uint32_t    tpm_info_list_off;
} acm_info_table_t;

typedef struct __packed {
    uint32_t id;
    uint32_t size;
    uint32_t rev;
} list_header_t;

typedef struct __packed {
    uint32_t  flags;
    uint16_t  vendor_id;
    uint16_t  device_id;
    uint16_t  revision_id;
    uint16_t  register_mask;  //  reserved if ACM info table version < 9
    uint32_t  extended_id;
} acm_chipset_id_t;

typedef struct __packed {
    uint32_t           count;
    acm_chipset_id_t   chipset_ids[];
} acm_chipset_id_list_t;

typedef struct __packed {
    uint32_t  fms;
    uint32_t  fms_mask;
    uint64_t  platform_id;
    uint64_t  platform_mask;
} acm_processor_id_t;

typedef struct __packed {
    uint32_t             count;
    acm_processor_id_t   processor_ids[];
} acm_processor_id_list_t;


typedef struct __packed {
    uint32_t sinit;
    struct {
        /* versions >= 5 */
        uint32_t ppi_supported : 1;
        /* versions >= 6 */
        uint32_t platform_type : 2;
        uint32_t reserved      : 29;
    } mle;
} bios_data_flags_t;

typedef struct __packed {
    uint32_t   type;
    uint32_t   size;
} heap_ext_data_element_t;

typedef struct __packed {
    uint32_t  version;              /* currently 2-6 */
    uint32_t  bios_sinit_size;
    uint64_t  lcp_pd_base;
    uint64_t  lcp_pd_size;
    uint32_t  num_logical_procs;
    /* versions >= 3 */
    union {
        uint64_t raw;
        bios_data_flags_t bits; /* For TPM2, it is divided into sinit_flag and mle_flag */
    } flags;
    /* versions >= 4 */
} bios_data_t;




#define HEAP_EXTDATA_TYPE_END             0
#define HEAP_EXTDATA_TYPE_BIOS_SPEC_VER   1
#define HEAP_EXTDATA_TYPE_ACM             2

typedef struct __packed {
    heap_ext_data_element_t hdr;
    uint16_t   spec_ver_major;
    uint16_t   spec_ver_minor;
    uint16_t   spec_ver_rev;
} heap_bios_spec_ver_element_t;

typedef struct __packed {
    heap_ext_data_element_t hdr;
    uint32_t   num_acms;
    uint64_t acm_addrs;
} heap_acm_element_t;


#ifdef WIN32
#pragma pack(pop)
#endif



/*
 * MLE header structure
 *   describes an MLE for SINIT and OS/loader SW
 */
typedef struct {
    uuid_t      uuid;
    uint32_t    length;
    uint32_t    version;
    uint32_t    entry_point;
    uint32_t    first_valid_page;
    uint32_t    mle_start_off;
    uint32_t    mle_end_off;
    txt_caps_t  capabilities;
    uint32_t    cmdline_start_off;
    uint32_t    cmdline_end_off;
} mle_hdr_t;




/* ACM UUID value */
#define ACM_UUID_V3        {0x7fc03aaa, 0x46a7, 0x18db, 0xac2e, \
                                {0x69, 0x8f, 0x8d, 0x41, 0x7f, 0x5a}}

/* chipset_acm_type field values */
#define ACM_CHIPSET_TYPE_BIOS         0x00
#define ACM_CHIPSET_TYPE_SINIT        0x01
#define ACM_CHIPSET_TYPE_BIOS_REVOC   0x08
#define ACM_CHIPSET_TYPE_SINIT_REVOC  0x09

/* flexible ACM info table list IDs */
#define CPUL 0x4350554C
#define CS1L 0x4353314C
#define CS2L 0x4353324C
#define TERM 0x4E554C4C
#define TPML 0x54504D4C
#define VERL 0x5645524C


#endif /* __TXT_CONFIG_REGS_H__ */

