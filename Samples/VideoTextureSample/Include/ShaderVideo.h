#ifndef SHADER_VIDEO_H
#define SHADER_VIDEO_H

#include <AppInterface.h>
#include <ButtonEventQueue.h>
#include <BrioOpenGLConfig.h>
#include <DisplayMPI.h>
#include <EventMPI.h>
#include <VideoMPI.h>
#include <GLES2/gl2.h>

class ShaderVideo : public CAppInterface
{
public:
	ShaderVideo(void *userData);
	~ShaderVideo();

	void Enter();
	void Update();
	void Exit();

	void Suspend();
	void Resume();

private:
	LeapFrog::Brio::CEventMPI mEventMPI;
	LeapFrog::Brio::CButtonEventQueue mButtonEventQueue;
	LeapFrog::Brio::BrioOpenGLConfig *mBrioOpenGLConfig;
	LeapFrog::Brio::CVideoMPI mVideoMPI;
	LeapFrog::Brio::tVideoHndl mVideoHandle;
	LeapFrog::Brio::tVideoSurf mVideoSurf;
	LeapFrog::Brio::CDisplayMPI mDisplayMPI;
	LeapFrog::Brio::tDisplayHandle mDisplayHandle;
	LeapFrog::Brio::U8*	mDisplayBuffers[3];
	GLuint mFragShader;
	GLuint mVertexShader;
	GLuint mProgramObject;
	GLuint mYUVTexture[3];
};


#endif // SHADER_VIDEO_H

