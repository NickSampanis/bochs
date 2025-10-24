#pragma once

#include <Windows.h>

typedef DWORD TSS2_RC;

#if BX_SUPPORT_TPM2
extern "C"  TSS2_RC TPM2Initialize(CHAR * Ip, WORD Port);
extern "C"  TSS2_RC TPM2Send(BYTE* Buffer, SIZE_T BufferSize);
extern "C"  TSS2_RC TPM2Receive(BYTE * Buffer, SIZE_T * BufferSize, ULONG Timeout);
extern "C"  TSS2_RC TPM2SetLocality(BYTE Locality);
extern "C"  TSS2_RC TPM2Cancel();
#else
extern "C" __declspec(dllexport) TSS2_RC TPM2Initialize(CHAR *Ip, WORD Port);
extern "C" __declspec(dllexport) TSS2_RC TPM2Send(BYTE *Buffer, SIZE_T BufferSize);
extern "C" __declspec(dllexport) TSS2_RC TPM2Receive(BYTE * Buffer, SIZE_T * BufferSize, ULONG Timeout);
extern "C" __declspec(dllexport) TSS2_RC TPM2SetLocality(BYTE Locality);
extern "C" __declspec(dllexport) TSS2_RC TPM2Cancel();

#endif
