/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2006-2021  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

//
// PIIX4 ACPI support
//

// Define BX_PLUGGABLE in files that can be compiled into plugins.  For
// platforms that require a special tag on exported symbols, BX_PLUGGABLE
// is used to know when we are exporting symbols and when we are importing.
#define BX_PLUGGABLE

#include "iodev.h"

#if BX_SUPPORT_PCI

#include "pci.h"
#include "acpi.h"
//#ifdef QEMU_CFG_FW
#include "acpi_fw_cfg.h"
//#endif


#define LOG_THIS theACPIController->

bx_acpi_ctrl_c* theACPIController = NULL;

// FIXME
const Bit8u acpi_pm_iomask[64] = {3, 0, 3, 0, 3, 0, 0, 0, 4, 0, 0, 0, 3, 1, 3, 1,
                                  7, 1, 3, 1, 1, 1, 0, 0, 3, 1, 0, 0, 7, 1, 3, 1,
                                  3, 1, 0, 0, 0, 0, 0, 0, 7, 1, 3, 1, 7, 1, 3, 1,
                                  1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
const Bit8u acpi_sm_iomask[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 0, 0, 0};

#define PM_FREQ 3579545

#define ACPI_DBG_IO_ADDR  0xb044

#define RSM_STS (1 << 15)
#define PWRBTN_STS (1 << 8)

#define RTC_EN (1 << 10)
#define PWRBTN_EN (1 << 8)
#define GBL_EN (1 << 5)
#define TMROF_EN (1 << 0)

#define SCI_EN (1 << 0)

#define SUS_EN (1 << 13)

#define ACPI_ENABLE 0xf1
#define ACPI_DISABLE 0xf0

extern void apic_bus_deliver_smi(void);

PLUGIN_ENTRY_FOR_MODULE(acpi)
{
  if (mode == PLUGIN_INIT) {
    theACPIController = new bx_acpi_ctrl_c();
    bx_devices.pluginACPIController = theACPIController;
    BX_REGISTER_DEVICE_DEVMODEL(plugin, type, theACPIController, BX_PLUGIN_ACPI);
  } else if (mode == PLUGIN_FINI) {
    delete theACPIController;
  } else if (mode == PLUGIN_PROBE) {
    return (int)PLUGTYPE_STANDARD;
  }
  return 0; // Success
}

/* ported from QEMU: compute with 96 bit intermediate result: (a*b)/c */
Bit64u muldiv64(Bit64u a, Bit32u b, Bit32u c)
{
  union {
    Bit64u ll;
    struct {
#if WORDS_BIGENDIAN
      Bit32u high, low;
#else
      Bit32u low, high;
#endif
    } l;
  } u, res;
  Bit64u rl, rh;

  u.ll = a;
  rl = (Bit64u)u.l.low * (Bit64u)b;
  rh = (Bit64u)u.l.high * (Bit64u)b;
  rh += (rl >> 32);
  rl &= 0xffffffff;

  res.l.high = (Bit32u)(rh / c);
  res.l.low = (Bit32u)((((rh % c) << 32) + rl) / c);

  return res.ll;
}
//#ifdef QEMU_CFG_FW
#include "bios/acpi-dsdt.hex"
static Bit32u qemu_idx;
static QEMU_LOADER_ENTRY qemu_entry[30];
static rsdt_descriptor_rev1 *rsdt;
static rsdp_descriptor *rsdp;
static fadt_descriptor_rev1 *fadt;
static facs_descriptor_rev1 *facs;
static multiple_apic_table *madt;
static acpi_20_hpet *hpet;
static acpi_dmar *dmar;
static Bit8u *ssdt, *dsdt;
static Bit64u msr_feature_control;

Bit32u madt_size;

static void add_qemu_entry_allocation(Bit8u *signature)
{
  qemu_entry[qemu_idx].Type = QemuLoaderCmdAllocate;
  memcpy(qemu_entry[qemu_idx].Command.Allocate.File, signature, 4);
  qemu_idx++;
}

static void add_qemu_entry_pointer(Bit8u *signature_structure, Bit32u offset, Bit8u *signature_pointer)
{
  qemu_entry[qemu_idx].Type = QemuLoaderCmdAddPointer;
  memcpy(qemu_entry[qemu_idx].Command.AddPointer.PointeeFile, signature_pointer, 4);
  memcpy(qemu_entry[qemu_idx].Command.AddPointer.PointerFile, signature_structure, 4);
  qemu_entry[qemu_idx].Command.AddPointer.PointerOffset = offset;
  qemu_entry[qemu_idx].Command.AddPointer.PointerSize = 4;
  qemu_idx++;
}

static void add_qemu_entry_checksum(Bit8u *signature_structure, Bit32u start, Bit32u length, Bit32u result)
{
  qemu_entry[qemu_idx].Type = QemuLoaderCmdAddChecksum;
  memcpy(qemu_entry[qemu_idx].Command.AddChecksum.File, signature_structure, 4);
  qemu_entry[qemu_idx].Command.AddChecksum.ResultOffset = result;
  qemu_entry[qemu_idx].Command.AddChecksum.Start = start;
  qemu_entry[qemu_idx].Command.AddChecksum.Length = length;
  qemu_idx++;
}

static int acpi_checksum(Bit8u *data, int len)
{
    int sum, i;
    sum = 0;
    for(i = 0; i < len; i++)
        sum += data[i];
    return (-sum) & 0xff;
}

static void acpi_build_table_header(acpi_table_header *h,
                                    char *sig, int len, uint8_t rev)
{
    memcpy(h->signature, sig, 4);
    h->length = len;
    h->revision = rev;
    memcpy(h->oem_id, "QEMU  ", 6);
    memcpy(h->oem_table_id, "QEMU", 4);
    memcpy(h->oem_table_id + 4, sig, 4);
    h->oem_revision = 1;
    memcpy(h->asl_compiler_id, "QEMU", 4);
    h->asl_compiler_revision = 1;
    h->checksum = acpi_checksum((Bit8u *)h, len);
}

static void acpi_build_processor_ssdt()
{
    uint8_t *ssdt_ptr;
    int i, length;
    int smp_cpus = BX_SMP_PROCESSORS;
    int acpi_cpus = smp_cpus > 0xff ? 0xff : smp_cpus;

    ssdt = (Bit8u *)malloc(0x200);
    memset(ssdt, 0, 0x200);
    ssdt_ptr = (uint8_t *)ssdt;
    ssdt_ptr[9] = 0; // checksum;
    ssdt_ptr += sizeof(acpi_table_header);

    // caluculate the length of processor block and scope block excluding PkgLength
    length = 0x0d * acpi_cpus + 4;

    // build processor scope header
    *(ssdt_ptr++) = 0x10; // ScopeOp
    if (length <= 0x3e) {
        // Handle 1-4 CPUs with one byte encoding 
        *(ssdt_ptr++) = length + 1;
    } else {
        // Handle 5-314 CPUs with two byte encoding 
        *(ssdt_ptr++) = 0x40 | ((length + 2) & 0xf);
        *(ssdt_ptr++) = (length + 2) >> 4;
    }
    *(ssdt_ptr++) = '_'; // Name
    *(ssdt_ptr++) = 'P';
    *(ssdt_ptr++) = 'R';
    *(ssdt_ptr++) = '_';

    // build object for each processor
    for(i = 0; i < acpi_cpus; i++) {
        *(ssdt_ptr++) = 0x5B; // ProcessorOp
        *(ssdt_ptr++) = 0x83;
        *(ssdt_ptr++) = 0x0B; // Length
        *(ssdt_ptr++) = 'C';  // Name (CPUxx)
        *(ssdt_ptr++) = 'P';
        if ((i & 0xf0) != 0)
            *(ssdt_ptr++) = (i >> 4) < 0xa ? (i >> 4) + '0' : (i >> 4) + 'A' - 0xa;
        else
            *(ssdt_ptr++) = 'U';
        *(ssdt_ptr++) = (i & 0xf) < 0xa ? (i & 0xf) + '0' : (i & 0xf) + 'A' - 0xa;
        *(ssdt_ptr++) = i;
        *(ssdt_ptr++) = 0x10; // Processor block address
        *(ssdt_ptr++) = 0xb0;
        *(ssdt_ptr++) = 0;
        *(ssdt_ptr++) = 0;
        *(ssdt_ptr++) = 6;    // Processor block length
    }

    acpi_build_table_header((acpi_table_header *)ssdt,
                            (char *)"SSDT", (int)(ssdt_ptr - ssdt), 1);
    add_qemu_entry_allocation((Bit8u *)"ssdt");

}

static void build_hpet()
{
  hpet = (acpi_20_hpet *)malloc(QEMU_ENTRY_SIZE(sizeof(*hpet)));
  memset(hpet, 0, QEMU_ENTRY_SIZE(sizeof(*hpet)));
  hpet->timer_block_id = 0x8086a201; //SWAP_32(0x8086a201);
  hpet->addr.address = ACPI_HPET_ADDRESS;
  acpi_build_table_header((acpi_table_header *)hpet,
                             (char *)"HPET", sizeof(*hpet), 1);
  add_qemu_entry_allocation((Bit8u *)"hpet");
                        
}
#if BX_SUPPORT_VTD

static void build_dmar()
{
  //acpi_dmar
  //dmar_remapping

  dmar = (acpi_dmar *)malloc(QEMU_ENTRY_SIZE(sizeof(*dmar)));
  memset(dmar, 0, sizeof(*dmar));
  dmar->host_address_width = 36;
  dmar->flags = DMAR_INTR_REMAP;
  
  dmar->table_offsets[0].type = DMAR_REMAPPING_DRHD;
  dmar->table_offsets[0].flags = 0;
  dmar->table_offsets[0].segment_number = 0;
  dmar->table_offsets[0].register_base_address = Q35_HOST_BRIDGE_IOMMU_ADDR;

  dmar->table_offsets[0].device_scope_entry[0].type = 3; //IOAPIC
  dmar->table_offsets[0].device_scope_entry[0].length = 8;
  dmar->table_offsets[0].device_scope_entry[0].enumeration_id = 0;
  dmar->table_offsets[0].device_scope_entry[0].start_bus_number = 0xff; //maybe not
  dmar->table_offsets[0].device_scope_entry[0].path[0] = 0;

  dmar->table_offsets[0].device_scope_entry[1].type = 2;
  dmar->table_offsets[0].device_scope_entry[1].length = 8;
  dmar->table_offsets[0].device_scope_entry[1].enumeration_id = 0;
  dmar->table_offsets[0].device_scope_entry[1].start_bus_number = 0; //maybe not
  dmar->table_offsets[0].device_scope_entry[1].path[0] = 0;
  
  //TODO add ATS, RMMR?
  acpi_build_table_header((acpi_table_header *)dmar, (char *)"DMAR", sizeof(*dmar), 1);
  add_qemu_entry_allocation((Bit8u *)"dmar");

}
#endif

static void build_madt()
{
  Bit32u smp_cpus;
  madt_processor_apic *apic;
  madt_io_apic *io_apic;
  madt_int_override *int_override;

  smp_cpus = BX_SMP_PROCESSORS;
  madt_size = sizeof(*madt) + sizeof(madt_processor_apic) * smp_cpus +
        sizeof(madt_io_apic) + sizeof(madt_int_override);
  madt = (multiple_apic_table *)malloc(QEMU_ENTRY_SIZE(madt_size));
  memset(madt, 0, QEMU_ENTRY_SIZE(madt_size));
  madt->local_apic_address = 0xfee00000; //SWAP_32(0xfee00000);
  madt->flags = 1;// SWAP_32(1);
  apic = (madt_processor_apic *)(madt + 1);

  for (Bit32u i = 0; i < smp_cpus; i++) {
    apic->type = APIC_PROCESSOR;
    apic->length = sizeof(*apic);
    apic->processor_id = i;
    apic->local_apic_id = i;
    apic->flags = 1; //SWAP_32(1);
    apic++;
  }
        
  io_apic = (madt_io_apic *)apic;
  io_apic->type = APIC_IO;
  io_apic->length = sizeof(*io_apic);
  io_apic->io_apic_id = smp_cpus;
  io_apic->address = 0xfec00000; //SWAP_32(0xfec00000);
  io_apic->interrupt = SWAP_32(0);
  io_apic++;

  int_override = (madt_int_override *)io_apic;
  int_override->type = APIC_XRUPT_OVERRIDE;
  int_override->length = sizeof(*int_override);
  int_override->bus = SWAP_32(0);
  int_override->source = SWAP_32(0);
  int_override->gsi = 2; //SWAP_32(2);
  int_override->flags = SWAP_32(0);

  acpi_build_table_header((acpi_table_header *)madt, (char *)"APIC", madt_size, 1);
  add_qemu_entry_allocation((Bit8u *)"madt");
}

static void build_facs()
{
  // FACS 
  facs = (facs_descriptor_rev1 *)malloc(QEMU_ENTRY_SIZE(sizeof(*facs)));
  memset(facs, 0, sizeof(*facs));
  memcpy(facs->signature, "FACS", 4);
  facs->length = sizeof(*facs);

  //BX_INFO("Firmware waking vector %p\n", &facs->firmware_waking_vector);
  add_qemu_entry_allocation((Bit8u *)"facs");
}

static void build_fadt()
{
  //amlcode
  add_qemu_entry_allocation((Bit8u *)"dsdt");
  dsdt = (Bit8u *)malloc(QEMU_ENTRY_SIZE(sizeof(AmlCode)));
  memset(dsdt, 0, QEMU_ENTRY_SIZE(sizeof(AmlCode)));
  memcpy(dsdt, AmlCode, sizeof(AmlCode));
  add_qemu_entry_checksum((Bit8u *)"dsdt", 0, sizeof(AmlCode), offsetof(fadt_descriptor_rev1,checksum));
  *(dsdt + 9) = 0; 
  fadt = (fadt_descriptor_rev1 *)malloc(QEMU_ENTRY_SIZE(sizeof(*fadt)));
  memset(fadt, 0, QEMU_ENTRY_SIZE(sizeof(*fadt)));
  //fadt->firmware_ctrl = cpu_to_le32(facs_addr);
  //fadt->dsdt = cpu_to_le32(dsdt_addr);
  fadt->model = 1;
  fadt->reserved1 = 0;
  fadt->sci_int = 0x9; //SWAP_16(9);
  fadt->smi_cmd = 0xb2; //SWAP_32(0xb2);
  fadt->acpi_enable = 0xf1;
  fadt->acpi_disable = 0xf0;
  fadt->pm1a_evt_blk = 0xb000;//SWAP_32(0xb000);
  fadt->pm1a_cnt_blk = 0xb004;//SWAP_32(0xb000 + 0x04);
  fadt->pm_tmr_blk = 0xb008;//SWAP_32(0xb000 + 0x08);
  fadt->gpe0_blk = 0xafe0; //SWAP_32(0xafe0);
  fadt->pm1_evt_len = 4;
  fadt->pm1_cnt_len = 2;
  fadt->pm_tmr_len = 4;
  fadt->gpe0_blk_len = 4;
  fadt->plvl2_lat = 0xfff; //SWAP_16(0xfff); // C2 state not supported
  fadt->plvl3_lat = 0xfff; //SWAP_16(0xfff); // C3 state not supported
  // WBINVD + PROC_C1 + PWR_BUTTON + SLP_BUTTON + FIX_RTC 
  fadt->flags = (1 << 0) | (1 << 2) | (1 << 4) | (1 << 5) | (1 << 6); //SWAP_32((1 << 0) | (1 << 2) | (1 << 4) | (1 << 5) | (1 << 6));
  acpi_build_table_header((acpi_table_header *)fadt, (char *)"FACP",  sizeof(*fadt), 1);
  fadt->checksum = 0;
  add_qemu_entry_allocation((Bit8u *)"fadt");
  add_qemu_entry_pointer((Bit8u *)"fadt", offsetof(fadt_descriptor_rev1, firmware_ctrl), (Bit8u *)"facs");
  add_qemu_entry_pointer((Bit8u *)"fadt", offsetof(fadt_descriptor_rev1, dsdt), (Bit8u *)"dsdt");
  add_qemu_entry_checksum((Bit8u *)"fadt", 0, sizeof(fadt_descriptor_rev1), offsetof(fadt_descriptor_rev1,checksum));
}


static void build_rsdt() 
{
  rsdt = (rsdt_descriptor_rev1 *)malloc(QEMU_ENTRY_SIZE(sizeof(*rsdt)));  
  memset(rsdt, 0, QEMU_ENTRY_SIZE(sizeof(*rsdt)));
  memcpy(rsdt->signature, "RSDT", 4);
  memcpy(rsdt->oem_id, "QEMU  ", 6);
  rsdt->length = sizeof(acpi_table_header);
  //rsdt->checksum = acpi_checksum((Bit8u *)rsdt, sizeof(acpi_table_header));
  rsdt->checksum = 0;
  /*
  rsdt.table_offset_entry[0] = fadt_addr;
  rsdt.table_offset_entry[1] = madt_addr;
  rsdt.table_offset_entry[2] = ssdt_addr;
  rsdt.table_offset_entry[3] = hpet_addr;
  */
  acpi_build_table_header((acpi_table_header *)rsdt, (char *)"RSDT", sizeof(*rsdt), 1);
  add_qemu_entry_allocation((Bit8u *)"rsdt");
  add_qemu_entry_pointer((Bit8u *)"rsdt", offsetof(rsdt_descriptor_rev1, table_offset_entry[0]), (Bit8u *)"fadt");
  add_qemu_entry_pointer((Bit8u *)"rsdt", offsetof(rsdt_descriptor_rev1, table_offset_entry[1]), (Bit8u *)"madt");
  add_qemu_entry_pointer((Bit8u *)"rsdt", offsetof(rsdt_descriptor_rev1, table_offset_entry[2]), (Bit8u *)"ssdt");
  add_qemu_entry_pointer((Bit8u *)"rsdt", offsetof(rsdt_descriptor_rev1, table_offset_entry[3]), (Bit8u *)"hpet");
  add_qemu_entry_pointer((Bit8u *)"rsdt", offsetof(rsdt_descriptor_rev1, table_offset_entry[4]), (Bit8u *)"dmar");
  add_qemu_entry_checksum((Bit8u *)"rsdt", 0, sizeof(rsdt_descriptor_rev1), offsetof(rsdt_descriptor_rev1,checksum));


}

static void build_rsdp() 
{
  rsdp = (rsdp_descriptor *)malloc(QEMU_ENTRY_SIZE(sizeof(*rsdp)));  
  memset(rsdp, 0, QEMU_ENTRY_SIZE(sizeof(*rsdp)));
  memcpy(rsdp->signature, "RSD ", 4);
  memcpy(rsdp->signature+4, "PTR ", 4);
  memcpy(rsdp->oem_id, "QEMU  ", 6);
  rsdp->rsdt_physical_address = 0;
  rsdp->revision = 0;
  rsdp->checksum = acpi_checksum((Bit8u *)rsdp, 20);
  add_qemu_entry_allocation((Bit8u *)"rsdp");
  add_qemu_entry_pointer((Bit8u *)"rsdp", offsetof(rsdp_descriptor, rsdt_physical_address), (Bit8u *)"rsdt");

}
//#endif
bx_acpi_ctrl_c::bx_acpi_ctrl_c()
{
  put("ACPI");
  memset(&s, 0, sizeof(s));
  s.timer_index = BX_NULL_TIMER_HANDLE;
}

bx_acpi_ctrl_c::~bx_acpi_ctrl_c()
{
  SIM->get_bochs_root()->remove("acpi");
  BX_DEBUG(("Exit"));
}

void bx_acpi_ctrl_c::init(void)
{
  // called once when bochs initializes

  Bit8u chipset = SIM->get_param_enum(BXPN_PCI_CHIPSET)->get();
  if (chipset == BX_PCI_CHIPSET_I440BX) {
    BX_ACPI_THIS s.devfunc = BX_PCI_DEVICE(7, 3);
  } else {
    BX_ACPI_THIS s.devfunc = BX_PCI_DEVICE(1, 3);
  }
  DEV_register_pci_handlers(this, &BX_ACPI_THIS s.devfunc, BX_PLUGIN_ACPI,
                            "ACPI Controller");

  if (BX_ACPI_THIS s.timer_index == BX_NULL_TIMER_HANDLE) {
    BX_ACPI_THIS s.timer_index =
      DEV_register_timer(this, timer_handler, 1000, 0, 0, "ACPI");
  }
  DEV_register_iowrite_handler(this, write_handler, ACPI_DBG_IO_ADDR, "ACPI", 4);
//#ifdef QEMU_CFG_FW
  DEV_register_iowrite_handler_range(this, write_handler, 0x510, 0x514,  "ACPI", 7);
  DEV_register_ioread_handler_range(this, read_handler, 0x510, 0x514,  "ACPI", 7);
  acpi_build_processor_ssdt();
  build_hpet();
  build_madt();
#if BX_SUPPORT_VTD
  build_dmar();
#endif
  build_facs();
  build_fadt();
  build_rsdt();
  build_rsdp();
  msr_feature_control = (1 << 0) | (1 << 2) | (1 << 15) | 0x7f00; // BX_IA32_FEATURE_CONTROL_LOCK_BIT | BX_IA32_FEATURE_CONTROL_VMX_ENABLE_BIT | IA32_FEATURE_CONTROL_MSR_ENABLE_SENTER | IA32_FEATURE_CONTROL_MSR_SENTER_PARAM_CTL
//#endif
  BX_ACPI_THIS s.pm_base = 0x0;
  BX_ACPI_THIS s.sm_base = 0x0;

  // initialize readonly registers
  init_pci_conf(0x8086, 0x7113, 0x03, 0x068000, 0x00, BX_PCI_INTA);
}

void bx_acpi_ctrl_c::reset(unsigned type)
{
  unsigned i;

  BX_ACPI_THIS pci_conf[0x04] = 0x00; // command_io + command_mem
  BX_ACPI_THIS pci_conf[0x05] = 0x00;
  BX_ACPI_THIS pci_conf[0x06] = 0x80; // status_devsel_medium
  BX_ACPI_THIS pci_conf[0x07] = 0x02;
  BX_ACPI_THIS pci_conf[0x3c] = 0x00; // IRQ

  // PM base 0x40 - 0x43
  BX_ACPI_THIS pci_conf[0x40] = 0x01;
  BX_ACPI_THIS pci_conf[0x41] = 0x00;
  BX_ACPI_THIS pci_conf[0x42] = 0x00;
  BX_ACPI_THIS pci_conf[0x43] = 0x00;

  // clear DEVACTB register on PIIX4 ACPI reset
  BX_ACPI_THIS pci_conf[0x58] = 0x00;
  BX_ACPI_THIS pci_conf[0x59] = 0x00;

  // device resources
  BX_ACPI_THIS pci_conf[0x5a] = 0x00;
  BX_ACPI_THIS pci_conf[0x5b] = 0x00;
  BX_ACPI_THIS pci_conf[0x5f] = 0x90;
  BX_ACPI_THIS pci_conf[0x63] = 0x60;
  BX_ACPI_THIS pci_conf[0x67] = 0x98;

  // SM base 0x90 - 0x93
  BX_ACPI_THIS pci_conf[0x90] = 0x01;
  BX_ACPI_THIS pci_conf[0x91] = 0x00;
  BX_ACPI_THIS pci_conf[0x92] = 0x00;
  BX_ACPI_THIS pci_conf[0x93] = 0x00;

  BX_ACPI_THIS s.pmsts = 0;
  BX_ACPI_THIS s.pmen = 0;
  BX_ACPI_THIS s.pmcntrl = 0;
  BX_ACPI_THIS s.tmr_overflow_time = 0xffffff;
  for (i = 0; i < 0x38; i++) {
    BX_ACPI_THIS s.pmreg[i] = 0;
  }

  BX_ACPI_THIS s.smbus.stat = 0;
  BX_ACPI_THIS s.smbus.ctl = 0;
  BX_ACPI_THIS s.smbus.cmd = 0;
  BX_ACPI_THIS s.smbus.addr = 0;
  BX_ACPI_THIS s.smbus.data0 = 0;
  BX_ACPI_THIS s.smbus.data1 = 0;
  BX_ACPI_THIS s.smbus.index = 0;

  for (i = 0; i < 32; i++) {
    BX_ACPI_THIS s.smbus.data[i] = 0;
  }
}

void bx_acpi_ctrl_c::register_state(void)
{
  bx_list_c *list = new bx_list_c(SIM->get_bochs_root(), "acpi", "ACPI Controller State");
  BXRS_HEX_PARAM_FIELD(list, pmsts, BX_ACPI_THIS s.pmsts);
  BXRS_HEX_PARAM_FIELD(list, pmen, BX_ACPI_THIS s.pmen);
  BXRS_HEX_PARAM_FIELD(list, pmcntrl, BX_ACPI_THIS s.pmcntrl);
  BXRS_HEX_PARAM_FIELD(list, tmr_overflow_time, BX_ACPI_THIS s.tmr_overflow_time);
  new bx_shadow_data_c(list, "pmreg", BX_ACPI_THIS s.pmreg, 0x38, 1);
  bx_list_c *smbus = new bx_list_c(list, "smbus", "ACPI SMBus");
  BXRS_HEX_PARAM_FIELD(smbus, stat, BX_ACPI_THIS s.smbus.stat);
  BXRS_HEX_PARAM_FIELD(smbus, ctl, BX_ACPI_THIS s.smbus.ctl);
  BXRS_HEX_PARAM_FIELD(smbus, cmd, BX_ACPI_THIS s.smbus.cmd);
  BXRS_HEX_PARAM_FIELD(smbus, addr, BX_ACPI_THIS s.smbus.addr);
  BXRS_HEX_PARAM_FIELD(smbus, data0, BX_ACPI_THIS s.smbus.data0);
  BXRS_HEX_PARAM_FIELD(smbus, data1, BX_ACPI_THIS s.smbus.data1);
  BXRS_HEX_PARAM_FIELD(smbus, index, BX_ACPI_THIS s.smbus.index);
  new bx_shadow_data_c(smbus, "data", BX_ACPI_THIS s.smbus.data, 32, 1);
  register_pci_state(list);
}

void bx_acpi_ctrl_c::after_restore_state(void)
{
  if (DEV_pci_set_base_io(BX_ACPI_THIS_PTR, read_handler, write_handler,
                         &BX_ACPI_THIS s.pm_base,
                         &BX_ACPI_THIS pci_conf[0x40],
                         64, &acpi_pm_iomask[0], "ACPI PM base"))
  {
     BX_INFO(("new PM base address: 0x%04x", BX_ACPI_THIS s.pm_base));
  }
  if (DEV_pci_set_base_io(BX_ACPI_THIS_PTR, read_handler, write_handler,
                         &BX_ACPI_THIS s.sm_base,
                         &BX_ACPI_THIS pci_conf[0x90],
                         16, &acpi_sm_iomask[0], "ACPI SM base"))
  {
     BX_INFO(("new SM base address: 0x%04x", BX_ACPI_THIS s.sm_base));
  }
}

void bx_acpi_ctrl_c::set_irq_level(bool level)
{
  DEV_pci_set_irq(BX_ACPI_THIS s.devfunc, BX_ACPI_THIS pci_conf[0x3d], level);
}

Bit32u bx_acpi_ctrl_c::get_pmtmr(void)
{
  Bit64u value = muldiv64(bx_pc_system.time_usec(), PM_FREQ, 1000000);
  return (Bit32u)(value & 0xffffff);
}

Bit16u bx_acpi_ctrl_c::get_pmsts(void)
{
  Bit16u pmsts = BX_ACPI_THIS s.pmsts;
  Bit64u value = muldiv64(bx_pc_system.time_usec(), PM_FREQ, 1000000);
  if (value >= BX_ACPI_THIS s.tmr_overflow_time)
    BX_ACPI_THIS s.pmsts |= TMROF_EN;
  return pmsts;
}

void bx_acpi_ctrl_c::pm_update_sci(void)
{
  Bit16u pmsts = get_pmsts();
  bool sci_level = (((pmsts & BX_ACPI_THIS s.pmen) &
                    (RTC_EN | PWRBTN_EN | GBL_EN | TMROF_EN)) != 0);
  BX_ACPI_THIS set_irq_level(sci_level);
  // schedule a timer interruption if needed
  if ((BX_ACPI_THIS s.pmen & TMROF_EN) && !(pmsts & TMROF_EN)) {
    Bit64u expire_time = muldiv64(BX_ACPI_THIS s.tmr_overflow_time, 1000000, PM_FREQ);
      bx_pc_system.activate_timer(BX_ACPI_THIS s.timer_index, (Bit32u)expire_time, 0);
    } else {
      bx_pc_system.deactivate_timer(BX_ACPI_THIS s.timer_index);
    }
}

void bx_acpi_ctrl_c::generate_smi(Bit8u value)
{
  /* ACPI specs 3.0, 4.7.2.5 */
  if (value == ACPI_ENABLE) {
    BX_ACPI_THIS s.pmcntrl |= SCI_EN;
  } else if (value == ACPI_DISABLE) {
    BX_ACPI_THIS s.pmcntrl &= ~SCI_EN;
  }

  if (BX_ACPI_THIS pci_conf[0x5b] & 0x02) {
    apic_bus_deliver_smi();
  }
}

// static IO port read callback handler
// redirects to non-static class handler to avoid virtual functions

Bit32u bx_acpi_ctrl_c::read_handler(void *this_ptr, Bit32u address, unsigned io_len)
{
#if !BX_USE_ACPI_SMF
  bx_acpi_ctrl_c *class_ptr = (bx_acpi_ctrl_c *) this_ptr;
  return class_ptr->read(address, io_len);
}

Bit32u bx_acpi_ctrl_c::read(Bit32u address, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_ACPI_SMF
  Bit8u reg = address & 0x3f;
  Bit32u value = 0xffffffff;

  if ((address & 0xffc0) == BX_ACPI_THIS s.pm_base) {
    if ((BX_ACPI_THIS pci_conf[0x80] & 0x01) == 0) {
      return value;
    }
    switch (reg) {
      case 0x00:
        value = BX_ACPI_THIS get_pmsts();
        break;
      case 0x02:
        value = BX_ACPI_THIS s.pmen;
        break;
      case 0x04:
        value = BX_ACPI_THIS s.pmcntrl;
        break;
      case 0x08:
        value = BX_ACPI_THIS get_pmtmr();
        break;
      default:
        value = BX_ACPI_THIS s.pmreg[reg];
        if (io_len >= 2) {
          value |= (BX_ACPI_THIS s.pmreg[reg + 1] << 8);
        }
        if (io_len == 4) {
          value |= (BX_ACPI_THIS s.pmreg[reg + 2] << 16);
          value |= (BX_ACPI_THIS s.pmreg[reg + 3] << 24);
        }
    }
    BX_DEBUG(("read from PM register 0x%02x returns 0x%08x (len=%d)", reg, value, io_len));
  } 
  //#ifdef QEMU_CFG_FW
  else if ((address & 0xfff0) == 0x510) {
    Bit8u cfg_data_signature[] = { 'Q', 'E', 'M', 'U'};
    Bit32u cfg_data_revision = SWAP_32(0x1);
    Bit32u cfg_data_kernel_size = SWAP_32(0);
    Bit16u cfg_data_nb_cpus = SWAP_16(0);
    Bit16u cfg_data_boot_menu = SWAP_16(0);
    //TODO ADD TPM2, VTD
    struct fw_cfg_files cfg_data_files[] = {
  #if BX_SUPPORT_VTD
      SWAP_32(11), //number of files- add dmar
  #else
      SWAP_32(10), //number of files
  #endif
      SWAP_32(sizeof(QEMU_LOADER_ENTRY) * qemu_idx), SWAP_16(0x40), 0, {"etc/table-loader"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(rsdp_descriptor))), SWAP_16(RSDP_FILE_ID), 0, {"rsdp"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(rsdt_descriptor_rev1))), SWAP_16(RSDT_FILE_ID), 0, {"rsdt"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(fadt_descriptor_rev1))), SWAP_16(FADT_FILE_ID), 0, {"fadt"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(facs_descriptor_rev1))), SWAP_16(FACS_FILE_ID), 0, {"facs"},
      SWAP_32(QEMU_ENTRY_SIZE(madt_size)), SWAP_16(MADT_FILE_ID), 0, {"madt"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(acpi_20_hpet))), SWAP_16(HPET_FILE_ID), 0, {"hpet"},
      SWAP_32(sizeof(QEMU_LOADER_ENTRY) * 3), SWAP_16(SSDT_FILE_ID), 0, {"ssdt"},
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(AmlCode))), SWAP_16(DSDT_FILE_ID), 0, {"dsdt"},
#if BX_SUPPORT_VTD
      SWAP_32(QEMU_ENTRY_SIZE(sizeof(acpi_dmar))), SWAP_16(DMAR_FILE_ID), 0, {"dmar"},
#endif
      SWAP_32(8), SWAP_16(MSR_FILE_ID), 0, {"etc/msr_feature_control"}
    };
    switch (reg) {
      case 0x10:
        value = BX_ACPI_THIS s.cfg_selector;
        break;
      case 0x11:
        switch (BX_ACPI_THIS s.cfg_selector) {
          case FW_CFG_SIGNATURE:
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_signature))
              break;
            value = cfg_data_signature[BX_ACPI_THIS s.cfg_state++];
            break;
          case FW_CFG_ID:
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_revision))
              break;
            value = *((Bit8u *)&cfg_data_revision + BX_ACPI_THIS s.cfg_state++);
            break;
          /*
          case FW_CFG_UUID: 
            break;
          case FW_CFG_RAM_SIZE: 
            break;
          case FW_CFG_NOGRAPHIC: 
            break;
          */
          case FW_CFG_NB_CPUS: 
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_nb_cpus))
              break;
            value = *((Bit8u *)&cfg_data_nb_cpus + BX_ACPI_THIS s.cfg_state++);
            break;
          /*
          case FW_CFG_MACHINE_ID: 
            break;
          case FW_CFG_KERNEL_ADDR: 
            break;
          */
          case FW_CFG_KERNEL_SIZE: 
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_kernel_size))
              break;
            value = *((Bit8u *)&cfg_data_kernel_size + BX_ACPI_THIS s.cfg_state++); 
            break;
          /*
          case FW_CFG_KERNEL_CMDLINE: 
            break;
          case FW_CFG_INITRD_ADDR: 
            break;
          */
          case FW_CFG_INITRD_SIZE:
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_kernel_size))
              break;
            value = *((Bit8u *)&cfg_data_kernel_size + BX_ACPI_THIS s.cfg_state++); 
            break;
          /*
          case FW_CFG_BOOT_DEVICE: 
            break;
          case FW_CFG_NUMA: 
            break;
          */
          case FW_CFG_BOOT_MENU: 
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_boot_menu))
              break;
            value = *((Bit8u *)&cfg_data_boot_menu + BX_ACPI_THIS s.cfg_state++);             
            break;
          /*
          case FW_CFG_MAX_CPUS: 
            break;
          case FW_CFG_KERNEL_ENTRY: 
            break;
          case FW_CFG_KERNEL_DATA: 
            break;
          case FW_CFG_INITRD_DATA: 
            break;
          case FW_CFG_CMDLINE_ADDR: 
            break;
          */
          case FW_CFG_CMDLINE_SIZE:
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_kernel_size))
              break;
            value = *((Bit8u *)&cfg_data_kernel_size + BX_ACPI_THIS s.cfg_state++); 
            break;
          /*
          case FW_CFG_CMDLINE_DATA: 
            break;
          case FW_CFG_SETUP_ADDR: 
            break;
          */
          case FW_CFG_SETUP_SIZE: 
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_kernel_size))
              break;
            value = *((Bit8u *)&cfg_data_kernel_size + BX_ACPI_THIS s.cfg_state++); 
            break;
          /*
          case FW_CFG_SETUP_DATA: 
            break;
          */
          case FW_CFG_FILE_DIR:
            if (BX_ACPI_THIS s.cfg_state > sizeof(cfg_data_files))
              break;
            value = *((Bit8u *)cfg_data_files + BX_ACPI_THIS s.cfg_state++); 
            break;
          /*
          case FW_CFG_FILE_FIRST: 
            break;
          */
          case TABLE_FILE_ID:
            value = *((Bit8u *)&qemu_entry + BX_ACPI_THIS s.cfg_state++);
            break;
            
          case RSDP_FILE_ID:
            /*
            memset(buf, 0, sizeof(buf));
            build_rsdp(buf);
            */
            value = *((Bit8u *)rsdp + BX_ACPI_THIS s.cfg_state++);
            break;
          case RSDT_FILE_ID:
            /*
            memset(buf, 0, sizeof(buf));
            build_rsdt(buf);
            */
            value = *((Bit8u *)rsdt + BX_ACPI_THIS s.cfg_state++);
            break;
          case FADT_FILE_ID:
            value = *((Bit8u *)fadt + BX_ACPI_THIS s.cfg_state++);
            break;
          case FACS_FILE_ID:
            value = *((Bit8u *)facs + BX_ACPI_THIS s.cfg_state++);
            break;
          case MADT_FILE_ID:
            value = *((Bit8u *)madt + BX_ACPI_THIS s.cfg_state++);
            break;
          case HPET_FILE_ID:
            value = *((Bit8u *)hpet + BX_ACPI_THIS s.cfg_state++);
            break;
          case DSDT_FILE_ID:
            value = *((Bit8u *)dsdt + BX_ACPI_THIS s.cfg_state++);
            break;
          case SSDT_FILE_ID:
            value = *((Bit8u *)ssdt + BX_ACPI_THIS s.cfg_state++);
            break;
          case DMAR_FILE_ID:
            value = *((Bit8u *)dmar + BX_ACPI_THIS s.cfg_state++);
            break;
          case MSR_FILE_ID:
            value = *((Bit8u *)&msr_feature_control + BX_ACPI_THIS s.cfg_state++);
            break;
          default:
            BX_PANIC(("bx_acpi_ctrl_c::read BX_ACPI_THIS s.cfg_selector 0x", BX_ACPI_THIS s.cfg_selector));

        }
        break;
      case 0x14:
        break;      
    }
  }
//#endif 

  else {
    if (((BX_ACPI_THIS pci_conf[0x04] & 0x01) == 0) &&
        ((BX_ACPI_THIS pci_conf[0xd2] & 0x01) == 0)) {
      return value;
    }
    switch (reg) {
      case 0x00:
        value = BX_ACPI_THIS s.smbus.stat;
        break;
      case 0x02:
        BX_ACPI_THIS s.smbus.index = 0;
        value = BX_ACPI_THIS s.smbus.ctl & 0x1f;
        break;
      case 0x03:
        value = BX_ACPI_THIS s.smbus.cmd;
        break;
      case 0x04:
        value = BX_ACPI_THIS s.smbus.addr;
        break;
      case 0x05:
        value = BX_ACPI_THIS s.smbus.data0;
        break;
      case 0x06:
        value = BX_ACPI_THIS s.smbus.data1;
        break;
      case 0x07:
        value = BX_ACPI_THIS s.smbus.data[BX_ACPI_THIS s.smbus.index++];
        if (BX_ACPI_THIS s.smbus.index > 31) {
          BX_ACPI_THIS s.smbus.index = 0;
        }
        break;
      default:
        value = 0;
        BX_INFO(("read from SMBus register 0x%02x not implemented yet", reg));
    }
    BX_DEBUG(("read from SMBus register 0x%02x returns 0x%08x", reg, value));
  }
  return value;
}

// static IO port write callback handler
// redirects to non-static class handler to avoid virtual functions

void bx_acpi_ctrl_c::write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len)
{
#if !BX_USE_ACPI_SMF
  bx_acpi_ctrl_c *class_ptr = (bx_acpi_ctrl_c *) this_ptr;
  class_ptr->write(address, value, io_len);
}

void bx_acpi_ctrl_c::write(Bit32u address, Bit32u value, unsigned io_len)
{
#else
  UNUSED(this_ptr);
#endif // !BX_USE_ACPI_SMF
  Bit8u reg = address & 0x3f;

  if ((address & 0xffc0) == BX_ACPI_THIS s.pm_base) {
    if ((BX_ACPI_THIS pci_conf[0x80] & 0x01) == 0) {
      return;
    }
    BX_DEBUG(("write to PM register 0x%02x, value = 0x%08x (len=%d)", reg, value, io_len));
    switch (reg) {
      case 0x00:
        {
          Bit16u pmsts = BX_ACPI_THIS get_pmsts();
          if (pmsts & value & TMROF_EN) {
            // if TMRSTS is reset, then compute the new overflow time
            Bit64u d = muldiv64(bx_pc_system.time_usec(), PM_FREQ, 1000000);
            BX_ACPI_THIS s.tmr_overflow_time = (d + BX_CONST64(0x800000)) & ~BX_CONST64(0x7fffff);
          }
          BX_ACPI_THIS s.pmsts &= ~value;
          BX_ACPI_THIS pm_update_sci();
        }
        break;
      case 0x02:
        BX_ACPI_THIS s.pmen = value;
        BX_ACPI_THIS pm_update_sci();
        break;
      case 0x04:
        {
          BX_ACPI_THIS s.pmcntrl = value & ~(SUS_EN);
          if (value & SUS_EN) {
            // change suspend type
            Bit16u sus_typ = (value >> 10) & 7;
            switch (sus_typ) {
              case 0: // soft power off
                bx_user_quit = 1;
                BX_FATAL(("ACPI control: soft power off"));
                break;
              case 1:
                BX_INFO(("ACPI control: suspend to ram"));
                BX_ACPI_THIS s.pmsts |= (RSM_STS | PWRBTN_STS);
                DEV_cmos_set_reg(0xF, 0xFE);
                bx_pc_system.Reset(BX_RESET_HARDWARE);
                break;
              default:
                break;
            }
          }
        }
        break;
      case 0x0c: // GPSTS
      case 0x0d: // GPSTS
      case 0x14: // PLVL2
      case 0x15: // PLVL3
      case 0x18: // GLBSTS
      case 0x19: // GLBSTS
      case 0x1c: // DEVSTS
      case 0x1d: // DEVSTS
      case 0x1e: // DEVSTS
      case 0x1f: // DEVSTS
      case 0x30: // GPIREG
      case 0x31: // GPIREG
      case 0x32: // GPIREG
        break;
      default:
        BX_ACPI_THIS s.pmreg[reg] = value;
        if (io_len >= 2) {
          BX_ACPI_THIS s.pmreg[reg + 1] = (value >> 8);
        }
        if (io_len == 4) {
          BX_ACPI_THIS s.pmreg[reg + 2] = (value >> 16);
          BX_ACPI_THIS s.pmreg[reg + 3] = (value >> 24);
        }
    }
  } else if ((address & 0xfff0) == BX_ACPI_THIS s.sm_base) {
    if (((BX_ACPI_THIS pci_conf[0x04] & 0x01) == 0) &&
        ((BX_ACPI_THIS pci_conf[0xd2] & 0x01) == 0)) {
      return;
    }
    BX_DEBUG(("write to SMBus register 0x%02x, value = 0x%04x", reg, value));
    switch (reg) {
      case 0x00:
        BX_ACPI_THIS s.smbus.stat = 0;
        BX_ACPI_THIS s.smbus.index = 0;
        break;
      case 0x02:
        BX_ACPI_THIS s.smbus.ctl = 0;
        // TODO: execute SMBus command
        break;
      case 0x03:
        BX_ACPI_THIS s.smbus.cmd = 0;
        break;
      case 0x04:
        BX_ACPI_THIS s.smbus.addr = 0;
        break;
      case 0x05:
        BX_ACPI_THIS s.smbus.data0 = 0;
        break;
      case 0x06:
        BX_ACPI_THIS s.smbus.data1 = 0;
        break;
      case 0x07:
        BX_ACPI_THIS s.smbus.data[BX_ACPI_THIS s.smbus.index++] = value;
        if (BX_ACPI_THIS s.smbus.index > 31) {
          BX_ACPI_THIS s.smbus.index = 0;
        }
        break;
      default:
        BX_INFO(("write to SMBus register 0x%02x not implemented yet", reg));
    }
  } 
//#ifdef QEMU_CFG_FW
  else if ((address & 0xfff0) == 0x510) {
    switch (reg) {
      case 0x10:
        if (reg > FW_CFG_FILE_FIRST)
          break;
        BX_ACPI_THIS s.cfg_state = 0;
        BX_ACPI_THIS s.cfg_selector = value;
        break;
      case 0x11:
        switch (BX_ACPI_THIS s.cfg_selector) {
          case FW_CFG_SIGNATURE: 
            break;
          case FW_CFG_ID: 
            break;
          case FW_CFG_UUID: 
            break;
          case FW_CFG_RAM_SIZE: 
            break;
          case FW_CFG_NOGRAPHIC: 
            break;
          case FW_CFG_NB_CPUS: 
            break;
          case FW_CFG_MACHINE_ID: 
            break;
          case FW_CFG_KERNEL_ADDR: 
            break;
          case FW_CFG_KERNEL_SIZE: 
            break;
          case FW_CFG_KERNEL_CMDLINE: 
            break;
          case FW_CFG_INITRD_ADDR: 
            break;
          case FW_CFG_INITRD_SIZE: 
            break;
          case FW_CFG_BOOT_DEVICE: 
            break;
          case FW_CFG_NUMA: 
            break;
          case FW_CFG_BOOT_MENU: 
            break;
          case FW_CFG_MAX_CPUS: 
            break;
          case FW_CFG_KERNEL_ENTRY: 
            break;
          case FW_CFG_KERNEL_DATA: 
            break;
          case FW_CFG_INITRD_DATA: 
            break;
          case FW_CFG_CMDLINE_ADDR: 
            break;
          case FW_CFG_CMDLINE_SIZE: 
            break;
          case FW_CFG_CMDLINE_DATA: 
            break;
          case FW_CFG_SETUP_ADDR: 
            break;
          case FW_CFG_SETUP_SIZE: 
            break;
          case FW_CFG_SETUP_DATA: 
            break;
          case FW_CFG_FILE_DIR: 
            break;
          case FW_CFG_FILE_FIRST: 
            break;
        }
        break;
      case 0x14:
        break;
    }
  }
//#endif

  else {
    BX_DEBUG(("DBG: 0x%08x", value));
  }
}

void bx_acpi_ctrl_c::timer_handler(void *this_ptr)
{
  bx_acpi_ctrl_c *class_ptr = (bx_acpi_ctrl_c *) this_ptr;
  class_ptr->timer();
}

void bx_acpi_ctrl_c::timer()
{
  BX_ACPI_THIS pm_update_sci();
}


// static pci configuration space write callback handler
void bx_acpi_ctrl_c::pci_write_handler(Bit8u address, Bit32u value, unsigned io_len)
{
  Bit8u value8, oldval;
  bool pm_base_change = 0, sm_base_change = 0;

  if ((address >= 0x10) && (address < 0x34))
    return;

  BX_DEBUG_PCI_WRITE(address, value, io_len);
  for (unsigned i=0; i<io_len; i++) {
    value8 = (value >> (i*8)) & 0xFF;
    oldval = BX_ACPI_THIS pci_conf[address+i];
    switch (address+i) {
      case 0x04:
        value8 = (value8 & 0xfe) | (value & 0x01);
        goto set_value;
        break;
      case 0x06: // disallowing write to status lo-byte (is that expected?)
        break;
      case 0x40:
        value8 = (value8 & 0xc0) | 0x01;
      case 0x41:
      case 0x42:
      case 0x43:
        pm_base_change |= (value8 != oldval);
        goto set_value;
        break;
      case 0x90:
        value8 = (value8 & 0xf0) | 0x01;
      case 0x91:
      case 0x92:
      case 0x93:
        sm_base_change |= (value8 != oldval);
      default:
set_value:
        BX_ACPI_THIS pci_conf[address+i] = value8;
    }
  }
  if (pm_base_change) {
    if (DEV_pci_set_base_io(BX_ACPI_THIS_PTR, read_handler, write_handler,
                            &BX_ACPI_THIS s.pm_base,
                            &BX_ACPI_THIS pci_conf[0x40],
                            64, &acpi_pm_iomask[0], "ACPI PM base"))
    {
       BX_INFO(("new PM base address: 0x%04x", BX_ACPI_THIS s.pm_base));
    }
  }
  if (sm_base_change) {
    if (DEV_pci_set_base_io(BX_ACPI_THIS_PTR, read_handler, write_handler,
                            &BX_ACPI_THIS s.sm_base,
                            &BX_ACPI_THIS pci_conf[0x90],
                            16, &acpi_sm_iomask[0], "ACPI SM base"))
    {
       BX_INFO(("new SM base address: 0x%04x", BX_ACPI_THIS s.sm_base));
    }
  }
}

#endif // BX_SUPPORT_PCI
