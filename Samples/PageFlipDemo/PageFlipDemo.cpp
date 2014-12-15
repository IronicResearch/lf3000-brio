/******************************************************************************

 @Description  Page-flipping demo via Display MPI

******************************************************************************/
#include <DisplayMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <EmulationConfig.h>
#include <FontMPI.h>
#include <ButtonMPI.h>
#include <KernelMPI.h>
#include <stdio.h>
#include <string.h>
LF_USING_BRIO_NAMESPACE()

#define WIDTH	320
#define HEIGHT	240
#define LEFT	0
#define TOP 	0
#define BUFS	3 

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

//============================================================================
// Application Resource folder setup
//============================================================================
CPath GetAppRsrcFolder( )
{
#ifdef EMULATION
	return EmulationConfig::Instance().GetCartResourceSearchPath();
#else	// EMULATION
	return "/LF/Bulk/ProgramFiles/PageFlipDemo/rsrc/";
#endif	// EMULATION
}
	
//============================================================================
// Button Listener
//============================================================================

const tEventType kMyHandledTypes[] = { kAllButtonEvents };

//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
 public:
	MyBtnEventListener( tDisplayHandle handle ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),
		dbg_(kMyApp) 
		{
			screen_ 	= 0;
			layer_ 		= handle;
			isDone_ 	= false;
		}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			type_ = msg.GetEventType();
			const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
			data_ = btnmsg.GetButtonState();
			switch (data_.buttonState & data_.buttonTransition)
			{
				case kButtonPause:
				case kButtonMenu:
					isDone_ = true;
					break;
				case kButtonA:
					break;
				case kButtonB:
					break;
				case kButtonUp:
					break;
				case kButtonDown:
					break;
				case kButtonRight:
					break;
				case kButtonLeft:
					break;
			}
			return kEventStatusOKConsumed;
		}

	bool IsDone()
		{
			return isDone_;
		}

private:
	tEventType		type_;
	tButtonData		data_;
	tDisplayScreen	screen_;
	tDisplayHandle	layer_;
	bool			isDone_;
	CDebugMPI		dbg_;
	CDisplayMPI		disp_;
};

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	U8* 			buffer[BUFS];
	tDisplayHandle 		handle[BUFS];
	tPixelFormat		format;
	int 			width, height, depth, pitch;

	// Access display manager
	CKernelMPI		kernel;
	CDebugMPI 		dbg(kMyApp);
	dbg.SetDebugLevel(kDbgLvlVerbose);
	CDisplayMPI 		dispmgr;

	// Create multiple display surfaces of same size for page flipping 
	for (int i = 0; i < BUFS; i++)
	{
		handle[i] = dispmgr.CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, NULL);
		dispmgr.Register(handle[i], 0, 0, kDisplayOnTop);
		buffer[i] = dispmgr.GetBuffer(handle[i]);
		format = dispmgr.GetPixelFormat(handle[i]);
		width = dispmgr.GetWidth(handle[i]);
		pitch = dispmgr.GetPitch(handle[i]);
		depth = dispmgr.GetDepth(handle[i]);
		height = dispmgr.GetHeight(handle[i]);
	}

	// Setup font manager and surface descriptor
	CFontMPI		fontmgr;
	tFontSurf		surf = {width, height, pitch, buffer[0], format};
	tFontHndl		font;
	font = fontmgr.LoadFont(GetAppRsrcFolder() + "DidjPropBold.ttf", 24);
	fontmgr.SetFontColor(0xFFFF00FF);

	// Setup button event manager to change display attributes
	CButtonMPI		button;
	MyBtnEventListener	handler(handle[0]);
	button.RegisterEventListener(&handler);

	// Main rendering loop
	for (int i = 0; !handler.IsDone(); i++) 
	{
		char cstr[40];
		sprintf(cstr, "Quick Brown Fox %d", i);
		CString text = CString(cstr);
		memset(buffer[i%BUFS], 0, pitch * height);
		for (int y = 0; y < height; y++) 
		{
			for (int x = 0, m = y*pitch; x < width; x++, m+=4)
								{
				U8 val = (i+m) % 0xFF;
				U8* pbuf = buffer[i%BUFS];
				pbuf[m+0] = val;
				pbuf[m+1] = val;
				pbuf[m+2] = val;
				pbuf[m+3] = 0xFF;
			}
		}
		surf.buffer = buffer[i%BUFS];
		fontmgr.DrawString(&text, 0, i%height, &surf);
		dispmgr.SwapBuffers(handle[i%BUFS], false);
	}		

	button.UnregisterEventListener(&handler);
	fontmgr.UnloadFont(font);
	for (int i = 0; i < BUFS; i++)
	{
		dispmgr.UnRegister(handle[i], 0);
		dispmgr.DestroyHandle(handle[i], false);
	}
	return 0;
}
