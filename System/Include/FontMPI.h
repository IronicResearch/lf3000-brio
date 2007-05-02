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
    Boolean     LoadFont(const CString* pName, tFontProp Prop);
    Boolean     UnloadFont();
    Boolean		SetFontAttr(tFontAttr Attr);
    Boolean		GetFontAttr(tFontAttr* pAttr);
    Boolean     DrawString(CString* pStr, int X, int Y, void* pCtx);

private:
	class CFontModule*	pModule_;
	U32					id_;
	tFontProp			prop_;
	tFontAttr			attr_;
//	tFontInt			handle_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
