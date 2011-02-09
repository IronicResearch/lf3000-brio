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
#include <KernelMPI.h>
#include <DisplayTypes.h> 

#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>
#include <ftglyph.h>
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
const tDebugLevel		kFontDebugLevel		= kDbgLvlCritical;

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
	FT_Size			size;			// for instanced point size
#if USE_FONT_CACHE_MGR
	FTC_ScalerRec	scaler;			// for instanced point size
#endif
	int				height;			// line-to-line spacing
	int				ascent;			// baseline location
	int				descent;		// remainder below baseline
	int				advance;		// max advance width
	U32				encoding;		// encoding property
	U32				loadFlags;		// load flags property
} TFont, *PFont;

// Font library internal management
struct tFontInt {
	FT_Library		library;		// FreeType library instance
	FT_Face			face;			// font face instance
#if USE_FONT_CACHE_MGR
	FTC_Manager     	cacheManager;      // the cache manager               
    FTC_ImageCache  	imageCache;        // the glyph image cache           
    FTC_SBitCache   	sbitsCache;        // the glyph small bitmaps cache   
    FTC_CMapCache   	cmapCache;         // the charmap cache..             
    FTC_ImageTypeRec	imageType;         // cached image record
#endif
    PFont           	currentFont;       // selected font
   
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
	VTABLE_EXPORT tErrType		SetFontResourcePath(const CPath &path);
	VTABLE_EXPORT CPath*		GetFontResourcePath() const;
	VTABLE_EXPORT tFontHndl		LoadFont(const CString* pName, tFontProp prop);
    VTABLE_EXPORT tFontHndl		LoadFont(const CPath& name, tFontProp prop);
    VTABLE_EXPORT tFontHndl		LoadFont(const CPath& name, U8 size);
    VTABLE_EXPORT tFontHndl		LoadFont(const CPath& name, U8 size, U32 encoding);
    VTABLE_EXPORT Boolean     	UnloadFont(tFontHndl hFont);
    VTABLE_EXPORT Boolean     	SelectFont(tFontHndl hFont);
    VTABLE_EXPORT Boolean		SetFontAttr(tFontAttr attr);
    VTABLE_EXPORT Boolean		GetFontAttr(tFontAttr* pAttr);
    VTABLE_EXPORT tFontAttr*	GetFontAttr();
    VTABLE_EXPORT Boolean		SetFontColor(U32 color) { attr_.color = color; return true; }
    VTABLE_EXPORT U32			GetFontColor() { return attr_.color; }
    VTABLE_EXPORT Boolean		SetFontAntiAliasing(Boolean antialias) { attr_.antialias = antialias; return true; }
    VTABLE_EXPORT Boolean		GetFontAntiAliasing() { return attr_.antialias; }
    VTABLE_EXPORT Boolean		SetFontKerning(Boolean kern) { attr_.useKerning = kern; return true; }
    VTABLE_EXPORT Boolean		GetFontKerning() { return attr_.useKerning; }
    VTABLE_EXPORT Boolean		SetFontUnderlining(Boolean underline) { attr_.useUnderlining = underline; return true; }
    VTABLE_EXPORT Boolean		GetFontUnderlining() { return attr_.useUnderlining; }
    VTABLE_EXPORT Boolean		SetFontRotation(tFontRotation rotation);
	VTABLE_EXPORT tFontRotation	GetFontRotation() { return attr_.rotation; }
	VTABLE_EXPORT Boolean     	DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx);
    VTABLE_EXPORT Boolean     	DrawString(CString& str, S32& x, S32& y, tFontSurf& surf);
    VTABLE_EXPORT Boolean     	DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap);
	VTABLE_EXPORT S32			GetX();
	VTABLE_EXPORT S32			GetY();
    VTABLE_EXPORT Boolean		GetFontMetrics(tFontMetrics* pMtx);
    VTABLE_EXPORT tFontMetrics*	GetFontMetrics();
    VTABLE_EXPORT Boolean     	GetStringRect(CString* pStr, tRect* pRect);
    VTABLE_EXPORT tRect*		GetStringRect(CString& str);

private:
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
		
	// FreeType-specific functionality
	tFontInt			handle_;
	tFontProp			prop_;
	tFontAttr			attr_;
	int					curX_;
	int					curY_;
	void				GetFace(FT_Face* pFace);
    Boolean     		GetGlyph(tWChar ch, FT_Glyph* pGlyph, int* pIndex);
    Boolean     		DrawGlyph(tWChar ch, int x, int y, tFontSurf* pCtx, bool isFirst);
    void				ConvertBitmapToRGB32(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertBitmapToRGB24(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertBitmapToRGB4444(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertBitmapToRGB565(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertGraymapToRGB32(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertGraymapToRGB24(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertGraymapToRGB4444(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
    void				ConvertGraymapToRGB565(FT_Bitmap* source, int x, int y, tFontSurf* pCtx);
	tFontHndl			LoadFontInt(const CString* pName, tFontProp prop, void* pFileImage, int fileSize);
	void 				ExpandBitmap(FT_Bitmap* source, FT_Bitmap* dest, int width, int height);
	void 				FreeBitmap(FT_Bitmap* dest);

	// Limit object creation to the Module Manager interface functions
	CFontModule();
	virtual ~CFontModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

	FT_Matrix			matrix_;		// auto calculated if using any rotation other than kFontCustomRotation
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_FONTPRIV_H

// eof
