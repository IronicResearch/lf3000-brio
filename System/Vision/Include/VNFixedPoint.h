#ifndef __VNFIXEDPOINT_H__
#define __VNFIXEDPOINT_H__

// 24.8 fixed point
// IMPORTANT NOTE: this is UNSIGNED on purpose to get the full 8 bit whole number!  
typedef uint32_t fixed_t;

#define FRACT_BITS 		24
#define FRACT_BITS_D2 	(FRACT_BITS/2)
#define FIXED_ONE 		(1 << FRACT_BITS)
#define INT2FIXED(x) 	((x) << FRACT_BITS)
#define FLOAT2FIXED(x) 	((int32_t)((x) * (1 << FRACT_BITS))) 
#define FIXED2INT(x) 	((x) >> FRACT_BITS)
#define FIXED2DOUBLE(x) (((double)(x)) / (1 << FRACT_BITS))
#define MULT(x, y) 		( ((x) >> FRACT_BITS_D2) * ((y)>> FRACT_BITS_D2) )

#endif // __VNFIXEDPOINT_H__
