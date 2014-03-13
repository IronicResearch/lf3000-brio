#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <BluetopiaIO.h>

extern "C" {
#include <SS1BTPM.h>
};

// Hack port of LinuxGATM_CLT.c
//#include "LinuxGATM_CLT.c"	// FIXME

static unsigned int        DEVMCallbackID = 0;
static unsigned int        GATMCallbackID = 0;

static BTAddr*             device = NULL;
static unsigned char       packet[256] = {'\0'};

// Callback function to ControllerMPI client
static pFnCallback         callbackfunc = NULL;
static pFnCallback         callbackmain = NULL;

// BTPM Server Un-Registration Callback function
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
	printf("%s: %p\n", __func__, CallbackParameter);
}

// BTPM Local Device Manager Callback function
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter)
{
//	printf("%s: %p, %p\n", __func__, EventData, CallbackParameter);

	if (EventData) {
		switch (EventData->EventType) {
		case detDevicePoweredOn:
			printf("Device Powered On\n");
			BTIO_ScanForDevices(DEVMCallbackID, 0);
			break;
		case detDevicePoweredOff:
			printf("Device Powered Off\n");
			break;
		case detRemoteDeviceFound:
			printf("Remote Device Found\n");
			device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties.BD_ADDR);
			BTIO_ConnectToDevice(DEVMCallbackID, device);
			if (callbackmain) {
				(*callbackmain)(CallbackParameter,
						device,
						sizeof(device));
			}
			break;
		case detRemoteDeviceDeleted:
			printf("Remote Device Deleted\n");
			break;
		case detRemoteDevicePropertiesChanged:
			printf("Remote Device Properties Changed\n");
			break;
		default:
			printf("%s: unhandled type %d, %p\n", __func__, EventData->EventType, CallbackParameter);
			break;
		}
	}
	else {
		printf("%s: invalid %p, %p\n", __func__, EventData, CallbackParameter);
	}
}

// GATM Manager Callback Function
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
//	printf("%s: %p, %p\n", __func__, EventData, CallbackParameter);

	if (EventData) {
		switch (EventData->EventType) {
		case getGATTConnected:
			printf("GATT Connection\n");
			break;
		case getGATTDisconnected:
			printf("GATT Disconnect\n");
			break;
		case getGATTHandleValueData:
//			printf("GATT Handle Value Data\n");
			// Compare incoming packet with cached packet to minimize callbacks
			if (0 == memcmp(packet, EventData->EventData.HandleValueDataEventData.AttributeValue, EventData->EventData.HandleValueDataEventData.AttributeValueLength))
				break;
			memcpy(packet, EventData->EventData.HandleValueDataEventData.AttributeValue, EventData->EventData.HandleValueDataEventData.AttributeValueLength);
			if (callbackfunc) {
				(*callbackfunc)(CallbackParameter,
						EventData->EventData.HandleValueDataEventData.AttributeValue,
						EventData->EventData.HandleValueDataEventData.AttributeValueLength);
			}
			break;
		default:
			printf("%s: unhandled type %d, %p\n", __func__, EventData->EventType, CallbackParameter);
			break;
		}
	}
	else {
		printf("%s: invalid %p, %p\n", __func__, EventData, CallbackParameter);
	}
}

int BTIO_Init(void* callback)
{
	printf("%s: %p\n", __func__, callback);

	int r = BTPM_Initialize(getpid(), NULL, NULL, NULL);
	printf("BTPM_Initialize() returned %d\n", r);

	r = DEVM_RegisterEventCallback(DEVM_Event_Callback, callback);
	printf("DEVM_RegisterEventCallback() returned %d\n", r);
	DEVMCallbackID = r;

#if 0
	r = GATM_RegisterEventCallback(GATM_Event_Callback, callback);
	printf("GATM_RegisterEventCallback() returned %d\n", r);
	GATMCallbackID = r;
#endif

	r = DEVM_PowerOnDevice();
	printf("DEVM_PowerOnDevice() returned %d\n", r);

	if (0 == r || BTPM_ERROR_CODE_LOCAL_DEVICE_ALREADY_POWERED_UP == r)
		r = BTIO_ScanForDevices(DEVMCallbackID, 0);

	return DEVMCallbackID;
}

int BTIO_Exit(int handle)
{
	printf("%s: %d\n", __func__, handle);

//	DEVM_PowerOffDevice();
	DEVM_StopDeviceScan();

	if (DEVMCallbackID)
		DEVM_UnRegisterEventCallback(DEVMCallbackID);

	if (GATMCallbackID)
		GATM_UnRegisterEventCallback(GATMCallbackID);

	BTPM_Cleanup();
	return 0;
}

int BTIO_SendCommand(int handle, int command, void* data, int length)
{
	printf("%s: %d: %d, %p, %d\n", __func__, handle, command, data, length);

	switch (command) {
	case kBTIOCmdSetScanCallback:
		break;
	case kBTIOCmdSetDeviceCallback:
		callbackmain = (pFnCallback)data;
		break;
	case kBTIOCmdSetInputCallback:
		callbackfunc = (pFnCallback)data;
		break;
	case kBTIOCmdSetInputContext:
		GATMCallbackID = GATM_RegisterEventCallback(GATM_Event_Callback, data);
		printf("GATM_RegisterEventCallback() returned %d\n", GATMCallbackID);
		return GATMCallbackID;
	case kBTIOCmdSetUpdateRate:
		break;
	case kBTIOCmdSetLEDState:
		break;
	case kBTIOCmdSetControllerMode:
		break;
	case kBTIOCmdSetAccelerometerMode:
		break;
	case kBTIOCmdSetAnalogStickMode:
		break;
	case kBTIOCmdSetAnalogStickDeadZone:
		break;
	default:
		return -1;
	}

	return 0;
}

int BTIO_QueryStatus(int handle, int command, void* data, int length)
{
	printf("%s: %d: %d, %p, %d\n", __func__, handle, command, data, length);

	switch (command) {
	case kBTIOCmdGetNumControllers:
		break;
	case kBTIOCmdGetControllerCaps:
		break;
	case kBTIOCmdGetLEDCaps:
		break;
	case kBTIOCmdGetButtonCaps:
		break;
	case kBTIOCmdGetUpdateRate:
		break;
	case kBTIOCmdGetLEDState:
		break;
	case kBTIOCmdGetControllerMode:
		break;
	case kBTIOCmdGetAccelerometerMode:
		break;
	case kBTIOCmdGetAnalogStickMode:
		break;
	case kBTIOCmdGetAnalogStickDeadZone:
		break;
	case kBTIOCmdGetButtonData:
		break;
	case kBTIOCmdGetAccelerometerData:
		break;
	case kBTIOCmdGetAnalogStickData:
		break;
	default:
		return -1;
	}

	return 0;
}

int BTIO_ScanForDevices(int handle, int scan_time)
{
	printf("%s: %d: %d\n", __func__, handle, scan_time);

	int r = DEVM_StartDeviceScan(scan_time);
	printf("DEVM_StartDeviceScan() returned %d\n", r);
	return r;
}

int BTIO_ConnectToDevice(int handle, const BTAddr* device)
{
	BD_ADDR_t     BD_ADDR;
	unsigned long ConnectFlags = DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY;

	printf("%s: %d: %s\n", __func__, handle, device->toString().c_str());

	device->toByteArray((char*)&BD_ADDR);
	int r = DEVM_ConnectWithRemoteDevice(BD_ADDR, ConnectFlags);
	printf("DEVM_ConnectWithRemoteDevice() returned %d\n", r);
	return r;
}

BTAddr* BTIO_GetLocalAddress(int handle)
{
	printf("%s: %d\n", __func__, handle);
	return NULL;
}
