/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef BX_IODEV_FW_CFG_H
#define BX_IODEV_FW_CFG_H
#include <stdint.h>

#ifdef __linux__
#include <cstddef.h>
#define UINT8 uint8_t
#define UINT32 uint32_t
#endif

#define FW_CFG_ACPI_DEVICE_ID	"QEMU0002"

/* selector key values for "well-known" fw_cfg entries */
#define FW_CFG_SIGNATURE	0x00
#define FW_CFG_ID		0x01
#define FW_CFG_UUID		0x02
#define FW_CFG_RAM_SIZE		0x03
#define FW_CFG_NOGRAPHIC	0x04
#define FW_CFG_NB_CPUS		0x05
#define FW_CFG_MACHINE_ID	0x06
#define FW_CFG_KERNEL_ADDR	0x07
#define FW_CFG_KERNEL_SIZE	0x08
#define FW_CFG_KERNEL_CMDLINE	0x09
#define FW_CFG_INITRD_ADDR	0x0a
#define FW_CFG_INITRD_SIZE	0x0b
#define FW_CFG_BOOT_DEVICE	0x0c
#define FW_CFG_NUMA		0x0d
#define FW_CFG_BOOT_MENU	0x0e
#define FW_CFG_MAX_CPUS		0x0f
#define FW_CFG_KERNEL_ENTRY	0x10
#define FW_CFG_KERNEL_DATA	0x11
#define FW_CFG_INITRD_DATA	0x12
#define FW_CFG_CMDLINE_ADDR	0x13
#define FW_CFG_CMDLINE_SIZE	0x14
#define FW_CFG_CMDLINE_DATA	0x15
#define FW_CFG_SETUP_ADDR	0x16
#define FW_CFG_SETUP_SIZE	0x17
#define FW_CFG_SETUP_DATA	0x18
#define FW_CFG_FILE_DIR		0x19
#define FW_CFG_FILE_FIRST	0x20
#define FW_CFG_FILE_SLOTS_MIN	0x10

#define FW_CFG_WRITE_CHANNEL	0x4000
#define FW_CFG_ARCH_LOCAL	0x8000
#define FW_CFG_ENTRY_MASK	(~(FW_CFG_WRITE_CHANNEL | FW_CFG_ARCH_LOCAL))

#define FW_CFG_INVALID		0xffff

/* width in bytes of fw_cfg control register */
#define FW_CFG_CTL_SIZE		0x02

/* fw_cfg "file name" is up to 56 characters (including terminating nul) */
#define FW_CFG_MAX_FILE_PATH	56

/* size in bytes of fw_cfg signature */
#define FW_CFG_SIG_SIZE 4

/* FW_CFG_ID bits */
#define FW_CFG_VERSION		0x01
#define FW_CFG_VERSION_DMA	0x02


/* FW_CFG_DMA_CONTROL bits */
#define FW_CFG_DMA_CTL_ERROR	0x01
#define FW_CFG_DMA_CTL_READ	0x02
#define FW_CFG_DMA_CTL_SKIP	0x04
#define FW_CFG_DMA_CTL_SELECT	0x08
#define FW_CFG_DMA_CTL_WRITE	0x10

#define FW_CFG_DMA_SIGNATURE    0x51454d5520434647ULL /* "QEMU CFG" */
/*
#define SWAP_16(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define SWAP_32(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24)) 

//#if defined (BX_LITTLE_ENDIAN)
*/
/*
#define SWAP_16(val) (val)
#define SWAP_32(val) (val)
#define SWAP_64(val) (val)

#else
*/

#define SWAP_16(val) bx_bswap16(val)
#define SWAP_32(val) bx_bswap32(val)
#define SWAP_64(val) bx_bswap64(val)

//#endif


#define QEMU_ENTRY_SIZE(size) (((size) & 0x7f) ? ((size) + 0x80) & ~0x80 : (size))

#define TABLE_FILE_ID 0x40
#define RSDP_FILE_ID 0x41
#define RSDT_FILE_ID 0x42
#define FADT_FILE_ID 0x43
#define FACS_FILE_ID 0x44
#define MADT_FILE_ID 0x45
#define HPET_FILE_ID 0x46
#define DSDT_FILE_ID 0x47
#define SSDT_FILE_ID 0x48
#define DMAR_FILE_ID 0x49
#define MSR_FILE_ID  0x4a

#define Q35_HOST_BRIDGE_IOMMU_ADDR  0xfed90000ULL


//0x11223344
//0x44332211
/* Control as first field allows for different structures selected by this
 * field, which might be useful in the future
 */


#define FW_CFG_VMCOREINFO_FILENAME "etc/vmcoreinfo"

#define FW_CFG_VMCOREINFO_FORMAT_NONE 0x0
#define FW_CFG_VMCOREINFO_FORMAT_ELF 0x1



#define QEMU_LOADER_FNAME_SIZE  FW_CFG_MAX_FILE_PATH

typedef enum {
  QemuLoaderCmdAllocate = 1,
  QemuLoaderCmdAddPointer,
  QemuLoaderCmdAddChecksum,
  QemuLoaderCmdWritePointer,
} QEMU_LOADER_COMMAND_TYPE;

typedef enum {
  QemuLoaderAllocHigh = 1,
  QemuLoaderAllocFSeg
} QEMU_LOADER_ALLOC_ZONE;

#if defined(_MSC_VER)
#pragma pack(push, 1)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=packed
#endif

/* fw_cfg file directory entry type */
struct fw_cfg_file {
	uint32_t size;
	uint16_t select;
	uint16_t reserved;
	char name[FW_CFG_MAX_FILE_PATH];
};

struct fw_cfg_files {
	uint32_t count;
	//abomination
#if BX_SUPPORT_VTD
      struct fw_cfg_file cfg_files[11];
#else
    struct fw_cfg_file cfg_files[10];

#endif
};
struct fw_cfg_dma_access {
	uint32_t control;
	uint32_t length;
	uint64_t address;
};

struct fw_cfg_vmcoreinfo {
	uint16_t host_format;
	uint16_t guest_format;
	uint32_t size;
	uint64_t paddr;
};

//
// QemuLoaderCmdAllocate: download the fw_cfg file named File, to a buffer
// allocated in the zone specified by Zone, aligned at a multiple of Alignment.
//
typedef struct {
  UINT8     File[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT32    Alignment;                    // power of two
  UINT8     Zone;                         // QEMU_LOADER_ALLOC_ZONE values
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
QEMU_LOADER_ALLOCATE;

//
// QemuLoaderCmdAddPointer: the bytes at
// [PointerOffset..PointerOffset+PointerSize) in the file PointerFile contain a
// relative pointer (an offset) into PointeeFile. Increment the relative
// pointer's value by the base address of where PointeeFile's contents have
// been placed (when QemuLoaderCmdAllocate has been executed for PointeeFile).
//
typedef struct {
  UINT8     PointerFile[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT8     PointeeFile[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT32    PointerOffset;
  UINT8     PointerSize;                      // one of 1, 2, 4, 8
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
QEMU_LOADER_ADD_POINTER;

//
// QemuLoaderCmdAddChecksum: calculate the UINT8 checksum (as per
// CalculateChecksum8()) of the range [Start..Start+Length) in File. Store the
// UINT8 result at ResultOffset in the same File.
//
typedef struct {
  UINT8     File[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT32    ResultOffset;
  UINT32    Start;
  UINT32    Length;
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
QEMU_LOADER_ADD_CHECKSUM;

//
// QemuLoaderCmdWritePointer: the bytes at
// [PointerOffset..PointerOffset+PointerSize) in the writeable fw_cfg file
// PointerFile are to receive the absolute address of PointeeFile, as allocated
// and downloaded by the firmware, incremented by the value of PointeeOffset.
// Store the sum of (a) the base address of where PointeeFile's contents have
// been placed (when QemuLoaderCmdAllocate has been executed for PointeeFile)
// and (b) PointeeOffset, to this portion of PointerFile.
//
// This command is similar to QemuLoaderCmdAddPointer; the difference is that
// the "pointer to patch" does not exist in guest-physical address space, only
// in "fw_cfg file space". In addition, the "pointer to patch" is not
// initialized by QEMU in-place with a possibly nonzero offset value: the
// relative offset into PointeeFile comes from the explicit PointeeOffset
// field.
//
typedef struct {
  UINT8     PointerFile[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT8     PointeeFile[QEMU_LOADER_FNAME_SIZE]; // NUL-terminated
  UINT32    PointerOffset;
  UINT32    PointeeOffset;
  UINT8     PointerSize;                      // one of 1, 2, 4, 8
} 
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
QEMU_LOADER_WRITE_POINTER;

typedef struct {
  UINT32    Type;                          // QEMU_LOADER_COMMAND_TYPE values
  union {
    QEMU_LOADER_ALLOCATE         Allocate;
    QEMU_LOADER_ADD_POINTER      AddPointer;
    QEMU_LOADER_ADD_CHECKSUM     AddChecksum;
    QEMU_LOADER_WRITE_POINTER    WritePointer;
    UINT8                        Padding[124];
  } Command;
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
QEMU_LOADER_ENTRY;


/****************************************************/
/* ACPI tables init */

/* Table structure from Linux kernel (the ACPI tables are under the
   BSD license) */

/*
 * All tables must be byte-packed to match the ACPI specification, since
 * the tables are provided by the system BIOS.
 */

#define ACPI_TABLE_HEADER_DEF   /* ACPI common table header */ \
	uint8_t                            signature [4];          /* ACPI signature (4 ASCII characters) */\
	uint32_t                             length;                 /* Length of table, in bytes, including header */\
	uint8_t                              revision;               /* ACPI Specification minor version # */\
	uint8_t                              checksum;               /* To make sum of entire table == 0 */\
	uint8_t                            oem_id [6];             /* OEM identification */\
	uint8_t                            oem_table_id [8];       /* OEM table identification */\
	uint32_t                             oem_revision;           /* OEM revision number */\
	uint8_t                            asl_compiler_id [4];    /* ASL compiler vendor ID */\
	uint32_t                             asl_compiler_revision;  /* ASL compiler revision number */


typedef struct         /* ACPI common table header */
{
	ACPI_TABLE_HEADER_DEF
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
acpi_table_header;

typedef struct         /* Root System Descriptor Pointer */
{
	uint8_t                            signature [8];          /* ACPI signature, contains "RSD PTR " */
	uint8_t                              checksum;               /* To make sum of struct == 0 */
	uint8_t                            oem_id [6];             /* OEM identification */
	uint8_t                              revision;               /* Must be 0 for 1.0, 2 for 2.0 */
	uint32_t                             rsdt_physical_address;  /* 32-bit physical address of RSDT */
	uint32_t                             length;                 /* XSDT Length in bytes including hdr */
	uint64_t                             xsdt_physical_address;  /* 64-bit physical address of XSDT */
	uint8_t                              extended_checksum;      /* Checksum of entire table */
	uint8_t                            reserved [3];           /* Reserved field must be 0 */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
rsdp_descriptor;

/*
 * ACPI 1.0 Root System Description Table (RSDT)
 */
typedef struct rsdt_descriptor_rev1
{
	ACPI_TABLE_HEADER_DEF                           /* ACPI common table header */
	uint32_t                             table_offset_entry [5]; /* Array of pointers to other */
			 /* ACPI tables */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
rsdt_descriptor_rev1;

/*
 * ACPI 1.0 Firmware ACPI Control Structure (FACS)
 */
typedef struct
{
	uint8_t                            signature[4];           /* ACPI Signature */
	uint32_t                             length;                 /* Length of structure, in bytes */
	uint32_t                             hardware_signature;     /* Hardware configuration signature */
	uint32_t                             firmware_waking_vector; /* ACPI OS waking vector */
	uint32_t                             global_lock;            /* Global Lock */
	uint32_t                             S4bios_f        : 1;    /* Indicates if S4BIOS support is present */
	uint32_t                             reserved1       : 31;   /* Must be 0 */
	uint8_t                              resverved3 [40];        /* Reserved - must be zero */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
facs_descriptor_rev1;

/*
 * ACPI 1.0 Fixed ACPI Description Table (FADT)
 */
typedef struct
{
	ACPI_TABLE_HEADER_DEF                           /* ACPI common table header */
	uint32_t                             firmware_ctrl;          /* Physical address of FACS */
	uint32_t                             dsdt;                   /* Physical address of DSDT */
	uint8_t                              model;                  /* System Interrupt Model */
	uint8_t                              reserved1;              /* Reserved */
	uint16_t                             sci_int;                /* System vector of SCI interrupt */
	uint32_t                             smi_cmd;                /* Port address of SMI command port */
	uint8_t                              acpi_enable;            /* Value to write to smi_cmd to enable ACPI */
	uint8_t                              acpi_disable;           /* Value to write to smi_cmd to disable ACPI */
	uint8_t                              S4bios_req;             /* Value to write to SMI CMD to enter S4BIOS state */
	uint8_t                              reserved2;              /* Reserved - must be zero */
	uint32_t                             pm1a_evt_blk;           /* Port address of Power Mgt 1a acpi_event Reg Blk */
	uint32_t                             pm1b_evt_blk;           /* Port address of Power Mgt 1b acpi_event Reg Blk */
	uint32_t                             pm1a_cnt_blk;           /* Port address of Power Mgt 1a Control Reg Blk */
	uint32_t                             pm1b_cnt_blk;           /* Port address of Power Mgt 1b Control Reg Blk */
	uint32_t                             pm2_cnt_blk;            /* Port address of Power Mgt 2 Control Reg Blk */
	uint32_t                             pm_tmr_blk;             /* Port address of Power Mgt Timer Ctrl Reg Blk */
	uint32_t                             gpe0_blk;               /* Port addr of General Purpose acpi_event 0 Reg Blk */
	uint32_t                             gpe1_blk;               /* Port addr of General Purpose acpi_event 1 Reg Blk */
	uint8_t                              pm1_evt_len;            /* Byte length of ports at pm1_x_evt_blk */
	uint8_t                              pm1_cnt_len;            /* Byte length of ports at pm1_x_cnt_blk */
	uint8_t                              pm2_cnt_len;            /* Byte Length of ports at pm2_cnt_blk */
	uint8_t                              pm_tmr_len;              /* Byte Length of ports at pm_tm_blk */
	uint8_t                              gpe0_blk_len;           /* Byte Length of ports at gpe0_blk */
	uint8_t                              gpe1_blk_len;           /* Byte Length of ports at gpe1_blk */
	uint8_t                              gpe1_base;              /* Offset in gpe model where gpe1 events start */
	uint8_t                              reserved3;              /* Reserved */
	uint16_t                             plvl2_lat;              /* Worst case HW latency to enter/exit C2 state */
	uint16_t                             plvl3_lat;              /* Worst case HW latency to enter/exit C3 state */
	uint16_t                             flush_size;             /* Size of area read to flush caches */
	uint16_t                             flush_stride;           /* Stride used in flushing caches */
	uint8_t                              duty_offset;            /* Bit location of duty cycle field in p_cnt reg */
	uint8_t                              duty_width;             /* Bit width of duty cycle field in p_cnt reg */
	uint8_t                              day_alrm;               /* Index to day-of-month alarm in RTC CMOS RAM */
	uint8_t                              mon_alrm;               /* Index to month-of-year alarm in RTC CMOS RAM */
	uint8_t                              century;                /* Index to century in RTC CMOS RAM */
	uint8_t                              reserved4;              /* Reserved */
	uint8_t                              reserved4a;             /* Reserved */
	uint8_t                              reserved4b;             /* Reserved */
#if 0
	uint32_t                             wb_invd         : 1;    /* The wbinvd instruction works properly */
	uint32_t                             wb_invd_flush   : 1;    /* The wbinvd flushes but does not invalidate */
	uint32_t                             proc_c1         : 1;    /* All processors support C1 state */
	uint32_t                             plvl2_up        : 1;    /* C2 state works on MP system */
	uint32_t                             pwr_button      : 1;    /* Power button is handled as a generic feature */
	uint32_t                             sleep_button    : 1;    /* Sleep button is handled as a generic feature, or not present */
	uint32_t                             fixed_rTC       : 1;    /* RTC wakeup stat not in fixed register space */
	uint32_t                             rtcs4           : 1;    /* RTC wakeup stat not possible from S4 */
	uint32_t                             tmr_val_ext     : 1;    /* The tmr_val width is 32 bits (0 = 24 bits) */
	uint32_t                             reserved5       : 23;   /* Reserved - must be zero */
#else
        uint32_t flags;
#endif
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
fadt_descriptor_rev1;

/*
 * MADT values and structures
 */

/* Values for MADT PCATCompat */

#define DUAL_PIC                0
#define MULTIPLE_APIC           1


/* Master MADT */

typedef struct
{
	ACPI_TABLE_HEADER_DEF                           /* ACPI common table header */
	uint32_t                             local_apic_address;     /* Physical address of local APIC */
#if 0
	uint32_t                             PCATcompat      : 1;    /* A one indicates system also has dual 8259s */
	uint32_t                             reserved1       : 31;
#else
        uint32_t                             flags;
#endif
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
multiple_apic_table;


/* Values for Type in APIC_HEADER_DEF */

#define APIC_PROCESSOR          0
#define APIC_IO                 1
#define APIC_XRUPT_OVERRIDE     2
#define APIC_NMI                3
#define APIC_LOCAL_NMI          4
#define APIC_ADDRESS_OVERRIDE   5
#define APIC_IO_SAPIC           6
#define APIC_LOCAL_SAPIC        7
#define APIC_XRUPT_SOURCE       8
#define APIC_RESERVED           9           /* 9 and greater are reserved */

/*
 * MADT sub-structures (Follow MULTIPLE_APIC_DESCRIPTION_TABLE)
 */
#define APIC_HEADER_DEF                     /* Common APIC sub-structure header */\
	uint8_t                              type; \
	uint8_t                              length;

/* Sub-structures for MADT */

typedef struct
{
	APIC_HEADER_DEF
	uint8_t                              processor_id;           /* ACPI processor id */
	uint8_t                              local_apic_id;          /* Processor's local APIC id */
#if 0
	uint32_t                             processor_enabled: 1;   /* Processor is usable if set */
	uint32_t                             reserved2       : 31;   /* Reserved, must be zero */
#else
        uint32_t flags;
#endif
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
madt_processor_apic;

/*
 *  * ACPI 2.0 Generic Address Space definition.
 *   */
typedef struct  {
    uint8_t  address_space_id;
    uint8_t  register_bit_width;
    uint8_t  register_bit_offset;
    uint8_t  reserved;
    uint64_t address;
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
acpi_20_generic_address;

/*
 *  * HPET Description Table
 *   */
typedef struct {
    ACPI_TABLE_HEADER_DEF                           /* ACPI common table header */
    uint32_t           timer_block_id;
    acpi_20_generic_address addr;
    uint8_t            hpet_number;
    uint16_t           min_tick;
    uint8_t            page_protect;
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
acpi_20_hpet;

#define ACPI_HPET_ADDRESS 0xFED00000UL

typedef struct
{
	APIC_HEADER_DEF
	uint8_t                              io_apic_id;             /* I/O APIC ID */
	uint8_t                              reserved;               /* Reserved - must be zero */
	uint32_t                             address;                /* APIC physical address */
	uint32_t                             interrupt;              /* Global system interrupt where INTI* lines start */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
madt_io_apic;

typedef struct
{
	APIC_HEADER_DEF
	uint8_t                bus;     /* Identifies ISA Bus */
	uint8_t                source;  /* Bus-relative interrupt source */
	uint32_t               gsi;     /* GSI that source will signal */
	uint16_t               flags;   /* MPS INTI flags */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
madt_int_override;

#define DMAR_REMAPPING_DRHD 0
#define DMAR_REMAPPING_RMRR 1
#define DMAR_REMAPPING_ATSR 2
#define DMAR_REMAPPING_RHSA 3
#define DMAR_REMAPPING_RESERVED 4
#define REMAPPING_INCLUDE_PCI_ALL Ox01
#define DMAR_SIG "DMAR"
#define DMAR_INTR_REMAP 0x01

typedef struct  {
    uint8_t type;
    uint8_t length;
    uint16_t reserved;
    uint8_t enumeration_id;
    uint8_t start_bus_number;
    uint16_t path[1];  /* Path starts here */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
device_scope;

typedef struct  {
    uint16_t type;
    uint16_t length;
    uint8_t flags;
    uint8_t reserved;
    uint16_t segment_number;
    uint64_t register_base_address;
    device_scope device_scope_entry[2]; /* Device Scope, One for IOAPIC, One for PCI ALL */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
dmar_remapping;

typedef struct {
    ACPI_TABLE_HEADER_DEF
    uint8_t host_address_width;
    uint8_t flags;
    uint8_t reserved[10];
    dmar_remapping table_offsets[1]; /* dmar_remapping structure starts here */
}
#if !defined(_MSC_VER)
  GCC_ATTRIBUTE((packed))
#endif
acpi_dmar;

#if defined(_MSC_VER) && (_MSC_VER<1300)
#pragma pack(pop)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=reset
#endif

#endif
