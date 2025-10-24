#ifndef BX_CPU_TPM2_H
#define BX_CPU_TPM2_H 1

#if BX_SUPPORT_TPM2

#define TPM_TIS_ADDR_BASE           0xFED40000
#define TPM_TIS_ADDR_SIZE           0x5000

#define TPM_TIS_IRQ                 5

#define TPM_TIS_NUM_LOCALITIES      5     /* per spec */
#define TPM_TIS_LOCALITY_SHIFT      12

/* tis registers */
#define TPM_TIS_REG_ACCESS                0x00
#define TPM_TIS_REG_INT_ENABLE            0x08
#define TPM_TIS_REG_INT_VECTOR            0x0c
#define TPM_TIS_REG_INT_STATUS            0x10
#define TPM_TIS_REG_INTF_CAPABILITY       0x14
#define TPM_TIS_REG_STS                   0x18
#define TPM_TIS_REG_DATA_FIFO             0x24
#define TPM_TIS_REG_INTERFACE_ID          0x30
#define TPM_TIS_REG_DATA_XFIFO            0x80
#define TPM_TIS_REG_DATA_XFIFO_END        0xbc
#define TPM_TIS_REG_DID_VID               0xf00
#define TPM_TIS_REG_RID                   0xf04

/* vendor-specific registers */
#define TPM_TIS_REG_DEBUG                 0xf90

#define TPM_TIS_STS_TPM_FAMILY_MASK         (0x3 << 26)/* TPM 2.0 */
#define TPM_TIS_STS_TPM_FAMILY1_2           (0 << 26)  /* TPM 2.0 */
#define TPM_TIS_STS_TPM_FAMILY2_0           (1 << 26)  /* TPM 2.0 */
#define TPM_TIS_STS_RESET_ESTABLISHMENT_BIT (1 << 25)  /* TPM 2.0 */
#define TPM_TIS_STS_COMMAND_CANCEL          (1 << 24)  /* TPM 2.0 */

#define TPM_TIS_STS_VALID                 (1 << 7)
#define TPM_TIS_STS_COMMAND_READY         (1 << 6)
#define TPM_TIS_STS_TPM_GO                (1 << 5)
#define TPM_TIS_STS_DATA_AVAILABLE        (1 << 4)
#define TPM_TIS_STS_EXPECT                (1 << 3)
#define TPM_TIS_STS_SELFTEST_DONE         (1 << 2)
#define TPM_TIS_STS_RESPONSE_RETRY        (1 << 1)

#define TPM_TIS_BURST_COUNT_SHIFT         8
#define TPM_TIS_BURST_COUNT(X) \
    ((X) << TPM_TIS_BURST_COUNT_SHIFT)

#define TPM_TIS_ACCESS_TPM_REG_VALID_STS  (1 << 7)
#define TPM_TIS_ACCESS_ACTIVE_LOCALITY    (1 << 5)
#define TPM_TIS_ACCESS_BEEN_SEIZED        (1 << 4)
#define TPM_TIS_ACCESS_SEIZE              (1 << 3)
#define TPM_TIS_ACCESS_PENDING_REQUEST    (1 << 2)
#define TPM_TIS_ACCESS_REQUEST_USE        (1 << 1)
#define TPM_TIS_ACCESS_TPM_ESTABLISHMENT  (1 << 0)

#define TPM_TIS_INT_ENABLED               (1 << 31)
#define TPM_TIS_INT_DATA_AVAILABLE        (1 << 0)
#define TPM_TIS_INT_STS_VALID             (1 << 1)
#define TPM_TIS_INT_LOCALITY_CHANGED      (1 << 2)
#define TPM_TIS_INT_COMMAND_READY         (1 << 7)

#define TPM_TIS_INT_POLARITY_MASK         (3 << 3)
#define TPM_TIS_INT_POLARITY_LOW_LEVEL    (1 << 3)

#define TPM_TIS_INTERRUPTS_SUPPORTED (TPM_TIS_INT_LOCALITY_CHANGED | \
                                      TPM_TIS_INT_DATA_AVAILABLE   | \
                                      TPM_TIS_INT_STS_VALID | \
                                      TPM_TIS_INT_COMMAND_READY)

#define TPM_TIS_CAP_INTERFACE_VERSION1_3 (2 << 28)
#define TPM_TIS_CAP_INTERFACE_VERSION1_3_FOR_TPM2_0 (3 << 28)
#define TPM_TIS_CAP_DATA_TRANSFER_64B    (3 << 9)
#define TPM_TIS_CAP_DATA_TRANSFER_LEGACY (0 << 9)
#define TPM_TIS_CAP_BURST_COUNT_DYNAMIC  (0 << 8)
#define TPM_TIS_CAP_BURST_COUNT_STATIC   (1 << 8)
#define TPM_TIS_CAP_INTERRUPT_LOW_LEVEL  (1 << 4) /* support is mandatory */
#define TPM_TIS_CAPABILITIES_SUPPORTED1_3 \
    (TPM_TIS_CAP_INTERRUPT_LOW_LEVEL | \
     TPM_TIS_CAP_BURST_COUNT_DYNAMIC | \
     TPM_TIS_CAP_DATA_TRANSFER_64B | \
     TPM_TIS_CAP_INTERFACE_VERSION1_3 | \
     TPM_TIS_INTERRUPTS_SUPPORTED)

#define TPM_TIS_CAPABILITIES_SUPPORTED2_0 \
    (TPM_TIS_CAP_INTERRUPT_LOW_LEVEL | \
     TPM_TIS_CAP_BURST_COUNT_DYNAMIC | \
     TPM_TIS_CAP_DATA_TRANSFER_64B | \
     TPM_TIS_CAP_INTERFACE_VERSION1_3_FOR_TPM2_0 | \
     TPM_TIS_INTERRUPTS_SUPPORTED)

#define TPM_TIS_IFACE_ID_INTERFACE_TIS1_3   (0xf)     /* TPM 2.0 */
#define TPM_TIS_IFACE_ID_INTERFACE_FIFO     (0x0)     /* TPM 2.0 */
#define TPM_TIS_IFACE_ID_INTERFACE_VER_FIFO (0 << 4)  /* TPM 2.0 */
#define TPM_TIS_IFACE_ID_CAP_5_LOCALITIES   (1 << 8)  /* TPM 2.0 */
#define TPM_TIS_IFACE_ID_CAP_TIS_SUPPORTED  (1 << 13) /* TPM 2.0 */
#define TPM_TIS_IFACE_ID_INT_SEL_LOCK       (1 << 19) /* TPM 2.0 */

#define TPM_TIS_IFACE_ID_SUPPORTED_FLAGS1_3 \
    (TPM_TIS_IFACE_ID_INTERFACE_TIS1_3 | \
     (~0u << 4)/* all of it is don't care */)

/* if backend was a TPM 2.0: */
#define TPM_TIS_IFACE_ID_SUPPORTED_FLAGS2_0 \
    (TPM_TIS_IFACE_ID_INTERFACE_FIFO | \
     TPM_TIS_IFACE_ID_INTERFACE_VER_FIFO | \
     TPM_TIS_IFACE_ID_CAP_5_LOCALITIES | \
     TPM_TIS_IFACE_ID_CAP_TIS_SUPPORTED)

#define PCI_VENDOR_ID_IBM                0x1014

#define TPM_TIS_TPM_DID       0x0001
#define TPM_TIS_TPM_VID       PCI_VENDOR_ID_IBM
#define TPM_TIS_TPM_RID       0x0001

#define TPM_TIS_NO_DATA_BYTE  0xff


#define TPM_TIS_NUM_LOCALITIES      5     /* per spec */
#define TPM_TIS_LOCALITY_SHIFT      12
#define TPM_TIS_NO_LOCALITY         0xff

#define TPM_TIS_IS_VALID_LOCTY(x)   ((x) < TPM_TIS_NUM_LOCALITIES)

#define TPM_TIS_BUFFER_MAX          4096

typedef enum TPMVersion {
    TPM_VERSION_UNSPEC = 0,
    TPM_VERSION_1_2 = 1,
    TPM_VERSION_2_0 = 2,
} TPMVersion;


typedef enum {
    TPM_TIS_STATE_IDLE = 0,
    TPM_TIS_STATE_READY,
    TPM_TIS_STATE_COMPLETION,
    TPM_TIS_STATE_EXECUTION,
    TPM_TIS_STATE_RECEPTION,
} TPMTISState;

/* locality data  -- all fields are persisted */
typedef struct TPMLocality {
    TPMTISState state;
    uint8_t access;
    uint32_t sts;
    uint32_t iface_id;
    uint32_t int_enabled;
    uint32_t int_status;
} TPMLocality;



class BOCHSAPI bx_tpm2_c : public logfunctions
{
    //bx_phy_address base_addr;
    uint8_t tpm_version;
    uint8_t irq_num;
    uint8_t int_enabled;
    uint8_t active_local;
    uint8_t next_locty;
    uint8_t aborting_locty;
    TPMLocality local[TPM_TIS_NUM_LOCALITIES];
    unsigned char buffer[TPM_TIS_BUFFER_MAX];
    uint32_t buffer_size;
    uint32_t rw_offset;
public:
    bx_tpm2_c(BX_CPU_C *cpu);
    bool is_selected(bx_phy_address addr);
    void read(bx_phy_address addr, void *data, unsigned len);
    void write(bx_phy_address addr, void *data, unsigned len);
    void tpm_tis_new_active_locality(uint8_t new_active_local);
    void tpm_tis_raise_irq(uint8_t i, uint32_t irqmask);
    uint32_t tpm_tis_data_read(uint8_t local);
    void tpm_tis_abort(void);
    void tpm_tis_prep_abort(uint8_t locty, uint8_t newlocty);




};



#endif
#endif