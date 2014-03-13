#ifndef __BLUETOPIAIOTYPES_H__
#define __BLUETOPIAIOTYPES_H__

///< Enumerated commands for BTIO_SendCommand()
enum {
	kBTIOCmdSetScanCallback,
	kBTIOCmdSetDeviceCallback,
	kBTIOCmdSetInputCallback,
	kBTIOCmdSetUpdateRate,
	kBTIOCmdSetLEDState,
	kBTIOCmdSetControllerMode,
	kBTIOCmdSetAccelerometerMode,
	kBTIOCmdSetAnalogStickMode,
	kBTIOCmdSetAnalogStickDeadZone,
	kBTIOCmdSetInputContext,
};

///< Enumerated commands for BTIO_QueryStatus()
enum {
	kBTIOCmdGetNumControllers,
	kBTIOCmdGetControllerCaps,
	kBTIOCmdGetLEDCaps,
	kBTIOCmdGetButtonCaps,
	kBTIOCmdGetUpdateRate,
	kBTIOCmdGetLEDState,
	kBTIOCmdGetControllerMode,
	kBTIOCmdGetAccelerometerMode,
	kBTIOCmdGetAnalogStickMode,
	kBTIOCmdGetAnalogStickDeadZone,
	kBTIOCmdGetButtonData,
	kBTIOCmdGetAccelerometerData,
	kBTIOCmdGetAnalogStickData,
};


#endif // __BLUETOPIAIOTYPES_H__
