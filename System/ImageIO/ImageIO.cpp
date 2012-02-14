//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ImageIO.cpp
//
// Description:
//		ImageIO class for importing/exporting image files. 
//
//==============================================================================

#include <ImageIO.h>
#include <stdio.h>
#include <string.h>

using namespace LeapFrog::Brio;

//----------------------------------------------------------------------------
CImageIO::CImageIO()
{
}

//----------------------------------------------------------------------------
CImageIO::~CImageIO()
{
}

//----------------------------------------------------------------------------
bool CImageIO::Load(CPath& path, tVideoSurf& surf)
{
	if (path.rfind(".png") != std::string::npos)
	{
		return PNG_Load(path, surf);

	}else if (path.rfind(".jpg") != std::string::npos)
	{
		return JPEG_Load(path, surf);

	}else if (path.rfind(".tga") != std::string::npos)
	{
		return TARGA_Load(path, surf);

	}

	return false;
}

//----------------------------------------------------------------------------
bool CImageIO::GetInfo(CPath& path, tVideoSurf& surf)
{
	if (path.rfind(".png") != std::string::npos)
	{
		return PNG_GetInfo(path, surf);

	}else if (path.rfind(".jpg") != std::string::npos)
	{
		return JPEG_GetInfo(path, surf);

	}else if (path.rfind(".tga") != std::string::npos)
	{
		return TARGA_GetInfo(path, surf);

	}

	return false;
}
//----------------------------------------------------------------------------
bool CImageIO::Save(CPath& path, tVideoSurf& surf)
{
	if (path.rfind(".png") != std::string::npos)
	{
		printf(" In ImageIO : now saving as png ");
		return PNG_Save(path, surf);

	}else if (path.rfind(".jpg") != std::string::npos)
	{
		printf(" In ImageIO : now saving as jpg ");
		return JPEG_Save(path, surf);

	}else if (path.rfind(".tga") != std::string::npos)
	{
		printf(" In ImageIO : now saving as tga ");
		return TARGA_Save(path, surf);

	}

	return false;
}
//----------------------------------------------------------------------------
bool CImageIO::Save(CPath& path, tVideoSurf& surf, int imageQuality)
{
	if (path.rfind(".png") != std::string::npos)
	{
		printf(" In ImageIO : now saving as png ");
		return PNG_Save(path, surf);

	}else if (path.rfind(".jpg") != std::string::npos)
	{
		printf(" In ImageIO : now saving as jpg ");
		return JPEG_Save(path, surf, imageQuality);

	}else if (path.rfind(".tga") != std::string::npos)
	{
		printf(" In ImageIO : now saving as tga ");
		return TARGA_Save(path, surf);

	}

	return false;
}

//----------------------------------------------------------------------------
// EOF
