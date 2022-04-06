#include <DisplayMPI.h>
#include <DisplayPriv.h>
#include <BrioOpenGLConfig.h>
#include <BrioOpenGLConfigPrivate.h>
#include <map>

LF_USING_BRIO_NAMESPACE()

std::map<BrioOpenGLConfig *, BrioOpenGLConfigPrivate *> gBrioOpenGLConfigPrivateMap;

BrioOpenGLConfig::BrioOpenGLConfig(U32 size1D, U32 size2D)
{
	CDisplayMPI display_mpi;
	BrioOpenGLConfigPrivate *brio_opengl_config_private = display_mpi.pModule_->CreateBrioOpenGLConfigPrivate(size1D, size2D);
	gBrioOpenGLConfigPrivateMap[this] = brio_opengl_config_private;

	eglDisplay = brio_opengl_config_private->eglDisplay;
	eglConfig = brio_opengl_config_private->eglConfig;
	eglSurface = brio_opengl_config_private->eglSurface;
	eglContext = brio_opengl_config_private->eglContext;
	hndlDisplay = brio_opengl_config_private->hndlDisplay;
}

BrioOpenGLConfig::BrioOpenGLConfig(enum tBrioOpenGLVersion brioOpenGLVersion)
{
	CDisplayMPI display_mpi;
	BrioOpenGLConfigPrivate *brio_opengl_config_private = display_mpi.pModule_->CreateBrioOpenGLConfigPrivate(brioOpenGLVersion);
	gBrioOpenGLConfigPrivateMap[this] = brio_opengl_config_private;

	eglDisplay = brio_opengl_config_private->eglDisplay;
	eglConfig = brio_opengl_config_private->eglConfig;
	eglSurface = brio_opengl_config_private->eglSurface;
	eglContext = brio_opengl_config_private->eglContext;
	hndlDisplay = brio_opengl_config_private->hndlDisplay;
}

BrioOpenGLConfig::~BrioOpenGLConfig()
{
	BrioOpenGLConfigPrivate *brio_opengl_config_private = gBrioOpenGLConfigPrivateMap[this];
	CDisplayMPI display_mpi;
	display_mpi.pModule_->DestroyBrioOpenGLConfigPrivate(brio_opengl_config_private);

	gBrioOpenGLConfigPrivateMap.erase(this);
}
