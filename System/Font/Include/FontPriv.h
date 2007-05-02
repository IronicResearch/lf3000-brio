#ifndef LF_BRIO_FONTPRIV_H
#define LF_BRIO_FONTPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		FontPriv.h
//
// Description:
//		Defines the interface for the private underlying Font manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <FontTypes.h>
#include <DebugMPI.h>

#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>
#define  USE_FONT_CACHE_MGR		1   
#if 	 USE_FONT_CACHE_MGR
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#endif

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kFontModuleName		= "Font";
const tVersion			kFontModuleVersion	= 2;
const tEventPriority	kFontEventPriority	= 0;

//==============================================================================
// Typedefs
//==============================================================================

// FreeType example struct for installed font face
typedef struct  TFont_
{
    const char*  	filepathname;	// cached font file name
    int          	face_index;
    int          	cmap_index;
    int          	num_indices;
    void*        	file_address;	// for preloaded files 
    size_t       	file_size;
	FT_Face			face;
} TFont, *PFont;

// Font library internal management
struct tFontInt {
	FT_Library		library;		// FreeType library instance
	FT_Face			face;			// font face instance
    PFont*          fonts;          // list of installed fonts
    int             num_fonts;		// number of fonts so far
    int             max_fonts;		// alloc'ed space for font list
    int				cur_font;		// current font index
    
#if USE_FONT_CACHE_MGR
	FTC_Manager     	cache_manager;     // the cache manager               
    FTC_ImageCache  	image_cache;       // the glyph image cache           
    FTC_SBitCache   	sbits_cache;       // the glyph small bitmaps cache   
    FTC_CMapCache   	cmap_cache;        // the charmap cache..             
    PFont           	current_font;      // selected font
    FTC_ImageTypeRec	image_type;
#endif
   
};

//==============================================================================
class CFontModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
    VTABLE_EXPORT Boolean		LoadFont(const CString* pName, tFontProp Prop);
    VTABLE_EXPORT Boolean     	UnloadFont();
    VTABLE_EXPORT Boolean		SetFontAttr(tFontAttr Attr);
    VTABLE_EXPORT Boolean		GetFontAttr(tFontAttr* pAttr);
    VTABLE_EXPORT Boolean     	DrawString(CString* pStr, int X, int Y, void* pCtx);
 
private:
	//void				InitModule( );
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CFontModule();
	virtual ~CFontModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
	
	// FreeType-specific functionality
	tFontInt			handle_;
	tFontAttr			attr_;
	int					curX_;
	int					curY_;
    Boolean     		DrawGlyph(char ch, int X, int Y, void* pCtx);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTPRIV_H

// eof
