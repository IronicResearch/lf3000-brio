/******************************************************************************

 @Description  A very simple app that uses the Display

******************************************************************************/
#include <DisplayMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <EmulationConfig.h>
#include <FontMPI.h>
#include <ButtonMPI.h>
#include <KernelMPI.h>
LF_USING_BRIO_NAMESPACE()

#define WIDTH	300
#define HEIGHT	200
#define LEFT	10
#define TOP 	20
#define ALPHA	50

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
	return "/LF/Bulk/ProgramFiles/DisplayDemo/rsrc";
#endif	// EMULATION
}
	
//============================================================================
// Button Listener
//============================================================================

const U8	kAlphaStep = 10;
const S8	kBrightnessStep = 4;
const S8	kContrastStep = 16;

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
			alpha_ 		= ALPHA;
			brightness_ = disp_.GetBrightness(0);
			contrast_ 	= disp_.GetContrast(0);
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
					alpha_ += kAlphaStep;
					disp_.SetAlpha(layer_, alpha_, true); 
					break;
				case kButtonB:
					alpha_ -= kAlphaStep;
					disp_.SetAlpha(layer_, alpha_, true); 
					break;
				case kButtonUp:
					brightness_ += kBrightnessStep;
					disp_.SetBrightness(screen_, brightness_); 
					break;
				case kButtonDown:
					brightness_ -= kBrightnessStep;
					disp_.SetBrightness(screen_, brightness_); 
					break;
				case kButtonRight:
					contrast_ += kContrastStep;
					disp_.SetContrast(screen_, contrast_); 
					break;
				case kButtonLeft:
					contrast_ -= kContrastStep;
					disp_.SetContrast(screen_, contrast_); 
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
	U8				alpha_;
	S8				brightness_;
	S8				contrast_;
	CDebugMPI		dbg_;
	CDisplayMPI		disp_;
};

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	U8* 			buffer;
	tDisplayHandle 	handle;
	int 			offset = 0;
	int 			left = LEFT, top = TOP, width = WIDTH, height = HEIGHT;
	int 			alpha = ALPHA;
	int 			pitch;
	int				bpp,cpp;
	
	if(argc >= 5) {
		left = atoi(argv[1]);
		top  = atoi(argv[2]);
		width = atoi(argv[3]);
		height = atoi(argv[4]);
	}

	if(argc >= 6) {
		alpha = atoi(argv[5]);
	}

	// Access display manager
	CKernelMPI		kernel;
	CDisplayMPI 	displaymgr;
	CDebugMPI 		dbg(kMyApp);
	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "DisplayDemo -- buttons change brightness, contrast, or quit.\n");

	// Create display surface to draw on
	handle = displaymgr.CreateHandle(height, width, kPixelFormatARGB8888,NULL);
	displaymgr.Register(handle, left, top, 0, 0);
	buffer = displaymgr.GetBuffer(handle);

	// Need to know pitch for font surface descriptor
	pitch = displaymgr.GetPitch(handle);
	dbg.DebugOut(kDbgLvlVerbose, "the pitch is: %d bytes per line\n", pitch);
	bpp = displaymgr.GetDepth(handle);
	cpp = bpp / 8;
	dbg.DebugOut(kDbgLvlVerbose, "the depth is: %d bits per pixel (%d bytes)\n", bpp, cpp);

	// Put graduated pixel pattern into draw surface buffer
	dbg.DebugOut(kDbgLvlVerbose, "drawing some stripes in a rectangle...\n");
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++, offset += cpp) {
			for(int p = 0; p < cpp; p++) {
				buffer[offset+p] = static_cast<unsigned char>((i % (HEIGHT/2)) * 0xFF);
			}
		}
	}

	// Set alpha channel for display surface transparency
	dbg.DebugOut(kDbgLvlVerbose, "setting alpha to %d%%\n", alpha);
	displaymgr.SetAlpha(handle, alpha, true);

	// Render font text strings into draw surface buffer
	CFontMPI			myFont;
	tFontSurf			mySurf = {width, height, pitch, buffer, kPixelFormatARGB8888};
	CString				myStr1 = CString("DidjPropBold.ttf");
	CString				myStr2 = CString("The Quick Brown Fox");
	CString				myStr3 = CString("Jumps Over the Lazy Dog");
	// Set font properties and drawing attributes
	myFont.SetFontResourcePath(GetAppRsrcFolder());
	myFont.LoadFont(myStr1, 18); // point size
	myFont.SetFontColor(0xFF44CCFF); // AA:RR:GG:BB
	myFont.DrawString(&myStr1, 0, 0, &mySurf);
	myFont.DrawString(&myStr2, 0, HEIGHT/2, &mySurf);
	myFont.DrawString(&myStr3, 0, HEIGHT/2+20, &mySurf);

	// Update display
	displaymgr.Invalidate(0, NULL);

	// Setup button event manager to change display attributes
	CButtonMPI			button;
	MyBtnEventListener	handler(handle);
	
	button.RegisterEventListener(&handler);
	while (!handler.IsDone())
		kernel.TaskSleep(1);			

	button.UnregisterEventListener(&handler);
	displaymgr.UnRegister(handle, 0);
	displaymgr.DestroyHandle(handle, false);
	return 0;
}
