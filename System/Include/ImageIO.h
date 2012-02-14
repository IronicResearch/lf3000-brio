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
//		ImageIO class for importing/exporting image files. 
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

	bool Load(CPath& path, tVideoSurf& surf);
	bool Save(CPath& path, tVideoSurf& surfn);
	bool Save(CPath& path, tVideoSurf& surfn, int imageQuality);
	bool GetInfo(CPath& path, tVideoSurf& surf);
};

#endif // IMAGEIO_H
