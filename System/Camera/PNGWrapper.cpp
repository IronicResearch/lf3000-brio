//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PNGWrapper.cpp
//
// Description:
//		PNG file save routine. 
//
//==============================================================================

#include <png.h>
#include <AtomicFile.h>

static jmp_buf	_jmpbuf;

//----------------------------------------------------------------------------
static void _png_write(png_structp pp, png_bytep data, png_size_t length)
{
	fwrite(data, 1, length, (FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
static void _png_flush(png_structp pp)
{
	fflush((FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
bool PNG_save(const char* file, int width, int height, int pitch, char* data)
{
	FILE*		fp = NULL;
	png_structp	pp = NULL;
	png_infop	pi = NULL;
	png_bytep 	bp = (png_bytep)data;
	
	// Create PNG file for writing
	fp = fopenAtomic(file, "wb");
	if (!fp)
		return false;

	// Init PNG contexts for writing
	pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pp) {
		fabortAtomic(fp);
		return false;
	}
	
	pi = png_create_info_struct(pp);
	if (!pi) {
		fabortAtomic(fp);
		return false;
	}
	
	// Arcane PNG longjmp handling (despite no error callbacks?)
	setjmp(_jmpbuf); 
	
	// Set write callbacks
	png_set_write_fn(pp, fp, _png_write, _png_flush);

	// Write PNG info header
	png_set_IHDR(pp, pi, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(pp, pi);

	// Write PNG image data per scanline
	for (int y = 0; y < height; y++) {
		png_write_rows(pp, &bp, 1);
		bp += pitch;
	}

	// Finalize PNG file for closing
	png_write_end(pp, pi);
	
	png_destroy_write_struct(&pp, NULL);
	
	fcloseAtomic(fp);
	
	return true;
}

// EOF
