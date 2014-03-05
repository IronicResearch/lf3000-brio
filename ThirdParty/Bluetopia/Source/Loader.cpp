#include <stdio.h>
#include <dlfcn.h>

#undef   USE_EXPORTS
#include <BluetopiaIO.h>

pFnInit	    		pBTIO_Init = NULL;
pFnExit 			pBTIO_Exit = NULL;
pFnSendCommand		pBTIO_SendCommand = NULL;
pFnQueryStatus		pBTIO_QueryStatus = NULL;

void*  				dll = NULL;

int BTIO_Init(void* callback)
{
	dll = dlopen("libBluetopiaIO.so", RTLD_LAZY);
	if (dll == NULL) {
		printf("%s: dlopen failed to load libBluetopiaIO.so, error=%s\n", __func__, dlerror());
		return -1;
	}

	pBTIO_Init 			= (pFnInit)dlsym(dll, "BTIO_Init");
	pBTIO_Exit 			= (pFnExit)dlsym(dll, "BTIO_Exit");
	pBTIO_SendCommand 	= (pFnSendCommand)dlsym(dll, "BTIO_SendCommand");
	pBTIO_QueryStatus	= (pFnQueryStatus)dlsym(dll, "BTIO_QueryStatus");

	if (pBTIO_Init)
		return pBTIO_Init(callback);
	return -1;
}

int BTIO_Exit(int handle)
{
	if (pBTIO_Exit)
		return pBTIO_Exit(handle);
	return -1;
}

int BTIO_SendCommand(int handle, int command, void* data, int length)
{
	if (pBTIO_SendCommand)
		return pBTIO_SendCommand(handle, command, data, length);
	return -1;
}

int BTIO_QueryStatus(int handle, int command, void* data, int length)
{
	if (pBTIO_QueryStatus)
		return pBTIO_QueryStatus(handle, command, data, length);
	return -1;
}

#ifdef TEST_HARNESS

int main(void)
{
	int r =
	BTIO_Init(NULL);
	BTIO_Exit(r);

	return 0;
}

#endif // TEST_HARNESS
