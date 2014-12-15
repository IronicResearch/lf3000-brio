#include <VtsApp.h>
#include <CSystemData.h>
#include <AppManager.h>
#include <ShaderVideo.h>

#define DEFAULT_FONT_NAME 	"LFClove.ttf"
#define DEFAULT_FONT_SIZE	17

LF_USING_BRIO_NAMESPACE()

VtsApp::VtsApp(void *userData)
{
}

VtsApp::~VtsApp()
{
}

void VtsApp::Enter()
{
	mDisplayHandle = mDisplayMPI.CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
	mDisplayMPI.Register(mDisplayHandle, 0, 0, kDisplayOnTop, kDisplayScreenAllScreens);

	mEventMPI.RegisterEventListener(&mTouchEventQueue);

	mScreenBlitBuffer.setFromDisplayHandle(mDisplayHandle);

	tRect rect = {0,320,0,240};
	mScreenBlitBuffer.blitRect(rect, 0,0,0,255);
	rect.left = 10;
	rect.right = 150;
	rect.top = 10;
	rect.bottom = 50;
	mScreenBlitBuffer.blitRect(rect, 255,0,0,255);

	mFontMPI.LoadFont(CSystemData::Instance()->FindFont(DEFAULT_FONT_NAME), DEFAULT_FONT_SIZE);
	CString label = "Shader Video";
	mFontMPI.DrawString(&label, 20, 20, &(mScreenBlitBuffer.surface));

					boost::shared_ptr<ShaderVideo> shader_video(new ShaderVideo(&mTitleRootFolder));
					CAppManager::Instance()->PushApp(shader_video);

	mDisplayMPI.Invalidate(0);
}

void VtsApp::Update()
{
	std::vector<tTouchData> *touch_queue = mTouchEventQueue.GetQueue();
	std::vector<tTouchData>::iterator touch_reader =  touch_queue->begin();
	std::vector<tTouchData>::iterator touch_end = touch_queue->end();
	for(;touch_reader != touch_end; ++touch_reader)
	{
		if(touch_reader->touchState)
		{
			if(10 < touch_reader->touchX && touch_reader->touchX < 150) {
				if(10 < touch_reader->touchY && touch_reader->touchY < 50) {
					boost::shared_ptr<ShaderVideo> shader_video(new ShaderVideo(&mTitleRootFolder));
					CAppManager::Instance()->PushApp(shader_video);
				}
			}
		}
	}
}

void VtsApp::Exit()
{
	mEventMPI.UnregisterEventListener(&mTouchEventQueue);

	mDisplayMPI.UnRegister(mDisplayHandle, kDisplayScreenAllScreens);
	mDisplayMPI.DestroyHandle(mDisplayHandle, false);
}

void VtsApp::Suspend()
{
	mEventMPI.UnregisterEventListener(&mTouchEventQueue);

	mDisplayMPI.UnRegister(mDisplayHandle, kDisplayScreenAllScreens);
}

void VtsApp::Resume()
{
	mDisplayMPI.Register(mDisplayHandle, 0, 0, kDisplayOnTop, kDisplayScreenAllScreens);

	mTouchEventQueue.GetQueue();
	mEventMPI.RegisterEventListener(&mTouchEventQueue);
}
