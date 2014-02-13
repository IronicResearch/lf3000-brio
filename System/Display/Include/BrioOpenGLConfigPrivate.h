#ifndef LF_BRIO_BRIOOPENGLCONFIG_PRIVATE_H
#define LF_BRIO_BRIOOPENGLCONFIG_PRIVATE_H

#include <BrioOpenGLConfig.h>

LF_BEGIN_BRIO_NAMESPACE()

class BrioOpenGLConfigPrivate
{
public:
	BrioOpenGLConfigPrivate(U32 size1D, U32 size2D);
	BrioOpenGLConfigPrivate(enum tBrioOpenGLVersion brioOpenGLVersion);
	~BrioOpenGLConfigPrivate();

	// EGL variables
	EGLDisplay			eglDisplay;		///< EGL display returned by eglGetDisplay()
	EGLConfig			eglConfig;		///< EGL config returned by eglChooseConfig()
	EGLSurface			eglSurface;		///< EGL surface returned by eglCreateWindowSurface()
	EGLContext			eglContext;		///< EGL context returned by eglCreateContext()

	// Display MPI handle
	tDisplayHandle		hndlDisplay;	///< DisplayMPI context bound to EGL context

	CDisplayMPI			disp_;
	tOpenGLContext		ctx;
};
LF_END_BRIO_NAMESPACE()
#endif //LF_BRIO_BRIOOPENGLCONFIG_PRIVATE_H
