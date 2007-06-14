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
#include <ResourceTypes.h>

// TODO: The FreeType headers may need to be neutralized for target OS
#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>
#define  USE_FONT_CACHE_MGR		1   
#if 	 USE_FONT_CACHE_MGR
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#endif

#define DBG						0
#if		DBG
#define	PRINTF					printf
#else
#define	PRINTF					(void)
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
    int          	faceIndex;
    int          	cmapIndex;
    int          	numIndices;
    void*        	fileAddress;	// for preloaded files 
    size_t       	fileSize;
	FT_Face			face;
	int				height;			// line-to-line spacing
	int				ascent;			// baseline location
	int				descent;		// remainder below baseline
	int				advance;		// max advance width
} TFont, *PFont;

// Font library internal management
struct tFontInt {
	FT_Library		library;		// FreeType library instance
	FT_Face			face;			// font face instance
    PFont*          fonts;          // list of installed fonts
    int             numFonts;		// number of fonts so far
    int             maxFonts;		// alloc'ed space for font list
    int				curFont;		// current font index
    
#if USE_FONT_CACHE_MGR
	FTC_Manager     	cacheManager;      // the cache manager               
    FTC_ImageCache  	imageCache;        // the glyph image cache           
    FTC_SBitCache   	sbitsCache;        // the glyph small bitmaps cache   
    FTC_CMapCache   	cmapCache;         // the charmap cache..             
    PFont           	currentFont;       // selected font
    FTC_ImageTypeRec	imageType;         // cached image record
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
    VTABLE_EXPORT tFontHndl		LoadFont(const CString* pName, tFontProp prop);
    VTABLE_EXPORT tFontHndl		LoadFont(tRsrcHndl hRsrc, tFontProp prop);
    VTABLE_EXPORT Boolean     	UnloadFont(tFontHndl hFont);
    VTABLE_EXPORT Boolean		SetFontAttr(tFontAttr attr);
    VTABLE_EXPORT Boolean		GetFontAttr(tFontAttr* pAttr);
    VTABLE_EXPORT Boolean     	DrawString(CString* pStr, int x, int y, tFontSurf* pCtx);
	VTABLE_EXPORT U32			GetX();
	VTABLE_EXPORT U32			GetY();
    VTABLE_EXPORT Boolean		GetFontMetrics(tFontMetrics* pMtx);

private:
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
    Boolean     		DrawGlyph(char ch, int x, int y, tFontSurf* pCtx);
    void				ConvertBitmapToRGB32(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTPRIV_H

// eof
