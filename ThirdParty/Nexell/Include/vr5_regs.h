//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#ifndef __NX_VR5_REGS_H__
#define __NX_VR5_REGS_H__

//+-----------------------------------------------------------------------------
//| macro defines
//+-----------------------------------------------------------------------------
#if 0
#define VR5_FORMAT_GL_ALPHA               0x44400033
#define VR5_FORMAT_GL_LUMINANCE           0x00050033
#define VR5_FORMAT_GL_LUMINANCE_ALPHA     0x00010064
#define VR5_FORMAT_GL_RGB_565             0x21050094
#define VR5_FORMAT_GL_RGB_888             0x210500A6
#define VR5_FORMAT_GL_RGBA_4444           0x321000C4
#define VR5_FORMAT_GL_RGBA_5551           0x321000D4
#define VR5_FORMAT_GL_RGBA_8888_REV       0x321000E5
#define VR5_FORMAT_GL_RGBA_8888	          0x012300E5
#define VR5_FORMAT_GL_BGRA_8888_REV       0x301200E5
#define VR5_FORMAT_GL_BGRA_8888	          0x210300E5
#define VR5_FORMAT_GL_ETC1_RGB8_OES       0x00051002
#define VR5_FORMAT_GL_STENCIL_8           0x00000033
#define VR5_FORMAT_GL_DEPTH_16            0x00000044
#define VR5_FORMAT_GL_DEPTH_32            0x00000055
#define VR5_FORMAT_GL_DEPTH_FLOAT         0x000000F5
#define VR5_FORMAT_GL_INTENSITY           0x00000033
#define VR5_FORMAT_GL_YUYV_8888           0x013208E5
#define VR5_FORMAT_GL_YVYU_8888           0x031208E5
#define VR5_FORMAT_GL_UYVY_8888           0x102308E5
#define VR5_FORMAT_GL_VYUY_8888           0x120308E5
#define VR5_FORMAT_GL_BW_1                0x00050400
#define VR5_FORMAT_GL_BW_2                0x00050411
#define VR5_FORMAT_GL_BW_4                0x00050422

#define VR5_FORMAT_VG_sRGBX_8888			0x321501E5
#define VR5_FORMAT_VG_sRGBA_8888			0x321001E5
#define VR5_FORMAT_VG_sRGBA_8888_PRE		0x321003E5
#define VR5_FORMAT_VG_sRGB_565				0x21050194
#define VR5_FORMAT_VG_sRGBA_5551			0x321001D4
#define VR5_FORMAT_VG_sRGBA_4444			0x321001C4
#define VR5_FORMAT_VG_sL_8					0x00050533
#define VR5_FORMAT_VG_lRGBX_8888			0x321500E5
#define VR5_FORMAT_VG_lRGBA_8888			0x321000E5
#define VR5_FORMAT_VG_lRGBA_8888_PRE		0x321002E5
#define VR5_FORMAT_VG_lL_8					0x00050433
#define VR5_FORMAT_VG_A_8						0x55500033
#define VR5_FORMAT_VG_BW_1					0x00050400
#define VR5_FORMAT_VG_sXRGB_8888			0x210501E5
#define VR5_FORMAT_VG_sARGB_8888			0x210301E5
#define VR5_FORMAT_VG_sARGB_8888_PRE		0x210303E5
#define VR5_FORMAT_VG_sARGB_1555			0x210301B4
#define VR5_FORMAT_VG_sARGB_4444			0x210301C4
#define VR5_FORMAT_VG_lXRGB_8888			0x210500E5
#define VR5_FORMAT_VG_lARGB_8888			0x210300E5
#define VR5_FORMAT_VG_lARGB_8888_PRE		0x210302E5
#define VR5_FORMAT_VG_sBGRX_8888			0x123501E5
#define VR5_FORMAT_VG_sBGRA_8888			0x123001E5
#define VR5_FORMAT_VG_sBGRA_8888_PRE		0x123003E5
#define VR5_FORMAT_VG_sBGR_565				0x01250194
#define VR5_FORMAT_VG_sBGRA_5551			0x123001D4
#define VR5_FORMAT_VG_sBGRA_4444			0x123001C4
#define VR5_FORMAT_VG_lBGRX_8888			0x123500E5
#define VR5_FORMAT_VG_lBGRA_8888			0x123000E5
#define VR5_FORMAT_VG_lBGRA_8888_PRE		0x123002E5
#define VR5_FORMAT_VG_sXBGR_8888			0x012501E5
#define VR5_FORMAT_VG_sABGR_8888			0x012301E5
#define VR5_FORMAT_VG_sABGR_8888			0x012301E5
#define VR5_FORMAT_VG_sABGR_8888_PRE		0x012303E5
#define VR5_FORMAT_VG_sABGR_1555			0x012301B4
#define VR5_FORMAT_VG_sABGR_4444			0x012301C4
#define VR5_FORMAT_VG_lXBGR_8888			0x012500E5
#define VR5_FORMAT_VG_lABGR_8888			0x012300E5
#define VR5_FORMAT_VG_lABGR_8888_PRE		0x012302E5
#define VR5_FORMAT_VG_YUYV_8888				0x013208E5
#define VR5_FORMAT_VG_YVYU_8888				0x031208E5
#define VR5_FORMAT_VG_UYVY_8888				0x102308E5
#define VR5_FORMAT_VG_VYUY_8888				0x120308E5
#define VR5_FORMAT_VG_BW_2					0x00050411
#define VR5_FORMAT_VG_BW_4					0x00050422	
#endif

#if 1
enum vr5_format {
VR5_FORMAT_GL_ALPHA           =   0x44400033,
VR5_FORMAT_GL_LUMINANCE       =  0x00050033,
VR5_FORMAT_GL_LUMINANCE_ALPHA =  0x00010064,
VR5_FORMAT_GL_RGB_565         =  0x21050094,
VR5_FORMAT_GL_RGB_888         =  0x210500A6,
VR5_FORMAT_GL_RGBA_4444       =  0x321000C4,
VR5_FORMAT_GL_RGBA_5551       =  0x321000D4,
VR5_FORMAT_GL_RGBA_8888_REV   =  0x321000E5,
VR5_FORMAT_GL_RGBA_8888	      =   0x012300E5,
VR5_FORMAT_GL_BGRA_8888_REV   =   0x301200E5,
VR5_FORMAT_GL_BGRA_8888	      =   0x210300E5,
VR5_FORMAT_GL_ETC1_RGB8_OES   =   0x00051002,
VR5_FORMAT_GL_STENCIL_8       =   0x00000033,
VR5_FORMAT_GL_DEPTH_16        =   0x00000044,
VR5_FORMAT_GL_DEPTH_32        =   0x00000055,
VR5_FORMAT_GL_DEPTH_FLOAT     =   0x000000F5,
VR5_FORMAT_GL_INTENSITY       =   0x00000033,
VR5_FORMAT_GL_YUYV_8888       =   0x013208E5,
VR5_FORMAT_GL_YVYU_8888       =   0x031208E5,
VR5_FORMAT_GL_UYVY_8888       =   0x102308E5,
VR5_FORMAT_GL_VYUY_8888        =  0x120308E5,
VR5_FORMAT_GL_BW_1            =   0x00050400,
VR5_FORMAT_GL_BW_2             =  0x00050411,
VR5_FORMAT_GL_BW_4             =  0x00050422,

VR5_FORMAT_VG_sRGBX_8888		=	0x321501E5,
VR5_FORMAT_VG_sRGBA_8888	=		0x321001E5,//0x321001E5,
VR5_FORMAT_VG_sRGBA_8888_PRE	=	0x321003E5,
VR5_FORMAT_VG_sRGB_565			=	0x21050194,
VR5_FORMAT_VG_sRGBA_5551		=	0x321001D4,
VR5_FORMAT_VG_sRGBA_4444		=	0x321001C4,
VR5_FORMAT_VG_sL_8				=	0x00050533,
VR5_FORMAT_VG_lRGBX_8888		=	0x321500E5,
VR5_FORMAT_VG_lRGBA_8888		=	0x321000E5,
VR5_FORMAT_VG_lRGBA_8888_PRE	=	0x321002E5,
VR5_FORMAT_VG_lL_8				=	0x00050433,
VR5_FORMAT_VG_A_8				=		0x55500033,
VR5_FORMAT_VG_BW_1				=	0x00050400,
VR5_FORMAT_VG_sXRGB_8888		=	0x210501E5,
VR5_FORMAT_VG_sARGB_8888		=	0x210301E5,
VR5_FORMAT_VG_sARGB_8888_PRE	=	0x210303E5,
VR5_FORMAT_VG_sARGB_1555		=	0x210301B4,
VR5_FORMAT_VG_sARGB_4444		=	0x210301C4,
VR5_FORMAT_VG_lXRGB_8888		=	0x210500E5,
VR5_FORMAT_VG_lARGB_8888		=	0x210300E5,
VR5_FORMAT_VG_lARGB_8888_PRE	=	0x210302E5,
VR5_FORMAT_VG_sBGRX_8888		=	0x123501E5,
VR5_FORMAT_VG_sBGRA_8888		=	0x123001E5,
VR5_FORMAT_VG_sBGRA_8888_PRE	=	0x123003E5,
VR5_FORMAT_VG_sBGR_565			=	0x01250194,
VR5_FORMAT_VG_sBGRA_5551		=	0x123001D4,
VR5_FORMAT_VG_sBGRA_4444		=	0x123001C4,
VR5_FORMAT_VG_lBGRX_8888		=	0x123500E5,
VR5_FORMAT_VG_lBGRA_8888		=	0x123000E5,
VR5_FORMAT_VG_lBGRA_8888_PRE	=	0x123002E5,
VR5_FORMAT_VG_sXBGR_8888		=	0x012501E5,
VR5_FORMAT_VG_sABGR_8888		=	0x012301E5,
VR5_FORMAT_VG_sABGR_8888_PRE	=	0x012303E5,
VR5_FORMAT_VG_sABGR_1555		=	0x012301B4,
VR5_FORMAT_VG_sABGR_4444		=	0x012301C4,
VR5_FORMAT_VG_lXBGR_8888		=	0x012500E5,
VR5_FORMAT_VG_lABGR_8888		=	0x012300E5,
VR5_FORMAT_VG_lABGR_8888_PRE	=	0x012302E5,
VR5_FORMAT_VG_YUYV_8888			=	0x013208E5,
VR5_FORMAT_VG_YVYU_8888			=	0x031208E5,
VR5_FORMAT_VG_UYVY_8888			=	0x102308E5,
VR5_FORMAT_VG_VYUY_8888			=	0x120308E5,
VR5_FORMAT_VG_BW_2				=	0x00050411,
VR5_FORMAT_VG_BW_4				=	0x00050422	
} ;
#endif

#define VR5_TYPE_GL_BYTE					0 
#define VR5_TYPE_GL_UNSIGNED_BYTE			1 
#define VR5_TYPE_GL_SHORT					2 
#define VR5_TYPE_GL_UNSIGNED_SHORT			3 
#define VR5_TYPE_GL_INT						4 
#define VR5_TYPE_GL_UNSIGNED_INT			5 
#define VR5_TYPE_GL_BYTE_N					6 
#define VR5_TYPE_GL_UNSIGNED_BYTE_N			7 
#define VR5_TYPE_GL_SHORT_N					8 
#define VR5_TYPE_GL_UNSIGNED_SHORT_N		9 
#define VR5_TYPE_GL_INT_N					10 
#define VR5_TYPE_GL_UNSIGNED_INT_N			11 
#define VR5_TYPE_GL_FIXED					12 
#define VR5_TYPE_GL_FLOAT					13 

#define VR5_FILTER_NEAREST					0
#define VR5_FILTER_LINEAR					1
#define VR5_FILTER_NEAREST_MIPMAP_NEAREST	2
#define VR5_FILTER_LINEAR_MIPMAP_NEAREST	3
#define VR5_FILTER_NEAREST_MIPMAP_LINEAR	4
#define VR5_FILTER_LINEAR_MIPMAP_LINEAR		5

#define VR5_WRAP_REPEAT						0
#define VR5_WRAP_CLAMP_TO_EDGE				1
#define VR5_WRAP_MIRRORED_REPEAT			2
#define VR5_WRAP_CLAMP						3
#define VR5_WRAP_CLAMP_TO_BORDER			4

#define VR5_BACK							1
#define VR5_FRONT							2
#define VR5_FRONT_AND_BACK					3

#define VR5_NEVER							0
#define VR5_LESS							1
#define VR5_EQUAL							2
#define VR5_LEQUAL							3
#define VR5_GREATER							4
#define VR5_NOTEQUAL						5
#define VR5_GEQUAL							6
#define VR5_ALWAYS							7

#define VR5_KEEP							0
#define VR5_ZERO							1
#define VR5_REPLACE						2
#define VR5_INCR							3
#define VR5_DECR							4
#define VR5_INVERT							5
#define VR5_INCR_WRAP						6
#define VR5_DECR_WRAP						7

#define VR5_COMBINE_REPLACE					0
#define VR5_COMBINE_MODULATE				1
#define VR5_COMBINE_ADD						2
#define VR5_COMBINE_ADD_SIGNED				3
#define VR5_COMBINE_INTERPOLATE				4
#define VR5_COMBINE_SUBTRACT				5
#define VR5_COMBINE_DOT3_RGB				6
#define VR5_COMBINE_DOT3_RGBA				7
#define VR5_COMBINE_IMAGE_STENCIL			6 // for vg

#define VR5_SRC_ALPHA_TEXTUREn				0
#define VR5_SRC_ALPHA_CONSTANT				1
#define VR5_SRC_ALPHA_PRIMARY_COLOR			2
#define VR5_SRC_ALPHA_PREVIOUS				3

#define VR5_OP_ALPHA_SRC_ALPHA				0
#define VR5_OP_ALPHA_ONE_MINUS_SRC_ALPHA	2

#define VR5_SRC_RGB_TEXTUREn				0
#define VR5_SRC_RGB_CONSTANT				1
#define VR5_SRC_RGB_PRIMARY_COLOR			2
#define VR5_SRC_RGB_PREVIOUS				3

#define VR5_OP_RGB_SRC_COLOR				0
#define VR5_OP_RGB_ONE_MINUS_SRC_COLOR		1
#define VR5_OP_RGB_SRC_ALPHA				2
#define VR5_OP_RGB_ONE_MINUS_SRC_ALPHA		3

#define VR5_BLEND_EQUATION_ADD						0
#define VR5_BLEND_EQUATION_SUBTRACT					1
#define VR5_BLEND_EQUATION_REVERSE_SUBTRACT			2
#define VR5_BLEND_EQUATION_MIN						3
#define VR5_BLEND_EQUATION_MAX						4
#define VR5_BLEND_EQUATION_DARKEN					5
#define VR5_BLEND_EQUATION_LIGHTEN					6
#define VR5_BLEND_EQUATION_MULTIPLY					7

#define VR5_BLEND_ZERO								0
#define VR5_BLEND_ONE								1
#define VR5_BLEND_SRC_COLOR							2
#define VR5_BLEND_ONE_MINUS_SRC_COLOR				3
#define VR5_BLEND_DST_COLOR							4
#define VR5_BLEND_ONE_MINUS_DST_COLOR				5
#define VR5_BLEND_SRC_ALPHA							6
#define VR5_BLEND_ONE_MINUS_SRC_ALPHA				7
#define VR5_BLEND_DST_ALPHA							8
#define VR5_BLEND_ONE_MINUS_DST_ALPHA				9
#define VR5_BLEND_CONSTANT_COLOR					10
#define VR5_BLEND_ONE_MINUS_CONSTANT_COLOR			11
#define VR5_BLEND_CONSTANT_ALPHA					12
#define VR5_BLEND_ONE_MINUS_CONSTANT_ALPHA			13
#define VR5_BLEND_SRC_ALPHA_SATURATE				14
#define VR5_BLEND_ONE_MINUS_SRC_ALPHA_SATURATE		15

#define VR5_LOGICOP_CLEAR						0
#define VR5_LOGICOP_NOR						1
#define VR5_LOGICOP_AND_INVERTED			2
#define VR5_LOGICOP_COPY_INVERTED			3
#define VR5_LOGICOP_AND_REVERSE			4
#define VR5_LOGICOP_INVERT					5
#define VR5_LOGICOP_XOR						6
#define VR5_LOGICOP_NAND						7
#define VR5_LOGICOP_AND						8
#define VR5_LOGICOP_EQUIV						9
#define VR5_LOGICOP_NOOP						10
#define VR5_LOGICOP_OR_INVERTED				11
#define VR5_LOGICOP_COPY						12
#define VR5_LOGICOP_OR_REVERSE				13
#define VR5_LOGICOP_OR							14
#define VR5_LOGICOP_SET						15

#define VR5_UPPER_LEFT						0
#define VR5_LOWER_LEFT						1

#define VR5_SHADE_SMOOTH						0
#define VR5_SHADE_FLAT						1<<2	// varying mask bit

#define VR5_PRIMITIVE_TYPE_POINTS			0
#define VR5_PRIMITIVE_TYPE_LINE_STRIP		1
#define VR5_PRIMITIVE_TYPE_LINE_LOOP		2
#define VR5_PRIMITIVE_TYPE_LINES			3
#define VR5_PRIMITIVE_TYPE_TRIANGLE_STRIP	4
#define VR5_PRIMITIVE_TYPE_TRIANGLE_FAN		5
#define VR5_PRIMITIVE_TYPE_TRIANGLES		6

#define VR5_PRIMITIVE_INDEX_BYTE			1
#define VR5_PRIMITIVE_INDEX_SHORT			3
#define VR5_PRIMITIVE_INDEX_INT				5


//+-----------------------------------------------------------------------------
//| data types 
//+-----------------------------------------------------------------------------
typedef struct
{
    union
    {
        struct
        {
            unsigned char  SIZE ;
            unsigned char  Reserved00 : 4;
            unsigned char  TYPE       : 4;
            unsigned short STRIDE;
        };
        unsigned int INFO;
    };
    unsigned int ADDR;
} VR5REG_STREAM_INFO;

typedef struct
{
	union
	{
		struct
		{
			unsigned char  Reserved00         : 7;
			unsigned char  TEXTURE_CUBEMAP    : 1;
			unsigned char  TEXTURE_BASE_LEVEL : 4;
			unsigned char  TEXTURE_MAX_LEVEL  : 4;
			unsigned short TEXTURE_LOD_BIAS;
		};
		unsigned int TEXTURE_PARAM0;
	};
	union
	{
		struct
		{
			unsigned char  MAG_FILTER : 1;
			unsigned char  MIN_FILTER : 3;
			unsigned char  WRAP_R : 4;
			unsigned char  WRAP_T : 4;
			unsigned char  WRAP_S : 4;
			unsigned char  TEXTURE_MIN_LOD;
			unsigned char  TEXTURE_MAX_LOD;
		};
		unsigned int TEXTURE_PARAM1;
	};
	union
	{
		struct
		{
			unsigned short TEXTURE_WIDTH ;
			unsigned short TEXTURE_HEIGHT;			
		};
		unsigned int TEXTURE_SIZE;
	};
	unsigned int TEXTURE_FORMAT;
} VR5REG_TEXTURE_PARAM;

typedef struct
{
	union 
	{
		struct
		{
			unsigned char COMBINE_ALPHA  : 3;
			unsigned char Reserve00      : 5;
			unsigned char Reserve01      : 1;
			unsigned char OPERAND0_ALPHA : 1;
			unsigned char SRC0_ALPHA     : 2;
			unsigned char SRC0_ALPHA_TEXN: 4;
			unsigned char Reserve02      : 1;
			unsigned char OPERAND1_ALPHA : 1;
			unsigned char SRC1_ALPHA     : 2;
			unsigned char SRC1_ALPHA_TEXN: 4;
			unsigned char Reserve03      : 1;
			unsigned char OPERAND2_ALPHA : 1;
			unsigned char SRC2_ALPHA     : 2;
			unsigned char SRC2_ALPHA_TEXN: 4;
		};
		unsigned int TEXENV_COMB_ALPHA;
	};
	union 
	{
		struct
		{
			unsigned char COMBINE_RGB    : 3;
			unsigned char Reserve04      : 5;
			unsigned char OPERAND0_RGB   : 2;
			unsigned char SRC0_RGB       : 2;
			unsigned char SRC0_RGB_TEXN  : 4;
			unsigned char OPERAND1_RGB   : 2;
			unsigned char SRC1_RGB       : 2;
			unsigned char SRC1_RGB_TEXN  : 4;
			unsigned char OPERAND2_RGB   : 2;
			unsigned char SRC2_RGB       : 2;
			unsigned char SRC2_RGB_TEXN  : 4;
		};
		unsigned int TEXENV_COMB_RGB;
	};
	union 
	{
		struct
		{
			unsigned short SCALE_R;
			unsigned short BIAS_R ;
		};
		unsigned int TEXENV_SCALEBIAS_R;
	};
	union 
	{
		struct
		{
			unsigned short SCALE_G;
			unsigned short BIAS_G ;
		};
		unsigned int TEXENV_SCALEBIAS_G;
	};
	union 
	{
		struct
		{
			unsigned short SCALE_B;
			unsigned short BIAS_B ;
		};
		unsigned int TEXENV_SCALEBIAS_B;
	};
	union 
	{
		struct
		{
			unsigned short SCALE_A;
			unsigned short BIAS_A ;
		};
		unsigned int TEXENV_SCALEBIAS_A;
	};
	union 
	{
		struct
		{
			unsigned char TEXENV_COLOR_A;
			unsigned char TEXENV_COLOR_B;
			unsigned char TEXENV_COLOR_G;
			unsigned char TEXENV_COLOR_R;
		};
		unsigned int TEXENV_COLOR   ;
	};
	unsigned int TEXENV_Reserved;
} VR5REG_TEXENV;

typedef struct
{
    union
    {
        struct
        {
            unsigned char  BSIZE      : 4;
			unsigned char  LINEAR     : 1;
            unsigned char  Reserved00 : 3;
            unsigned char  Reserved01 ;
            unsigned short STRIDE     ;
        };            
        unsigned int INFO;
    };
    unsigned int ADDRESS;
} VR5REG_TEXIMAGE;

typedef struct
{	
	//---------------------------------------------------
	//  000h~ : GL states
	//---------------------------------------------------	    
	union
	{
        struct
        {
			unsigned char COMMAND_DONE_INTPEND : 1;
			unsigned char COMMAND_DONE_INTENB  : 1;
			unsigned char STATUS_Reserved00    : 6;
			unsigned char MEM_BSTSIZE          : 5;
			unsigned char COMMAND_IDLE         : 1;
			unsigned char STATUS_Reserved01    : 2;
			unsigned short SUBMODULE_IDLE      ;
        };
        unsigned int STATUS;
	};
	unsigned int POWER_CONTROL              ;
	union
	{
		unsigned int COMMAND_ADDRESS;
        struct
        {
			unsigned char COMMAND_BEGIN : 1;
			unsigned char COMMAND_STOP  : 1;
			unsigned char COMMAND_CONTROL_Reserved00 : 6;
			unsigned char COMMAND_CONTROL_Reserved01;
			unsigned char COMMAND_CONTROL_Reserved02;
			unsigned char COMMAND_CONTROL_Reserved03;
        };
        unsigned int COMMAND_CONTROL;
	};
	union
	{
        struct
        {
			unsigned char COMMAND_EVENT_NUM : 7;
			unsigned char COMMAND_EVENT_ENB : 1;
			unsigned char COMMAND_PROC_EVENT_NUM   : 7;
			unsigned char COMMAND_EVENT_Reserved00 : 1;
			unsigned short MEMORY_PAGE64K   ; // @todo
        };
		unsigned int COMMAND_EVENT;
	};
	unsigned int STREAM_ADDR_ELEMENT_ARRAY  ;
	unsigned int DRAW_FIRST                 ;
	unsigned int DRAW_COUNT                 ;
	union
	{
        struct
        {
			unsigned char PRIMITIVE_TYPE             : 4;
			unsigned char STREAM_TYPE_ELEMENT_ARRAY  : 3;
			unsigned char STREAM_VALID_ELEMENT_ARRAY : 1;
			unsigned char DRAW_CONTROL_Reserved00 : 4;
			unsigned char CLEAR_FRAMECACHE   : 1;	// must be zero!
			unsigned char CLEAR_VERTEXCACHE  : 1;	// must be zero!
			unsigned char CLEAR_TEXTURECACHE : 1;	// must be zero!
			unsigned char RUN_PRIMITIVE      : 1;
			unsigned short STREAM_VALID      ;
        };
		unsigned int DRAW_CONTROL;
	};
	union
	{
        struct
        {
			unsigned char R_GL_CULL_FACE                : 1;
			unsigned char R_GL_POINT_SPRITE             : 1;
			unsigned char R_GL_STENCIL_TEST             : 1;
			unsigned char R_GL_DEPTH_TEST               : 1;
			unsigned char R_GL_BLEND                    : 1;
			unsigned char R_VG_MASK_BLEND               : 1;
			unsigned char R_GL_POLYGON_OFFSET_FILL      : 1;
			unsigned char R_GL_SCISSOR_TEST             : 1;
			unsigned char R_GL_DITHER                   : 1;
			unsigned char R_GL_DEPTH_WRITEMASK          : 1;
			unsigned char R_GL_COLOR_LOGIC_OP           : 1;
			unsigned char R_GL_MULTISAMPLE              : 1;
			unsigned char R_GL_POINT_SMOOTH             : 1;
			unsigned char R_GL_LINE_SMOOTH              : 1;
			unsigned char R_GL_POLYGON_SMOOTH           : 1;
			unsigned char R_GL_SAMPLE_ALPHA_TO_COVERAGE : 1;
			unsigned char R_GL_SAMPLE_ALPHA_TO_ONE      : 1;
			unsigned char R_GL_SAMPLE_COVERAGE          : 1;
			unsigned char R_GL_COLOR_SUM                : 1;
			unsigned char R_GL_FOG                      : 1;
			unsigned char R_GL_ALPHA_TEST               : 1;
			unsigned char R_GL_CLIP_PLANE0              : 1;
			unsigned char SEC2ALPHA                   : 1;
			unsigned char DRAW_MODE00_Reserved00 : 1;
			unsigned char DRAW_MODE00_Reserved01 ;
        };
		unsigned int DRAW_MODE00;
	};
	union
	{
        struct
        {
			unsigned char COLOR_WRITEMASK        : 4;
			unsigned char LOGIC_OP_MODE          : 4;
			unsigned char CULL_FACE_MODE         : 2;
			unsigned char FRONT_FACE             : 1;
			unsigned char DRAW_MODE01_Reserved00 : 5;
			unsigned char SAMPLE_BUFFERS         : 2;
			unsigned char SAMPLE_COVERAGE_INVERT : 1;
			unsigned char DRAW_MODE01_Reserved01 : 5;
			unsigned char SAMPLE_COVERAGE_VALUE  ;
        };
		unsigned int DRAW_MODE01;
	};
	union
	{
        struct
        {
			unsigned char BLEND_SRC_RGB          : 4;
			unsigned char BLEND_DST_RGB          : 4;
			unsigned char BLEND_SRC_ALPHA        : 4;
			unsigned char BLEND_DST_ALPHA        : 4;
			unsigned char BLEND_EQUATION_RGB     : 3;
			unsigned char DRAW_MODE02_Reserved00 : 1;
			unsigned char BLEND_EQUATION_ALPHA   : 3;
			unsigned char DRAW_MODE02_Reserved01 : 1;
			unsigned char DEPTH_FUNC             : 3;
			unsigned char DRAW_MODE02_Reserved02 : 5;
        };
		unsigned int DRAW_MODE02;
	};
	union
	{
        struct
        {
			unsigned char BLEND_COLOR_A;
			unsigned char BLEND_COLOR_B;
			unsigned char BLEND_COLOR_G;
			unsigned char BLEND_COLOR_R;
		};
		unsigned int BLEND_COLOR ;
	};
	union
	{
        struct
        {
			unsigned char STENCIL_WRITEMASK     ;
			unsigned char STENCIL_PARAM00_Reserved00;
			unsigned char STENCIL_BACK_WRITEMASK;
			unsigned char LINE_MARGIN           ;
		};
		unsigned int STENCIL_PARAM00 ;
	};
	union
	{
        struct
        {
			unsigned char STENCIL_FUNC               : 3;
			unsigned char STENCIL_PARAM01_Reserved00 : 1;
			unsigned char STENCIL_FAIL               : 3;
			unsigned char STENCIL_PARAM01_Reserved01 : 1;
			unsigned char STENCIL_PASS_DEPTH_FAIL    : 3;
			unsigned char STENCIL_PARAM01_Reserved02 : 1;
			unsigned char STENCIL_PASS_DEPTH_PASS    : 3;
			unsigned char STENCIL_PARAM01_Reserved03 : 1;
			unsigned char STENCIL_REF                ;
			unsigned char STENCIL_VALUE_MASK         ;
		};
		unsigned int STENCIL_PARAM01 ;
	};
	union
	{
        struct
        {
			unsigned char STENCIL_BACK_FUNC            : 3;
			unsigned char STENCIL_PARAM02_Reserved00   : 1;
			unsigned char STENCIL_BACK_FAIL            : 3;
			unsigned char STENCIL_PARAM02_Reserved01   : 1;
			unsigned char STENCIL_BACK_PASS_DEPTH_FAIL : 3;
			unsigned char STENCIL_PARAM02_Reserved02   : 1;
			unsigned char STENCIL_BACK_PASS_DEPTH_PASS : 3;
			unsigned char STENCIL_PARAM02_Reserved03   : 1;
			unsigned char STENCIL_BACK_REF             ;
			unsigned char STENCIL_BACK_VALUE_MASK      ;
		};
		unsigned int STENCIL_PARAM02 ;
	};
	union
	{
        struct
        {
			unsigned short VIEWPORT_MINY ;
			unsigned short VIEWPORT_MINX ;
		};
		unsigned int VIEWPORT_MIN ;
	};
	union
	{
        struct
        {
			unsigned short VIEWPORT_MAXY ;
			unsigned short VIEWPORT_MAXX ;
		};
		unsigned int VIEWPORT_MAX ;
	};
	union
	{
        struct
        {
			unsigned short SCISSOR_BOX_MINY ;
			unsigned short SCISSOR_BOX_MINX ;
		};
		unsigned int SCISSOR_BOX_MIN ;
	};
	union
	{
        struct
        {
			unsigned short SCISSOR_BOX_MAXY ;
			unsigned short SCISSOR_BOX_MAXX ;
		};
		unsigned int SCISSOR_BOX_MAX ;
	};
	float        POLYGON_OFFSET_FACTOR      ;
	float        POLYGON_OFFSET_UNITS       ;
	float        RCP_LINE_WIDTH             ;
	unsigned int TEXTURE_2D                 ;
	union
	{
        struct
        {
			unsigned char FLAT_MASK                 ;
			unsigned char ACTIVE_VARYINGS           ;
			unsigned char POINT_SPRITE_COORD_ORIGIN ;
			unsigned char COORD_REPLACE             ;
		};
		unsigned int VARYING_PARAM ;
	};
	union
	{
        struct
        {
			unsigned char FOG_COLOR_A;
			unsigned char FOG_COLOR_B;
			unsigned char FOG_COLOR_G;
			unsigned char FOG_COLOR_R;
		};
		unsigned int FOG_COLOR ;
	};
	union
	{
        struct
        {
			unsigned char COLOR_CLEAR_VALUE_A;
			unsigned char COLOR_CLEAR_VALUE_B;
			unsigned char COLOR_CLEAR_VALUE_G;
			unsigned char COLOR_CLEAR_VALUE_R;
		};
		unsigned int COLOR_CLEAR_VALUE ;
	};
	union
	{
        struct
        {
			unsigned char STENCIL_CLEAR_VALUE        ;
			unsigned char ALPHATEST_PARAM_Reserved00 ;
			unsigned char ALPHA_TEST_REF             ;
			unsigned char ALPHA_TEST_FUNC            : 3;
			unsigned char ALPHATEST_PARAM_Reserved01 : 5;
		};
		unsigned int ALPHATEST_PARAM ;
	};
	unsigned int DEPTH_CLEAR_VALUE          ;
	//---------------------------------------------------
	//  070h~ : frame buffer info
	//---------------------------------------------------	    
	union
	{
        struct
        {
			unsigned char  FRAME_BSIZE                 : 4;
			unsigned char  FRAME_BUFFERINFO_Reserved00 : 4;
			unsigned char  FRAME_BUFFERINFO_Reserved01 ;
			unsigned short FRAME_STRIDE                ;
		};
		unsigned int FRAME_BUFFERINFO ;
	};		
	unsigned int FRAME_ADDRESS              ;
	unsigned int FRAME_FORMAT               ;
	union
	{
        struct
        {
			unsigned char  DEPTH_BSIZE                 : 4;
			unsigned char  DEPTH_BUFFERINFO_Reserved00 : 4;
			unsigned char  DEPTH_BUFFERINFO_Reserved01 ;
			unsigned short DEPTH_STRIDE                ;
		};
		unsigned int DEPTH_BUFFERINFO ;
	};		
	unsigned int DEPTH_ADDRESS              ;
	unsigned int DEPTH_FORMAT               ;
	union
	{
        struct
        {
			unsigned char  STENCIL_BSIZE                 : 4;
			unsigned char  STENCIL_BUFFERINFO_Reserved00 : 4;
			unsigned char  STENCIL_BUFFERINFO_Reserved01 ;
			unsigned short STENCIL_STRIDE                ;
		};
		unsigned int STENCIL_BUFFERINFO ;
	};		
	unsigned int STENCIL_ADDRESS            ;
	unsigned int STENCIL_FORMAT             ;
	union
	{
        struct
        {
			unsigned char  MASK_BSIZE                 : 4;
			unsigned char  MASK_BUFFERINFO_Reserved00 : 4;
			unsigned char  MASK_BUFFERINFO_Reserved01 ;
			unsigned short MASK_STRIDE                ;
		};
		unsigned int MASK_BUFFERINFO ;
	};		
	unsigned int MASK_ADDRESS               ;
	unsigned int MASK_FORMAT	            ;			
	unsigned int RESERVED_00[2];		
	unsigned int MULTISAMPLE_POS[4]         ;
	union
	{
        struct
        {
			unsigned short MASTER_SCISSOR_BOX_MINY;
			unsigned short MASTER_SCISSOR_BOX_MINX;
		};
		unsigned int MASTER_SCISSOR_BOX_MIN     ;
	};		
	union
	{
        struct
        {
			unsigned short MASTER_SCISSOR_BOX_MAXY;
			unsigned short MASTER_SCISSOR_BOX_MAXX;
		};
		unsigned int MASTER_SCISSOR_BOX_MAX     ;
	};
	//---------------------------------------------------
	//  0C0h~ : stream info
	//---------------------------------------------------	    
	VR5REG_STREAM_INFO STREAM[8];		
	unsigned int RESERVED_01[(0x0200 - 0x0100)/4];		
	//---------------------------------------------------
	//  200h~ : stream info
	//---------------------------------------------------	    
	VR5REG_TEXTURE_PARAM TEXTURE_PARAM[8];		
	//---------------------------------------------------
	//  280h~ : BitBlt
	//---------------------------------------------------
	union
	{
        struct
        {
            unsigned char  BLT_SRC_BISZE : 4;
            unsigned char  BLT_SRC_INFO_Reserved00 : 4;
            unsigned char  BLT_SRC_INFO_Reserved01 : 8;
            unsigned short BLT_SRC_STRIDE;
        };
        unsigned int BLT_SRC_INFO;
	};
	unsigned int BLT_SRC_ADDRESS;
	unsigned int BLT_SRC_FORMAT ;
	union
	{
        struct
        {
            unsigned short BLT_SRC_Y;
            unsigned short BLT_SRC_X;
        };
        unsigned int BLT_SRC_POS;
	};
	union
	{
        struct
        {
            unsigned char  BLT_DST_BISZE : 4;
            unsigned char  BLT_DST_INFO_Reserved00 : 4;
            unsigned char  BLT_DST_INFO_Reserved01 : 8;
            unsigned short BLT_DST_STRIDE;
        };
        unsigned int BLT_DST_INFO;
	};
	unsigned int BLT_DST_ADDRESS;
	unsigned int BLT_DST_FORMAT ;
	union
	{
        struct
        {
            unsigned short BLT_DST_Y;
            unsigned short BLT_DST_X;
        };
        unsigned int BLT_DST_POS;
	};
	union
	{
        struct
        {
            unsigned short BLT_HEIGHT;
            unsigned short BLT_WIDTH ;
        };
        unsigned int BLT_DST_SiZE;
	};
	union
	{
        struct
        {
    		unsigned char BLT_RUN       : 1;
    		unsigned char BLT_DITHER    : 1;
    		unsigned char BLT_SRC_SOLID : 1;
    		unsigned char BLT_DIRX      : 1;
    		unsigned char BLT_DIRY      : 1;
    		unsigned char BLT_CONTROL_Reserved00 : 3;
    		unsigned char BLT_CONTROL_Reserved01;        		    
    		unsigned char BLT_IDLE      : 1;
    		unsigned char BLT_CONTROL_Reserved02 : 7;
    		unsigned char BLT_CONTROL_Reserved03;
        };
        unsigned int BLT_CONTROL;
	};
	unsigned int RESERVED_02[(0x02C0 - 0x02A8)/4];		
	//---------------------------------------------------
	//  2C0h~ : Clock control
	//---------------------------------------------------
	unsigned int CLKCTRL;
	unsigned int RESERVED_03[(0x0300 - 0x02C4)/4];				
	//---------------------------------------------------
	//  300h~ : TexEnv
	//---------------------------------------------------
	VR5REG_TEXENV TEXENV[8];
	//---------------------------------------------------
	//  400h~ : TexImage
	//---------------------------------------------------
	VR5REG_TEXIMAGE TEXIMG[8][16];
	//---------------------------------------------------
	//  800h~ : MP register
	//---------------------------------------------------
	union
	{
        struct
        {
            unsigned short MP_VERTEX_ATTRIBUTE_BASE;
            unsigned short MP_VERTEX_ATTRIBUTE_STRIDE;
        };
        unsigned int MP_VERTEX_ATTRIBUTE;
	};
	union
	{
        struct
        {
            unsigned short MP_VERTEX_POSITION_BASE  ;
            unsigned short MP_VERTEX_POSITION_STRIDE;
        };
        unsigned int MP_VERTEX_POSITION;
	};
	union
	{
        struct
        {
            unsigned short MP_VERTEX_VARYING_BASE   ;
            unsigned short MP_VERTEX_VARYING_STRIDE ;
        };
        unsigned int MP_VERTEX_VARYING;
	};
	union
	{
        struct
        {
            unsigned char  MP_VERTEX_WARPSLOT   :  4  ;
            unsigned char  MP_VERTEX_Reserved00 :  4  ;
            unsigned char  MP_VERTEX_Reserved01 ;
            unsigned char  MP_VERTEX_BATCHSIZE  ;
            unsigned char  MP_VARYING_VECTORS   ;
        };
        unsigned int MP_VERTEX_BATCH;
	};
	unsigned int MP_VERTEX_CONSTADDR;
	unsigned int MP_VERTEX_LOCALADDR;
	unsigned int MP_VERTEX_LOCALSIZE;
	unsigned int MP_VERTEX_STARTPC  ;
	union
	{
        struct
        {
            unsigned char  MP_PIXEL_WARPSLOT   :  4  ;
            unsigned char  MP_PIXEL_Reserved00 :  4  ;
            unsigned char  MP_PIXEL_Reserved01 ;
            unsigned char  MP_PIXEL_MAXCELL    ;
            unsigned char  MP_PIXEL_Reserved02 ;
        };
        unsigned int MP_PIXEL_BATCH;
	};
	unsigned int MP_PIXEL_CONSTADDR;
	unsigned int MP_PIXEL_LOCALADDR;
	unsigned int MP_PIXEL_LOCALSIZE;
	unsigned int MP_PIXEL_STARTPC  ;
    
    unsigned int MP_PROG_ARGUMENT;
	union
	{
        struct
        {
            unsigned char  MP_PROG_WARPSLOT  : 4;
            unsigned char  MP_PROG_Reserved00: 4;
            unsigned char  MP_PROG_Reserved01;
            unsigned short MP_PROG_THREADMASK;
        };
        unsigned int MP_PROG_BATCH  ;            
	};
    unsigned int MP_PROG_CONSTADDR ;
    unsigned int MP_PROG_LOCALADDR ;
    unsigned int MP_PROG_LOCALSIZE ;
    unsigned int MP_PROG_STARTPC   ;        
	union
	{
        struct
        {
            unsigned char MP_INIT               : 1 ;
            unsigned char MP_CLEAR_INST_CACHE   : 1 ;
            unsigned char MP_CLEAR_CONST_CACHE  : 1 ;
            unsigned char MP_CLEAR_MEM_CACHE    : 1 ;
            unsigned char MP_CLEAR_REG_CACHE    : 1 ;
            unsigned char MP_PROG_ISSUE         : 1 ;
            unsigned char MP_CONTROL_Reserved00 : 2 ;
            unsigned char MP_CONTROL_Reserved01 ;
            unsigned char MP_CONTROL_Reserved02 ;       
            unsigned char MP_BURSTSIZE          : 5 ; 
            unsigned char MP_CONTROL_Reserved03 : 1 ;
            unsigned char MP_ROUNDMODE          : 2 ;                
        };
        unsigned int MP_CONTROL;
	};
	struct
	{
        unsigned short MP_IDLE_Reserved;
        unsigned short MP_IDLE;
	};
	union
	{
	    unsigned int MP_SHARED_OFFSET;
        struct
        {
            unsigned char MP_SHARED_FLAG : 1; // ( Write 0:NOP 1 : Do access,  Read 0:Access complete 1:Access busy )
            unsigned char MP_SHARED_MODE : 1; // ( 0:shared read 1:shared write )
            unsigned char MP_SHARED_Reserved00 : 6;
            unsigned char MP_SHARED_Reserved01;
            unsigned char MP_SHARED_Reserved02;
            unsigned char MP_SHARED_Reserved03;
        };
        unsigned int MP_SHARED_ACCESS;
	};
    unsigned int MP_SHARED_DATA;
    unsigned int RESERVED_04[(0x0880 - 0x085C)/4];
} VR5REG;


//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#endif // __NX_VR5_REGS_H__

