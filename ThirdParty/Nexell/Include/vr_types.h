//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#ifndef __VR_TYPES_H__
#define __VR_TYPES_H__


//+-----------------------------------------------------------------------------
//| pre-including files
//+-----------------------------------------------------------------------------


//+-----------------------------------------------------------------------------
//| new defines
//+-----------------------------------------------------------------------------
#ifndef INTERNALAPI
  #ifdef __cplusplus
#define INTERNALAPI	extern "C"
  #else
#define INTERNALAPI extern
  #endif //__cplusplus
#endif //INTERNALAPI


//+-----------------------------------------------------------------------------
//| new data types
//+-----------------------------------------------------------------------------
typedef float	vr_float_t;
typedef int	vr_int32_t;

typedef enum {
	VR_FALSE,
	VR_TRUE
} vr_bool_t;

typedef enum {
	VR_DATATYPE_BOOLEAN,
	VR_DATATYPE_FIXED,
	VR_DATATYPE_FLOAT,
	VR_DATATYPE_INT,
	VR_DATATYPE_COUNT
} vr_datatype_t;

typedef struct vr_vector3f {
	vr_float_t x,y,z;
} vr_vector3f_t;

typedef struct vr_vector4f {
	vr_float_t x,y,z,w;
} vr_vector4f_t;


//+-----------------------------------------------------------------------------
//| End of inclusion guard
//+-----------------------------------------------------------------------------
#endif //__VR_TYPES_H__
