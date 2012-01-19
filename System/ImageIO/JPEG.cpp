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
//		JPEG file save routine.
//
//==============================================================================
#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <ImageIO.h>

static FILE* fp;
/*static jmp_buf	_jmpbuf;

//----------------------------------------------------------------------------
static void _jpeg_write(png_structp pp, png_bytep data, png_size_t length)
{
	fwrite(data, 1, length, (FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
static void _jpeg_read(png_structp pp, png_bytep data, png_size_t length)
{
	fread(data, 1, length, (FILE*)pp->io_ptr);
}

//----------------------------------------------------------------------------
static void _jpeg_flush(png_structp pp)
{
	fflush((FILE*)pp->io_ptr);
}
*/
//----------------------------------------------------------------------------
bool JPEG_Save(CPath& path, tVideoSurf& surf)
{
	int width = surf.width;
	int height = surf.height;
	int bytes_per_pixel = surf.pitch / surf.width;
	//int color_space = JCS_RGB;

	U8* raw_image = surf.buffer;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	/* this is a pointer to one row of image data */
	JSAMPROW row_pointer[1];

	FILE *outfile = fopen( path.c_str(), "wb" );

	if ( !outfile )
	{
		printf("Error opening output jpeg file %s\n!",  path.c_str() );
		return false;
	}

	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	/* Setting the parameters of the output file here */
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = JCS_RGB;
	/* default compression parameters, we shouldn't be worried about these */
	jpeg_set_defaults( &cinfo );
	/* Now do the compression .. */
	jpeg_start_compress( &cinfo, TRUE );
	/* like reading a file, this time write one row at a time */
	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	/* similar to read file, clean up after we're done compressing */
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	fclose( outfile );
	/* success code is 1! */
	return true;
}

// Load is now split into 2 parts, getinfo and load,
// getinfo populates surf object passed by caller function, with height width pitch and format
// by just reading header
// caller can then create buffer and call load
// if buffer was created by caller, load fill in image data in that buffer else it will create buffer and then fill image data.

bool JPEG_GetInfo(CPath& path, tVideoSurf& surf)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];

	FILE *infile = fopen( path.c_str(), "rb" );
	unsigned long location = 0;
	int i = 0;

	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", path.c_str() );
		return -1;
	}


	cinfo.err = jpeg_std_error( &jerr );

	jpeg_create_decompress( &cinfo );

	jpeg_stdio_src( &cinfo, infile );

	jpeg_read_header( &cinfo, TRUE );

	surf.width = cinfo.image_width;
	surf.height = cinfo.image_height;
	surf.pitch = cinfo.image_width * 3;//cinfo.input_components;
	surf.format = kPixelFormatRGB888;

	fclose( infile );
	/* yup, we succeeded! */
	return true;
}


//----------------------------------------------------------------------------
bool JPEG_Load(CPath& path, tVideoSurf& surf)
{

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];

	FILE *infile = fopen( path.c_str(), "rb" );
	unsigned long location = 0;
	int i = 0;

	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", path.c_str() );
		return -1;
	}


	cinfo.err = jpeg_std_error( &jerr );

	jpeg_create_decompress( &cinfo );

	jpeg_stdio_src( &cinfo, infile );

	jpeg_read_header( &cinfo, TRUE );


	surf.width = cinfo.image_width;
	surf.height = cinfo.image_height;
	surf.pitch = cinfo.image_width * 3;//cinfo.input_components;
	surf.format = kPixelFormatRGB888;

	/* Start decompression jpeg here */
	jpeg_start_decompress( &cinfo );

	//raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	/* now actually read the jpeg into the raw buffer */

	/* read one scan line at a time */

	//U8* raw_image = surf.buffer;
	/* allocate memory to hold the uncompressed image */

	// Allocate buffer here? -- Must be released by caller!
	//[MD] if caller has already created a buffer, use that else make new one.

	//TODO: Discuss best way to manage buffers especially given the new get info feature.
	if(surf.buffer == NULL)
		surf.buffer = new U8[ (surf.pitch * surf.height) ];


	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );


	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
			surf.buffer[location++] = row_pointer[0][i];
	}


	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	free( row_pointer[0] );

	fclose( infile );
	/* yup, we succeeded! */
	return true;
}

// EOF
