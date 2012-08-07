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
//		ImageIO unnamed compatibility class for legacy support.
//
//==============================================================================

#ifdef LF1000
#include <ImageIO.h>

using namespace LeapFrog::Brio;

class CImageIO : public LeapFrog::Brio::CImageIO
{
public:
	CImageIO();
	~CImageIO();
	bool Load(CPath& path, tVideoSurf& surf);
	bool Save(CPath& path, tVideoSurf& surf);
	bool Save(CPath& path, tVideoSurf& surf, int imageQuality);
	bool GetInfo(CPath& path, tVideoSurf& surf);
};

//----------------------------------------------------------------------------
::CImageIO::CImageIO()
{
}

//----------------------------------------------------------------------------
::CImageIO::~CImageIO()
{
}

//----------------------------------------------------------------------------
bool ::CImageIO::Load(CPath& path, tVideoSurf& surf)
{
	return LeapFrog::Brio::CImageIO::Load(path, surf);
}

//----------------------------------------------------------------------------
bool ::CImageIO::GetInfo(CPath& path, tVideoSurf& surf)
{
	return LeapFrog::Brio::CImageIO::GetInfo(path, surf);
}
//----------------------------------------------------------------------------
bool ::CImageIO::Save(CPath& path, tVideoSurf& surf)
{
	return LeapFrog::Brio::CImageIO::Save(path, surf);
}
//----------------------------------------------------------------------------
bool ::CImageIO::Save(CPath& path, tVideoSurf& surf, int imageQuality)
{
	return LeapFrog::Brio::CImageIO::Save(path, surf, imageQuality);
}
#endif

//----------------------------------------------------------------------------
// EOF
