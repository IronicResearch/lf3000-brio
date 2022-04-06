//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#ifndef __VR5_FRAME_BUILDER_H__
#define __VR5_FRAME_BUILDER_H__

//+-----------------------------------------------------------------------------
//| pre-including files
//+-----------------------------------------------------------------------------
#include <frontend/drv_memory.h>

//+-----------------------------------------------------------------------------
//| defines
//+-----------------------------------------------------------------------------
#define FORMAT_COLOR_INDEX		0
#define FORMAT_DEPTH_INDEX		1
#define FORMAT_STENCIL_INDEX		2
#define FORMAT_MASK_INDEX			3

//+-----------------------------------------------------------------------------
//| data types 
//+-----------------------------------------------------------------------------
typedef enum {
	VR5_INTERNAL_NONE,
	VR5_INTERNAL_RGBA4,
	VR5_INTERNAL_RGB5_A1,
	VR5_INTERNAL_RGB565,
	VR5_INTERNAL_RGB8,
	VR5_INTERNAL_RGBA8,
	VR5_INTERNAL_DEPTH16,
	VR5_INTERNAL_DEPTH24,
	VR5_INTERNAL_DEPTH32,
	VR5_INTERNAL_STENCIL8,
	VR5_INTERNAL_COUNT	
} vr5_internal_format_t;

typedef struct vr5_frame_renderbuffer {
	int 	refcnt;

	/* attributes */
	int		red_bit;
	int		green_bit;
	int		blue_bit;
	int		alpha_bit;
	int		luminance_bit;
	int		depth_bit;
	int		stencil_bit;
	
	int		bpp;
	unsigned int name;
	unsigned int width;
	unsigned int height;
	unsigned int bsize;		// buffer swizzle size
	vr5_internal_format_t	internalformat;
	unsigned int vr5_format;
	
	/* surface memory */
	unsigned int current_surface;
	unsigned int num_of_surface_memory;
	drv_memory_t surface_memory[2];

	unsigned int stride;
} vr5_frame_renderbuffer_t;

typedef struct vr5_window{
	int	width;
	int height;
}vr5_window_t;

typedef struct vr5_frame_surface {
	int		samples;
	int		num_of_sample_buffers;
	vr5_frame_renderbuffer_t * color_rb;
	vr5_frame_renderbuffer_t * depth_rb;
	vr5_frame_renderbuffer_t * stencil_rb;
	vr5_frame_renderbuffer_t * mask_rb;
	vr5_window_t *	win;
} vr5_frame_surface_t;

//+-----------------------------------------------------------------------------
//| external function 
//+-----------------------------------------------------------------------------
#if defined(linux)
//INTERNALAPI vr5_frame_surface_t * __vr5_create_frame_surface(egl_surface_t * egl_surface);
//INTERNALAPI vr5_frame_surface_t * __vr5_create_frame_surface_with_ref(egl_surface_t * egl_surface, drv_memory_t * ref_memories, int num_of_memories, unsigned int width, unsigned int height);
INTERNALAPI void	__vr5_destroy_frame_surface(vr5_frame_surface_t * frame_surface);
#endif
INTERNALAPI void	__vr5_resize_frame_surface(vr5_frame_surface_t * frame_surface, unsigned int width, unsigned int height);

//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#endif //__VR5_FRAME_BUILDER_H__
