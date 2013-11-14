//
#include <SystemTypes.h>
#include <Utility.h>

#include <CameraPriv.h>


LF_BEGIN_BRIO_NAMESPACE()
//==============================================================================
// Defines
//==============================================================================

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CEmulCameraModule::GetModuleVersion() const
{
	return kEmulCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CEmulCameraModule::GetModuleName() const
{
	return &kEmulCameraModuleName;
}


//----------------------------------------------------------------------------


//============================================================================
// Ctor & dtor
//============================================================================
CEmulCameraModule::CEmulCameraModule()
{
	tCaptureMode VGA = {kCaptureFormatRAWYUYV, 640, 480, 1, 25};
	// Enumerate camera modes to match preferred default
	camCtx_.mode = VGA;
	valid = InitCameraInt(&camCtx_.mode, false);
}

//----------------------------------------------------------------------------
CEmulCameraModule::~CEmulCameraModule()
{
	DeinitCameraInt( true );
}

//----------------------------------------------------------------------------
tVidCapHndl CEmulCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	tVidCapHndl hndl = kInvalidVidCapHndl;
	videoSurface_ = *pSurf;
	// Re-init V4L device, as needed per instance
	if (camCtx_.fd == -1)
		InitCameraInt(&camCtx_.mode, true);

	hndl = CCameraModule::StartVideoCapture(path, &videoSurface_, pListener, maxLength, bAudio);

	return hndl;
}
LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CEmulCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CEmulCameraModule;
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
