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
///		\class CImageIO
///		CImageIO is an utility class to load and save images that apps can use
///		without worrying about image format and rendering it on the screen.
///		CImageIO is not a full fledge image manipulating library but still provides limited support to read files as raw image data or save raw image data in various formats.
///		Following file formats are supported at the moment:
///		1) simple TGAs (.tga) [ 24 bit RGB or 32 bit RGBA, encoded from left top]
///		2) JPEGs (.jpg) [ 24 bit RGB or 32 bit RGBA ]
///		3) PNGs (.png) [ 24 bit RGB or 32 bit RGBA ]
//
//==============================================================================
#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <CoreTypes.h>
#include <StringTypes.h>
#include <DisplayTypes.h>
#include <VideoTypes.h>

using namespace LeapFrog::Brio;

LF_BEGIN_BRIO_NAMESPACE()
class CImageIO
{
public:
	CImageIO();
	~CImageIO();

	/// \brief Loads image data from file specified by full path based on extension
	/// \param path full path to image file
	/// \param surf surface object to hold image information
	///			If apps allocate memory to hold image data based on info they got with GetInfo(),
	///			and pass that in as non-NULL surface.buffer pointer,
	///			then CimageIO will not create new buffer to hold image data.
	///			If they pass in NULL for tVideoSurf surface.buffer,
	///			then CImageIO will create new buffer.
	///			Note: In both cases, its app's responsibility to destroy the buffer after its usage.
	/// \return true on success
	bool Load(CPath& path, tVideoSurf& surf);

	/// \brief Saves raw image data to file specified by full path based on extension
	/// \param path fullpath including extension to store the file
	/// \param surf video surface object which holds raw image data and its info
	/// \return true on success.

	bool Save(CPath& path, tVideoSurf& surf);


	bool Save(CPath& path, tVideoSurf& surf, int imageQuality);

	/// \brief Gets basic info (height, width, pitch and format) about the image in question
	/// \param path full path to image file
	/// \param surf surface object to hold image information
	/// \return true on success
	bool GetInfo(CPath& path, tVideoSurf& surf);
};
LF_END_BRIO_NAMESPACE()

#endif // IMAGEIO_H
