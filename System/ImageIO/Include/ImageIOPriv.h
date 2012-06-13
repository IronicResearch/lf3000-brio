#ifndef LF_BRIO_IMAGEIOPRIV_H
#define LF_BRIO_IMAGEIOPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ImageIOPriv.h
//
// Description:
//		Private include header for ImageIO module.
//
//==============================================================================

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

#endif // LF_BRIO_IMAGEIOPRIV_H
