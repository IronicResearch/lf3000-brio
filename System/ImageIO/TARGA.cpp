#include <ImageIO.h>
#include <string.h>
#include <stdlib.h>
#include <AtomicFile.h>

static FILE* tga;

//----------------------------------------------------------------------------
bool TARGA_Save(CPath& path, tVideoSurf& surf)
{
	FILE	*fp;

	int byteCount;

	if(surf.format == kPixelFormatRGB888)
	{
		printf("\n ##### 3 #####\n");
		byteCount = 3;
	}else if(surf.format == kPixelFormatARGB8888)
	{
		printf("\n ##### 4 #####\n");
		byteCount = 4;
	}else{
		printf("\n Can not save a TGA file if its not 24 bit or 32 bit true color.");
		return false;
	}

	fp = fopenAtomic(path.c_str(), "wb");

	if( !fp )
		return false;

	fputc( 0, fp );							// identification field length
	fputc( 0, fp );							// no color map
	fputc( 2, fp );							// picture type = uncompressed RGB
	fputc( 0, fp ); fputc( 0, fp );			// 1st color index in the map
	fputc( 0, fp ); fputc( 0, fp );			// nb colors in the map
	fputc( 0, fp );							// nb bits/color in the map
	fputc( 0, fp ); fputc( 0, fp );			// x-coordinate of the origin
	fputc( 0, fp ); fputc( 0, fp );			// y-coordinate of the origin

	fputc( surf.width & 255, fp );
	fputc( surf.width>>8, fp );					// picture width

	fputc( surf.height & 255, fp );
	fputc( surf.height>>8, fp );					// picture height

	fputc( byteCount*8, fp );						// nb bits per pixel
	fputc(0, fp );							// FLAG		0  32

	U8	temp;
	for( int i=0; i < surf.width * surf.height * byteCount; i+= byteCount )
	{
		temp = surf.buffer[i];
		surf.buffer[i]   = surf.buffer[i+2];
		surf.buffer[i+2] = temp;
	}
	fwrite( surf.buffer, sizeof(U8), surf.width * surf.height * byteCount, fp );	// write graphic data

	fcloseAtomic(fp);

	return true;

}

// Load is now split into 2 parts, getinfo and load,
// getinfo populates surf object passed by caller function, with height width pitch and format
// by just reading header
// caller can then create buffer and call load
// if buffer was created by caller, load fill in image data in that buffer else it will create buffer and then fill image data.

bool TARGA_GetInfo(CPath& path, tVideoSurf& surf)
{
	FILE *file;
	unsigned char type[4];
	unsigned char info[6];

	file = fopen(path.c_str(), "rb");

	if (!file)
	return false;

	fread (&type, sizeof (char), 3, file);
	fseek (file, 12, SEEK_SET);
	fread (&info, sizeof (char), 6, file);

	surf.width = info[0] + info[1] * 256;
	surf.height = info[2] + info[3] * 256;

	int byteCount = info[4] / 8;
	switch(byteCount)
	{
	case 3:
		surf.format = kPixelFormatRGB888;
		surf.pitch = surf.width * 3;
		break;
	case 4:
		surf.format = kPixelFormatARGB8888;
		surf.pitch = surf.width * 4;
		break;
	}
	if (byteCount != 3 && byteCount != 4) {
		fclose(file);
		return false;
	}
	//close file
	fclose(file);

	return true;
}


bool TARGA_Load(CPath& path, tVideoSurf& surf)
{
	FILE *file;
	bool up = false;
	unsigned char headerInfo[18];//0 = id length, 1 = color map type, 2=image type

	file = fopen(path.c_str(), "rb");
	if (!file)
		return false;

	fread (&headerInfo, sizeof (char), 18, file);

	surf.width = headerInfo[12]+ headerInfo[13] * 256;
	surf.height = headerInfo[14] + headerInfo[15] * 256;

	int byteCount = headerInfo[16] / 8;

	switch(byteCount)
	{
	case 3:
		surf.format = kPixelFormatRGB888;
		surf.pitch = surf.width * 3;
		break;
	case 4:
		surf.format = kPixelFormatARGB8888;
		surf.pitch = surf.width * 4;
		break;
	}

	if (byteCount != 3 && byteCount != 4) {
		fclose(file);
		return false;
	}

	//decide how tga was encoded ( topleft / bottom right etc.. )
	if(headerInfo[17] & 32)
		up = true;


	long imageSize = surf.width * surf.height * byteCount;
	// Allocate buffer here? -- Must be released by caller!
	//[MD] if caller has already created a buffer, use that else make new one.

	//TODO: Discuss best way to manage buffers especially given the new get info feature.
	if(surf.buffer == NULL)
		surf.buffer = new U8[ imageSize ];

	//read in image data
	int imageDataOffset = headerInfo[0] + headerInfo[1] + 18;
	int a = fseek (file, imageDataOffset, SEEK_SET);

	//based on origin, read file
	size_t result;
	if(!up) // the pic is upside down
	{
		int	offset = (surf.height -1)* surf.width * byteCount;

		for( int y=0; y<surf.height; y++ )
		{
			result = fread( (U8*)surf.buffer+offset, sizeof(U8), surf.width*byteCount, file );
			offset -= surf.width*byteCount;
		}

	}else{ // the picture is not reversed

		result = fread((U8*)surf.buffer, sizeof(U8), imageSize, file);
	}

	if(result != imageSize) printf("\n Error reading file");

	//close file
	fclose(file);

	return true;

}


