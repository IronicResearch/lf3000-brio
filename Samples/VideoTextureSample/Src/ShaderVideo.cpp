#include <ShaderVideo.h>
#include <CSystemData.h>
#include <AppManager.h>
#include <math.h>

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

LF_USING_BRIO_NAMESPACE()

ShaderVideo::ShaderVideo(void *userData)
{
	mTitleRootFolder = *((LeapFrog::Brio::CPath*)userData);
}

ShaderVideo::~ShaderVideo()
{
}

void ShaderVideo::Enter()
{
	mVideoMPI.SetVideoResourcePath(CSystemData::Instance()->GetCurrentTitlePath());
	mVideoHandle = mVideoMPI.StartVideo("test160x120.ogg");
	tVideoInfo video_info;
	mVideoMPI.GetVideoInfo(mVideoHandle, &video_info);
	mVideoSurf.width = video_info.width;
	mVideoSurf.height = video_info.height;
	mVideoSurf.format = kPixelFormatYUV420;
	mDisplayHandle = mDisplayMPI.CreateHandle(video_info.height, video_info.width, kPixelFormatYUV420, NULL);
	mVideoSurf.pitch = mDisplayMPI.GetPitch(mDisplayHandle);
	mVideoSurf.buffer = mDisplayBuffers[0] = mDisplayMPI.GetBuffer(mDisplayHandle);
	mDisplayBuffers[1] = mDisplayBuffers[0] + mVideoSurf.pitch/2;
	mDisplayBuffers[2] = mDisplayBuffers[1] + mVideoSurf.height/2 * mVideoSurf.pitch;

	mBrioOpenGLConfig = new BrioOpenGLConfig(kBrioOpenGL20);

	const char* pszFragShader = "\
			uniform sampler2D samplery;\
			uniform sampler2D sampleru;\
			uniform sampler2D samplerv;\
			/*Precision qualifiers required on LF3000 fragment shader for vec and float*/\
			/*Precision qualifiers break on LF2000*/\
			varying mediump vec2	myTexCoord;\
			void main (void)\
			{\
				mediump float d = texture2D(sampleru, myTexCoord).r - 0.5;\
				mediump float e = texture2D(samplerv, myTexCoord).r - 0.5;\
				\
				/*NVidia post RGB from YUV*/\
				/*float c2 = texture2D(samplery, myTexCoord).r;\
				gl_FragColor.r = c2 + 1.403 * e;\
				gl_FragColor.g = c2 - 0.344 * d - 1.403 * e;\
				gl_FragColor.b = c2 + 1.770 * d;*/\
				\
				/*Brio conversion*/\
				mediump float c2 = 1.1640625 * (texture2D(samplery, myTexCoord).r - 0.0625);\
				gl_FragColor.r = c2 + 1.59765625 * e, 0.0, 1.0;\
				gl_FragColor.g = c2 - 0.390625 * d - 0.8125 * e, 0.0, 1.0;\
				gl_FragColor.b = c2 + 2.015625 * d, 0.0, 1.0;\
				\
				gl_FragColor.a = 1.0;\
			}";
	const char* pszVertShader =  "attribute vec4	myVertex;\
			attribute vec4	myUV;\
			uniform mat4	myPMVMatrix;\
			varying vec2	myTexCoord;\
			void main(void)\
			{\
				gl_Position = myPMVMatrix * myVertex;\
				myTexCoord = myUV.st;\
			}";

	mFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(mFragShader, 1, (const char**)&pszFragShader, NULL);
	glCompileShader(mFragShader);
	GLint status;
	glGetShaderiv(mFragShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(mFragShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(mFragShader, infoLogLength, &infoLogLength, strInfoLog);
		printf("Failed to compile fragment shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	mVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(mVertexShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(mVertexShader);
	glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(mVertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(mVertexShader, infoLogLength, &infoLogLength, strInfoLog);
		printf("Failed to compile vertex shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	mProgramObject = glCreateProgram();
	glAttachShader(mProgramObject, mFragShader);
	glAttachShader(mProgramObject, mVertexShader);
	glBindAttribLocation(mProgramObject, VERTEX_ARRAY, "myVertex");
	glBindAttribLocation(mProgramObject, TEXCOORD_ARRAY, "myUV");
	glLinkProgram(mProgramObject);
	GLint bLinked;
	glGetProgramiv(mProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int ui32InfoLogLength, ui32CharsWritten;
		glGetProgramiv(mProgramObject, GL_INFO_LOG_LENGTH, &ui32InfoLogLength);
		char* pszInfoLog = new char[ui32InfoLogLength];
		glGetProgramInfoLog(mProgramObject, ui32InfoLogLength, &ui32CharsWritten, pszInfoLog);
		printf("Failed to link program: %s\n", pszInfoLog);
		delete [] pszInfoLog;
	}

	glUseProgram(mProgramObject);
	glUniform1i(glGetUniformLocation(mProgramObject, "samplery"), 0);
	glUniform1i(glGetUniformLocation(mProgramObject, "sampleru"), 1);
	glUniform1i(glGetUniformLocation(mProgramObject, "samplerv"), 2);

	mVideoMPI.GetVideoFrame(mVideoHandle, NULL);
	mVideoMPI.PutVideoFrame(mVideoHandle, &mVideoSurf);

	GLubyte y[mVideoSurf.width*mVideoSurf.height];
	GLubyte u[mVideoSurf.width/2*mVideoSurf.height/2];
	GLubyte v[mVideoSurf.width/2*mVideoSurf.height/2];
	for(int i = 0; i < mVideoSurf.height; ++i)
	{
		memcpy(&y[i*mVideoSurf.width], &mDisplayBuffers[0][i*mVideoSurf.pitch], mVideoSurf.width);
		if(i%0x1 == 0)
		{
			memcpy(&u[i/2*mVideoSurf.width/2], &mDisplayBuffers[1][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			#ifdef EMULATION
			memset(&v[i/2*mVideoSurf.width/2], 128, mVideoSurf.width/2);
			#else
			memcpy(&v[i/2*mVideoSurf.width/2], &mDisplayBuffers[2][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			#endif
		}
	}

	//glEnable(GL_TEXTURE_2D);
	glGenTextures(3, mYUVTexture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width, mVideoSurf.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);

	mEventMPI.RegisterEventListener(&mButtonEventQueue);
}

void ShaderVideo::Update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static float theta = 0.0;
	theta += 3.14159265/90.0;
	if(theta >= 3.14159265 * 2)
		theta = 0.0;

	float costheta = cos(theta);
	float sintheta = sin(theta);
	GLfloat transform_matrix[] = {
		costheta,sintheta,0,0,
		-sintheta,costheta,0,0,
		0,0,1,0,
		0,0,0,1
	};

	glUniformMatrix4fv(glGetUniformLocation(mProgramObject, "myPMVMatrix"), 1, GL_FALSE, transform_matrix);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	GLfloat vertices[] = {-1.0, 1.0, 0.0,
			1.0, 1.0, 0.0,
			-1.0, -1.0, 0.0,
			1.0, -1.0, 0.0
	};
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, vertices);

	glEnableVertexAttribArray(TEXCOORD_ARRAY);
	GLfloat texcoords[] = {0.0, 0.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 1.0
	};
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, texcoords);

	if(!mVideoMPI.GetVideoFrame(mVideoHandle, NULL))
	{
		mVideoMPI.StopVideo(mVideoHandle);
		mVideoHandle = mVideoMPI.StartVideo("test160x120.ogg");
	}
	if(!mVideoMPI.PutVideoFrame(mVideoHandle, &mVideoSurf))
		printf("PutVideoFrame error\n");

	GLubyte y[mVideoSurf.width*mVideoSurf.height];
	GLubyte u[mVideoSurf.width/2*mVideoSurf.height/2];
	GLubyte v[mVideoSurf.width/2*mVideoSurf.height/2];
	for(int i = 0; i < mVideoSurf.height; ++i)
	{
		memcpy(&y[i*mVideoSurf.width], &mDisplayBuffers[0][i*mVideoSurf.pitch], mVideoSurf.width);
		if(i%0x1 == 0)
		{
			memcpy(&u[i/2*mVideoSurf.width/2], &mDisplayBuffers[1][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			#ifdef EMULATION
			memset(&v[i/2*mVideoSurf.width/2], 128, mVideoSurf.width/2);
			#else
			memcpy(&v[i/2*mVideoSurf.width/2], &mDisplayBuffers[2][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			#endif
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[0]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width, mVideoSurf.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width, mVideoSurf.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[1]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width/2, mVideoSurf.height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[2]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width/2, mVideoSurf.height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	eglSwapBuffers(mBrioOpenGLConfig->eglDisplay, mBrioOpenGLConfig->eglSurface);

	std::vector<tButtonData2> *button_queue = mButtonEventQueue.GetQueue();
	std::vector<tButtonData2>::iterator button_reader =  button_queue->begin();
	std::vector<tButtonData2>::iterator button_end = button_queue->end();
	for(;button_reader != button_end; ++button_reader)
	{
		if(button_reader->buttonState & button_reader->buttonTransition & (kButtonB | kButtonMenu))
		{
			CAppManager::Instance()->PopApp();
		}
	}
}

void ShaderVideo::Exit()
{
	mVideoMPI.StopVideo(mVideoHandle);

	mEventMPI.UnregisterEventListener(&mButtonEventQueue);

	glDeleteTextures(3, mYUVTexture);

	delete mBrioOpenGLConfig;
}

void ShaderVideo::Suspend()
{
}

void ShaderVideo::Resume()
{
}
