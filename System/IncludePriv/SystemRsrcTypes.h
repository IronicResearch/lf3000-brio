#ifndef SystemRsrcType_H
#define SystemRsrcType_H
//==============================================================================
// Copyright (c) 2002-2003 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
// File:
//		SystemRsrcType.h
//
// Description:
//		System-defined resource types.
//==============================================================================

#include <SystemTypes.h>

//==============================================================================	   
// System resource type groups
//==============================================================================

enum {
	kSystemRsrcTypeGroupBoot = kFirstNumSpaceGroup,
	kSystemRsrcTypeGroupDeviceInfo,				// definitions of the physical device part (flash memory, ROM, etc)
	kSystemRsrcTypeGroupProductInfo, 			// product definition & compatibility info
	kSystemRsrcTypeGroupModules,
	kSystemRsrcTypeGroupLocales,
	kSystemRsrcTypeGroupDebug,
	kSystemRsrcTypeGroupKernel,				
	kSystemRsrcTypeGroupAudio,
	kSystemRsrcTypeGroupGraphics,
	kSystemRsrcTypeGroupStorage,
	kSystemRsrcTypeGroupUI,
	kSystemRsrcTypeGroupFlashPlayer,
	kSystemRsrcTypeGroupSysMgr,
	kSystemRsrcTypeGroupDisplay,			
	kSystemRsrcTypeGroupUsb,               	
};

#define MakeFirstSystemGroupRsrcType(group)  MakeRsrcType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)

//==============================================================================	   
// System resource types
//==============================================================================

enum {
	//------------------------------------------------------------------------------
	// Boot resources
	//------------------------------------------------------------------------------
	kBootRsrcBootSafeFcnTable = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupBoot),
	kBootRsrcProcessorBootFcn,
	kBootRsrcProcessorBootConfig,
	kBootRsrcSystemBootFcn,				   
	kBootRsrcSystemBaseMemoryMap,		  
	kBootRsrcMemoryInitFcn,			   	   
	kBootRsrcMemoryAllocationMap,
	kBootRsrcKernelBootFcn,				   
	kBootRsrcKernelBootConfig,			   
	kBootRsrcBootTaskDescriptor,		   
	kBootRsrcModuleInstallerFcn,		   
	kBootRsrcAppLauncherFcn,
	kBootRsrcICacheControlConfig,
	kBootRsrcICacheLockConfig,
	kBootRsrcDCacheControlConfig,
	kBootRsrcDCacheLockConfig,
	kBootRsrcBusConfig,

	//------------------------------------------------------------------------------
	// Modules resources
	//------------------------------------------------------------------------------
	kModuleRsrcModulesTable = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupModules),

	//------------------------------------------------------------------------------
	// Device Info resources
	//------------------------------------------------------------------------------
	// <kDeviceInfoRsrcxxxxx>	= MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupDeviceInfo),

	//------------------------------------------------------------------------------
	// Product Info resources		
	//------------------------------------------------------------------------------
	kProductInfoRsrcProductID	= MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupProductInfo),
	kProductInfoRsrcCompatabilityNeeded,
	kProductInfoRsrcCompatabilityProvided,
	kProductInfoRsrcCopyrightStr,
	kProductInfoRsrcApprovalStr,
	kProductInfoRsrcSystemBootEnableStr,
	kProductInfoRsrcProductVersionStr,					
	kProductInfoRsrcProductVersion,
	kProductInfoRsrcTestPoints,
	kProductInfoRsrcPartNumberStr,
	kProductInfoRsrcProductName,
	kProductInfoRsrcBuildTool,
	kProductInfoRsrcBuildUser,
	kProductInfoRsrcBuildMachine,
	kProductInfoRsrcBuildDate,
	kProductInfoRsrcBuildValidation,				// Hash of preceeding build information

	//------------------------------------------------------------------------------
	// Audio resources		
	//------------------------------------------------------------------------------
	kAudioRsrcCodecConfig = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupAudio),
	kAudioRsrcGen2CodecConfig,
	kAudioRsrcAGCTable,	
	kAudioRsrcInstrumentTable,
	kAudioRsrcCodecAudioTable,

	//------------------------------------------------------------------------------
	// Storage resources	
	//------------------------------------------------------------------------------
	kStorageRsrcStorageDescriptor  = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupStorage),
	kStorageRsrcFlashAccessTable,
	kStorageRsrcEEpromDeviceDescriptor,
	kStorageRsrcFlashDeviceDescriptor,
	kStorageRsrcFlashDeviceCfiData,

	//------------------------------------------------------------------------------
	// Debug resources
	//------------------------------------------------------------------------------
	kDebugRsrcDebugConfig = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupDebug),

	//------------------------------------------------------------------------------
	// UI resources
	//------------------------------------------------------------------------------
	kUIRsrcButtonConfig = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupUI),
	kUIRsrcButtonGridConfig,
	kUIRsrcLightPipeConfig,

	//------------------------------------------------------------------------------
	// FlashPlayer resources
	//------------------------------------------------------------------------------
	kFlashPlayerRsrcFlashConfig = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupFlashPlayer),
	kFlashPlayerRsrcFlashDebug,
	kFlashPlayerRsrcProcessEventsFcn,
	kFlashPlayerRsrcOnIdleFcn,
	kFlashPlayerRsrcASExtensionTable,
	kFlashPlayerRsrcASCallbackTable,
	kFlashPlayerRsrcDeviceFontNames,
	kFlashPlayerRsrcTerminateFcn,
	kFlashPlayerRsrcRegisterLFFunctions,	 // which LF Calls to register with the flash player.
	kFlashPlayerRsrcFlashMovieTable,
	kFlashPlayerRsrcFlashBitmapTable,

	//------------------------------------------------------------------------------
	// System Manager resources
	//------------------------------------------------------------------------------
	kSysMgrRsrcPowerDownConfig = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupSysMgr),
	kSysMgrRsrcIdleTimerConfig,
	kSysMgrRsrcBatteryMonitorConfig,
	kSysMgrRsrcClockThrottlingFuncs,

	//------------------------------------------------------------------------------
	// Graphics resources
	//------------------------------------------------------------------------------
	kGraphicsRsrcBitmapFontTable = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupGraphics),

	//------------------------------------------------------------------------------
	// Tv resources
	//------------------------------------------------------------------------------
	kTvRsrcConfig			= MakeFirstSystemGroupRsrcType(0),  // FIXME/dg: need real group
	kTvRsrcDefaultCursor,
	kTvRsrcColorTable,
	kTvRsrcDisableScreenProperties,

	//------------------------------------------------------------------------------
	// Usb resources	
	//------------------------------------------------------------------------------
	kUsbRsrcDeviceInfoDescriptor      = MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupUsb),

	//------------------------------------------------------------------------------
	// Locale resource types
	//------------------------------------------------------------------------------
	kLocaleRsrcLocaleMgrConfig	= MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupLocales),
	kLocaleRsrcSystemLocale,

	//------------------------------------------------------------------------------
	// Kernel resource types
	//------------------------------------------------------------------------------
	kKernelRsrcConfig	= MakeFirstSystemGroupRsrcType(kSystemRsrcTypeGroupKernel),
	kKernelRsrcTaskOverrideParameters,
	kKernelRsrcTaskPropertiesTable,
	kKernelRsrcSharableMemory,
};


#endif // SystemRsrcType_H

// EOF




