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
//		Defines the Module Public Interface (MPI) for the Font module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <FontTypes.h>
#include <CoreMPI.h>
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
	
	/// Sets the default font resource path for subsequent LoadFont() calls
	tErrType	SetFontResourcePath(const CPath& path);
	CPath*		GetFontResourcePath() const;
	
	/// Loads a font via its file name for a selected property size
	tFontHndl   LoadFont(const CPath& name, tFontProp prop);
    tFontHndl   LoadFont(const CPath& name, U8 size);
    tFontHndl   LoadFont(const CPath& name, U8 size, U32 encoding);
    
    /// Unloads the font loaded by LoadFont()
    Boolean     UnloadFont(tFontHndl hFont);
    
    /// Selects the font loaded by LoadFont()
    Boolean     SelectFont(tFontHndl hFont);
    
    /// Sets the font's drawing attributes
    Boolean		SetFontAttr(tFontAttr attr);
    Boolean		SetFontColor(U32 color);
    Boolean		SetFontAntiAliasing(Boolean antialias);
    Boolean		SetFontKerning(Boolean kern);
    Boolean		SetFontUnderlining(Boolean underline);
    
    /// Gets the font's current drawing attributes
    Boolean		GetFontAttr(tFontAttr* pAttr);
    tFontAttr*	GetFontAttr();
    U32			GetFontColor();
    Boolean		GetFontAntiAliasing();
    Boolean		GetFontKerning();
    Boolean		GetFontUnderlining();
    
    /// Draws a text string at selected X,Y position in display surface context
    Boolean     DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx);
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf);
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap);
    
    /// Returns the current X drawing position updated by DrawString()
    S32			GetX();
    
    /// Returns the current Y drawing position updated by DrawString()
    S32			GetY();
    
    /// Returns the current metrics for the loaded font and property size
    Boolean		GetFontMetrics(tFontMetrics* pMtx);
    tFontMetrics* GetFontMetrics();
 
 	/// Returns the bounding rectangle for the selected text string
 	Boolean		GetStringRect(CString* pStr, tRect* pRect);
 	tRect*		GetStringRect(CString& str);
 	
private:
	class CFontModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
