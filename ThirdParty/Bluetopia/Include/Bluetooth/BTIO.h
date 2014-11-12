#ifndef __BTIO_H__
#define __BTIO_H__

#define BTIO_LIB_NAME		"libBTIO.so"

#define USE_PAIRED_MODE		1	// for connecting to controllers paired with console

#include <Bluetooth/BTAddr.h>
#include <Bluetooth/BTIOTypes.h>
#include <Bluetooth/BTIOOad.h>

typedef int (*pFnInit)( void* );
typedef int (*pFnExit)( int );
typedef int (*pFnSendCommand)( int, int, void*, int, char* );
typedef int (*pFnQueryStatus)( int, int, void*, int, char* );
typedef int (*pFnScanForDevices)( int, int );
typedef int (*pFnConnectToDevice)( int, const BTAddr* );
typedef int (*pFnDisconnectDevice)(const char*, unsigned long);
typedef int (*pFnPairWithRemoteDevice)(int);
typedef BTAddr* (*pFnGetLocalAddress)( int ); 
typedef int (*pFnGetControllerVersion)( const char*, unsigned char*, unsigned short*);
typedef int (*pFnDeleteRemoteDevice)( const char *);
typedef int (*pFnEnableBluetoothDebug)( bool, unsigned int, unsigned long, const char*);
typedef int (*pFnSetDebugZoneMaskPID)(unsigned long);

typedef void (*pFnCallback)( void*, void*, int );
typedef void (*pFnCallback2)( void*, void*, int, char* );

#ifdef USE_EXPORTS

extern pFnInit					pBTIO_Init;
extern pFnExit					pBTIO_Exit;
extern pFnSendCommand			pBTIO_SendCommand;
extern pFnQueryStatus			pBTIO_QueryStatus;
extern pFnScanForDevices		pBTIO_ScanForDevices;
extern pFnConnectToDevice		pBTIO_ConnectToDevice;
extern pFnDisconnectDevice		pBTIO_DisconnectDevice;
extern pFnPairWithRemoteDevice		pBTIO_PairWithRemoteDevice;
extern pFnGetLocalAddress		pBTIO_GetLocalAddress;
extern pFnGetControllerVersion	pBTIO_GetControllerVersion;
extern pFnDeleteRemoteDevice	pBTIO_DeleteRemoteDevice;
extern pFnEnableBluetoothDebug	pBTIO_EnableBluetoothDebug;
extern pFnSetDebugZoneMaskPID	pBTIO_SetDebugZoneMaskPID;

#define BTIO_Init   			pBTIO_Init
#define BTIO_Exit   			pBTIO_Exit
#define BTIO_SendCommand		pBTIO_SendCommand
#define BTIO_QueryStatus		pBTIO_QueryStatus
#define BTIO_ScanForDevices		pBTIO_ScanForDevices
#define BTIO_ConnectToDevice		pBTIO_ConnectToDevice
#define BTIO_DisconnectDevice	pBTIO_DisconnectDevice
#define BTIO_GetLocalAddress		pBTIO_GetLocalAddress
#define BTIO_GetControllerVersion	pBTIO_GetControllerVersion
#define BTIO_DeleteRemoteDevice		pBTIO_DeleteRemoteDevice
#define BTIO_EnableBluetoothDebug	pBTIO_EnableBluetoothDebug
#define BTIO_PairWithRemoteDevice	pBTIO_PairWithRemoteDevice
#define BTIO_SetDebugZoneMaskPID 	pBTIO_SetDebugZoneMaskPID

#else

extern "C" {

int BTIO_Init(void* callback);
int BTIO_Exit(int handle);
int BTIO_SendCommand(int handle, int command, void* data, int length, char* addr);
int BTIO_QueryStatus(int handle, int command, void* data, int length, char* addr);
int BTIO_ScanForDevices(int handle, int scan_time);
int BTIO_ConnectToDevice(int handle, const BTAddr* device);
int BTIO_DisconnectDevice(const char* device, unsigned long flags);
int BTIO_PairWithRemoteDevice(int handle);
BTAddr* BTIO_GetLocalAddress(int handle);
int BTIO_GetControllerVersion(const char* device, unsigned char* hwVersion, unsigned short* fwVersion);
int BTIO_DeleteRemoteDevice(const char *device);
int BTIO_EnableBluetoothDebug(bool enable, unsigned int type, unsigned long flags, const char *filename);
int BTIO_SetDebugZoneMaskPID(unsigned long debugZoneMask);
};

#endif // USE_EXPORTS

#endif // __BTIO_H__
