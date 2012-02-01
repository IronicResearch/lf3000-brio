#include <ImageIO.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <AtomicFile.h>

static FILE* tga;
#define TGA_TRUECOLOR_32      (4)
#define TGA_TRUECOLOR_24      (3)

static U32 htotl( U32 val ) {

#ifdef WORDS_BIGENDIAN
    return( ((val & 0x000000FF) << 24) +
            ((val & 0x0000FF00) << 8)  +
            ((val & 0x00FF0000) >> 8)  +
            ((val & 0xFF000000) >> 24) );
#else
    return( val );
#endif

}
//----------------------------------------------------------------------------
bool TARGA_Save(CPath& path, tVideoSurf& surf)
{
	int width  = surf.width;
	int height = surf.height;
	U8* dat    = surf.buffer;
	U8 format = TGA_TRUECOLOR_24;

		U32 i, j;

	    U32 size = width * height;

	    float red, green, blue, alpha;

	    char id[] = "written with libtarga";
	    U8 idlen = 21;
	    U8 zeroes[5] = { 0, 0, 0, 0, 0 };
	    U32 pixbuf;
	    U8 one = 1;
	    U8 cmap_type = 0;
	    U8 img_type  = 2;  // 2 - uncompressed truecolor  10 - RLE truecolor
	    U16 xorigin  = 0;
	    U16 yorigin  = 0;
	    U8  pixdepth = format * 8;  // bpp
	    U8 img_desc;

	    switch( format ) {

	    case TGA_TRUECOLOR_24:
	        img_desc = 0;
	        break;

	    case TGA_TRUECOLOR_32:
	        img_desc = 8;
	        break;

	    default:
	        //TargaError = TGA_ERR_BAD_FORMAT;
	        return( 0 );
	        break;

	    }

	    tga = fopen( path.c_str(), "wb" );

	    if( tga == NULL ) {
	        //TargaError = TGA_ERR_OPEN_FAILS;
	        return( 0 );
	    }

	    // write id length
	    fwrite( &idlen, 1, 1, tga );

	    // write colormap type
	    fwrite( &cmap_type, 1, 1, tga );

	    // write image type
	    fwrite( &img_type, 1, 1, tga );

	    // write cmap spec.
	    fwrite( &zeroes, 5, 1, tga );

	    // write image spec.
	    fwrite( &xorigin, 2, 1, tga );
	    fwrite( &yorigin, 2, 1, tga );
	    fwrite( &width, 2, 1, tga );
	    fwrite( &height, 2, 1, tga );
	    fwrite( &pixdepth, 1, 1, tga );
	    fwrite( &img_desc, 1, 1, tga );

	    // write image id.
	    fwrite( &id, idlen, 1, tga );

	    // color correction -- data is in RGB, need BGR.
	    for( i = 0; i < size; i++ ) {

	        pixbuf = 0;
	        for( j = 0; j < format; j++ ) {
	            pixbuf += dat[i*format+j] << (8 * j);
	        }

	        switch( format ) {

	        case TGA_TRUECOLOR_24:

	            pixbuf = ((pixbuf & 0xFF) << 16) +
	                     (pixbuf & 0xFF00) +
	                     ((pixbuf & 0xFF0000) >> 16);

	            pixbuf = htotl( pixbuf );

	            fwrite( &pixbuf, 3, 1, tga );

	            break;

	        case TGA_TRUECOLOR_32:

	            /* need to un-premultiply alpha.. */

	            red     = (pixbuf & 0xFF) / 255.0f;
	            green   = ((pixbuf & 0xFF00) >> 8) / 255.0f;
	            blue    = ((pixbuf & 0xFF0000) >> 16) / 255.0f;
	            alpha   = ((pixbuf & 0xFF000000) >> 24) / 255.0f;

	            if( alpha > 0.0001 ) {
	                red /= alpha;
	                green /= alpha;
	                blue /= alpha;
	            }

	            /* clamp to 1.0f */

	            red = red > 1.0f ? 255.0f : red * 255.0f;
	            green = green > 1.0f ? 255.0f : green * 255.0f;
	            blue = blue > 1.0f ? 255.0f : blue * 255.0f;
	            alpha = alpha > 1.0f ? 255.0f : alpha * 255.0f;

	            pixbuf = (U8)blue + (((U8)green) << 8) +
	                (((U8)red) << 16) + (((U8)alpha) << 24);

	            pixbuf = htotl( pixbuf );

	            fwrite( &pixbuf, 4, 1, tga );

	            break;

	        }

	    }

	    fclose( tga );

	    return true ;

}

// Load is now split into 2 parts, getinfo and load,
// getinfo populates surf object passed by caller function, with height width pitch and format
// by just reading header
// caller can then create buffer and call load
// if buffer was created by caller, load fill in image data in that buffer else it will create buffer and then fill image data.

bool TARGA_GetInfo(CPath& path, tVideoSurf& surf)
{

}


bool TARGA_Load(CPath& path, tVideoSurf& surf)
{
	return true;
}


