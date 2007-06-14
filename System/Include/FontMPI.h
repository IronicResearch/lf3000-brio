#ifndef LF_BRIO_FONTMPI_H
#define LF_BRIO_FONTMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		FontMPI.h
//
// Description:
//		Defines the interface for the private underlying Font module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <FontTypes.h>
#include <CoreMPI.h>
#include <ResourceTypes.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
class CFontMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;
			
	// class-specific functionality
	CFontMPI();
	virtual ~CFontMPI();

	// font-specific functionality
    tFontHndl   LoadFont(const CString* pName, tFontProp prop);
    tFontHndl   LoadFont(tRsrcHndl hRsrc, tFontProp prop);
    Boolean     UnloadFont(tFontHndl hFont);
    Boolean		SetFontAttr(tFontAttr attr);
    Boolean		GetFontAttr(tFontAttr* pAttr);
    Boolean     DrawString(CString* pStr, int x, int y, tFontSurf* pCtx);
    U32			GetX();
    U32			GetY();
    Boolean		GetFontMetrics(tFontMetrics* pMtx);
 
private:
	class CFontModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
