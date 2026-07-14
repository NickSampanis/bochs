#include "bochs.h"
#include "cpu.h"

#if BX_SUPPORT_VTD
#include "vtd.h"

void bx_vtd_c::read(bx_phy_address addr, void *data, unsigned len)
{
    #if 0
    uint64_t val;

    val = 0;

    if (addr + len > DMAR_REG_SIZE) 
        return;

    switch (addr) {
    /* Root Table Address Register, 64-bit */
    case DMAR_RTADDR_REG:
    /*
        val = vtd_get_quad_raw(s, DMAR_RTADDR_REG);
        if (size == 4) {
            val = val & ((1ULL << 32) - 1);
        }
    */
        break;

    case DMAR_RTADDR_REG_HI:
        
        //val = vtd_get_quad_raw(s, DMAR_RTADDR_REG) >> 32;
        break;

    /* Invalidation Queue Address Register, 64-bit */
    case DMAR_IQA_REG:
    /*
        val = s->iq | (vtd_get_quad(s, DMAR_IQA_REG) & VTD_IQA_QS);
        if (size == 4) {
            val = val & ((1ULL << 32) - 1);
        }
    */
        break;

    case DMAR_IQA_REG_HI:
        //val = s->iq >> 32;
        break;

    default:
    /*
        if (size == 4) {
            val = vtd_get_long(s, addr);
        } else {
            val = vtd_get_quad(s, addr);
        }
    */
    }
    #endif

}

void bx_vtd_c::write(bx_phy_address addr, void *data, unsigned len)
{
    #if 0
    if (addr + len > DMAR_REG_SIZE) {
        return;
    }

    switch (addr) {
    /* Global Command Register, 32-bit */
    case DMAR_GCMD_REG:
        //vtd_set_long(s, addr, val);
        //vtd_handle_gcmd_write(s);
        break;

    /* Context Command Register, 64-bit */
    case DMAR_CCMD_REG:
        /*
        if (len == 4) {
            //vtd_set_long(s, addr, val);
        } else {
            //vtd_set_quad(s, addr, val);
            //vtd_handle_ccmd_write(s);
        }
        */
        break;

    case DMAR_CCMD_REG_HI:
        
        //vtd_set_long(s, addr, val);
        //vtd_handle_ccmd_write(s);
        break;

    /* IOTLB Invalidation Register, 64-bit */
    case DMAR_IOTLB_REG:
        /*
        if (len == 4) {
            //vtd_set_long(s, addr, val);
        } else {
            //vtd_set_quad(s, addr, val);
            //vtd_handle_iotlb_write(s);
        }
        */
        break;

    case DMAR_IOTLB_REG_HI:
        
        //vtd_set_long(s, addr, val);
        //vtd_handle_iotlb_write(s);
        break;

    /* Invalidate Address Register, 64-bit */
    case DMAR_IVA_REG:
    /*
        if (len == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
        break;

    case DMAR_IVA_REG_HI:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Fault Status Register, 32-bit */
    case DMAR_FSTS_REG:
        
        //vtd_set_long(s, addr, val);
        //vtd_handle_fsts_write(s);
        break;

    /* Fault Event Control Register, 32-bit */
    case DMAR_FECTL_REG:
        
        //vtd_set_long(s, addr, val);
        //vtd_handle_fectl_write(s);
        break;

    /* Fault Event Data Register, 32-bit */
    case DMAR_FEDATA_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Fault Event Address Register, 32-bit */
    case DMAR_FEADDR_REG:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
        break;

    /* Fault Event Upper Address Register, 32-bit */
    case DMAR_FEUADDR_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Protected Memory Enable Register, 32-bit */
    case DMAR_PMEN_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Root Table Address Register, 64-bit */
    case DMAR_RTADDR_REG:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
        break;

    case DMAR_RTADDR_REG_HI:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Invalidation Queue Tail Register, 64-bit */
    case DMAR_IQT_REG:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
        vtd_handle_iqt_write(s);
    */
        break;

    case DMAR_IQT_REG_HI:
        
        //vtd_set_long(s, addr, val);
        /* 19:63 of IQT_REG is RsvdZ, do nothing here */
        break;

    /* Invalidation Queue Address Register, 64-bit */
    case DMAR_IQA_REG:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
        vtd_update_iq_dw(s);
    */
        break;

    case DMAR_IQA_REG_HI:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Invalidation Completion Status Register, 32-bit */
    case DMAR_ICS_REG:
        /*
        vtd_set_long(s, addr, val);
        vtd_handle_ics_write(s);
        */
        break;

    /* Invalidation Event Control Register, 32-bit */
    case DMAR_IECTL_REG:
        
        //vtd_set_long(s, addr, val);
        //vtd_handle_iectl_write(s);
        break;

    /* Invalidation Event Data Register, 32-bit */
    case DMAR_IEDATA_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Invalidation Event Address Register, 32-bit */
    case DMAR_IEADDR_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Invalidation Event Upper Address Register, 32-bit */
    case DMAR_IEUADDR_REG:
        
        //vtd_set_long(s, addr, val);
        break;

    /* Fault Recording Registers, 128-bit */
    case DMAR_FRCD_REG_0_0:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
        break;

    case DMAR_FRCD_REG_0_1:
        
        //vtd_set_long(s, addr, val);
        break;

    case DMAR_FRCD_REG_0_2:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
            vtd_update_fsts_ppf(s);
        }
    */
        break;

    case DMAR_FRCD_REG_0_3:
        
        //vtd_set_long(s, addr, val);
        /* May clear bit 127 (Fault), update PPF */
        //vtd_update_fsts_ppf(s);
        break;

    case DMAR_IRTA_REG:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
        break;

    case DMAR_IRTA_REG_HI:
        
        //vtd_set_long(s, addr, val);
        break;

    default:
    /*
        if (size == 4) {
            vtd_set_long(s, addr, val);
        } else {
            vtd_set_quad(s, addr, val);
        }
    */
    }
    #endif
}


bx_vtd_c::bx_vtd_c(BX_CPU_C *cpu)
{

}

#endif