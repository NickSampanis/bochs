#include "bochs.h"
#include "cpu.h"

#if BX_SUPPORT_TPM2

#include "tpm.h"
#include "iodev/iodev.h"
#include <tssClient.h>

#define LOG_THIS genlog->


#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "tssClient.lib")
#endif
/*
static int tpm_emulator_ctrlcmd(TPMEmulator *tpm, unsigned long cmd, void *msg,
                                size_t msg_len_in, size_t msg_len_out)
{
    CharBackend *dev = &tpm->ctrl_chr;
    uint32_t cmd_no = cpu_to_be32(cmd);
    ssize_t n = sizeof(uint32_t) + msg_len_in;
    uint8_t *buf = NULL;

    WITH_QEMU_LOCK_GUARD(&tpm->mutex) {
        buf = g_alloca(n);
        memcpy(buf, &cmd_no, sizeof(cmd_no));
        memcpy(buf + sizeof(cmd_no), msg, msg_len_in);

        n = qemu_chr_fe_write_all(dev, buf, n);
        if (n <= 0) {
            return -1;
        }

        if (msg_len_out != 0) {
            n = qemu_chr_fe_read_all(dev, msg, msg_len_out);
            if (n <= 0) {
                return -1;
            }
        }
    }

    return 0;
}
*/
PSOCKADDR get_local_ipv4(void)
{
    ULONG result, bufferSize;
    WSADATA wsaData;
    PSOCKADDR addr;
    IP_ADAPTER_ADDRESSES* adapterAddresses, *adapter;
    PIP_ADAPTER_UNICAST_ADDRESS_LH unicast;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result) 
        exit(-1);
    
    memset(&addr, 0, sizeof(addr));
    bufferSize = 15000;
    adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
    result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize);
    if (result == ERROR_BUFFER_OVERFLOW) {
        free(adapterAddresses);
        adapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
        result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize);
    }
    if (result != NO_ERROR) {
        BX_ERROR(("Error in GetAdaptersAddresses"));
        exit(-1);    
    }
    
    adapter = adapterAddresses;
    while (adapter) {
        unicast = adapter->FirstUnicastAddress;
        while (unicast) {
            addr = (PSOCKADDR)malloc(sizeof(SOCKADDR));
            memcpy(addr, unicast->Address.lpSockaddr, sizeof(unicast->Address.lpSockaddr));
            free(adapter);
            return addr;
        }
        adapter = adapter->Next;
    }
    return addr;
}

bx_tpm2_c::bx_tpm2_c(BX_CPU_C *cpu, char *ipv4, int port)
{
    char ipStr[INET6_ADDRSTRLEN];
    ULONG c, result;
    PSOCKADDR addr;

    this->active_local = TPM_TIS_NO_LOCALITY;
    this->next_locty = TPM_TIS_NO_LOCALITY;
    this->aborting_locty = TPM_TIS_NO_LOCALITY;

    for (c = 0; c < TPM_TIS_NUM_LOCALITIES; c++) {
        this->local[c].access = TPM_TIS_ACCESS_TPM_REG_VALID_STS;
        this->local[c].sts = TPM_TIS_STS_TPM_FAMILY2_0;
        this->local[c].iface_id = TPM_TIS_IFACE_ID_SUPPORTED_FLAGS2_0;
        this->local[c].int_enabled = TPM_TIS_INT_POLARITY_LOW_LEVEL;
        this->local[c].int_status = 0;
        this->local[c].state = TPM_TIS_STATE_IDLE;
        
    }
    this->tpm_version = TPM_VERSION_2_0;
    this->rw_offset = 0;
    this->irq_num = 13;
    this->buffer_size = TPM_TIS_BUFFER_MAX;
    memset(ipStr, 0, sizeof(ipStr));
    
    if (!ipv4) {
        addr = get_local_ipv4();
        getnameinfo(addr, sizeof(SOCKADDR), ipStr, sizeof(ipStr), NULL, 0, NI_NUMERICHOST);
        free(addr);
    }
    else 
        memcpy(ipStr, ipv4, strlen(ipv4) < sizeof(ipStr) ?  strlen(ipv4) : sizeof(ipStr) - 1);
    result = TPM2Initialize((CHAR *)ipStr, port);
    if (result) 
        BX_PANIC(("Error in TPM2Initialize connecting to %s:%d", ipStr, port));
}

/*
const struct tpm_interface TPM2Interface = {
    .MainInit = TPM2_MainInit,
    .Terminate = TPM2_Terminate,
    .Process = TPM2_Process,
    .VolatileAllStore = TPM2_VolatileAllStore,
    .CancelCommand = TPM2_CancelCommand,
    .GetTPMProperty = TPM2_GetTPMProperty,
    .GetInfo = TPM2_GetInfo,
    .TpmEstablishedGet = TPM2_IO_TpmEstablished_Get,
    .TpmEstablishedReset = TPM2_IO_TpmEstablished_Reset,
    .HashStart = TPM2_IO_Hash_Start,
    .HashData = TPM2_IO_Hash_Data,
    .HashEnd = TPM2_IO_Hash_End,
    .SetBufferSize = TPM2_SetBufferSize,
    .ValidateState = TPM2_ValidateState,
    .SetState = TPM2_SetState,
    .GetState = TPM2_GetState,
    .SetProfile = TPM2_SetProfile,
    .WasManufactured = TPM2_WasManufactured,
};
*/

bool bx_tpm2_c::is_selected(bx_phy_address addr)
{
  if((addr & ~0xffff) == TPM_TIS_ADDR_BASE) 
    return true;
  
  return false;
}

/*
const struct tpm_interface TPM2Interface = {
    .MainInit = TPM2_MainInit,
    .Terminate = TPM2_Terminate,
    .Process = TPM2_Process,
    .VolatileAllStore = TPM2_VolatileAllStore,
    .CancelCommand = TPM2_CancelCommand,
    .GetTPMProperty = TPM2_GetTPMProperty,
    .GetInfo = TPM2_GetInfo,
    .TpmEstablishedGet = TPM2_IO_TpmEstablished_Get,
    .TpmEstablishedReset = TPM2_IO_TpmEstablished_Reset,
    .HashStart = TPM2_IO_Hash_Start,
    .HashData = TPM2_IO_Hash_Data,
    .HashEnd = TPM2_IO_Hash_End,
    .SetBufferSize = TPM2_SetBufferSize,
    .ValidateState = TPM2_ValidateState,
    .SetState = TPM2_SetState,
    .GetState = TPM2_GetState,
    .SetProfile = TPM2_SetProfile,
    .WasManufactured = TPM2_WasManufactured,
};

TPM_RESULT TPM_IO_TpmEstablished_Get(TPM_BOOL *tpmEstablished)
{
    return tpm_iface[tpmvers_choice]->TpmEstablishedGet(tpmEstablished);
}


static int tpm_emulator_set_locality(TPMEmulator *tpm_emu, uint8_t locty_number,
                                     Error **errp)
{
    ptm_loc loc;

    if (tpm_emu->cur_locty_number == locty_number) {
        return 0;
    }

    trace_tpm_emulator_set_locality(locty_number);

    memset(&loc, 0, sizeof(loc));
    loc.u.req.loc = locty_number;
    if (tpm_emulator_ctrlcmd(tpm_emu, CMD_SET_LOCALITY, &loc,
                             sizeof(loc), sizeof(loc)) < 0) {
        error_setg(errp, "tpm-emulator: could not set locality : %s",
                   strerror(errno));
        return -1;
    }

    loc.u.resp.tpm_result = be32_to_cpu(loc.u.resp.tpm_result);
    if (loc.u.resp.tpm_result != 0) {
        error_setg(errp, "tpm-emulator: TPM result for set locality : 0x%x",
                   loc.u.resp.tpm_result);
        return -1;
    }

    tpm_emu->cur_locty_number = locty_number;

    return 0;
}

static int tpm_emulator_unix_tx_bufs(TPMEmulator *tpm_emu,
                                     const uint8_t *in, uint32_t in_len,
                                     uint8_t *out, uint32_t out_len,
                                     bool *selftest_done,
                                     Error **errp)
{
    ssize_t ret;
    bool is_selftest = false;

    if (selftest_done) {
        *selftest_done = false;
        is_selftest = tpm_util_is_selftest(in, in_len);
    }

    ret = qio_channel_write_all(tpm_emu->data_ioc, (char *)in, in_len, errp);
    if (ret != 0) {
        return -1;
    }

    ret = qio_channel_read_all(tpm_emu->data_ioc, (char *)out,
              sizeof(struct tpm_resp_hdr), errp);
    if (ret != 0) {
        return -1;
    }

    ret = qio_channel_read_all(tpm_emu->data_ioc,
              (char *)out + sizeof(struct tpm_resp_hdr),
              tpm_cmd_get_size(out) - sizeof(struct tpm_resp_hdr), errp);
    if (ret != 0) {
        return -1;
    }

    if (is_selftest) {
        *selftest_done = tpm_cmd_get_errcode(out) == 0;
    }

    return 0;
}

static void tpm_emulator_handle_request(TPMBackend *tb, TPMBackendCmd *cmd,
                                        Error **errp)
{
    TPMEmulator *tpm_emu = TPM_EMULATOR(tb);

    trace_tpm_emulator_handle_request();

    if (tpm_emulator_set_locality(tpm_emu, cmd->locty, errp) < 0 ||
        tpm_emulator_unix_tx_bufs(tpm_emu, cmd->in, cmd->in_len,
                                  cmd->out, cmd->out_len,
                                  &cmd->selftest_done, errp) < 0) {
        tpm_util_write_fatal_error_response(cmd->out, cmd->out_len);
    }
}
*/

//EMULATOR TPMBackendClass
//TPMEmulator
/*
static void tpm_emulator_class_init(ObjectClass *klass, void *data)
{
    TPMBackendClass *tbc = TPM_BACKEND_CLASS(klass);

    tbc->type = TPM_TYPE_EMULATOR;
    tbc->opts = tpm_emulator_cmdline_opts;
    tbc->desc = "TPM emulator backend driver";
    tbc->create = tpm_emulator_create;
    tbc->startup_tpm = tpm_emulator_startup_tpm;
    tbc->cancel_cmd = tpm_emulator_cancel_cmd;
    tbc->get_tpm_established_flag = tpm_emulator_get_tpm_established_flag;
    tbc->reset_tpm_established_flag = tpm_emulator_reset_tpm_established_flag;
    tbc->get_tpm_version = tpm_emulator_get_tpm_version;
    tbc->get_buffer_size = tpm_emulator_get_buffer_size;
    tbc->get_tpm_options = tpm_emulator_get_tpm_options;

    tbc->handle_request = tpm_emulator_handle_request;
}

static int tpm_emulator_ctrlcmd(TPMEmulator *tpm, unsigned long cmd, void *msg,
                                size_t msg_len_in, size_t msg_len_out)
{
    CharBackend *dev = &tpm->ctrl_chr;
    uint32_t cmd_no = cpu_to_be32(cmd);
    ssize_t n = sizeof(uint32_t) + msg_len_in;
    uint8_t *buf = NULL;

    WITH_QEMU_LOCK_GUARD(&tpm->mutex) {
        buf = g_alloca(n);
        memcpy(buf, &cmd_no, sizeof(cmd_no));
        memcpy(buf + sizeof(cmd_no), msg, msg_len_in);

        n = qemu_chr_fe_write_all(dev, buf, n);
        if (n <= 0) {
            return -1;
        }

        if (msg_len_out != 0) {
            n = qemu_chr_fe_read_all(dev, msg, msg_len_out);
            if (n <= 0) {
                return -1;
            }
        }
    }

    return 0;
}

static void tpm_emulator_cancel_cmd(TPMBackend *tb)
{
    TPMEmulator *tpm_emu = TPM_EMULATOR(tb);
    ptm_res res;

    if (!TPM_EMULATOR_IMPLEMENTS_ALL_CAPS(tpm_emu, PTM_CAP_CANCEL_TPM_CMD)) {
        return;
    }

    if (tpm_emulator_ctrlcmd(tpm_emu, CMD_CANCEL_TPM_CMD, &res, 0,
                             sizeof(res)) < 0) {
        error_report("tpm-emulator: Could not cancel command: %s",
                     strerror(errno));
    } else if (res != 0) {
        error_report("tpm-emulator: Failed to cancel TPM: 0x%x",
                     be32_to_cpu(res));
    }
}


static bool tpm_emulator_get_tpm_established_flag(TPMBackend *tb)
{
    TPMEmulator *tpm_emu = TPM_EMULATOR(tb);
    ptm_est est;

    if (tpm_emu->established_flag_cached) {
        return tpm_emu->established_flag;
    }

    if (tpm_emulator_ctrlcmd(tpm_emu, CMD_GET_TPMESTABLISHED, &est,
                             0, sizeof(est)) < 0) {
        error_report("tpm-emulator: Could not get the TPM established flag: %s",
                     strerror(errno));
        return false;
    }
    tpm_emu->established_flag_cached = 1;
    tpm_emu->established_flag = (est.u.resp.bit != 0);

    return tpm_emu->established_flag;
}

bool tpm_backend_get_tpm_established_flag(TPMBackend *s)
{
    TPMBackendClass *k = TPM_BACKEND_GET_CLASS(s);

    return k->get_tpm_established_flag ?
           k->get_tpm_established_flag(s) : false;
}

static uint32_t tpm_tis_check_request_use_except(TPMState *s, uint8_t i)
{
    uint8_t l;

    for (l = 0; l < TPM_TIS_NUM_LOCALITIES; l++) {
        if (l == i) {
            continue;
        }
        if ((this->local[l].access & TPM_TIS_ACCESS_REQUEST_USE)) {
            return 1;
        }
    }

    return 0;
}
*/

#define MIN(x, y) ((x) >= (y) ? (y) : (x))
#define TPM_TIS_BURST_COUNT_SHIFT         8
#define TPM_TIS_BURST_COUNT(X) \
    ((X) << TPM_TIS_BURST_COUNT_SHIFT)
#define TPM_TIS_IS_VALID_LOCTY(x)   ((x) < TPM_TIS_NUM_LOCALITIES)


static inline int ldl_he_p(const void *ptr)
{
    int32_t r;
    
    memcpy(&r, ptr, sizeof(r));

    return r;
}

static inline int ldl_be_p(const void *ptr)
{
    return bx_bswap32(ldl_he_p(ptr));
}

static inline uint32_t tpm_cmd_get_size(unsigned char *b)
{
    return ldl_be_p(b + 2);
}


static void tpm_tis_sts_set(TPMLocality *l, uint32_t flags)
{
    l->sts &= TPM_TIS_STS_SELFTEST_DONE | TPM_TIS_STS_TPM_FAMILY_MASK;
    l->sts |= flags;
}

void bx_tpm2_c::tpm_tis_raise_irq(uint8_t i, uint32_t irqmask)
{
    if (i >= TPM_TIS_NUM_LOCALITIES)
        return;

    if ((this->local[i].int_enabled & TPM_TIS_INT_ENABLED) &&
        (this->local[i].int_enabled & irqmask)) {
        DEV_pic_raise_irq(this->irq_num);
        this->local[i].int_status |= irqmask;
    }
}

uint8_t bx_tpm2_c::tpm_tis_data_read(uint8_t local)
{
    uint8_t ret = TPM_TIS_NO_DATA_BYTE;
    uint16_t len;

    if ((this->local[local].sts & TPM_TIS_STS_DATA_AVAILABLE)) {
        len = MIN(tpm_cmd_get_size((unsigned char *)this->buffer), this->buffer_size);
        //len = this->buffer_size;
        ret = this->buffer[this->rw_offset++];
        if (this->rw_offset >= len) {
            tpm_tis_sts_set(&this->local[local], TPM_TIS_STS_VALID);
            tpm_tis_raise_irq(local, TPM_TIS_INT_STS_VALID);
        }
    }

    return ret;
}

void bx_tpm2_c::tpm_tis_new_active_locality(uint8_t new_active_local)
{
    bool change = (this->active_local != new_active_local);
    bool is_seize;
    uint8_t mask;

    if (change && TPM_TIS_IS_VALID_LOCTY(this->active_local)) {
        is_seize = TPM_TIS_IS_VALID_LOCTY(new_active_local) &&
                   this->local[new_active_local].access & TPM_TIS_ACCESS_SEIZE;

        if (is_seize) {
            mask = ~(TPM_TIS_ACCESS_ACTIVE_LOCALITY);
        } else {
            mask = ~(TPM_TIS_ACCESS_ACTIVE_LOCALITY|
                     TPM_TIS_ACCESS_REQUEST_USE);
        }
        this->local[this->active_local].access &= mask;

        if (is_seize) {
            this->local[this->active_local].access |= TPM_TIS_ACCESS_BEEN_SEIZED;
        }
    }

    this->active_local = new_active_local;
    TPM2SetLocality(this->active_local);
    if (TPM_TIS_IS_VALID_LOCTY(new_active_local)) {
        this->local[new_active_local].access |= TPM_TIS_ACCESS_ACTIVE_LOCALITY;
        this->local[new_active_local].access &= ~(TPM_TIS_ACCESS_REQUEST_USE |
                                               TPM_TIS_ACCESS_SEIZE);
    }

    if (change) {
        tpm_tis_raise_irq(this->active_local, TPM_TIS_INT_LOCALITY_CHANGED);
    }
}

void bx_tpm2_c::tpm_tis_abort(void)
{
    this->rw_offset = 0;
    if (this->aborting_locty == this->next_locty) {
        this->local[this->aborting_locty].state = TPM_TIS_STATE_READY;
        tpm_tis_sts_set(&this->local[this->aborting_locty],
                        TPM_TIS_STS_COMMAND_READY);
        tpm_tis_raise_irq(this->aborting_locty, TPM_TIS_INT_COMMAND_READY);
    }

    tpm_tis_new_active_locality(this->next_locty);

    this->next_locty = TPM_TIS_NO_LOCALITY;
    this->aborting_locty = TPM_TIS_NO_LOCALITY;
}

void bx_tpm2_c::read(bx_phy_address addr, void *data, unsigned len)
{
    uint32_t i;

    for (i = 0; i < len; i += 8)
        read8(addr, (void *)((uint8_t *)data + i), (len > 8) ? 8 : len);
    
}

void bx_tpm2_c::read8(bx_phy_address addr, void *data, unsigned len)
{
    Bit8u i, j, l, shift;
    Bit64u val, ret, avail, status;
    size_t resp_size;

    val = 0xffffffffffffffff;
    shift = (addr & 0x3) * 8;
    i = (uint8_t)((addr >> TPM_TIS_LOCALITY_SHIFT) & 0x7);
    if (i >= TPM_TIS_NUM_LOCALITIES)
        return;
    switch (addr & 0xffc) {
    case TPM_TIS_REG_ACCESS:
        val = this->local[i].access & ~TPM_TIS_ACCESS_SEIZE;
        for (j = 0; j < TPM_TIS_NUM_LOCALITIES; j++) {
            if (j == i)
                continue;
            if (this->local[j].access & TPM_TIS_ACCESS_REQUEST_USE) {
                val |= TPM_TIS_ACCESS_PENDING_REQUEST;
                break;
            }
        }
        //TPM_SIGNAL_RESET
        val |= !(this->local[j].access & TPM_TIS_ACCESS_TPM_ESTABLISHMENT);
        break;
    case TPM_TIS_REG_INT_ENABLE:
        val = this->local[i].int_enabled;
        break;
    case TPM_TIS_REG_INT_VECTOR:
        val = this->irq_num;
        break;
    case TPM_TIS_REG_INT_STATUS:
        val = this->local[i].int_status;
        break;
    case TPM_TIS_REG_INTF_CAPABILITY:
        /*
        switch (this->tpm_version) {
        case TPM_VERSION_UNSPEC:
            val = 0;
            break;
        case TPM_VERSION_1_2:
            val = TPM_TIS_CAPABILITIES_SUPPORTED1_3;
            break;
        case TPM_VERSION_2_0:
            val = TPM_TIS_CAPABILITIES_SUPPORTED2_0;
            break;
        }
        */
        val = TPM_TIS_CAPABILITIES_SUPPORTED2_0;
        break;
    case TPM_TIS_REG_STS:
        if (this->active_local == i) {
            if (this->local[i].state == TPM_TIS_STATE_EXECUTION) {
                    //move to sts read
                    resp_size = TPM_TIS_BUFFER_MAX;
                    status = TPM2Receive(this->buffer, &resp_size, 0);
                    if (status) {

                    }
                    //i = buffer.locty?
                    tpm_tis_sts_set(&this->local[i], TPM_TIS_STS_VALID | TPM_TIS_STS_DATA_AVAILABLE);
                    this->local[i].state = TPM_TIS_STATE_COMPLETION;
                    this->rw_offset = 0;
                    if (TPM_TIS_IS_VALID_LOCTY(this->next_locty)) {
                        tpm_tis_abort();
                    }
                    tpm_tis_raise_irq(i, TPM_TIS_INT_DATA_AVAILABLE | TPM_TIS_INT_STS_VALID);
            }
            if ((this->local[i].sts & TPM_TIS_STS_DATA_AVAILABLE)) {
                
                val = TPM_TIS_BURST_COUNT(
                       MIN(tpm_cmd_get_size((unsigned char *)this->buffer), this->buffer_size)
                       - this->rw_offset) | this->local[i].sts;
                
                //val = TPM_TIS_BURST_COUNT(this->buffer_size - this->rw_offset) | this->local[i].sts;
                
            } else {
                avail = this->buffer_size - this->rw_offset;
                if (len == 1 && avail > 0xff) {
                    avail = 0xff;
                }
                val = TPM_TIS_BURST_COUNT(avail) | this->local[i].sts;
            }
        }
        break;
    case TPM_TIS_REG_DATA_FIFO:
    case TPM_TIS_REG_DATA_XFIFO:
    case TPM_TIS_REG_DATA_XFIFO_END:
        l = len;
        if (this->active_local == i) {
            /*
            if (l > 4 - (addr & 0x3))
                l = 4 - (addr & 0x3);
            */
            val = 0;
            shift = 0;
            while (l > 0) {
                switch (this->local[i].state) {
                case TPM_TIS_STATE_COMPLETION:
                    ret = tpm_tis_data_read(i);
                    break;
                default:
                    ret = TPM_TIS_NO_DATA_BYTE;
                    break;
                }
                val |= (ret << shift);
                shift += 8;
                l--;
            }
            shift = 0;
        }
        break;
    case TPM_TIS_REG_INTERFACE_ID:
        val = this->local[i].iface_id;
        break;
    case TPM_TIS_REG_DID_VID:
        val = (TPM_TIS_TPM_DID << 16) | TPM_TIS_TPM_VID;
        break;
    case TPM_TIS_REG_RID:
        val = TPM_TIS_TPM_RID;
        break;        
    }
    if (shift)
        val >>= shift;
    
    switch (len) {
        case 1:
            *(Bit8u*)data = val;
            break;
        case 2:
            *(Bit16u*)data = (val);
            break;
        case 4:
            *(Bit32u*)data = (val);
            break;
        case 8:
            *(Bit64u*)data = (val);
    }
    
    //*(Bit32u*)data = val;
}




void bx_tpm2_c::tpm_tis_prep_abort(uint8_t locty, uint8_t newlocty)
{
    uint8_t busy_locty;


    this->aborting_locty = locty; 
    this->next_locty = newlocty; 

    for (busy_locty = 0; busy_locty < TPM_TIS_NUM_LOCALITIES; busy_locty++) {
        if (this->local[busy_locty].state == TPM_TIS_STATE_EXECUTION) {
            TPM2Cancel();
            //tpm_backend_cancel_cmd(this->be_driver);
            return;
        }
    }

    tpm_tis_abort();
}


/*
 * Send a request to the TPM.
 */
 /*
static void tpm_tis_tpm_send(uint8_t i)
{
    //tpm_util_show_buffer(s->buffer, s->be_buffer_size, "To TPM");
    this->local[i].state = TPM_TIS_STATE_EXECUTION;

    s->cmd = (TPMBackendCmd) {
        .locty = i,
        .in = s->buffer,
        .in_len = s->rw_offset,
        .out = s->buffer,
        .out_len = s->be_buffer_size,
    };

    tpm_backend_deliver_request(s->be_driver, &s->cmd);
}
*/
void bx_tpm2_c::write(bx_phy_address addr, void *data, unsigned len)
{
    uint32_t i;

    for (i = 0; i < len; i += 8)
        write8(addr, (void *)((uint8_t *)data + i), (len > 8) ? 8 : len);
}

void bx_tpm2_c::write8(bx_phy_address addr, void *data, unsigned len)
{
    uint8_t shift, active_local, l, newlocty;
    int c, set_new_locty;
    uint64_t val, mask, status;
    Bit8u i;

    set_new_locty = 1;
    i = (uint8_t)((addr >> TPM_TIS_LOCALITY_SHIFT) & 0x7);
    if (i >= TPM_TIS_NUM_LOCALITIES - 1)
        return;
    shift = (addr & 0x3) * 8;
    mask = (len == 1) ? 0xff : ((len == 2) ? 0xffff : ~0ULL);
    switch (len) {
        case 1:
            val = *(uint8_t*)data;
            break;
        case 2:
            val = *(uint16_t*)data;
            break;
        case 4:
            val = *(uint32_t*)data;
            break;
        case 8:
            val = *(uint64_t*)data;
    }
    val &= mask;

    if (shift) {
        val <<= shift;
        mask <<= shift;
    }

    mask ^= 0xffffffff;

    switch (addr & 0xffc) {
    case TPM_TIS_REG_ACCESS:

        if ((val & TPM_TIS_ACCESS_SEIZE)) {
            val &= ~(TPM_TIS_ACCESS_REQUEST_USE |
                     TPM_TIS_ACCESS_ACTIVE_LOCALITY);
        }

        active_local = this->active_local;

        if ((val & TPM_TIS_ACCESS_ACTIVE_LOCALITY)) {
            /* give up locality if currently owned */
            if (this->active_local == i) {

                newlocty = TPM_TIS_NO_LOCALITY;
                /* anybody wants the locality ? */
                for (c = TPM_TIS_NUM_LOCALITIES - 1; c >= 0; c--) {
                    if ((this->local[c].access & TPM_TIS_ACCESS_REQUEST_USE)) {
                        newlocty = c;
                        break;
                    }
                }

                if (TPM_TIS_IS_VALID_LOCTY(newlocty)) {
                    set_new_locty = 0;
                    tpm_tis_prep_abort(i, newlocty);
                } else {
                    active_local = TPM_TIS_NO_LOCALITY;
                }
            } else {
                /* not currently the owner; clear a pending request */
                this->local[i].access &= ~TPM_TIS_ACCESS_REQUEST_USE;
            }
        }

        if ((val & TPM_TIS_ACCESS_BEEN_SEIZED)) {
            this->local[i].access &= ~TPM_TIS_ACCESS_BEEN_SEIZED;
        }

        if ((val & TPM_TIS_ACCESS_SEIZE)) {
            /*
             * allow seize if a locality is active and the requesting
             * locality is higher than the one that's active
             * OR
             * allow seize for requesting locality if no locality is
             * active
             */
            while ((TPM_TIS_IS_VALID_LOCTY(this->active_local) &&
                    i > this->active_local) ||
                    !TPM_TIS_IS_VALID_LOCTY(this->active_local)) {
                bool higher_seize = false;

                /* already a pending SEIZE ? */
                if ((this->local[i].access & TPM_TIS_ACCESS_SEIZE)) {
                    break;
                }

                /* check for ongoing seize by a higher locality */
                for (l = i + 1; l < TPM_TIS_NUM_LOCALITIES; l++) {
                    if ((this->local[l].access & TPM_TIS_ACCESS_SEIZE)) {
                        higher_seize = true;
                        break;
                    }
                }

                if (higher_seize) {
                    break;
                }

                /* cancel any seize by a lower locality */
                for (l = 0; l < i; l++) {
                    this->local[l].access &= ~TPM_TIS_ACCESS_SEIZE;
                }

                this->local[i].access |= TPM_TIS_ACCESS_SEIZE;

                set_new_locty = 0;
                tpm_tis_prep_abort(this->active_local, i);
                break;
            }
        }

        if ((val & TPM_TIS_ACCESS_REQUEST_USE)) {
            if (this->active_local != i) {
                if (TPM_TIS_IS_VALID_LOCTY(this->active_local)) {
                    this->local[i].access |= TPM_TIS_ACCESS_REQUEST_USE;
                } else {
                    /* no locality active -> make this one active now */
                    active_local = i;
                }
            }
        }

        if (set_new_locty) 
            tpm_tis_new_active_locality(active_local);
        

        break;
    case TPM_TIS_REG_INT_ENABLE:
        this->local[i].int_enabled &= mask;
        this->local[i].int_enabled |= (val & (TPM_TIS_INT_ENABLED |
                                        TPM_TIS_INT_POLARITY_MASK |
                                        TPM_TIS_INTERRUPTS_SUPPORTED));
        break;
    case TPM_TIS_REG_INT_VECTOR:
        /* hard wired -- ignore */
        break;
    case TPM_TIS_REG_INT_STATUS:
        /* clearing of interrupt flags */
        if (((val & TPM_TIS_INTERRUPTS_SUPPORTED)) &&
            (this->local[i].int_status & TPM_TIS_INTERRUPTS_SUPPORTED)) {
            this->local[i].int_status &= ~val;
            if (this->local[i].int_status == 0) 
                DEV_pic_lower_irq(this->irq_num);
            
        }
        this->local[i].int_status &= ~(val & TPM_TIS_INTERRUPTS_SUPPORTED);
        break;
    case TPM_TIS_REG_STS:
        if (this->active_local!= i) {
            break;
        }

        if (this->tpm_version == TPM_VERSION_2_0) {
            /* some flags that are only supported for TPM 2 */
            if (val & TPM_TIS_STS_COMMAND_CANCEL) {
                if (this->local[i].state == TPM_TIS_STATE_EXECUTION) {
                    /*
                     * request the backend to cancel. Some backends may not
                     * support it
                     */
                    TPM2Cancel();
                    //tpm_backend_cancel_cmd(this->be_driver);
                }
            }

            if (val & TPM_TIS_STS_RESET_ESTABLISHMENT_BIT) {
                if (i == 3 || i == 4) {
                    //tpm_backend_reset_tpm_established_flag(this->be_driver, i);
                }
            }
        }

        val &= (TPM_TIS_STS_COMMAND_READY | TPM_TIS_STS_TPM_GO |
                TPM_TIS_STS_RESPONSE_RETRY);

        if (val == TPM_TIS_STS_COMMAND_READY) {
            switch (this->local[i].state) {

            case TPM_TIS_STATE_READY:
                this->rw_offset = 0;
            break;

            case TPM_TIS_STATE_IDLE:
                tpm_tis_sts_set(&this->local[i], TPM_TIS_STS_COMMAND_READY);
                this->local[i].state = TPM_TIS_STATE_READY;
                tpm_tis_raise_irq(i, TPM_TIS_INT_COMMAND_READY);
            break;

            case TPM_TIS_STATE_EXECUTION:
            case TPM_TIS_STATE_RECEPTION:
                /* abort currently running command */
                tpm_tis_prep_abort(i, i);
            break;

            case TPM_TIS_STATE_COMPLETION:
                this->rw_offset = 0;
                /* shortcut to ready state with C/R set */
                this->local[i].state = TPM_TIS_STATE_READY;
                if (!(this->local[i].sts & TPM_TIS_STS_COMMAND_READY)) {
                    
                    tpm_tis_sts_set(&this->local[i],
                                    TPM_TIS_STS_COMMAND_READY);
                    tpm_tis_raise_irq(i, TPM_TIS_INT_COMMAND_READY);
                    
                }
                this->local[i].sts &= ~(TPM_TIS_STS_DATA_AVAILABLE);
            break;

            }
        } else if (val == TPM_TIS_STS_TPM_GO) {
            switch (this->local[i].state) {
            case TPM_TIS_STATE_RECEPTION:
                if ((this->local[i].sts & TPM_TIS_STS_EXPECT) == 0) {
                    //tpm_tis_tpm_send(i);
                    this->local[i].state = TPM_TIS_STATE_EXECUTION;
                    status = TPM2Send(this->buffer, this->rw_offset);
                    if (status) {

                    }
                    
                }
                break;
            default:
                /* ignore */
                break;
            }
        } else if (val == TPM_TIS_STS_RESPONSE_RETRY) {
            switch (this->local[i].state) {
            case TPM_TIS_STATE_COMPLETION:
                this->rw_offset = 0;
                
                tpm_tis_sts_set(&this->local[i],
                                TPM_TIS_STS_VALID|
                                TPM_TIS_STS_DATA_AVAILABLE);
                
                break;
            default:
                /* ignore */
                break;
            }
        }
        break;
    case TPM_TIS_REG_DATA_FIFO:
    case TPM_TIS_REG_DATA_XFIFO:
    case TPM_TIS_REG_DATA_XFIFO_END:
        /* data fifo */
        if (this->active_local != i) {
            break;
        }

        if (this->local[i].state == TPM_TIS_STATE_IDLE ||
            this->local[i].state == TPM_TIS_STATE_EXECUTION ||
            this->local[i].state == TPM_TIS_STATE_COMPLETION) {
            /* drop the byte */
        } else {
            if (this->local[i].state == TPM_TIS_STATE_READY) {
                this->local[i].state = TPM_TIS_STATE_RECEPTION;
                
                tpm_tis_sts_set(&this->local[i],
                                TPM_TIS_STS_EXPECT | TPM_TIS_STS_VALID);
                
            }

            val >>= shift;
            l = len;
            /*
            if (l > 4 - (addr & 0x3)) {
                l = 4 - (addr & 0x3);
            }
            */
            while ((this->local[i].sts & TPM_TIS_STS_EXPECT) && l > 0) {
                if (this->rw_offset < this->buffer_size) {
                    this->buffer[this->rw_offset++] = (uint8_t)val;
                    val >>= 8;
                    l--;
                } else {
                    tpm_tis_sts_set(&this->local[i], TPM_TIS_STS_VALID);
                }
            }

            /* check for complete packet */
            if (this->rw_offset > 5 &&
                (this->local[i].sts & TPM_TIS_STS_EXPECT)) {
                /* we have a packet length - see if we have all of it */
                bool need_irq = !(this->local[i].sts & TPM_TIS_STS_VALID);
                
                len = tpm_cmd_get_size((unsigned char *)this->buffer);
                if (len > this->rw_offset) {
                    tpm_tis_sts_set(&this->local[i],
                                    TPM_TIS_STS_EXPECT | TPM_TIS_STS_VALID);
                } else {
                    tpm_tis_sts_set(&this->local[i], TPM_TIS_STS_VALID);
                }
                if (need_irq) {
                    tpm_tis_raise_irq(i, TPM_TIS_INT_STS_VALID);
                }
                
            }
        }
        break;
    case TPM_TIS_REG_INTERFACE_ID:
        if (val & TPM_TIS_IFACE_ID_INT_SEL_LOCK) {
            for (l = 0; l < TPM_TIS_NUM_LOCALITIES; l++) {
                this->local[l].iface_id |= TPM_TIS_IFACE_ID_INT_SEL_LOCK;
            }
        }
        break;
    }
}


#endif