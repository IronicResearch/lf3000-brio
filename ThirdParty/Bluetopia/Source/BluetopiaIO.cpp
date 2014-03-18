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
static pFnCallback2        callbackfunc = NULL;
static pFnCallback         callbackmain = NULL;
static pFnCallback         callbackscan = NULL;
static void*               callbackobj  = NULL;


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
			break;
		case detLocalDevicePropertiesChanged:
			printf("Local Device Properties Changed\n");
			break;
		case detRemoteDeviceDeleted:
			printf("Remote Device Deleted\n");
			break;
		case detRemoteDevicePropertiesChanged:
			printf("Remote Device Properties Changed\n");
			if (EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE) {
				if (EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE) {
					device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR);
					if (callbackmain) {
						char buf[7] = {'\0'};
						device->toByteArray(buf);
						buf[6] = '\0';
						(*callbackmain)(CallbackParameter, buf, sizeof(buf));
					}
				}
				else {
					device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR);
					BTIO_ConnectToDevice(DEVMCallbackID, device);
				}
			}
			break;
		case detRemoteDevicePropertiesStatus:
			printf("Remote Device Properties Status\n");
			if (EventData->EventData.RemoteDevicePropertiesStatusEventData.Success) {
				if (EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE) {
					device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.BD_ADDR);
					if (callbackmain) {
						char buf[7] = {'\0'};
						device->toByteArray(buf);
						buf[6] = '\0';
						(*callbackmain)(CallbackParameter, buf, sizeof(buf));
					}
				}
			}
			break;
		case detRemoteDeviceConnectionStatus:
			printf("Remote Device Connection Status\n");
			device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress);
			break;
		case detDeviceScanStarted:
			printf("Device Scan Started\n");
			break;
		case detDeviceScanStopped:
			printf("Device Scan Stopped\n");
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
				char buf[7] = {'\0'};
				memcpy(&buf, &EventData->EventData.HandleValueDataEventData.RemoteDeviceAddress, 6);
				buf[6] = '\0';
				(*callbackfunc)(CallbackParameter,
						EventData->EventData.HandleValueDataEventData.AttributeValue,
						EventData->EventData.HandleValueDataEventData.AttributeValueLength,
						buf);
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

	callbackobj = callback;
	if (0 == r || BTPM_ERROR_CODE_LOCAL_DEVICE_ALREADY_POWERED_UP == r)
		r = BTIO_ScanForDevices(DEVMCallbackID, 0);

	return DEVMCallbackID;
}

int BTIO_Exit(int handle)
{
	printf("%s: %d\n", __func__, handle);

//	DEVM_PowerOffDevice();
//	DEVM_StopDeviceScan();

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
		callbackscan = (pFnCallback)data;
//		BTIO_ScanForDevices(DEVMCallbackID, 0);
		break;
	case kBTIOCmdSetDeviceCallback:
		callbackmain = (pFnCallback)data;
		break;
	case kBTIOCmdSetInputCallback:
		callbackfunc = (pFnCallback2)data;
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

int BTIO_QueryForDevices(int handle)
{
	BD_ADDR_t         *BD_ADDRList;
	unsigned int       Filter = 0; //DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_CONNECTED;
	unsigned int       TotalNumberDevices = 0;
	Class_of_Device_t  ClassOfDevice = { 0, 0, 0 };

	int r = DEVM_QueryRemoteDeviceList(Filter, ClassOfDevice, 0, NULL, &TotalNumberDevices);
	printf("DEVM_QueryRemoteDeviceList() returned %d, devices = %d\n", r, TotalNumberDevices);
	if (r < 0)
		return r;

	BD_ADDRList = (BD_ADDR_t *)BTPS_AllocateMemory(sizeof(BD_ADDR_t) * TotalNumberDevices);

	r = DEVM_QueryRemoteDeviceList(Filter, ClassOfDevice, TotalNumberDevices, BD_ADDRList, NULL);

	for (unsigned int i = 0; i < TotalNumberDevices; i++) {
		DEVM_Remote_Device_Properties_t prop;
		unsigned int flags = DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY;
		BTAddr* addr = BTAddr::fromByteArray((const char*)&BD_ADDRList[i]);
		DEVM_QueryRemoteDeviceProperties(BD_ADDRList[i], flags, &prop);
		printf("%s: %d: %s, stats=%08x\n", __func__, i, addr->toString().c_str(), prop.RemoteDeviceFlags);
		if (!(prop.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) {
			flags |= DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_FORCE_UPDATE;
			DEVM_QueryRemoteDeviceProperties(BD_ADDRList[i], flags, &prop);
//			BTIO_ConnectToDevice(handle, addr);
		}
		else {
			if (callbackscan) {
				char buf[7] = {'\0'};
				addr->toByteArray(buf);
				buf[6] = '\0';
				(*callbackscan)(callbackobj, buf, sizeof(buf));
			}
		}
		delete addr;
	}

	BTPS_FreeMemory(BD_ADDRList);

	return r;
}

int BTIO_ScanForDevices(int handle, int scan_time)
{
	printf("%s: %d: %d\n", __func__, handle, scan_time);

	int r = DEVM_StartDeviceScan(scan_time);
	printf("DEVM_StartDeviceScan() returned %d\n", r);

	if (BTPM_ERROR_CODE_DEVICE_DISCOVERY_IN_PROGRESS == r)
		return BTIO_QueryForDevices(handle);
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

