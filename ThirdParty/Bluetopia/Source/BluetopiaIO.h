#ifndef __BLUETOPIAIO_H__
#define __BLUETOPIAIO_H__

typedef int (*pFnInit)( void* );
typedef int (*pFnExit)( int );
typedef int (*pFnSendCommand)( int, int, void*, int );
typedef int (*pFnQueryStatus)( int, int, void*, int );

#ifdef USE_EXPORTS

extern pFnInit					pBTIO_Init;
extern pFnExit					pBTIO_Exit;
extern pFnSendCommand			pBTIO_SendCommand;
extern pFnQueryStatus			pBTIO_QueryStatus;

#define BTIO_Init   			pBTIO_Init
#define BTIO_Exit   			pBTIO_Exit
#define BTIO_SendCommand		pBTIO_SendCommand
#define BTIO_QueryStatus		pBTIO_QueryStatus

#else

int BTIO_Init(void* callback);
int BTIO_Exit(int handle);
int BTIO_SendCommand(int handle, int command, void* data, int length);
int BTIO_QueryStatus(int handle, int command, void* data, int length);

#endif // USE_EXPORTS

#endif // __BLUETOPIAIO_H__
