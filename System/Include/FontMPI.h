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
	///
	/// \param path	Base path for files referenced by calls to LoadFont()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetFontResourcePath(const CPath& path);

	/// Returns the font resource path set by SetFontResourcePath()
	CPath*		GetFontResourcePath() const;
	
	/// Loads a font via its file name for a selected property size
	///
	/// \param	name	Font file name relative to SetFontResourcePath(),
	/// or full absolute path name if leading slash
	///
	/// \param	prop	Font property struct for size and encoding
	///
	/// \return	Returns valid font handle on success, 
	/// or kInvalidFontHndl on failure.
	tFontHndl   LoadFont(const CPath& name, tFontProp prop);
	/// \param	size	Font property for size only
    tFontHndl   LoadFont(const CPath& name, U8 size);
    /// \param	encoding	Font property for encoding 
    tFontHndl   LoadFont(const CPath& name, U8 size, U32 encoding);
    
    /// Unloads the font loaded by LoadFont()
    ///
    /// \param	hFont	Font handle returned by LoadFont()
    ///
    /// \return	Returns true on success.
    Boolean     UnloadFont(tFontHndl hFont);
    
    /// Selects the font loaded by LoadFont()
    ///
    /// \param	hFont	Font handle returned by LoadFont()
    ///
    /// \return	Returns true on success.
    Boolean     SelectFont(tFontHndl hFont);
    
    /// Sets the font's drawing attributes
    ///
    /// \param	attr	Font drawing attributes struct (versioned)
    Boolean		SetFontAttr(tFontAttr attr);
    /// \param	color	Font drawing color attribute only
    Boolean		SetFontColor(U32 color);
    /// \param	antialias	Font anti-aliasing attribute only
    Boolean		SetFontAntiAliasing(Boolean antialias);
    /// \param	kern		Font kerning attribute only
    Boolean		SetFontKerning(Boolean kern);
    ///	\param	underlining	Font underling attribute only
    Boolean		SetFontUnderlining(Boolean underline);
    
    /// Gets the font's current drawing attributes
    ///
    /// \param	pAttr	Pointer to font drawing attributes struct (versioned)
    ///
    /// \return	Returns true on success.
    Boolean		GetFontAttr(tFontAttr* pAttr);
    /// \return	Returns pointer to font drawing attributes struct (alternate)
    tFontAttr*	GetFontAttr();
    /// \return	Returns the current font drawing color attribute
    U32			GetFontColor();
    /// \return	Returns the current font anti-aliasing attribute
    Boolean		GetFontAntiAliasing();
    /// \return Returns the current font kerning attribute
    Boolean		GetFontKerning();
    /// \return Returns the current font underlining attribute
    Boolean		GetFontUnderlining();
    
    /// Draws a text string at selected X,Y position in display surface context
    ///
    /// \param	pStr	Pointer to text string to draw
    ///
    /// \param	x		X destination to draw 1st string character
    ///
    /// \param	y		Y destination to draw 1st string character
    ///
    /// \param 	pCtx	Pointer to font surface descriptor
    ///
    /// \return			Returns true on success.
    Boolean     DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx);
    /// DrawString() variation with parameters passed by reference.
    /// X and Y parameters are updated with the last character glyph
    /// pixel coordinate positions for subsequent calls.
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf);
    /// DrawString() variation with additional parameter for text wrapping.
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap);
    
    /// Returns the current X drawing position updated by DrawString()
    S32			GetX();
    
    /// Returns the current Y drawing position updated by DrawString()
    S32			GetY();
    
    /// Returns the current metrics for the loaded font and property size
    ///
    /// \param	pMtx	Pointer to font metrics struct to be filled in
    Boolean		GetFontMetrics(tFontMetrics* pMtx);
    /// \return	Returns pointer to font metrics struct (alternate)
    tFontMetrics* GetFontMetrics();
 
 	/// Returns the bounding rectangle for the selected text string
    /// 
    /// \param	pStr	Pointer to text string to calculate bounding rectangle
    ///
    /// \param	pRect	Pointer to bounding rectangle to be filled in
 	Boolean		GetStringRect(CString* pStr, tRect* pRect);
 	/// \return	Returns pointer to bounding rectangle (alternate) 
 	tRect*		GetStringRect(CString& str);
 	
private:
	class CFontModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
