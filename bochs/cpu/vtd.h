#ifndef BX_CPU_VTD_H
#define BX_CPU_VTD_H 1
#if BX_SUPPORT_VTD

#ifndef __packed
#ifdef WIN32
#define __packed
#else  //linux
#define __packed   __attribute__ ((packed))
#endif //end WIN32
#endif //end __packed

/* DMAR Hardware Unit Definition address (IOMMU unit) */
#define Q35_HOST_BRIDGE_IOMMU_ADDR  0xfed90000ULL

#define VTD_PCI_BUS_MAX             256
#define VTD_PCI_SLOT_MAX            32
#define VTD_PCI_FUNC_MAX            8
#define VTD_PCI_SLOT(devfn)         (((devfn) >> 3) & 0x1f)
#define VTD_PCI_FUNC(devfn)         ((devfn) & 0x07)
#define VTD_SID_TO_BUS(sid)         (((sid) >> 8) & 0xff)
#define VTD_SID_TO_DEVFN(sid)       ((sid) & 0xff)

#define DMAR_REG_SIZE               0x230
#define VTD_HOST_AW_39BIT           39
#define VTD_HOST_AW_48BIT           48
#define VTD_HOST_ADDRESS_WIDTH      VTD_HOST_AW_39BIT
#define VTD_HAW_MASK(aw)            ((1ULL << (aw)) - 1)

#define DMAR_REPORT_F_INTR          (1)

#define  VTD_MSI_ADDR_HI_MASK        (0xffffffff00000000ULL)
#define  VTD_MSI_ADDR_HI_SHIFT       (32)
#define  VTD_MSI_ADDR_LO_MASK        (0x00000000ffffffffULL)


/* Context-Entry */
struct VTDContextEntry {
    union {
        struct {
            uint64_t lo;
            uint64_t hi;
        };
        struct {
            uint64_t val[4];
        };
    };
};

struct VTDContextCacheEntry {
    /* The cache entry is obsolete if
     * context_cache_gen!=IntelIOMMUState.context_cache_gen
     */
    uint32_t context_cache_gen;
    struct VTDContextEntry context_entry;
};

/* PASID Directory Entry */
struct VTDPASIDDirEntry {
    uint64_t val;
};

/* PASID Table Entry */
struct VTDPASIDEntry {
    uint64_t val[8];
};


struct VTDIOTLBEntry {
    uint64_t gfn;
    uint16_t domain_id;
    uint64_t slpte;
    uint64_t mask;
    uint8_t access_flags;
};

/* VT-d Source-ID Qualifier types */
enum {
    VTD_SQ_FULL = 0x00,     /* Full SID verification */
    VTD_SQ_IGN_3 = 0x01,    /* Ignore bit 3 */
    VTD_SQ_IGN_2_3 = 0x02,  /* Ignore bits 2 & 3 */
    VTD_SQ_IGN_1_3 = 0x03,  /* Ignore bits 1-3 */
    VTD_SQ_MAX,
};

/* VT-d Source Validation Types */
enum {
    VTD_SVT_NONE = 0x00,    /* No validation */
    VTD_SVT_ALL = 0x01,     /* Do full validation */
    VTD_SVT_BUS = 0x02,     /* Validate bus range */
    VTD_SVT_MAX,
};

/* Interrupt Remapping Table Entry Definition */
union VTD_IR_TableEntry {
    struct {
#if HOST_BIG_ENDIAN
        uint32_t __reserved_1:8;     /* Reserved 1 */
        uint32_t vector:8;           /* Interrupt Vector */
        uint32_t irte_mode:1;        /* IRTE Mode */
        uint32_t __reserved_0:3;     /* Reserved 0 */
        uint32_t __avail:4;          /* Available spaces for software */
        uint32_t delivery_mode:3;    /* Delivery Mode */
        uint32_t trigger_mode:1;     /* Trigger Mode */
        uint32_t redir_hint:1;       /* Redirection Hint */
        uint32_t dest_mode:1;        /* Destination Mode */
        uint32_t fault_disable:1;    /* Fault Processing Disable */
        uint32_t present:1;          /* Whether entry present/available */
#else
        uint32_t present:1;          /* Whether entry present/available */
        uint32_t fault_disable:1;    /* Fault Processing Disable */
        uint32_t dest_mode:1;        /* Destination Mode */
        uint32_t redir_hint:1;       /* Redirection Hint */
        uint32_t trigger_mode:1;     /* Trigger Mode */
        uint32_t delivery_mode:3;    /* Delivery Mode */
        uint32_t __avail:4;          /* Available spaces for software */
        uint32_t __reserved_0:3;     /* Reserved 0 */
        uint32_t irte_mode:1;        /* IRTE Mode */
        uint32_t vector:8;           /* Interrupt Vector */
        uint32_t __reserved_1:8;     /* Reserved 1 */
#endif
        uint32_t dest_id;            /* Destination ID */
        uint16_t source_id;          /* Source-ID */
#if HOST_BIG_ENDIAN
        uint64_t __reserved_2:44;    /* Reserved 2 */
        uint64_t sid_vtype:2;        /* Source-ID Validation Type */
        uint64_t sid_q:2;            /* Source-ID Qualifier */
#else
        uint64_t sid_q:2;            /* Source-ID Qualifier */
        uint64_t sid_vtype:2;        /* Source-ID Validation Type */
        uint64_t __reserved_2:44;    /* Reserved 2 */
#endif
    } __packed irte;
    uint64_t data[2];
};

#define VTD_IR_INT_FORMAT_COMPAT     (0) /* Compatible Interrupt */
#define VTD_IR_INT_FORMAT_REMAP      (1) /* Remappable Interrupt */

/* Programming format for MSI/MSI-X addresses */
union VTD_IR_MSIAddress {
    struct {
#if HOST_BIG_ENDIAN
        uint32_t __head:12;          /* Should always be: 0x0fee */
        uint32_t index_l:15;         /* Interrupt index bit 14-0 */
        uint32_t int_mode:1;         /* Interrupt format */
        uint32_t sub_valid:1;        /* SHV: Sub-Handle Valid bit */
        uint32_t index_h:1;          /* Interrupt index bit 15 */
        uint32_t __not_care:2;
#else
        uint32_t __not_care:2;
        uint32_t index_h:1;          /* Interrupt index bit 15 */
        uint32_t sub_valid:1;        /* SHV: Sub-Handle Valid bit */
        uint32_t int_mode:1;         /* Interrupt format */
        uint32_t index_l:15;         /* Interrupt index bit 14-0 */
        uint32_t __head:12;          /* Should always be: 0x0fee */
#endif
    } __packed addr;
    uint32_t data;
};

typedef struct VTDContextEntry VTDContextEntry;
typedef struct VTDContextCacheEntry VTDContextCacheEntry;
typedef struct VTDAddressSpace VTDAddressSpace;
typedef struct VTDIOTLBEntry VTDIOTLBEntry;
typedef struct VTDBus VTDBus;
typedef union VTD_IR_TableEntry VTD_IR_TableEntry;
typedef union VTD_IR_MSIAddress VTD_IR_MSIAddress;
typedef struct VTDPASIDDirEntry VTDPASIDDirEntry;
typedef struct VTDPASIDEntry VTDPASIDEntry;


/* When IR is enabled, all MSI/MSI-X data bits should be zero */
#define VTD_IR_MSI_DATA          (0)


/*
 * Intel IOMMU register specification
 */
#define DMAR_VER_REG            0x0  /* Arch version supported by this IOMMU */
#define DMAR_CAP_REG            0x8  /* Hardware supported capabilities */
#define DMAR_CAP_REG_HI         0xc  /* High 32-bit of DMAR_CAP_REG */
#define DMAR_ECAP_REG           0x10 /* Extended capabilities supported */
#define DMAR_ECAP_REG_HI        0X14
#define DMAR_GCMD_REG           0x18 /* Global command */
#define DMAR_GSTS_REG           0x1c /* Global status */
#define DMAR_RTADDR_REG         0x20 /* Root entry table */
#define DMAR_RTADDR_REG_HI      0X24
#define DMAR_CCMD_REG           0x28 /* Context command */
#define DMAR_CCMD_REG_HI        0x2c
#define DMAR_FSTS_REG           0x34 /* Fault status */
#define DMAR_FECTL_REG          0x38 /* Fault control */
#define DMAR_FEDATA_REG         0x3c /* Fault event interrupt data */
#define DMAR_FEADDR_REG         0x40 /* Fault event interrupt addr */
#define DMAR_FEUADDR_REG        0x44 /* Upper address */
#define DMAR_AFLOG_REG          0x58 /* Advanced fault control */
#define DMAR_AFLOG_REG_HI       0X5c
#define DMAR_PMEN_REG           0x64 /* Enable protected memory region */
#define DMAR_PLMBASE_REG        0x68 /* PMRR low addr */
#define DMAR_PLMLIMIT_REG       0x6c /* PMRR low limit */
#define DMAR_PHMBASE_REG        0x70 /* PMRR high base addr */
#define DMAR_PHMBASE_REG_HI     0X74
#define DMAR_PHMLIMIT_REG       0x78 /* PMRR high limit */
#define DMAR_PHMLIMIT_REG_HI    0x7c
#define DMAR_IQH_REG            0x80 /* Invalidation queue head */
#define DMAR_IQH_REG_HI         0X84
#define DMAR_IQT_REG            0x88 /* Invalidation queue tail */
#define DMAR_IQT_REG_HI         0X8c
#define DMAR_IQA_REG            0x90 /* Invalidation queue addr */
#define DMAR_IQA_REG_HI         0x94
#define DMAR_ICS_REG            0x9c /* Invalidation complete status */
#define DMAR_IRTA_REG           0xb8 /* Interrupt remapping table addr */
#define DMAR_IRTA_REG_HI        0xbc
#define DMAR_IECTL_REG          0xa0 /* Invalidation event control */
#define DMAR_IEDATA_REG         0xa4 /* Invalidation event data */
#define DMAR_IEADDR_REG         0xa8 /* Invalidation event address */
#define DMAR_IEUADDR_REG        0xac /* Invalidation event address */
#define DMAR_PQH_REG            0xc0 /* Page request queue head */
#define DMAR_PQH_REG_HI         0xc4
#define DMAR_PQT_REG            0xc8 /* Page request queue tail*/
#define DMAR_PQT_REG_HI         0xcc
#define DMAR_PQA_REG            0xd0 /* Page request queue address */
#define DMAR_PQA_REG_HI         0xd4
#define DMAR_PRS_REG            0xdc /* Page request status */
#define DMAR_PECTL_REG          0xe0 /* Page request event control */
#define DMAR_PEDATA_REG         0xe4 /* Page request event data */
#define DMAR_PEADDR_REG         0xe8 /* Page request event address */
#define DMAR_PEUADDR_REG        0xec /* Page event upper address */
#define DMAR_MTRRCAP_REG        0x100 /* MTRR capability */
#define DMAR_MTRRCAP_REG_HI     0x104
#define DMAR_MTRRDEF_REG        0x108 /* MTRR default type */
#define DMAR_MTRRDEF_REG_HI     0x10c

/* IOTLB registers */
#define DMAR_IOTLB_REG_OFFSET   0xf0 /* Offset to the IOTLB registers */
#define DMAR_IVA_REG            DMAR_IOTLB_REG_OFFSET /* Invalidate address */
#define DMAR_IVA_REG_HI         (DMAR_IVA_REG + 4)
/* IOTLB invalidate register */
#define DMAR_IOTLB_REG          (DMAR_IOTLB_REG_OFFSET + 0x8)
#define DMAR_IOTLB_REG_HI       (DMAR_IOTLB_REG + 4)

/* FRCD */
#define DMAR_FRCD_REG_OFFSET    0x220 /* Offset to the fault recording regs */
/* NOTICE: If you change the DMAR_FRCD_REG_NR, please remember to change the
 * DMAR_REG_SIZE in include/hw/i386/intel_iommu.h.
 * #define DMAR_REG_SIZE   (DMAR_FRCD_REG_OFFSET + 16 * DMAR_FRCD_REG_NR)
 */
#define DMAR_FRCD_REG_NR        1ULL /* Num of fault recording regs */

#define DMAR_FRCD_REG_0_0       0x220 /* The 0th fault recording regs */
#define DMAR_FRCD_REG_0_1       0x224
#define DMAR_FRCD_REG_0_2       0x228
#define DMAR_FRCD_REG_0_3       0x22c

/* Interrupt Address Range */
#define VTD_INTERRUPT_ADDR_FIRST    0xfee00000ULL
#define VTD_INTERRUPT_ADDR_LAST     0xfeefffffULL
#define VTD_INTERRUPT_ADDR_SIZE     (VTD_INTERRUPT_ADDR_LAST - VTD_INTERRUPT_ADDR_FIRST + 1)

/* The shift of source_id in the key of IOTLB hash table */
#define VTD_IOTLB_SID_SHIFT         36
#define VTD_IOTLB_LVL_SHIFT         52
#define VTD_IOTLB_MAX_SIZE          1024    /* Max size of the hash table */

/* IOTLB_REG */
#define VTD_TLB_GLOBAL_FLUSH        (1ULL << 60) /* Global invalidation */
#define VTD_TLB_DSI_FLUSH           (2ULL << 60) /* Domain-selective */
#define VTD_TLB_PSI_FLUSH           (3ULL << 60) /* Page-selective */
#define VTD_TLB_FLUSH_GRANU_MASK    (3ULL << 60)
#define VTD_TLB_GLOBAL_FLUSH_A      (1ULL << 57)
#define VTD_TLB_DSI_FLUSH_A         (2ULL << 57)
#define VTD_TLB_PSI_FLUSH_A         (3ULL << 57)
#define VTD_TLB_FLUSH_GRANU_MASK_A  (3ULL << 57)
#define VTD_TLB_IVT                 (1ULL << 63)
#define VTD_TLB_DID(val)            (((val) >> 32) & VTD_DOMAIN_ID_MASK)

/* IVA_REG */
#define VTD_IVA_ADDR(val)       ((val) & ~0xfffULL)
#define VTD_IVA_AM(val)         ((val) & 0x3fULL)

/* GCMD_REG */
#define VTD_GCMD_TE                 (1UL << 31)
#define VTD_GCMD_SRTP               (1UL << 30)
#define VTD_GCMD_SFL                (1UL << 29)
#define VTD_GCMD_EAFL               (1UL << 28)
#define VTD_GCMD_WBF                (1UL << 27)
#define VTD_GCMD_QIE                (1UL << 26)
#define VTD_GCMD_IRE                (1UL << 25)
#define VTD_GCMD_SIRTP              (1UL << 24)
#define VTD_GCMD_CFI                (1UL << 23)

/* GSTS_REG */
#define VTD_GSTS_TES                (1UL << 31)
#define VTD_GSTS_RTPS               (1UL << 30)
#define VTD_GSTS_FLS                (1UL << 29)
#define VTD_GSTS_AFLS               (1UL << 28)
#define VTD_GSTS_WBFS               (1UL << 27)
#define VTD_GSTS_QIES               (1UL << 26)
#define VTD_GSTS_IRES               (1UL << 25)
#define VTD_GSTS_IRTPS              (1UL << 24)
#define VTD_GSTS_CFIS               (1UL << 23)

/* CCMD_REG */
#define VTD_CCMD_ICC                (1ULL << 63)
#define VTD_CCMD_GLOBAL_INVL        (1ULL << 61)
#define VTD_CCMD_DOMAIN_INVL        (2ULL << 61)
#define VTD_CCMD_DEVICE_INVL        (3ULL << 61)
#define VTD_CCMD_CIRG_MASK          (3ULL << 61)
#define VTD_CCMD_GLOBAL_INVL_A      (1ULL << 59)
#define VTD_CCMD_DOMAIN_INVL_A      (2ULL << 59)
#define VTD_CCMD_DEVICE_INVL_A      (3ULL << 59)
#define VTD_CCMD_CAIG_MASK          (3ULL << 59)
#define VTD_CCMD_DID(val)           ((val) & VTD_DOMAIN_ID_MASK)
#define VTD_CCMD_SID(val)           (((val) >> 16) & 0xffffULL)
#define VTD_CCMD_FM(val)            (((val) >> 32) & 3ULL)

/* RTADDR_REG */
#define VTD_RTADDR_SMT              (1ULL << 10)
#define VTD_RTADDR_ADDR_MASK(aw)    (VTD_HAW_MASK(aw) ^ 0xfffULL)

/* IRTA_REG */
#define VTD_IRTA_ADDR_MASK(aw)      (VTD_HAW_MASK(aw) ^ 0xfffULL)
#define VTD_IRTA_EIME               (1ULL << 11)
#define VTD_IRTA_SIZE_MASK          (0xfULL)

/* ECAP_REG */
/* (offset >> 4) << 8 */
#define VTD_ECAP_IRO                (DMAR_IOTLB_REG_OFFSET << 4)
#define VTD_ECAP_QI                 (1ULL << 1)
#define VTD_ECAP_DT                 (1ULL << 2)
/* Interrupt Remapping support */
#define VTD_ECAP_IR                 (1ULL << 3)
#define VTD_ECAP_EIM                (1ULL << 4)
#define VTD_ECAP_PT                 (1ULL << 6)
#define VTD_ECAP_SC                 (1ULL << 7)
#define VTD_ECAP_MHMV               (15ULL << 20)
#define VTD_ECAP_SRS                (1ULL << 31)
#define VTD_ECAP_SMTS               (1ULL << 43)
#define VTD_ECAP_SLTS               (1ULL << 46)

/* CAP_REG */
/* (offset >> 4) << 24 */
#define VTD_CAP_FRO                 (DMAR_FRCD_REG_OFFSET << 20)
#define VTD_CAP_NFR                 ((DMAR_FRCD_REG_NR - 1) << 40)
#define VTD_DOMAIN_ID_SHIFT         16  /* 16-bit domain id for 64K domains */
#define VTD_DOMAIN_ID_MASK          ((1UL << VTD_DOMAIN_ID_SHIFT) - 1)
#define VTD_CAP_ND                  (((VTD_DOMAIN_ID_SHIFT - 4) / 2) & 7ULL)
#define VTD_ADDRESS_SIZE(aw)        (1ULL << (aw))
#define VTD_CAP_MGAW(aw)            ((((aw) - 1) & 0x3fULL) << 16)
#define VTD_MAMV                    18ULL
#define VTD_CAP_MAMV                (VTD_MAMV << 48)
#define VTD_CAP_PSI                 (1ULL << 39)
#define VTD_CAP_SLLPS               ((1ULL << 34) | (1ULL << 35))
#define VTD_CAP_DRAIN_WRITE         (1ULL << 54)
#define VTD_CAP_DRAIN_READ          (1ULL << 55)
#define VTD_CAP_DRAIN               (VTD_CAP_DRAIN_READ | VTD_CAP_DRAIN_WRITE)
#define VTD_CAP_CM                  (1ULL << 7)

/* Supported Adjusted Guest Address Widths */
#define VTD_CAP_SAGAW_SHIFT         8
#define VTD_CAP_SAGAW_MASK          (0x1fULL << VTD_CAP_SAGAW_SHIFT)
 /* 39-bit AGAW, 3-level page-table */
#define VTD_CAP_SAGAW_39bit         (0x2ULL << VTD_CAP_SAGAW_SHIFT)
 /* 48-bit AGAW, 4-level page-table */
#define VTD_CAP_SAGAW_48bit         (0x4ULL << VTD_CAP_SAGAW_SHIFT)

/* IQT_REG */
#define VTD_IQT_QT(dw_bit, val)     (dw_bit ? (((val) >> 5) & 0x3fffULL) : \
                                     (((val) >> 4) & 0x7fffULL))
#define VTD_IQT_QT_256_RSV_BIT      0x10

/* IQA_REG */
#define VTD_IQA_IQA_MASK(aw)        (VTD_HAW_MASK(aw) ^ 0xfffULL)
#define VTD_IQA_QS                  0x7ULL
#define VTD_IQA_DW_MASK             0x800

/* IQH_REG */
#define VTD_IQH_QH_SHIFT_4          4
#define VTD_IQH_QH_SHIFT_5          5
#define VTD_IQH_QH_MASK             0x7fff0ULL

/* ICS_REG */
#define VTD_ICS_IWC                 1UL

/* IECTL_REG */
#define VTD_IECTL_IM                (1UL << 31)
#define VTD_IECTL_IP                (1UL << 30)

/* FSTS_REG */
#define VTD_FSTS_FRI_MASK       0xff00UL
#define VTD_FSTS_FRI(val)       ((((uint32_t)(val)) << 8) & VTD_FSTS_FRI_MASK)
#define VTD_FSTS_IQE            (1UL << 4)
#define VTD_FSTS_PPF            (1UL << 1)
#define VTD_FSTS_PFO            1UL

/* FECTL_REG */
#define VTD_FECTL_IM            (1UL << 31)
#define VTD_FECTL_IP            (1UL << 30)

/* Fault Recording Register */
/* For the high 64-bit of 128-bit */
#define VTD_FRCD_F              (1ULL << 63)
#define VTD_FRCD_T              (1ULL << 62)
#define VTD_FRCD_FR(val)        (((val) & 0xffULL) << 32)
#define VTD_FRCD_SID_MASK       0xffffULL
#define VTD_FRCD_SID(val)       ((val) & VTD_FRCD_SID_MASK)
/* For the low 64-bit of 128-bit */
#define VTD_FRCD_FI(val)        ((val) & ~0xfffULL)

/* DMA Remapping Fault Conditions */
typedef enum VTDFaultReason {
    VTD_FR_RESERVED = 0,        /* Reserved for Advanced Fault logging */
    VTD_FR_ROOT_ENTRY_P = 1,    /* The Present(P) field of root-entry is 0 */
    VTD_FR_CONTEXT_ENTRY_P,     /* The Present(P) field of context-entry is 0 */
    VTD_FR_CONTEXT_ENTRY_INV,   /* Invalid programming of a context-entry */
    VTD_FR_ADDR_BEYOND_MGAW,    /* Input-address above (2^x-1) */
    VTD_FR_WRITE,               /* No write permission */
    VTD_FR_READ,                /* No read permission */
    /* Fail to access a second-level paging entry (not SL_PML4E) */
    VTD_FR_PAGING_ENTRY_INV,
    VTD_FR_ROOT_TABLE_INV,      /* Fail to access a root-entry */
    VTD_FR_CONTEXT_TABLE_INV,   /* Fail to access a context-entry */
    /* Non-zero reserved field in a present root-entry */
    VTD_FR_ROOT_ENTRY_RSVD,
    /* Non-zero reserved field in a present context-entry */
    VTD_FR_CONTEXT_ENTRY_RSVD,
    /* Non-zero reserved field in a second-level paging entry with at lease one
     * Read(R) and Write(W) or Execute(E) field is Set.
     */
    VTD_FR_PAGING_ENTRY_RSVD,
    /* Translation request or translated request explicitly blocked dut to the
     * programming of the Translation Type (T) field in the present
     * context-entry.
     */
    VTD_FR_CONTEXT_ENTRY_TT,
    /* Output address in the interrupt address range */
    VTD_FR_INTERRUPT_ADDR = 0xE,

    /* Interrupt remapping transition faults */
    VTD_FR_IR_REQ_RSVD = 0x20, /* One or more IR request reserved
                                * fields set */
    VTD_FR_IR_INDEX_OVER = 0x21, /* Index value greater than max */
    VTD_FR_IR_ENTRY_P = 0x22,    /* Present (P) not set in IRTE */
    VTD_FR_IR_ROOT_INVAL = 0x23, /* IR Root table invalid */
    VTD_FR_IR_IRTE_RSVD = 0x24,  /* IRTE Rsvd field non-zero with
                                  * Present flag set */
    VTD_FR_IR_REQ_COMPAT = 0x25, /* Encountered compatible IR
                                  * request while disabled */
    VTD_FR_IR_SID_ERR = 0x26,   /* Invalid Source-ID */

    VTD_FR_PASID_TABLE_INV = 0x58,  /*Invalid PASID table entry */

    /* Output address in the interrupt address range for scalable mode */
    VTD_FR_SM_INTERRUPT_ADDR = 0x87,
    VTD_FR_MAX,                 /* Guard */
} VTDFaultReason;

#define VTD_CONTEXT_CACHE_GEN_MAX       0xffffffffUL

/* Interrupt Entry Cache Invalidation Descriptor: VT-d 6.5.2.7. */
struct VTDInvDescIEC {
    uint32_t type:4;            /* Should always be 0x4 */
    uint32_t granularity:1;     /* If set, it's global IR invalidation */
    uint32_t resved_1:22;
    uint32_t index_mask:5;      /* 2^N for continuous int invalidation */
    uint32_t index:16;          /* Start index to invalidate */
    uint32_t reserved_2:16;
};
typedef struct VTDInvDescIEC VTDInvDescIEC;

/* Queued Invalidation Descriptor */
union VTDInvDesc {
    struct {
        uint64_t lo;
        uint64_t hi;
    };
    struct {
        uint64_t val[4];
    };
    union {
        VTDInvDescIEC iec;
    };
};
typedef union VTDInvDesc VTDInvDesc;

/* Masks for struct VTDInvDesc */
#define VTD_INV_DESC_TYPE               0xf
#define VTD_INV_DESC_CC                 0x1 /* Context-cache Invalidate Desc */
#define VTD_INV_DESC_IOTLB              0x2
#define VTD_INV_DESC_DEVICE             0x3
#define VTD_INV_DESC_IEC                0x4 /* Interrupt Entry Cache Invalidate Descriptor */
#define VTD_INV_DESC_WAIT               0x5 /* Invalidation Wait Descriptor */
#define VTD_INV_DESC_PIOTLB             0x6 /* PASID-IOTLB Invalidate Desc */
#define VTD_INV_DESC_PC                 0x7 /* PASID-cache Invalidate Desc */
#define VTD_INV_DESC_NONE               0   /* Not an Invalidate Descriptor */

/* Masks for Invalidation Wait Descriptor*/
#define VTD_INV_DESC_WAIT_SW            (1ULL << 5)
#define VTD_INV_DESC_WAIT_IF            (1ULL << 4)
#define VTD_INV_DESC_WAIT_FN            (1ULL << 6)
#define VTD_INV_DESC_WAIT_DATA_SHIFT    32
#define VTD_INV_DESC_WAIT_RSVD_LO       0Xffffff80ULL
#define VTD_INV_DESC_WAIT_RSVD_HI       3ULL

/* Masks for Context-cache Invalidation Descriptor */
#define VTD_INV_DESC_CC_G               (3ULL << 4)
#define VTD_INV_DESC_CC_GLOBAL          (1ULL << 4)
#define VTD_INV_DESC_CC_DOMAIN          (2ULL << 4)
#define VTD_INV_DESC_CC_DEVICE          (3ULL << 4)
#define VTD_INV_DESC_CC_DID(val)        (((val) >> 16) & VTD_DOMAIN_ID_MASK)
#define VTD_INV_DESC_CC_SID(val)        (((val) >> 32) & 0xffffUL)
#define VTD_INV_DESC_CC_FM(val)         (((val) >> 48) & 3UL)
#define VTD_INV_DESC_CC_RSVD            0xfffc00000000ffc0ULL

/* Masks for IOTLB Invalidate Descriptor */
#define VTD_INV_DESC_IOTLB_G            (3ULL << 4)
#define VTD_INV_DESC_IOTLB_GLOBAL       (1ULL << 4)
#define VTD_INV_DESC_IOTLB_DOMAIN       (2ULL << 4)
#define VTD_INV_DESC_IOTLB_PAGE         (3ULL << 4)
#define VTD_INV_DESC_IOTLB_DID(val)     (((val) >> 16) & VTD_DOMAIN_ID_MASK)
#define VTD_INV_DESC_IOTLB_ADDR(val)    ((val) & ~0xfffULL)
#define VTD_INV_DESC_IOTLB_AM(val)      ((val) & 0x3fULL)
#define VTD_INV_DESC_IOTLB_RSVD_LO      0xffffffff0000ff00ULL
#define VTD_INV_DESC_IOTLB_RSVD_HI      0xf80ULL

/* Mask for Device IOTLB Invalidate Descriptor */
#define VTD_INV_DESC_DEVICE_IOTLB_ADDR(val) ((val) & 0xfffffffffffff000ULL)
#define VTD_INV_DESC_DEVICE_IOTLB_SIZE(val) ((val) & 0x1)
#define VTD_INV_DESC_DEVICE_IOTLB_SID(val) (((val) >> 32) & 0xFFFFULL)
#define VTD_INV_DESC_DEVICE_IOTLB_RSVD_HI 0xffeULL
#define VTD_INV_DESC_DEVICE_IOTLB_RSVD_LO 0xffff0000ffe0fff8

/* Rsvd field masks for spte */
#define VTD_SPTE_SNP 0x800ULL

#define VTD_SPTE_PAGE_L1_RSVD_MASK(aw, dt_supported) \
        dt_supported ? \
        (0x800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM | VTD_SL_TM)) : \
        (0x800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))
#define VTD_SPTE_PAGE_L2_RSVD_MASK(aw) \
        (0x800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))
#define VTD_SPTE_PAGE_L3_RSVD_MASK(aw) \
        (0x800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))
#define VTD_SPTE_PAGE_L4_RSVD_MASK(aw) \
        (0x880ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))

#define VTD_SPTE_LPAGE_L2_RSVD_MASK(aw, dt_supported) \
        dt_supported ? \
        (0x1ff800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM | VTD_SL_TM)) : \
        (0x1ff800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))
#define VTD_SPTE_LPAGE_L3_RSVD_MASK(aw, dt_supported) \
        dt_supported ? \
        (0x3ffff800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM | VTD_SL_TM)) : \
        (0x3ffff800ULL | ~(VTD_HAW_MASK(aw) | VTD_SL_IGN_COM))

/* Information about page-selective IOTLB invalidate */
struct VTDIOTLBPageInvInfo {
    uint16_t domain_id;
    uint64_t addr;
    uint8_t mask;
};
typedef struct VTDIOTLBPageInvInfo VTDIOTLBPageInvInfo;

/* Pagesize of VTD paging structures, including root and context tables */
#define VTD_PAGE_SHIFT              12
#define VTD_PAGE_SIZE               (1ULL << VTD_PAGE_SHIFT)

#define VTD_PAGE_SHIFT_4K           12
#define VTD_PAGE_MASK_4K            (~((1ULL << VTD_PAGE_SHIFT_4K) - 1))
#define VTD_PAGE_SHIFT_2M           21
#define VTD_PAGE_MASK_2M            (~((1ULL << VTD_PAGE_SHIFT_2M) - 1))
#define VTD_PAGE_SHIFT_1G           30
#define VTD_PAGE_MASK_1G            (~((1ULL << VTD_PAGE_SHIFT_1G) - 1))

struct VTDRootEntry {
    uint64_t lo;
    uint64_t hi;
};
typedef struct VTDRootEntry VTDRootEntry;

/* Masks for struct VTDRootEntry */
#define VTD_ROOT_ENTRY_P            1ULL
#define VTD_ROOT_ENTRY_CTP          (~0xfffULL)

#define VTD_ROOT_ENTRY_NR           (VTD_PAGE_SIZE / sizeof(VTDRootEntry))
#define VTD_ROOT_ENTRY_RSVD(aw)     (0xffeULL | ~VTD_HAW_MASK(aw))

#define VTD_DEVFN_CHECK_MASK        0x80

/* Masks for struct VTDContextEntry */
/* lo */
#define VTD_CONTEXT_ENTRY_P         (1ULL << 0)
#define VTD_CONTEXT_ENTRY_FPD       (1ULL << 1) /* Fault Processing Disable */
#define VTD_CONTEXT_ENTRY_TT        (3ULL << 2) /* Translation Type */
#define VTD_CONTEXT_TT_MULTI_LEVEL  0
#define VTD_CONTEXT_TT_DEV_IOTLB    (1ULL << 2)
#define VTD_CONTEXT_TT_PASS_THROUGH (2ULL << 2)
/* Second Level Page Translation Pointer*/
#define VTD_CONTEXT_ENTRY_SLPTPTR   (~0xfffULL)
#define VTD_CONTEXT_ENTRY_RSVD_LO(aw) (0xff0ULL | ~VTD_HAW_MASK(aw))
/* hi */
#define VTD_CONTEXT_ENTRY_AW        7ULL /* Adjusted guest-address-width */
#define VTD_CONTEXT_ENTRY_DID(val)  (((val) >> 8) & VTD_DOMAIN_ID_MASK)
#define VTD_CONTEXT_ENTRY_RSVD_HI   0xffffffffff000080ULL

#define VTD_CONTEXT_ENTRY_NR        (VTD_PAGE_SIZE / sizeof(VTDContextEntry))

#define VTD_CTX_ENTRY_LEGACY_SIZE     16
#define VTD_CTX_ENTRY_SCALABLE_SIZE   32

#define VTD_SM_CONTEXT_ENTRY_RID2PASID_MASK 0xfffff
#define VTD_SM_CONTEXT_ENTRY_RSVD_VAL0(aw)  (0x1e0ULL | ~VTD_HAW_MASK(aw))
#define VTD_SM_CONTEXT_ENTRY_RSVD_VAL1      0xffffffffffe00000ULL

/* PASID Table Related Definitions */
#define VTD_PASID_DIR_BASE_ADDR_MASK  (~0xfffULL)
#define VTD_PASID_TABLE_BASE_ADDR_MASK (~0xfffULL)
#define VTD_PASID_DIR_ENTRY_SIZE      8
#define VTD_PASID_ENTRY_SIZE          64
#define VTD_PASID_DIR_BITS_MASK       (0x3fffULL)
#define VTD_PASID_DIR_INDEX(pasid)    (((pasid) >> 6) & VTD_PASID_DIR_BITS_MASK)
#define VTD_PASID_DIR_FPD             (1ULL << 1) /* Fault Processing Disable */
#define VTD_PASID_TABLE_BITS_MASK     (0x3fULL)
#define VTD_PASID_TABLE_INDEX(pasid)  ((pasid) & VTD_PASID_TABLE_BITS_MASK)
#define VTD_PASID_ENTRY_FPD           (1ULL << 1) /* Fault Processing Disable */

/* PASID Granular Translation Type Mask */
#define VTD_PASID_ENTRY_P              1ULL
#define VTD_SM_PASID_ENTRY_PGTT        (7ULL << 6)
#define VTD_SM_PASID_ENTRY_FLT         (1ULL << 6)
#define VTD_SM_PASID_ENTRY_SLT         (2ULL << 6)
#define VTD_SM_PASID_ENTRY_NESTED      (3ULL << 6)
#define VTD_SM_PASID_ENTRY_PT          (4ULL << 6)

#define VTD_SM_PASID_ENTRY_AW          7ULL /* Adjusted guest-address-width */
#define VTD_SM_PASID_ENTRY_DID(val)    ((val) & VTD_DOMAIN_ID_MASK)

/* Second Level Page Translation Pointer*/
#define VTD_SM_PASID_ENTRY_SLPTPTR     (~0xfffULL)

/* Paging Structure common */
#define VTD_SL_PT_PAGE_SIZE_MASK    (1ULL << 7)
/* Bits to decide the offset for each level */
#define VTD_SL_LEVEL_BITS           9

/* Second Level Paging Structure */
#define VTD_SL_PML4_LEVEL           4
#define VTD_SL_PDP_LEVEL            3
#define VTD_SL_PD_LEVEL             2
#define VTD_SL_PT_LEVEL             1
#define VTD_SL_PT_ENTRY_NR          512

/* Masks for Second Level Paging Entry */
#define VTD_SL_RW_MASK              3ULL
#define VTD_SL_R                    1ULL
#define VTD_SL_W                    (1ULL << 1)
#define VTD_SL_PT_BASE_ADDR_MASK(aw) (~(VTD_PAGE_SIZE - 1) & VTD_HAW_MASK(aw))
#define VTD_SL_IGN_COM              0xbff0000000000000ULL
#define VTD_SL_TM                   (1ULL << 62)


class BOCHSAPI bx_vtd_c : public logfunctions
{
   public:
    bx_vtd_c(BX_CPU_C *cpu);
    void write(bx_phy_address addr, void *data, unsigned len);
    void read(bx_phy_address addr, void *data, unsigned len);

};
#endif
#endif