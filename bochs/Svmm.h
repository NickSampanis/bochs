#pragma once
#include <Windows.h>
#include <stdint.h>

//#define BOCHS 1
#define GVM 1
#define ALIGNED(x) __declspec(align(x))
#define PACKED ALIGNED(1)

#define BX_CR3_PAGING_MASK			0x000ffffffffff000ULL
#define PAGING_MASK					0x000ffffffffff000ULL
#define PAGE_1GB_ENABLED			(1 << 7)
#define PAGE_2MB_ENABLED			(1 << 7)

#define CR0_PG						(1ULL << 31)
#define EFER_LME					(1 << 8)
#define CR4_PAE						(1 << 5)

#define CPU_MODE_REAL			0
#define CPU_MODE_PROTECTED		1
#define CPU_MODE_LONG_MODE		2

#define GB (1024ULL * 1024ULL * 1024ULL)
#define MB (1024ULL * 1024ULL)
#define BIOSROMSZ		(1 << 21)    /* 2M BIOS ROM @0xffe00000, must be a power of 2 */
#define EXROMSIZE		(0x20000)    /* ROMs 0xc0000-0xdffff (area 0xe0000-0xfffff=bios mapped) */
#define PAGE_SIZE		0x1000
#define SMM_RAM_SIZE	0x20000
#define ROM_SIZE		(BIOSROMSZ + EXROMSIZE)
#define BIOS_MASK		(BIOSROMSZ - 1)
#define EXROM_MASK		(EXROMSIZE - 1)
#define TYPE_SYSTEM_BIOS  0
#define TYPE_VGA_BIOS	  1	

#define ROM_ADDR_BIOS	  0x00000
#define ROM_ADDR_VGA	  0xc0000

#define EVENT_NMI                          (1 <<  0)
#define EVENT_SMI                          (1 <<  1)
#define EVENT_INIT                         (1 <<  2)
#define EVENT_CODE_BREAKPOINT_ASSIST       (1 <<  3)
#define EVENT_VMX_MONITOR_TRAP_FLAG        (1 <<  4)
#define EVENT_VMX_PREEMPTION_TIMER_EXPIRED (1 <<  5)
#define EVENT_VMX_INTERRUPT_WINDOW_EXITING (1 <<  6)
#define EVENT_VMX_VIRTUAL_NMI              (1 <<  7)
#define EVENT_SVM_VIRQ_PENDING             (1 <<  8)
#define EVENT_PENDING_VMX_VIRTUAL_INTR     (1 <<  9)
#define EVENT_PENDING_INTR                 (1 << 10)
#define EVENT_PENDING_LAPIC_INTR           (1 << 11)
#define EVENT_VMX_VTPR_UPDATE              (1 << 12)
#define EVENT_VMX_VEOI_UPDATE              (1 << 13)
#define EVENT_VMX_VIRTUAL_APIC_WRITE       (1 << 14)
#define EVENT_DMA						   (1 << 15)


#define BX_CR3_PAGING_MASK			0x000ffffffffff000ULL
#define PAGING_MASK					0x000ffffffffff000ULL
#define PAGE_1GB_ENABLED			(1 << 7)
#define PAGE_2MB_ENABLED			(1 << 7)

#define CR0_PG						(1ULL << 31)
#define EFER_LME					(1 << 8)
#define CR4_PAE						(1 << 5)

#define EFLAGS_IF					(1 << 9)


#define EFLAGS_IF					(1 << 9)
// Segment descriptor

struct segment_desc_t {
    uint16_t selector;
    uint16_t _dummy;
    uint32_t limit;
    uint64_t base;
    union {
        struct {
            uint32_t type : 4;
            uint32_t desc : 1;
            uint32_t dpl : 2;
            uint32_t present : 1;
            uint32_t : 4;
            uint32_t available : 1;
            uint32_t long_mode : 1;
            uint32_t operand_size : 1;
            uint32_t granularity : 1;
            uint32_t : 1;
            uint32_t : 15;
        };
        uint32_t ar;
    };
    uint32_t ipad;
} PACKED;
typedef struct segment_desc_t segment_desc_t;

union interruptibility_state_t {
    uint32_t raw;
    struct {
        uint32_t sti_blocking : 1;
        uint32_t movss_blocking : 1;
        uint32_t smi_blocking : 1;
        uint32_t nmi_blocking : 1;
        uint32_t reserved : 28;
    };
    uint64_t pad;
} PACKED;

typedef union interruptibility_state_t interruptibility_state_t;

struct vcpu_state_t {
    union {
        uint64_t _regs[16];
        struct {
            union {
                struct {
                    uint8_t _al,
                        _ah;
                };
                uint16_t    _ax;
                uint32_t    _eax;
                uint64_t    _rax;
            };
            union {
                struct {
                    uint8_t _cl,
                        _ch;
                };
                uint16_t    _cx;
                uint32_t    _ecx;
                uint64_t    _rcx;
            };
            union {
                struct {
                    uint8_t _dl,
                        _dh;
                };
                uint16_t    _dx;
                uint32_t    _edx;
                uint64_t    _rdx;
            };
            union {
                struct {
                    uint8_t _bl,
                        _bh;
                };
                uint16_t    _bx;
                uint32_t    _ebx;
                uint64_t    _rbx;
            };
            union {
                uint16_t    _sp;
                uint32_t    _esp;
                uint64_t    _rsp;
            };
            union {
                uint16_t    _bp;
                uint32_t    _ebp;
                uint64_t    _rbp;
            };
            union {
                uint16_t    _si;
                uint32_t    _esi;
                uint64_t    _rsi;
            };
            union {
                uint16_t    _di;
                uint32_t    _edi;
                uint64_t    _rdi;
            };

            uint64_t _r8;
            uint64_t _r9;
            uint64_t _r10;
            uint64_t _r11;
            uint64_t _r12;
            uint64_t _r13;
            uint64_t _r14;
            uint64_t _r15;
        };
    };

    union {
        uint32_t _eip;
        uint64_t _rip;
    };

    union {
        uint32_t _eflags;
        uint64_t _rflags;
    };

    segment_desc_t _cs;
    segment_desc_t _ss;
    segment_desc_t _ds;
    segment_desc_t _es;
    segment_desc_t _fs;
    segment_desc_t _gs;
    segment_desc_t _ldt;
    segment_desc_t _tr;

    segment_desc_t _gdt;
    segment_desc_t _idt;

    uint64_t _cr0;
    uint64_t _cr2;
    uint64_t _cr3;
    uint64_t _cr4;

    uint64_t _dr0;
    uint64_t _dr1;
    uint64_t _dr2;
    uint64_t _dr3;
    uint64_t _dr6;
    uint64_t _dr7;
    uint64_t _pde;

    uint32_t _efer;

    uint32_t _sysenter_cs;
    uint64_t _sysenter_eip;
    uint64_t _sysenter_esp;

    uint32_t _activity_state;
    uint32_t pad;
    interruptibility_state_t _interruptibility_state;
} PACKED;

struct vmx_msr {
    uint64_t entry;
    uint64_t value;
} PACKED;

#define HAX_MAX_MSR_ARRAY 0x20
struct hax_msr_data {
    uint16_t nr_msr;
    uint16_t done;
    uint16_t pad[2];
    struct vmx_msr entries[HAX_MAX_MSR_ARRAY];
} PACKED;

/* fx_layout has 3 formats table 3-56, 512bytes */
struct ALIGNED(16) fx_layout {
    uint16_t  fcw;
    uint16_t  fsw;
    uint8_t   ftw;
    uint8_t   res1;
    uint16_t  fop;
    union {
        struct {
            uint32_t  fip;
            uint16_t  fcs;
            uint16_t  res2;
        };
        uint64_t  fpu_ip;
    };
    union {
        struct {
            uint32_t  fdp;
            uint16_t  fds;
            uint16_t  res3;
        };
        uint64_t  fpu_dp;
    };
    uint32_t  mxcsr;
    uint32_t  mxcsr_mask;
    uint8_t   st_mm[8][16];
    uint8_t   mmx_1[8][16];
    uint8_t   mmx_2[8][16];
    uint8_t   pad[96];
};
__pragma(pack(push, 1))

typedef struct _HOST_STATE
{
    uint64_t host_cr0;
    uint64_t host_cr3;
    uint64_t host_cr4;
    uint64_t host_rsp;
    uint64_t host_rip;
    /*
    uint16_t segreg_selector[6];

    uint64_t fs_base;
    uint64_t gs_base;

    uint64_t gdtr_base;
    uint64_t idtr_base;

    uint32_t tr_selector;
    uint64_t tr_base;

    uint64_t sysenter_esp_msr;
    uint64_t sysenter_eip_msr;
    uint32_t sysenter_cs_msr;

    uint64_t efer_msr;
    uint64_t pat_msr;
    uint64_t ia32_spec_ctrl_msr;
    */
} HOST_STATE;

typedef struct _GUEST_STATE
{
    uint64_t guest_cr0;
    uint64_t guest_cr3;
    uint64_t guest_cr4;
    uint64_t guest_rsp;
    uint64_t guest_rip;
    /*
    uint32_t activity_state;
    uint32_t interruptibility_state;
    uint64_t pdptr[4];
    */
} GUEST_STATE;
__pragma(pack(pop))

struct Registers {
    struct vcpu_state_t context;
    struct hax_msr_data msrs;
    struct fx_layout fx;
    uint64_t xcr0;
    uint64_t cr8;
    uint64_t smbase;
    uint64_t tsc;
    uint64_t kernelgsbase;
    uint64_t apicbase;
    uint64_t pat;
    uint64_t star;
    uint64_t lstar;
    uint64_t cstar;
    uint64_t fmask;
    uint64_t tsc_aux;
    HOST_STATE vmcs_host;
    GUEST_STATE vmcs_guest;
    struct vcpu_state_t host_saved_regs;
    struct vcpu_state_t guest_saved_regs;
    uint8_t vmx_enabled;
    uint8_t vmx_in_guest;
    uint64_t last_accessed_addr;
    uint8_t cpu_number;
};



/*
NTSTATUS SvmmGetRegisters(BYTE CpuNumber, struct Registers* Registers);
NTSTATUS SvmmSetRegisters(BYTE CpuNumber, struct Registers* Registers);
//BYTE* SvmmGetHostAddress(ULONG64 GuestAddress);
BYTE* SvmmGetHostPageFromGPA(BYTE CpuNumber, ULONG64 gpaAddress);
*/