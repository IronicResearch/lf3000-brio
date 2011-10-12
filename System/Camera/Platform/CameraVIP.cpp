#include <SystemTypes.h>

#include <CameraPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CVIPCameraModule::GetModuleVersion() const
{
	return kVIPCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CVIPCameraModule::GetModuleName() const
{
	return &kVIPCameraModuleName;
}

//============================================================================
// Ctor & dtor
//============================================================================
CVIPCameraModule::CVIPCameraModule()
{
	struct tCaptureMode XGA = {kCaptureFormatRAWYUYV, 1024, 768, 1, 15};

	valid = InitCameraInt(&XGA);
}

//----------------------------------------------------------------------------
CVIPCameraModule::~CVIPCameraModule()
{
}

LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CVIPCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CVIPCameraModule;
		return sinst;
	}

	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
		(void )ptr;	/* Prevent unused variable warnings. */
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG
