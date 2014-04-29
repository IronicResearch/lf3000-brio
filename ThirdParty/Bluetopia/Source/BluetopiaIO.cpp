#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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

static bool                nopairing = true;

bool BTIO_IgnorePairing(void)
{
#if USE_PAIRED_MODE
	struct stat stbuf;
	return (0 == stat("/flags/nopairing", &stbuf));
#else
	return true;
#endif
}

int BTIO_QueryForServices(int handle, BTAddr* device, bool update);

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
//					BTIO_QueryForServices(DEVMCallbackID, device, true);
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
		case detRemoteDeviceServicesStatus:
			printf("Remote Device Services Status\n");
			device = BTAddr::fromByteArray((const char*)&EventData->EventData.RemoteDeviceServicesStatusEventData.RemoteDeviceAddress);
//			BTIO_QueryForServices(DEVMCallbackID, device, false);
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
		case getGATTWriteRequest:
			printf("GATT Write Request\n");
			break;
		case getGATTWriteResponse:
			printf("GATT Write Response\n");
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

	r = DEVM_PowerOnDevice();
	printf("DEVM_PowerOnDevice() returned %d\n", r);

	callbackobj = callback;
	// Wait until callbacks are all set before scanning for devices

	nopairing = BTIO_IgnorePairing();

	return DEVMCallbackID;
}

int BTIO_Exit(int handle)
{
	printf("%s: %d\n", __func__, handle);

	// Disable further callbacks
	callbackfunc = NULL;
	callbackmain = NULL;

//	DEVM_PowerOffDevice();
//	DEVM_StopDeviceScan();

	if (DEVMCallbackID)
		DEVM_UnRegisterEventCallback(DEVMCallbackID);

	if (GATMCallbackID)
		GATM_UnRegisterEventCallback(GATMCallbackID);

	BTPM_Cleanup();
	return 0;
}

int BTIO_QueryForServices(int handle, BTAddr* device, bool update)
{
	BD_ADDR_t       BDADDR;
	unsigned int    TotalServiceSize = 0;
	unsigned int    ServiceSize;
	unsigned char*  ServiceData = NULL;
	unsigned long   QueryFlags = DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY; // | DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE;
    int r;

	if (!device)
		return -1;
	device->toByteArray((char*)&BDADDR);

	// 1st pass needs to force update before 2nd pass queries services
	if (update)
		QueryFlags |= DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE;
	r = DEVM_QueryRemoteDeviceServices(BDADDR, QueryFlags, 0, NULL, &TotalServiceSize);
	printf("DEVM_QueryRemoteDeviceServices() returned %d, size=%d\n", r, TotalServiceSize);
	if (update)
		return r;

	if (TotalServiceSize == 0)
		TotalServiceSize = 5000;
	ServiceSize = TotalServiceSize;
	ServiceData = (unsigned char *)BTPS_AllocateMemory(ServiceSize);

	QueryFlags &= ~DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE;
	r = DEVM_QueryRemoteDeviceServices(BDADDR, QueryFlags, ServiceSize, ServiceData, &TotalServiceSize);
	printf("DEVM_QueryRemoteDeviceServices() returned %d, size=%d\n", r, ServiceSize);

	// TODO: Parse GATT services for magic attribute handle 0x0029
	DEVM_Parsed_Services_Data_t  ParsedGATTData;
	DEVM_ConvertRawServicesStreamToParsedServicesData(r, ServiceData, &ParsedGATTData);
	DEVM_FreeParsedServicesData(&ParsedGATTData);

	BTPS_FreeMemory(ServiceData);

	return r;
}

int BTIO_WriteValue(int handle, int command, void* data, int length, char* addr)
{
	BD_ADDR_t BDADDR;
	Byte_t value = *(Byte_t*)data;

	if (addr) {
		memcpy(&BDADDR, addr, sizeof(BDADDR));
	}
	else if (device) {
		device->toByteArray((char*)&BDADDR);
	}

	int r = GATM_WriteValue(GATMCallbackID, BDADDR, command, sizeof(value), (Byte_t*)&value);
	printf("GATM_WriteValue() returned %d for %02x\n", r, value);
	return r;
}

int BTIO_SendCommand(int handle, int command, void* data, int length, char* addr)
{
	printf("%s: %d: %d, %p, %d\n", __func__, handle, command, data, length);

	switch (command) {
	case kBTIOCmdSetScanCallback:
		callbackscan = (pFnCallback)data;
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
		return BTIO_WriteValue(handle, 0x0029, data, length, addr);
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

int BTIO_QueryStatus(int handle, int command, void* data, int length, char* addr)
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
		if (!strstr(prop.DeviceName, "LeapTV") && !strstr(prop.DeviceName, "Simple"))
			continue;
#if USE_PAIRED_MODE
		if (!(prop.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE))
			if (!nopairing)
				continue;
#endif
		printf("%s: %d: %s, stats=%08x, name=%s\n", __func__, i, addr->toString().c_str(), (unsigned int)prop.RemoteDeviceFlags, prop.DeviceName);

		if (!(prop.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) {
			flags |= DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_FORCE_UPDATE;
			DEVM_QueryRemoteDeviceProperties(BD_ADDRList[i], flags, &prop);
		}

		if ((prop.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) {
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

//	if (BTPM_ERROR_CODE_DEVICE_DISCOVERY_IN_PROGRESS == r)
		return BTIO_QueryForDevices(handle);
	return r;
}

int BTIO_ConnectToDevice(int handle, const BTAddr* device)
{
	BD_ADDR_t     BD_ADDR;
	unsigned long ConnectFlags = DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY;

	printf("%s: %d: %s\n", __func__, handle, device->toString().c_str());

#if USE_PAIRED_MODE
	device->toByteArray((char*)&BD_ADDR);
	DEVM_Remote_Device_Properties_t prop;
	unsigned int flags = DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY;
	int q = DEVM_QueryRemoteDeviceProperties(BD_ADDR, flags, &prop);
	if (0 == q) {
		if (!strstr(prop.DeviceName, "LeapTV") && !strstr(prop.DeviceName, "Simple")) {
			printf("DEVM_QueryRemoteDeviceProperties() returned %d for %s\n", q, prop.DeviceName);
			return q;
		}
		if (!(prop.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)) {
			printf("DEVM_QueryRemoteDeviceProperties() returned %d for %s, unpaired=%08x\n", q, prop.DeviceName, (unsigned)prop.RemoteDeviceFlags);
			if (!nopairing)
				return q;
		}
	}
#endif

	device->toByteArray((char*)&BD_ADDR);
	int r = DEVM_ConnectWithRemoteDevice(BD_ADDR, ConnectFlags);
	printf("DEVM_ConnectWithRemoteDevice() returned %d\n", r);
	return r;
}

BTAddr* BTIO_GetLocalAddress(int handle)
{
	BTAddr* addr = NULL;
	DEVM_Local_Device_Properties_t prop;

	printf("%s: %d\n", __func__, handle);
	int r = DEVM_QueryLocalDeviceProperties(&prop);
	if (0 == r) {
		addr = BTAddr::fromByteArray((const char*)&prop.BD_ADDR);
		return addr;
	}
	return NULL;
}

int BTIO_GetControllerVersion(const char* device, unsigned char* hwVersion, unsigned short* fwVersion)
{
	BD_ADDR_t address;											//MAC address of the desired controller
	Advertising_Data_t adData;									//Advertising data response
	Advertising_Data_t scanResponseData;						//The scan response data
	const int maxScanResponseDataEntries = 10;					//Maximum number of expected scan response entries
	bool versionFound = false;									//Has the version number information been found?

	ASSIGN_BD_ADDR(address, device[5], device[4], device[3], device[2], device[1], device[0]);

	int dataResult = DEVM_QueryRemoteDeviceAdvertisingData(address, &adData, &scanResponseData);

	/*
	 //This code block is for debugging leaving it here as it will be very helpful if the packet ever changes
	printf("****Query remote device data returned %d\r\n", dataResult);
	printf("****Scan response data packet:");
	for(int count = 0; count < ADVERTISING_DATA_MAXIMUM_SIZE; count++)
	{
		printf("%u:", unsigned(scanResponseData.Advertising_Data[count]));
	}
	printf("\r\n");
	 */
	if(dataResult == 2)
	{
		const Byte_t GAP_ADTYPE_MANUFACTURER_SPECIFIC  = 0xff;				//Identifier tag for the entry containing the version number information
		const Byte_t GAP_ADTYPE_MANUFACTURER_DATA_LENGTH = 0x06;			//Expected length for the entry containing the version number information

		//Walk the data structure looking for the GAP_ADTYPE_MANUFACTURER_SPECIFIC block
		Byte_t *pCurrentDataBlock = &(scanResponseData.Advertising_Data[0]);		//Pointer into the data block to be searched for the version number
		while(
				(pCurrentDataBlock - scanResponseData.Advertising_Data) < SCAN_RESPONSE_DATA_MAXIMUM_SIZE
				&& pCurrentDataBlock[0]
			 )
		{
			if(pCurrentDataBlock[0] != GAP_ADTYPE_MANUFACTURER_DATA_LENGTH ||
			   pCurrentDataBlock[1] != GAP_ADTYPE_MANUFACTURER_SPECIFIC)
			{
				pCurrentDataBlock += (pCurrentDataBlock[0] + 1);
				continue;
			}

			//Found the correct block, extract the version numbers
			*hwVersion = pCurrentDataBlock[4];
			*fwVersion = (unsigned short)(pCurrentDataBlock[5]) * 256 + pCurrentDataBlock[6];
			versionFound = true;
			break;
		}
	}

	return (versionFound ? 0 : 1);
}

int BTIO_DeleteRemoteDevice(const char *device)
{
	BD_ADDR_t address;											//MAC address of the desired controller

	if (!device)
		return BTPM_ERROR_CODE_INVALID_PARAMETER;

	ASSIGN_BD_ADDR(address, device[5], device[4], device[3], device[2], device[1], device[0]);

	int result = DEVM_DeleteRemoteDevice(address);

	return result;
}

int BTIO_EnableBluetoothDebug(bool enable, unsigned int type, unsigned long flags, const char *filename)
{
	unsigned int nameLength = 0;			//Current length of the filename, if present

	if(filename)
	{
		nameLength = strlen(filename) + 1;
	}

	int result = DEVM_EnableBluetoothDebug((enable ? 1 : 0), type, flags, nameLength, (unsigned char *)(filename));

	return result;

}

