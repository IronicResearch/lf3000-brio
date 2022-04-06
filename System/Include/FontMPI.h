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

/// \class CFontMPI
///
/// The font manager is a lightweight MPI which is used in conjunction with the 
/// Display manager or OpenGL. It is used for loading fonts of various styles and sizes, 
/// and rendering strings of text to any display surface. This rendering surface may be 
/// a 2D framebuffer surface accessed directly from the Display manager, or a memory 
/// buffer which is loaded as a 3D texture via OpenGL. In the present Brio implementation, 
/// the underlying font rendering engine used is FreeType.
///
/// Fonts are loaded as files from filesystem resources. Each font instance is created 
/// in a particular size property via the LoadFont() function. Upon successful loading, 
/// the font can be queried for metrics specific to that point size instance via 
/// GetFontMetrics().
///
/// In order to render a text string in the selected font, a font surface descriptor 
/// must be filled in with essential surface info such as the memory buffer address and 
/// dimensions in width, height, and pitch (bytes-per-scanline). The DrawString() 
/// function is used to render the selected text at X,Y position within the surface. 
///
/// Drawing attributes such as text color may be set as state variables via the 
/// SetFontAttr() function. This Brio implementation supports foreground color 
/// rendering only. For transparency effect over a background image, background pixels 
/// must already be present in the font surface buffer passed to DrawString().
///

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
	///
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

	/// LoadFont() variation for font size only
	///
	/// \param	name	Font file name relative to SetFontResourcePath(),
	/// or full absolute path name if leading slash
	///
	/// \param	size	Font property for size only
	///
	/// \return	Returns valid font handle on success, 
	/// or kInvalidFontHndl on failure.
	tFontHndl   LoadFont(const CPath& name, U8 size);

	/// LoadFont() variation for font size and encoding
	///
	/// \param	name	Font file name relative to SetFontResourcePath(),
	/// or full absolute path name if leading slash
	///
	/// \param	size	Font property for size 
	///
	/// \param	encoding	Font property for encoding type (StringTypes.h)
	///
	/// \return	Returns valid font handle on success, 
	/// or kInvalidFontHndl on failure.
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
    ///
    /// \return	Returns true on success.
    Boolean		SetFontAttr(tFontAttr attr);
    ///
    /// \param	color	Font drawing color attribute only 
    Boolean		SetFontColor(U32 color);
    ///
    /// \param	antialias	Font anti-aliasing attribute only 
    Boolean		SetFontAntiAliasing(Boolean antialias);
    ///
    /// \param	kern		Font kerning attribute only
    Boolean		SetFontKerning(Boolean kern);
    ///
    ///	\param	underline	Font underling attribute only
    Boolean		SetFontUnderlining(Boolean underline);
    ///
    ///	\param	rotation	Font underling attribute only
    Boolean		SetFontRotation(tFontRotation rotation);
    
    /// Gets the font's current drawing attributes
    ///
    /// \param	pAttr	Pointer to font drawing attributes struct (versioned)
    ///
    /// \return	Returns true on success.
    Boolean		GetFontAttr(tFontAttr* pAttr);
    /// 
    /// \return	Returns pointer to font drawing attributes struct (alternate)
    tFontAttr*	GetFontAttr();
    ///
    /// \return	Returns the current font drawing color attribute
    U32			GetFontColor();
    ///
    /// \return	Returns the current font anti-aliasing attribute
    Boolean		GetFontAntiAliasing();
    ///
    /// \return Returns the current font kerning attribute
    Boolean		GetFontKerning();
    ///
    /// \return Returns the current font underlining attribute
    Boolean		GetFontUnderlining();
    ///
    /// \return Returns the current font rotation attribute
    tFontRotation	GetFontRotation();
    
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
    ///
    /// \param	str		Reference to text string to draw
    ///
    /// \param	x		X destination to draw 1st string character
    ///
    /// \param	y		Y destination to draw 1st string character
    ///
    /// \param 	surf	Reference to font surface descriptor
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf);

    /// DrawString() variation with additional parameter for text wrapping.
    ///
    /// \param	str		Reference to text string to draw
    ///
    /// \param	x		X destination to draw 1st string character
    ///
    /// \param	y		Y destination to draw 1st string character
    ///
    /// \param 	surf	Reference to font surface descriptor
    ///
    /// \param 	bWrap	Option for wrapping text to font surface area
    Boolean     DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap);
    
    /// Returns the current X drawing position updated by DrawString()
    S32			GetX();
    
    /// Returns the current Y drawing position updated by DrawString()
    S32			GetY();
    
    /// Returns the current metrics for the loaded font and property size
    ///
    /// \param	pMtx	Pointer to font metrics struct to be filled in
    Boolean		GetFontMetrics(tFontMetrics* pMtx);
    ///
    /// \return	Returns pointer to font metrics struct (alternate)
    tFontMetrics* GetFontMetrics();
 
 	/// Returns the bounding rectangle for the selected text string
    /// 
    /// \param	pStr	Pointer to text string to calculate bounding rectangle
    ///
    /// \param	pRect	Pointer to bounding rectangle to be filled in
 	Boolean		GetStringRect(CString* pStr, tRect* pRect);
 	///
 	/// \return	Returns pointer to bounding rectangle (alternate) 
 	tRect*		GetStringRect(CString& str);
 	
private:
	class CFontModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTMPI_H

// EOF
