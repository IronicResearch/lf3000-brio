#ifndef __BLUETOPIAIO_H__
#define __BLUETOPIAIO_H__

#include <BTAddr.h>
#include <BTIOTypes.h>

typedef int (*pFnInit)( void* );
typedef int (*pFnExit)( int );
typedef int (*pFnSendCommand)( int, int, void*, int );
typedef int (*pFnQueryStatus)( int, int, void*, int );
typedef int (*pFnScanForDevices)( int, int );
typedef int (*pFnConnectToDevice)( int, const BTAddr* );
typedef BTAddr* (*pFnGetLocalAddress)( int ); 

typedef void (*pFnCallback)( void*, void*, int );

#ifdef USE_EXPORTS

extern pFnInit					pBTIO_Init;
extern pFnExit					pBTIO_Exit;
extern pFnSendCommand			pBTIO_SendCommand;
extern pFnQueryStatus			pBTIO_QueryStatus;
extern pFnScanForDevices		pBTIO_ScanForDevices;
extern pFnConnectToDevice		pBTIO_ConnectToDevice;
extern pFnGetLocalAddress		pBTIO_GetLocalAddress;

#define BTIO_Init   			pBTIO_Init
#define BTIO_Exit   			pBTIO_Exit
#define BTIO_SendCommand		pBTIO_SendCommand
#define BTIO_QueryStatus		pBTIO_QueryStatus
#define BTIO_ScanForDevices		pBTIO_ScanForDevices
#define BTIO_ConnectToDevice		pBTIO_ConnectToDevice
#define BTIO_GetLocalAddress		pBTIO_GetLocalAddress

#else

extern "C" {

int BTIO_Init(void* callback);
int BTIO_Exit(int handle);
int BTIO_SendCommand(int handle, int command, void* data, int length);
int BTIO_QueryStatus(int handle, int command, void* data, int length);
int BTIO_ScanForDevices(int handle, int scan_time);
int BTIO_ConnectToDevice(int handle, const BTAddr* device);
BTAddr* BTIO_GetLocalAddress(int handle);

};

#endif // USE_EXPORTS

#endif // __BLUETOPIAIO_H__
