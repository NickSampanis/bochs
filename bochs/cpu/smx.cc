#include "bochs.h"
#include "cpu.h"

#if BX_SUPPORT_SMX
#include "smx.h"
#include "txt_config_regs.h"
#include "iodev/iodev.h"


#define LOG_THIS genlog->

/*
static Bit8u *dpr_va;

bool bx_smx_c::read_dpr(bx_phy_address addr, unsigned len, void *data, void *param)
{
  Bit64u offset;

  if (addr + len > DPR_END)
    return false;
  offset = addr - DPR_GPA;
  memcpy(data, (void *)(dpr_va + offset), len);

  return true;
}

bool bx_smx_c::write_dpr(bx_phy_address addr, unsigned len, void *data, void *param)
{
  Bit64u offset;

  if (addr + len > DPR_END)
    return false;
  offset = addr - DPR_GPA;
  memcpy((void *)(dpr_va + offset), data, len);  

  return true;
}
*/

bx_smx_c::bx_smx_c(BX_CPU_C *cpu)
{
  Bit8u *sinit, *heap;

  memset(txt_space, 0, sizeof(txt_space));
  txt_space[TXT_AREA_PUBLIC].txt_didvid.vendor_id = 0x8086;
  txt_space[TXT_AREA_PRIVATE].txt_didvid.vendor_id = 0x8086;
  //Q35
  
  txt_space[TXT_AREA_PUBLIC].txt_didvid.device_id = 0x8001;
  txt_space[TXT_AREA_PUBLIC].txt_didvid.revision_id = 0x7;
  txt_space[TXT_AREA_PUBLIC].txt_ver_fsbif._raw = 0x9d003000;
  txt_space[TXT_AREA_PRIVATE].txt_didvid.device_id = 0x8001;
  txt_space[TXT_AREA_PRIVATE].txt_didvid.revision_id = 0x7;
  txt_space[TXT_AREA_PRIVATE].txt_ver_fsbif._raw = 0x9d003000;


  //TIGERLAKE
  /*
  txt_space[TXT_AREA_PUBLIC].txt_didvid.device_id = 0xb00c;
  txt_space[TXT_AREA_PUBLIC].txt_didvid.revision_id = 0x1;
  txt_space[TXT_AREA_PUBLIC].txt_ver_fsbif._raw = 0x9d003000;
  txt_space[TXT_AREA_PRIVATE].txt_didvid.device_id = 0xb00c;
  txt_space[TXT_AREA_PRIVATE].txt_didvid.revision_id = 0x1;
  txt_space[TXT_AREA_PRIVATE].txt_ver_fsbif._raw = 0x9d003000;
  */
  #if WIN32
    LARGE_INTEGER fileSize;
    HANDLE hFile;
    DWORD bytesRead;
    BOOL ret;

    //allocate txtheap
    //before cpu_reset, so we have to set the a20 bit, without cache invalidation
    bx_pc_system.a20_mask = BX_CONST64(0xffffffffffffffff);
    txt_space[TXT_AREA_PUBLIC].txt_dpr._raw = DPR_ENTRY;
    txt_space[TXT_AREA_PUBLIC].txt_sinit_base = SINIT_GPA;
    txt_space[TXT_AREA_PUBLIC].txt_sinit_size = SINIT_SIZE;
    txt_space[TXT_AREA_PUBLIC].txt_heap_base = TXT_HEAP_GPA;
    txt_space[TXT_AREA_PUBLIC].txt_heap_size = TXT_HEAP_SIZE;
    txt_space[TXT_AREA_PRIVATE].txt_dpr._raw = DPR_ENTRY;
    txt_space[TXT_AREA_PRIVATE].txt_sinit_base = SINIT_GPA;
    txt_space[TXT_AREA_PRIVATE].txt_sinit_size = SINIT_SIZE;
    txt_space[TXT_AREA_PRIVATE].txt_heap_base = TXT_HEAP_GPA;
    txt_space[TXT_AREA_PRIVATE].txt_heap_size = TXT_HEAP_SIZE;
    
    //load sinit
    sinit = (Bit8u *)cpu->getHostMemAddr(SINIT_GPA, BX_READ);
    if (sinit == NULL)
      BX_PANIC(("Error in getHostMemAddr(SINIT_GPA) %d", GetLastError()));
    hFile = CreateFileA("bios/acm.bin", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
      BX_PANIC(("Failed to open bios/acm.bin"));
    fileSize.QuadPart = 0;
    ret = GetFileSizeEx(hFile, &fileSize);
    if (!ret) 
      BX_PANIC(("Error in GetFileSize()"));
    if (fileSize.QuadPart > SINIT_SIZE) 
      BX_PANIC(("Error acm greater than 0x50000"));
    bytesRead = 0;
    ret = ReadFile(hFile, (void *)sinit, (DWORD)fileSize.QuadPart, (LPDWORD)&bytesRead, NULL);
    if (!ret || bytesRead != (DWORD)fileSize.QuadPart)
      BX_PANIC(("Error in ReadFile() %d", GetLastError()));
    CloseHandle(hFile);
                              
  #endif
  //DEV_register_memory_handlers(this, bx_smx_c::read_dpr, bx_smx_c::write_dpr, DPR_GPA, DPR_END - 1);

  //edk2 doesn't setup bios txt structure, so here we go
  //version 6
  bios_data_t *bios_data;
  heap_bios_spec_ver_element_t *spec_element;
  heap_acm_element_t *acm_element;
  heap_ext_data_element_t *end_element;

  //should be 0x56, (+16 is for end_element->size + the size of heap)
  heap = (Bit8u *)cpu->getHostMemAddr(TXT_HEAP_GPA, BX_READ);
    if (heap == NULL)
      BX_PANIC(("Error in getHostMemAddr(TXT_HEAP_GPA) %d", GetLastError()));
  memset(heap, 0, TXT_HEAP_SIZE);
  *(Bit64u *)(heap) = sizeof(bios_data_t) + sizeof(heap_bios_spec_ver_element_t) + sizeof(heap_acm_element_t) + 16;
  
  bios_data = (bios_data_t *)(heap + 8);
  memset(bios_data, 0, sizeof(bios_data_t));
  bios_data->version = 6;
  bios_data->num_logical_procs = 1;
  spec_element = (heap_bios_spec_ver_element_t *)(bios_data + 1);
  spec_element->hdr.type = 1;
  spec_element->hdr.size = sizeof(heap_bios_spec_ver_element_t);
  spec_element->spec_ver_major = 2;
  spec_element->spec_ver_minor = 1;
  acm_element = (heap_acm_element_t *)(spec_element + 1);
  acm_element->hdr.type = HEAP_EXTDATA_TYPE_ACM;
  acm_element->hdr.size = sizeof(heap_bios_spec_ver_element_t);
  acm_element->num_acms = 1;
  acm_element->acm_addrs = SINIT_GPA;
  end_element = (heap_ext_data_element_t *)(acm_element + 1);
  end_element->type = HEAP_EXTDATA_TYPE_END;
  end_element->size = 8;

  //todo version 4

  //ugly hack 
  /*
  BX_ACPI_THIS s.devfunc = BX_PCI_DEVICE(1, 3);
  DEV_register_pci_handlers(this, &BX_ACPI_THIS s.devfunc, BX_PLUGIN_ACPI,
                            "ACPI Controller");
  DEV_register_iowrite_handler(this, write_handler, ACPI_DBG_IO_ADDR, "ACPI", 4);
  */
}

bool bx_smx_c::is_selected(bx_phy_address addr)
{
  addr &= ~(TXT_CONFIG_REGS_SIZE - 1);
  if(addr == TXT_PUB_CONFIG_REGS_BASE || addr == TXT_PRIV_CONFIG_REGS_BASE) 
    return true;
  
  return false;
}
/*
bool bx_smx_c::is_dpr_selected(bx_phy_address addr)
{
  addr &= ~(DPR_SIZE - 1);
  if(addr == DPR_GPA) 
    return true;
  
  return false;
}
*/
void bx_smx_c::read(bx_phy_address addr, void *data, unsigned len)
{
  Bit64u offset, value;
  Bit8u area;

  offset = addr & (TXT_CONFIG_REGS_SIZE - 1);
  area = (addr & ~(TXT_CONFIG_REGS_SIZE - 1)) - TXT_PRIV_CONFIG_REGS_BASE ? TXT_AREA_PUBLIC : TXT_AREA_PRIVATE;
  value = 0;
  if ((offset & 7) + len > 8)
    return;
  switch (offset & -8) {
    case TXTCR_STS:
      value = txt_space[area].txt_status._raw;
      break;
    case TXTCR_ESTS:
      value = txt_space[area].txt_error_status._raw;
      break;
    case TXTCR_ERRORCODE:
      value = txt_space[area].txt_errorcode._raw;
      break;
    case TXTCR_SACM_STATUS:
      value = 0x40000000LL;
      break;
    case TXTCR_VER_FSBIF:
      value = txt_space[area].txt_ver_fsbif._raw;
      break;
    case TXTCR_DIDVID:
      value = txt_space[area].txt_didvid._raw;
      break;
    case TXTCR_VER_QPIIF:
      value = txt_space[area].txt_ver_fsbif._raw;
      break;
    case TXTCR_CMD_UNLOCK_MEM_CONFIG:
      break;
    case TXTCR_SINIT_BASE:
      value = txt_space[area].txt_sinit_base;
      break;
    case TXTCR_SINIT_SIZE:
      value = txt_space[area].txt_sinit_size;
      break;
    case TXTCR_MLE_JOIN:
      value = txt_space[area].txt_mle_join.gdt_limit;
      break;
    case TXTCR_MLE_JOIN + 4:
      value = txt_space[area].txt_mle_join.gdt_base;
      break;
    case TXTCR_MLE_JOIN + 8:
      value = txt_space[area].txt_mle_join.seg_sel;
      break;
    case TXTCR_MLE_JOIN + 0xc:
      value = txt_space[area].txt_mle_join.entry_point;
      break;
    case TXTCR_HEAP_BASE:
      value = txt_space[area].txt_heap_base;
      break;
    case TXTCR_HEAP_SIZE:
      value = txt_space[area].txt_heap_size;
      break;
    case TXTCR_MSEG_BASE:
      break;
    case TXTCR_MSEG_SIZE:
      break;
    case TXTCR_DPR:
      value = txt_space[area].txt_dpr._raw;
      break;
    case TXTCR_CMD_OPEN_LOCALITY1:
      break;
    case TXTCR_CMD_CLOSE_LOCALITY1:
      break;
    case TXTCR_CMD_OPEN_LOCALITY2:
      break;
    case TXTCR_CMD_CLOSE_LOCALITY2:
      break;
    case TXTCR_PUBLIC_KEY:
      break;
    case TXTCR_TPM_SUPPORTED:
      value = txt_space[area].txt_tpm_supported;
      break;
    case TXTCR_CMD_SECRETS:
      break;
    case TXTCR_CMD_NO_SECRETS:
      break;
    case TXTCR_E2STS:
      value = txt_space[area].txt_e2sts._raw;
      break;
    default:
      break;
  }
  //allows misaligned reads
  memcpy(data, ((Bit8u *)&value) + (offset & 0x7), len);
  /*
  switch (len) {
    case 1:
      *(Bit8u *)data = (Bit8u)value;
      break;
    case 2:
      *(Bit16u *)data = (Bit16u)value;
      break;
    case 4:
      *(Bit32u *)data = (Bit32u)value;
      break;
    case 8:
      *(Bit64u *)data = value;
      break;
  }
  */
}

#if 0
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


struct txt_space {
    txt_sts_t txt_status;
    txt_ests_t txt_error_status;
    txt_errorcode_t txt_errorcode;
    Bit8u txt_cmd_reset;
    Bit8u txt_cmd_close;
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
typedef struct {
    uint32_t	gdt_limit;
    uint32_t	gdt_base;
    uint32_t	seg_sel;               /* cs (ds, es, ss are seg_sel+8) */
    uint32_t	entry_point;           /* phys addr */
} mle_join_t;
#endif 

void bx_smx_c::write(bx_phy_address addr, void *data, unsigned len)
{
  Bit64u offset, value;
  Bit8u area;

  value = *(Bit64u *)data;
  offset = addr & (TXT_CONFIG_REGS_SIZE - 1);
  area = (addr & ~(TXT_CONFIG_REGS_SIZE - 1)) - TXT_PRIV_CONFIG_REGS_BASE ? TXT_AREA_PUBLIC : TXT_AREA_PRIVATE;

  switch (len) {
    case 1:
      value = *(Bit8u *)data;
      break;
    case 2:
      value = *(Bit16u *)data;
      break;
    case 4:
      value = *(Bit32u *)data;
      break;
    case 8:
      value = *(Bit64u *)data;
      break;
  }

  switch (offset) {
    case TXTCR_STS:
      txt_space[area].txt_status._raw = value;
      break;
    case TXTCR_ESTS:
      txt_space[area].txt_error_status._raw = value;
      break;
    case TXTCR_ERRORCODE:
      txt_space[area].txt_errorcode._raw = value;
      break;
    case TXTCR_CMD_RESET:
      txt_space[area].txt_cmd_reset = value;
      break;
    case TXTCR_CMD_CLOSE_PRIVATE:
      txt_space[area].txt_cmd_close = value;
      break;
    case TXTCR_SACM_STATUS:
      break;
    case TXTCR_VER_FSBIF:
      txt_space[area].txt_ver_fsbif._raw = value;
      break;
    case TXTCR_VER_QPIIF:
      txt_space[area].txt_ver_fsbif._raw = value;
      break;
    case TXTCR_CMD_UNLOCK_MEM_CONFIG:
      break;
    case TXTCR_SINIT_BASE:
      txt_space[area].txt_sinit_base = (uint32_t)value;
      break;
    case TXTCR_SINIT_SIZE:
      txt_space[area].txt_sinit_size = (uint32_t)value;
      break;
    case TXTCR_MLE_JOIN:
      txt_space[area].txt_mle_join.gdt_limit = value;
    case TXTCR_MLE_JOIN + 4:
      txt_space[area].txt_mle_join.gdt_base = value;
      break;
    case TXTCR_MLE_JOIN + 8:
      txt_space[area].txt_mle_join.seg_sel = value;
      break;
    case TXTCR_MLE_JOIN + 0xc:
      txt_space[area].txt_mle_join.entry_point = value;
      break;
    case TXTCR_HEAP_BASE:
      txt_space[area].txt_heap_base = (uint32_t)value;
      break;
    case TXTCR_HEAP_SIZE:
      txt_space[area].txt_heap_size = (uint32_t)value;
      break;
    case TXTCR_MSEG_BASE:
      break;
    case TXTCR_MSEG_SIZE:
      break;
    case TXTCR_DPR:
      txt_space[area].txt_dpr._raw = value;
      break;
    case TXTCR_CMD_OPEN_LOCALITY1:
      if (!value)
        txt_space[area].txt_status._raw = 0x8000;
      break;
    case TXTCR_CMD_CLOSE_LOCALITY1:
      if (!value)
        txt_space[area].txt_status._raw = 0;
      break;
    case TXTCR_CMD_OPEN_LOCALITY2:
      if (!value)
        txt_space[area].txt_status._raw = 0x10000;
      break;
    case TXTCR_CMD_CLOSE_LOCALITY2:
      if (!value)
        txt_space[area].txt_status._raw = 0;
      break;
    case TXTCR_PUBLIC_KEY:
      break;
    case TXTCR_TPM_SUPPORTED:
      break;
    case TXTCR_CMD_SECRETS:
      break;
    case TXTCR_CMD_NO_SECRETS:
      break;
    case TXTCR_E2STS:
      txt_space[area].txt_e2sts._raw = value;
      break;
    default:
      break;
  }
}


#endif //BX_SUPPORT_SMX