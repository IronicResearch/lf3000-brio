#include <stdio.h>
#include <unistd.h>
#include <BluetopiaIO.h>

extern "C" {
#include <SS1BTPM.h>
};

int BTIO_Init(void* callback)
{
	printf("%s: %p\n", __func__, callback);
	int r = BTPM_Initialize(getpid(), NULL, NULL, NULL);
	printf("BTPM_Initialize() returned %d\n", r);
	return r;
}

int BTIO_Exit(int handle)
{
	printf("%s: %d\n", __func__, handle);
	BTPM_Cleanup();
	return 0;
}

int BTIO_SendCommand(int handle, int command, void* data, int length)
{
	printf("%s: %d: %d, %p, %d\n", __func__, handle, command, data, length);
	return 0;
}

int BTIO_QueryStatus(int handle, int command, void* data, int length)
{
	printf("%s: %d: %d, %p, %d\n", __func__, handle, command, data, length);
	return 0;
}

int BTIO_ScanForDevices(int handle, int scan_time)
{
	printf("%s: %d: %d\n", __func__, handle, scan_time);
	return 0;
}

int BTIO_ConnectToDevice(int handle, const BTAddr* device)
{
	printf("%s: %d: %s\n", __func__, handle, device->toString().c_str());
	return 0;
}

BTAddr* BTIO_GetLocalAddress(int handle)
{
	printf("%s: %d\n", __func__, handle);
	return NULL;
}

