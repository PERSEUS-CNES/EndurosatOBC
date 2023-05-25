#ifndef __dl_ftdi_h
#define __dl_ftdi_h
#include <stdint.h>
#include "ftd2xx.h"


/*
La librairie ftd2xx ne fonctionne pas lorsqu'un l'appelle au sein d'un fils
Ainsi on la charge dynamiquement afin de pouvoir utiliser ses fonctions dans notre fils
*/

//creation de types definis pour l'import des fonctions 
typedef FT_STATUS  (*FT_OpenFunc)(int, FT_HANDLE *);

/*FT_STATUS WINAPI FT_Write(
		FT_HANDLE ftHandle,
		LPVOID lpBuffer,
		DWORD dwBytesToWrite,
		LPDWORD lpBytesWritten
		);*/FT_STATUS WINAPI FT_OpenEx(
		PVOID pArg1,
		DWORD Flags,
		FT_HANDLE *pHandle
		);
typedef FT_STATUS (*FT_WriteFunc)(FT_HANDLE, LPVOID, DWORD,LPDWORD);

/*FT_STATUS WINAPI FT_Read(
		FT_HANDLE ftHandle,
		LPVOID lpBuffer,
		DWORD dwBytesToRead,
		LPDWORD lpBytesReturned*/

typedef FT_STATUS (*FT_ReadFunc)(FT_HANDLE, LPVOID, DWORD,LPDWORD);

/*FT_STATUS WINAPI FT_ResetDevice(
		FT_HANDLE ftHandle
		);*/
typedef FT_STATUS (*FT_ResetDeviceFunc)(FT_HANDLE);

/*FT_STATUS WINAPI FT_SetBaudRate(
		FT_HANDLE ftHandle,
		ULONG BaudRate
		);*/
typedef FT_STATUS (*FT_SetBaudRateFunc)(FT_HANDLE, ULONG);

/*FT_STATUS WINAPI FT_SetDataCharacteristics(
		FT_HANDLE ftHandle,
		UCHAR WordLength,
		UCHAR StopBits,
		UCHAR Parity
		);*/
typedef FT_STATUS (*FT_SetDataCharacteristicsFunc)(FT_HANDLE, UCHAR, UCHAR, UCHAR);

/*FT_STATUS WINAPI FT_SetDtr(
		FT_HANDLE ftHandle
		);*/
typedef FT_STATUS (*FT_SetDtrFunc)(FT_HANDLE);

/*FT_STATUS WINAPI FT_SetFlowControl(
		FT_HANDLE ftHandle,
		USHORT FlowControl,
		UCHAR XonChar,
		UCHAR XoffChar
		);*/
typedef FT_STATUS (*FT_SetFlowControlFunc)(FT_HANDLE, USHORT, UCHAR,UCHAR);

/*FT_STATUS WINAPI FT_SetTimeouts(
		FT_HANDLE ftHandle,
		ULONG ReadTimeout,
		ULONG WriteTimeout
		);*/
typedef FT_STATUS (*FT_SetTimeoutsFunc)(FT_HANDLE, ULONG, ULONG);

/*FT_STATUS WINAPI FT_Purge(
		FT_HANDLE ftHandle,
		ULONG Mask
		);*/
typedef FT_STATUS (*FT_PurgeFunc)(FT_HANDLE, ULONG);

/*FT_STATUS WINAPI FT_GetQueueStatus(
		FT_HANDLE ftHandle,
		DWORD *dwRxBytes
		);*/
typedef FT_STATUS (*FT_GetQueueStatusFunc)(FT_HANDLE, DWORD);

/*FT_STATUS WINAPI FT_OpenEx(
		PVOID pArg1,
		DWORD Flags,
		FT_HANDLE *pHandle
		);*/
typedef FT_STATUS (*FT_OpenExFunc)(PVOID, DWORD, FT_HANDLE*);

//fonction permettant d'importer les fonctions necessaires
uint8_t load_func();
#endif