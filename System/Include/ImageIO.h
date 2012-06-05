//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ImageIO.h
//
// Description:
//		CmageIO is an utility class to load and save images that apps can use
//		without worrying about image format and rendering it on the screen.
//		CImageIO is not a full fledge image manipulating library but still provides limited support to read files as raw image data or save raw image data in various formats.
//		Follwing file formats are supported at the moment:
//		1) simple TGAs [ 24 bit RGB or 32 bit RGBA, encoded from left top]
//		2) jpegs [ 24 bit RGB or 32 bit RGBA ]
//		3) pngs [ 24 bit RGB or 32 bit RGBA ]
//
//==============================================================================
#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <CoreTypes.h>
#include <StringTypes.h>
#include <DisplayTypes.h>
#include <VideoTypes.h>

using namespace LeapFrog::Brio;

bool PNG_Load(CPath& path, tVideoSurf& surf);
bool PNG_Save(CPath& path, tVideoSurf& surf);
bool PNG_GetInfo(CPath& path, tVideoSurf& surf);

bool JPEG_Load(CPath& path, tVideoSurf& surf);
bool JPEG_Save(CPath& path, tVideoSurf& surf);
bool JPEG_Save(CPath& path, tVideoSurf& surfn, int imageQuality);
bool JPEG_GetInfo(CPath& path, tVideoSurf& surf);

bool TARGA_Load(CPath& path, tVideoSurf& surf);
bool TARGA_Save(CPath& path, tVideoSurf& surf);
bool TARGA_GetInfo(CPath& path, tVideoSurf& surf);

class CImageIO
{
public:
	CImageIO();
	~CImageIO();

	/// gets basic info ( height,width, pitch and format about the image in question
	/// \param path full path to image file
	/// \param surf surface object to hold image information
	///			If apps allocate memory to hold imagedata based on info they got with getInfo,
	///			then CimageIO will not create new buffer to hold image data.
	///			If they pass in NULL for tVideoSurf surface.buffer,
	///			then CImageIO will create new buffer.
	///			Note: In both cases, its app's responsibility to destroy the buffer after its usage.
	/// \retun true on success
	bool Load(CPath& path, tVideoSurf& surf);

	/// saves raw image data to file specified by full path based on extension
	/// \param path fullpath including extension to store the file
	/// \param surf video surface object which holds raw image data and its info
	/// \return true on success.

	bool Save(CPath& path, tVideoSurf& surfn);


	bool Save(CPath& path, tVideoSurf& surfn, int imageQuality);

	/// gets basic info ( height,width, pitch and format about the image in question
	/// \param path full path to image file
	/// \param surf surface object to hold image information
	/// \retun true on success
	bool GetInfo(CPath& path, tVideoSurf& surf);
};

#endif // IMAGEIO_H
