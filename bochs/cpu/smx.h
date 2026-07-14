#ifndef BX_CPU_TXT_H
#define BX_CPU_TXT_H 1
#if BX_SUPPORT_SMX
#include "txt_config_regs.h"
/*
//tigerlake
#define DPR_ENTRY           0xfc400041ULL
#define DPR_GPA             0xfc000000ULL
#define DPR_SIZE            0x00400000ULL
#define DPR_END             (DPR_GPA + DPR_SIZE)

#define SINIT_GPA           0xfc2c0000ULL
#define SINIT_SIZE          0x00050000ULL
#define SINIT_DPR_OFFSET    (SINIT_GPA - DPR_GPA)

#define TXT_HEAP_GPA        0xfc310000ULL
#define TXT_HEAP_SIZE       0x000f0000ULL
#define TXT_HEAP_DPR_OFFSET (TXT_HEAP_GPA - DPR_GPA)
*/

//Q35

#define DPR_SIZE            0x00300000ULL
#define DPR_GPA             (0xE0000000UL -  DPR_SIZE)
#define DPR_ENTRY           (0xE0000000UL |  (3 << 4) | 1)
#define DPR_END             (DPR_GPA + DPR_SIZE)

#define SINIT_GPA           (DPR_END - 0x100000UL) //f0200000-0xdff00000
#define SINIT_SIZE          0x00020000ULL
#define SINIT_DPR_OFFSET    (SINIT_GPA - DPR_GPA)

#define TXT_HEAP_GPA        ((DPR_END + 0x20000UL) - 0x100000UL) //f01e0000 dff20000
#define TXT_HEAP_SIZE       0x000E0000ULL
#define TXT_HEAP_DPR_OFFSET (TXT_HEAP_GPA - DPR_GPA)

/*
//TigerLake
#define DPR_SIZE            0x00400000ULL
#define DPR_GPA             (0xE0000000UL -  DPR_SIZE)
#define DPR_ENTRY           (0xE0000000UL |  ((DPR_SIZE >> 20) << 4) | 1)
#define DPR_END             (DPR_GPA + DPR_SIZE)

#define SINIT_GPA           (DPR_END - (SINIT_SIZE + TXT_HEAP_SIZE)) //f0200000-0xdff00000
#define SINIT_SIZE          0x00050000ULL
#define SINIT_DPR_OFFSET    (SINIT_GPA - DPR_GPA)

#define TXT_HEAP_GPA        ((DPR_END + SINIT_SIZE) - (SINIT_SIZE + TXT_HEAP_SIZE)) //f01e0000 dff20000
#define TXT_HEAP_SIZE       0x000f0000ULL
#define TXT_HEAP_DPR_OFFSET (TXT_HEAP_GPA - DPR_GPA)
*/

struct txt_space {
    txt_sts_t txt_status;
    txt_ests_t txt_error_status;
    txt_errorcode_t txt_errorcode;
    Bit8u txt_cmd_reset;
    Bit8u txt_cmd_close;
    Bit32u txt_tpm_supported;
    Bit64u txt_boot_status;
    txt_didvid_t txt_didvid;
    txt_ver_fsbif_qpiif_t txt_ver_fsbif;
    Bit32u txt_sinit_base;
    Bit32u txt_sinit_size;
    mle_join_t txt_mle_join;
    Bit32u txt_heap_base;
    Bit32u txt_heap_size;
    txt_dpr_t txt_dpr;
    Bit64u txt_policy_status;
    txt_e2sts_t txt_e2sts;
    mseg_hdr_t txt_mseg_hdr;
};

class BOCHSAPI bx_smx_c : public logfunctions
{
    //bx_phy_address base_addr;
    

public:
    struct txt_space txt_space[2];
    bx_smx_c(BX_CPU_C *cpu);
    bool is_selected(bx_phy_address addr);
    
    void read(bx_phy_address addr, void *data, unsigned len);
    void write(bx_phy_address addr, void *data, unsigned len);
    //static bool read_dpr(bx_phy_address addr, unsigned len, void *data, void *param);
    //static bool write_dpr(bx_phy_address addr, unsigned len, void *data, void *param);
    //bool is_dpr_selected(bx_phy_address addr);
};

#endif
#endif