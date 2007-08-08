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
#include <DisplayTypes.h>

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
	
	// Loads a font via its file name for a selected property size (deprecated)
//	tFontHndl   LoadFont(const CString* pName, tFontProp prop);
    
    // Loads a font via its associated resource for a selected property size
    tFontHndl   LoadFont(tRsrcHndl hRsrc, tFontProp prop);
    
    // Unloads the font loaded by LoadFont()
    Boolean     UnloadFont(tFontHndl hFont);
    
    // Selects the font loaded by LoadFont()
    Boolean     SelectFont(tFontHndl hFont);
    
    // Sets the font's drawing attributes
    Boolean		SetFontAttr(tFontAttr attr);
    
    // Gets the font's current drawing attributes
    Boolean		GetFontAttr(tFontAttr* pAttr);
    
    // Draws a text string at selected X,Y position in display surface context
    Boolean     DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx);
    
    // Returns the current X drawing position updated by DrawString()
    S32			GetX();
    
    // Returns the current Y drawing position updated by DrawString()
    S32			GetY();
    
    // Returns the current metrics for the loaded font and property size
    Boolean		GetFontMetrics(tFontMetrics* pMtx);
 
 	// Returns the bounding rectangle for the selected text string
 	Boolean		GetStringRect(CString *pStr, tRect* pRect);
 
private:
	class CFontModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
