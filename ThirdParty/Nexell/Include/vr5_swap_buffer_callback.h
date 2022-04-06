#ifndef VR5_SWAP_BUFFER_CALLBACK

#ifndef INTERNALAPI
  #ifdef __cplusplus
#define INTERNALAPI	extern "C"
  #else
#define INTERNALAPI extern
  #endif //__cplusplus
#endif //INTERNALAPI

typedef void (*SwapBufferCallback)( void );
INTERNALAPI void	__vr5_set_swap_buffer_callback(SwapBufferCallback callback);
typedef void (*SwapBufferCallback2)(EGLDisplay dpy, EGLSurface surface);
INTERNALAPI void	__vr5_set_swap_buffer_callback2(SwapBufferCallback2 callback);

#endif
