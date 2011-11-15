//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PNG.cpp
//
// Description:
//		PNG file save routine. 
//
//==============================================================================

#include <png.h>
#include <ImageIO.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <AtomicFile.h>

static jmp_buf	_jmpbuf;

//----------------------------------------------------------------------------
static void _png_write(png_structp pp, png_bytep data, png_size_t length)
{
	fwrite(data, 1, length, (FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
static void _png_read(png_structp pp, png_bytep data, png_size_t length)
{
	fread(data, 1, length, (FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
static void _png_flush(png_structp pp)
{
	fflush((FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
bool PNG_Save(CPath& path, tVideoSurf& surf)
{
	FILE*		fp = NULL;
	png_structp	pp = NULL;
	png_infop	pi = NULL;
	png_bytep 	bp = (png_bytep)surf.buffer;
	int			color; 

	// Supported RGB formats for PNG
	switch (surf.format) {
	case kPixelFormatRGB888:
		color = PNG_COLOR_TYPE_RGB;
		break;
	case kPixelFormatARGB8888:
		color = PNG_COLOR_TYPE_RGB_ALPHA;
		break;
	default:
		return false;
	}

	// Create PNG file for writing
	fp = fopenAtomic(path.c_str(), "wb");
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
		png_destroy_write_struct(&pp, NULL);
		fabortAtomic(fp);
		return false;
	}

	// Arcane PNG longjmp handling (despite no error callbacks?)
	setjmp(_jmpbuf); 
	
	// Set write callbacks
	png_set_write_fn(pp, fp, _png_write, _png_flush);

	// Write PNG info header
	png_set_IHDR(pp, pi, surf.width, surf.height, 8, color,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(pp, pi);

	// Write PNG image data per scanline
	for (int y = 0; y < surf.height; y++) {
		png_write_rows(pp, &bp, 1);
		bp += surf.pitch;
	}

	// Finalize PNG file for closing
	png_write_end(pp, pi);
	
	png_destroy_write_struct(&pp, NULL);
	
	fcloseAtomic(fp);
	return true;
}

//----------------------------------------------------------------------------
bool PNG_Load(CPath& path, tVideoSurf& surf)
{
	//FILE*		fp = NULL;
	png_structp	pp = NULL;
	png_infop	pi = NULL;
	
	// Open PNG file for reading
	fp = fopen(path.c_str(), "rb");
	if (!fp)
		return false;

	// Init PNG contexts for reading
	pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pp) {
		fclose(fp);
		return false;
	}
	
	pi = png_create_info_struct(pp);
	if (!pi) {
		png_destroy_read_struct(&pp, NULL, NULL);
		fclose(fp);
		return false;
	}
	
	// Arcane PNG longjmp handling (despite no error callbacks?)
	setjmp(_jmpbuf); 
	
	// Set read callbacks
	png_set_read_fn(pp, fp, _png_read);

	// Read PNG info header
	png_read_info(pp, pi);

	png_uint_32 width, height;
	int depth, color, interlace;
	png_get_IHDR(pp, pi, &width, &height, &depth, &color, 
			&interlace, NULL, NULL);
	
	// Dimension surface to hold image
	surf.width	= width;
	surf.height = height;
	switch (color) {
	case PNG_COLOR_TYPE_RGB:
		surf.pitch  = 3 * width;
		surf.format	= kPixelFormatRGB888;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		surf.pitch  = 4 * width;
		surf.format	= kPixelFormatARGB8888;
		break;
	default:
		png_destroy_read_struct(&pp, &pi, NULL);
		fclose(fp);
		return false;
	}

	// Allocate buffer here? -- Must be released by caller!
	surf.buffer = new U8[ (surf.pitch * surf.height) ];
	
	// Read PNG image data per scanline
	png_bytep 	bp = (png_bytep)surf.buffer;
	png_bytep 	dp = (png_bytep)surf.buffer;
	for (int y = 0; y < height; y++) {
		png_read_rows(pp, &bp, &dp, 1);
		bp += surf.pitch;
		dp += surf.pitch;
	}

	// Finalize PNG file for closing
	png_read_end(pp, pi);
	
	png_destroy_read_struct(&pp, &pi, NULL);
	
	fclose(fp);
	
	return true;
}

// EOF
