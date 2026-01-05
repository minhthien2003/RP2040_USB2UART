#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "setupapi.lib")

// GUID của Ports (COM & LPT)
static const GUID GUID_DEVCLASS_PORTS =
{ 0x4d36e978, 0xe325, 0x11ce,
  { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

int main(void)
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA devInfo;
    DWORD i;

    hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_PORTS,
        NULL,
        NULL,
        DIGCF_PRESENT
    );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        printf("Failed to get device list\n");
        return 1;
    }

    printf("=========================================\n");
    printf("  RP2040 USB Dual CDC - UART Mapping\n");
    printf("=========================================\n\n");

    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

    for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); i++) {

        char friendlyName[256];
        char pnpId[512];
        DWORD size;

        // Lấy tên COM (USB Serial Device (COM12))
        if (!SetupDiGetDeviceRegistryPropertyA(
            hDevInfo,
            &devInfo,
            SPDRP_FRIENDLYNAME,
            NULL,
            (PBYTE)friendlyName,
            sizeof(friendlyName),
            &size))
            continue;

        // Lấy PNPDeviceID (chứa MI_00 / MI_02)
        if (!SetupDiGetDeviceRegistryPropertyA(
            hDevInfo,
            &devInfo,
            SPDRP_HARDWAREID,
            NULL,
            (PBYTE)pnpId,
            sizeof(pnpId),
            &size))
            continue;

        if (strstr(pnpId, "MI_00")) {
            printf("[UART0]  %s\n", friendlyName);
        }
        else if (strstr(pnpId, "MI_02")) {
            printf("[UART1]  %s\n", friendlyName);
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    printf("\n=========================================\n");
    system("pause");
    return 0;
}
