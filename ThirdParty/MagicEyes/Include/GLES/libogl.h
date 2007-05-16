//------------------------------------------------------------------------------
//
//	libogl interface
//------------------------------------------------------------------------------
#ifndef __libogl_h__
#define __libogl_h__

//------------------------------------------------------------------------------
//
//
//  interface between the OpenGL ES 1.1 lite and operating systems
//
//
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

    typedef int GLESOALbool;
    
    typedef struct
    {
        unsigned int	VirtualAddressOf3DCore  ; // virtual address of the 3D core register
        
    	unsigned int	Memory1D_VirtualAddress ; // must be 8byte aligned, non-cacheable
    	unsigned int    Memory1D_PhysicalAddress; // must be 8byte aligned, non-cacheable
    	unsigned int	Memory1D_SizeInMbyte    ; // size (Mbyte)

    	unsigned int	Memory2D_VirtualAddress ; // must be 4Mbyte aligned, non-cacheable
    	unsigned int    Memory2D_PhysicalAddress; // must be 4Mbyte aligned, non-cacheable
    	unsigned int	Memory2D_SizeInMbyte    ; // size (Mbyte), must be multiple of 4
    } ___OAL_MEMORY_INFORMATION__;
        
    GLESOALbool  GLESOAL_Initalize( ___OAL_MEMORY_INFORMATION__* pMemoryInfomation );
    void         GLESOAL_Finalize ( void );    
    void         GLESOAL_SwapBufferCallback( void );
    void         GLESOAL_SetWindow    ( void* pNativeWindow  );
    void         GLESOAL_GetWindowSize( int* pWidth, int* pHeight );
    
#ifdef __cplusplus
}
#endif

#endif // ___libogl_h__
