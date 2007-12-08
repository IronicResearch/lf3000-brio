//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Font.cpp
//
// Description:
//		Configure emulation for the environment
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <FontTypes.h>
#include <FontPriv.h>
#include <KernelMPI.h>
#if USE_RSRC_MGR
#include <ResourceMPI.h>
#endif

#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>
#include <ftglyph.h>
#include <ftsizes.h>
#include FT_TRUETYPE_IDS_H

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Font";

namespace
{
	CPath				gpath = "";
}

//============================================================================
// CFontModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CFontModule::GetModuleVersion() const
{
	return kFontModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CFontModule::GetModuleName() const
{
	return &kFontModuleName;
}

//----------------------------------------------------------------------------
const CURI* CFontModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//============================================================================
// C function support
//============================================================================

//----------------------------------------------------------------------------
// Select FreeType/TrueType character encoding from Brio enums
//----------------------------------------------------------------------------
static int FindEncoding(FT_Face face, U32 encoding, int* numcodes)
{
	FT_Int		platformid = TT_PLATFORM_MICROSOFT; //3; //TT_PLATFORM_ISO;
	FT_Int		encodingid = TT_MS_ID_UNICODE_CS; //1; //TT_ISO_ID_10646;
	
	// Unicode, ASCII, Apple, and Adobe encodings set by FT_Select_Charmap
	switch (encoding)
	{
	case kASCIICharEncoding:
		*numcodes = 0x100L;
		return FT_Select_Charmap(face, FT_ENCODING_NONE);
	case kUTF8CharEncoding:
	case kUTF16CharEncoding:
	case kUTF16BECharEncoding:
	case kUTF16LECharEncoding:
	case kUTF32CharEncoding:
	case kUTF32BECharEncoding:
	case kUTF32LECharEncoding:
		*numcodes = 0x110000L;
		return FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	case kMacRomanCharEncoding:
		*numcodes = 0x100L;
		return FT_Select_Charmap(face, FT_ENCODING_APPLE_ROMAN);
	case kISOLatin1CharEncoding:
		*numcodes = 0x100L;
		platformid = TT_PLATFORM_ISO; 
		encodingid = TT_ISO_ID_8859_1;
		break;
	case kWindowsUSCharEncoding:
		*numcodes = 0x100L;
		platformid = TT_PLATFORM_MICROSOFT; 
		encodingid = TT_MS_LANGID_ENGLISH_UNITED_STATES;
		break;
	default:
		// TODO: Other ISO code tables?
		// TODO: Other Microsoft code pages?
		return -1;
	}

	// Loop thru charmaps for matching TrueType platform and encoding IDs
	for (int i = 0; i < face->num_charmaps; i++)
	{
		FT_CharMap cmap = face->charmaps[i];
		if (cmap->platform_id == platformid && cmap->encoding_id == encodingid)
			return FT_Set_Charmap(face, cmap);
	}
	
	return -1;
}

#if USE_FONT_CACHE_MGR
//----------------------------------------------------------------------------
// Callback for font cache manager
//----------------------------------------------------------------------------
static FT_Error
OnFaceRequest( FTC_FaceID  faceId, 
	FT_Library  lib, 
	FT_Pointer  request_data, 
	FT_Face*    aface )
{
    int error;
    
    PFont  font = (PFont)faceId;

    FT_UNUSED( request_data );

    if ( font->fileAddress != NULL )
		error = FT_New_Memory_Face( lib, (const FT_Byte*)font->fileAddress, font->fileSize, font->faceIndex, aface );
    else
    	error = FT_New_Face( lib, font->filepathname, font->faceIndex, aface );
    if ( !error )
    {
		char*  suffix;
		char   orig[4];
		
		// Support for separate kerning metrics files for Adobe Type 1 fonts
		suffix = strrchr( font->filepathname, '.' );
		if ( suffix && ( strcasecmp( suffix, ".pfa" ) == 0 || strcasecmp( suffix, ".pfb" ) == 0 ) )
	    {
	        suffix++;
	
	        memcpy( orig, suffix, 4 );
	        memcpy( suffix, "afm", 4 );
	        FT_Attach_File( *aface, font->filepathname );
	
	        memcpy( suffix, "pfm", 4 );
	        FT_Attach_File( *aface, font->filepathname );
	        memcpy( suffix, orig, 4 );
	    }

    if ( (*aface)->charmaps )
    	(*aface)->charmap = (*aface)->charmaps[font->cmapIndex];
    }

    return error;
}
#endif

//============================================================================
// Ctor & dtor
//============================================================================
CFontModule::CFontModule() : dbg_(kGroupFont)
{
#if !defined SET_DEBUG_LEVEL_DISABLE
	dbg_.SetDebugLevel(kFontDebugLevel);
#endif
	
	// Zero out static global struct
	memset(&handle_, 0, sizeof(handle_));

	// Default font properties and drawing attributes
	prop_.version = 2;
	prop_.size = 12;
	prop_.encoding = 0;
	prop_.useEncoding = false;
	attr_.version = 2;
	attr_.color = 0xFFFFFFFF;
	attr_.direction = false;
	attr_.antialias = true;
	attr_.horizJust = 0;
	attr_.vertJust = 0;
	attr_.spaceExtra = 0;
	attr_.leading = 0;
	attr_.useKerning = false;
	attr_.useUnderlining = false;
	curX_ = 0;
	curY_ = 0;
	
	// Load FreeType library
	int error = FT_Init_FreeType(&handle_.library);
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule: FT_Init_FreeType returned = %d, %p\n", 
				error, static_cast<void*>(handle_.library));
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule: FT_Init_FreeType failed, error = %d\n", error);
		return;	
	}
	
#if USE_FONT_CACHE_MGR
	// FT samples setup FT cache manager too
	error = FTC_Manager_New( handle_.library, 0, 0, 0, OnFaceRequest, 0, &handle_.cacheManager );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize cache manager\n");
    	return;
    }

    // Small bitmaps cache
    error = FTC_SBitCache_New( handle_.cacheManager, &handle_.sbitsCache );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize small bitmaps cache\n" );
    	return;
    }

    // Glyph image cache
    error = FTC_ImageCache_New( handle_.cacheManager, &handle_.imageCache );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize glyph image cache\n" );
    	return;
    }

    // Character code mapping cache
    error = FTC_CMapCache_New( handle_.cacheManager, &handle_.cmapCache );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize charmap cache\n" );
    	return;
    }
#endif	
}

//----------------------------------------------------------------------------
CFontModule::~CFontModule()
{
	CKernelMPI	kernel;
	
	if (!handle_.library)
		return;

	// Release memory used by fonts and font list
	for (int i = 0; i < handle_.numFonts; i++) 
	{
		if (handle_.fonts[i])
		{
			if (handle_.fonts[i]->filepathname)
				kernel.Free((char*)(handle_.fonts[i]->filepathname));
			kernel.Free(handle_.fonts[i]);
		}
	}
	kernel.Free(handle_.fonts);

#if USE_FONT_CACHE_MGR
	// Unload FreeType font cache manager
	FTC_Manager_Done( handle_.cacheManager );
#endif 
		
	// Unload FreeType library
	FT_Done_FreeType(handle_.library);

	dbg_.DebugOut(kDbgLvlVerbose, "~CFontModule: FT_Done_FreeType unloaded\n");
}

//----------------------------------------------------------------------------
Boolean	CFontModule::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
// MPI-specific methods
//----------------------------------------------------------------------------
tErrType CFontModule::SetFontResourcePath(const CPath &path)
{
	gpath = path;
	if (gpath.length() > 1 && gpath.at(gpath.length()-1) != '/')
		gpath += '/';
	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CFontModule::GetFontResourcePath() const
{
	return &gpath;
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFontInt(const CString* pName, tFontProp prop, void* pFileImage, int fileSize)
{
	FT_Face			face;
	PFont			font;
	int				error, numcodes = 0;
	CPath			filepath = (pName->at(0) == '/') ? *pName : gpath + *pName;
	const char*		filename = filepath.c_str();
	FT_Size      	size;
    CKernelMPI		kernel;
	
    // Font instance property versioning
    if (prop.version == 1)
    {
    	prop.encoding = 0;
    	prop.useEncoding = false;
    }
    prop_ = prop;
    
#if 0
    // Bogus font file name?
	if (filename == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: invalid filename passed\n");
		return false;
	}
#endif
	
	// Load font file
	if (pFileImage != NULL)
		error = FT_New_Memory_Face(handle_.library, (const FT_Byte*)pFileImage, fileSize, 0, &face );
	else
		error = FT_New_Face(handle_.library, filename, 0, &face );
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule::LoadFont: FT_New_Face (%s) returned = %d\n", filename, error);
	if (error)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: FT_New_Face (%s) failed, error = %d\n", filename, error);
		return false;
	}
		
	// Handle character encoding cases
	if (prop.encoding)
	{
		error = FindEncoding(face, prop.encoding, &numcodes);
		if (error)
		{
			FT_Done_Face(face);
			dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: FindEncoding for %d (%s) failed, error = %d\n", static_cast<int>(prop.encoding), filename, error);
			return false;
		}
	}
	
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule::LoadFont: metrics: horizontal=%s, vertical=%s, kerning=%s\n", 
		FT_HAS_HORIZONTAL(face) ? "yes" : "no",	FT_HAS_VERTICAL(face) ? "yes" : "no", FT_HAS_KERNING(face) ? "yes" : "no");

	// Allocate mem for font face
	font = static_cast<PFont>(kernel.Malloc( sizeof ( TFont ) ) );
	dbg_.Assert(font != NULL, "CFontModule::LoadFont: font struct could not be allocated\n");
	memset(font, 0, sizeof(TFont));
	font->filepathname = static_cast<char*>(kernel.Malloc( strlen( filename ) + 1 ) );
	dbg_.Assert(font->filepathname != NULL, "CFontModule::LoadFont: font filepathname could not be allocated\n");
	strcpy( (char*)font->filepathname, filename );
	font->fileAddress = pFileImage;
	font->fileSize = fileSize;
	font->faceIndex = 0;
	font->cmapIndex = face->charmap ? FT_Get_Charmap_Index( face->charmap ) : 0;
    font->numIndices = (numcodes) ? numcodes : face->num_glyphs;

#if USE_FONT_CACHE_MGR
	// Don't need any more font face info after this point
    FT_Done_Face( face );
    face = NULL;
#else    
    // Cache face for subsequent use until caching is figured out
    handle_.face = font->face = face;
#endif

	// Allocate space to load this font in our font list to date
    if ( handle_.maxFonts == 0 ) 
    {
		handle_.maxFonts = 16;
		handle_.fonts     = static_cast<PFont*>(kernel.Malloc( handle_.maxFonts * sizeof ( PFont ) ) );
		dbg_.Assert(handle_.fonts != NULL, "CFontModule::LoadFont: fonts list could not be allocated\n");
		memset( &handle_.fonts[0], 0, handle_.maxFonts * sizeof ( PFont ) );
    }
    else if ( handle_.numFonts >= handle_.maxFonts ) 
    {
    	PFont* 	oldlist = handle_.fonts;
		handle_.maxFonts *= 2;
		handle_.fonts      = static_cast<PFont*>(kernel.Malloc( handle_.maxFonts * sizeof ( PFont ) ) );
		dbg_.Assert(handle_.fonts != NULL, "CFontModule::LoadFont: fonts list could not be reallocated\n");
		memcpy( &handle_.fonts[0], oldlist, handle_.numFonts * sizeof ( PFont ) );
		memset( &handle_.fonts[handle_.numFonts], 0, ( handle_.maxFonts - handle_.numFonts ) * sizeof ( PFont ) );
		kernel.Free(oldlist);
    }

	// Add this font to our list of fonts
    handle_.fonts[handle_.numFonts] = font;
    handle_.curFont = handle_.numFonts++;
    handle_.currentFont = font;
    
#if USE_FONT_CACHE_MGR
	FTC_ScalerRec	scaler;
	int				pixelSize;

	// Set current font for cache manager   
    scaler.face_id = handle_.imageType.face_id = (FTC_FaceID)font;

    // Set font loading flags -- hinting off is key to glyph spacing
    handle_.imageType.flags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH | FT_LOAD_NO_HINTING;
    
    // Set selected font size
    pixelSize = (prop.size * 72 + 36) / 72; 
    scaler.width   = handle_.imageType.width  = (FT_UShort)pixelSize;
    scaler.height  = handle_.imageType.height = (FT_UShort)pixelSize;
    scaler.pixel   = 1;
    
    error = FTC_Manager_LookupSize( handle_.cacheManager, &scaler, &size );
    if ( error ) 
    {
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: FTC_Manager_LookupSize failed for font size = %d, pixels = %d, error = %d\n", prop.size, pixelSize, error);
		return false;
    }
    font->size = size;
    font->scaler = scaler;
#else
	// Select font size explicitly
	error = FT_Set_Char_Size(face, prop.size << 6, prop.size << 6, 72, 72);
    if ( error ) 
    {
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: FT_Set_Char_Size failed for font size = %d, error = %d\n", prop.size, error);
		return false;
    }
	font->size = size = face->size;
#endif	

	// Get font metrics for all glyphs (26:6 fixed-point)
    font->height = size->metrics.height >> 6;
	font->ascent = size->metrics.ascender >> 6;
	font->descent = size->metrics.descender >> 6;
	font->advance = size->metrics.max_advance >> 6;
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule::LoadFont: font size = %d, height = %d, ascent = %d, descent = %d\n", prop.size, font->height, font->ascent, font->descent);
		
	return reinterpret_cast<tFontHndl>(font);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(const CString* pName, tFontProp prop)
{
	return LoadFontInt(pName, prop, NULL, 0);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(const CPath& name, tFontProp prop)
{
//	CString path = CString(name);
	return LoadFontInt(&name, prop, NULL, 0);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(const CPath& name, U8 size)
{
//	CString path = CString(name);
	prop_.version = 2;
	prop_.size = size;
	prop_.encoding = 0;
	return LoadFontInt(&name, prop_, NULL, 0);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(const CPath& name, U8 size, U32 encoding)
{
//	CString path = CString(name);
	prop_.version = 2;
	prop_.size = size;
	prop_.encoding = encoding;
	return LoadFontInt(&name, prop_, NULL, 0);
}

#if USE_RSRC_MGR
//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(tRsrcHndl hRsrc, tFontProp prop)
{
	CResourceMPI		rsrcmgr;
	const CString*		fontname;
	tPtr				fileimage;
	U32					filesize; 
	tErrType			r;
	tFontHndl			hFont;
	PFont				pFont;
		
	if (hRsrc == kInvalidRsrcHndl)
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::LoadFont: invalid resource handle passed\n");
		return kInvalidFontHndl;
	}
	
	// Load font via resource manager
	r = rsrcmgr.LoadRsrc(hRsrc, kOpenRsrcOptionRead, NULL);
	if (r != kNoErr) 
	{
		dbg_.DebugOutErr(kDbgLvlCritical, r, "FontModule::LoadFont: LoadRsrc failed for %0X\n", 
				static_cast<unsigned int>(hRsrc));
		return kInvalidFontHndl;
	}
	 
	// Use file image loaded in memory by resource manager
	fontname = rsrcmgr.GetPath(hRsrc);
	filesize = rsrcmgr.GetUnpackedSize(hRsrc);
	fileimage = rsrcmgr.GetPtr(hRsrc);
	dbg_.DebugOut(kDbgLvlVerbose, "FontModule::LoadFont: resource font name = %s, file size = %d\n", 
				fontname->c_str(), static_cast<int>(filesize));
	
	hFont = LoadFontInt(fontname, prop, fileimage, filesize);

	if (hFont == kInvalidFontHndl)
		return kInvalidFontHndl;

	// Save resource handle for unloading
	pFont = (PFont)hFont;		
	pFont->hRsrcFont = hRsrc;
	return hFont;
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(tRsrcHndl hRsrc, U8 size)
{
	prop_.size = size;
	prop_.encoding = 0;
	return LoadFont(hRsrc, prop_);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(tRsrcHndl hRsrc, U8 size, U32 encoding)
{
	prop_.version = 2;
	prop_.size = size;
	prop_.encoding = encoding;
	return LoadFont(hRsrc, prop_);
}
#endif // USE_RSRC_MGR

//----------------------------------------------------------------------------
Boolean CFontModule::UnloadFont(tFontHndl hFont)
{
	PFont	pFont = (PFont)hFont;

	// Unload font if loaded via resource manager 
	if (pFont)
	{
#if USE_RSRC_MGR
		if (pFont->hRsrcFont != kInvalidRsrcHndl)
		{
			CResourceMPI	rsrcmgr;
			rsrcmgr.UnloadRsrc(pFont->hRsrcFont);
			pFont->hRsrcFont = kInvalidRsrcHndl;
		}
#endif
		
#if !USE_FONT_CACHE_MGR
		// Release FreeType font face instance (no cache manager)
		if (pFont->face != NULL)
		{
			FT_Done_Face(pFont->face);
			pFont->face = NULL;
		}
#endif
	}

	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::SelectFont(tFontHndl hFont)
{
	PFont	pFont = (PFont)hFont;

	if (pFont == NULL /* || pFont->hRsrcFont == kInvalidRsrcHndl */)
		return false;

	// Reload cached settings
	handle_.currentFont = pFont;
//	FT_Activate_Size(pFont->size);
#if USE_FONT_CACHE_MGR
    handle_.imageType.face_id = reinterpret_cast<FTC_FaceID>(pFont);
    handle_.imageType.width = pFont->scaler.width;
    handle_.imageType.height = pFont->scaler.height;
    handle_.imageType.flags = FT_LOAD_DEFAULT | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH | FT_LOAD_NO_HINTING;
#endif
    return true;
}

//----------------------------------------------------------------------------
Boolean	CFontModule::SetFontAttr(tFontAttr attr)
{
	if (attr.version == 1)
	{
		attr_.version 	= 1;
		attr_.color 	= attr.color;
		attr_.direction	= attr.direction;
		attr_.antialias	= true;
		return true;
	}
	attr_ = attr;
	return true;
}

//----------------------------------------------------------------------------
Boolean	CFontModule::GetFontAttr(tFontAttr* pAttr)
{
	if (attr_.version == 1)
	{
		pAttr->version		= attr_.version;
		pAttr->color		= attr_.color;
		pAttr->direction	= attr_.direction;
		return true;
	}
	*pAttr = attr_;
	return true;
}

//----------------------------------------------------------------------------
tFontAttr* CFontModule::GetFontAttr()
{
	static tFontAttr attr = attr_;
	return &attr;
}

//----------------------------------------------------------------------------
// Clip source bitmap bounds to destination drawable surface area
//----------------------------------------------------------------------------
inline Boolean ClipBounds(int& sx, int& sy, int& sw, int& sh, int& x, int& y, int dw, int dh)
{
	int dx = 0;
	int dy = 0;

	// Out of bounds?
	if (x > dw || y > dh)
		return true;
	
	// Right edge clipping?
	if (x + sw > dw)
		dx = x + sw - dw;
	if (dx > sw)
		return true;
	// Left edge clipping?
	if (x < 0) {
		dx -= x;
		sx -= x;
		x = 0;
	}
	if (sw < dx)
		return true;

	// Bottom edge clipping?
	if (y + sh > dh)
		dy = y + sh - dh;
	if (dy > sh)
		return true;
	// Top edge clipping?
	if (y < 0) {
		dy -= y;
		sy -= y;
		y = 0;
	}
	if (sh < dy)
		return true;

	sw -= dx;
	sh -= dy;
	return false;
}

//----------------------------------------------------------------------------
// Convert mono bitmap to RGB color buffer
//----------------------------------------------------------------------------
void CFontModule::ConvertBitmapToRGB32(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h,i,mask;
	U8	 		*s,*t,*u;
	U32 		*d;
	U32	 		color = attr_.color;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = (source->width+7) / 8;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to mono bitmap mask
	s = t = source->buffer + y * source->pitch + x/8;
	d = (U32*)(u = surf->buffer + y0 * surf->pitch + x0 * 4);
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = (U32*)u;
		for (x = 0; x < w; x++) 
		{
			mask = 0x80;
			for (i = 0; i < 8; i++) 
			{
				if (mask & *s)
					*d = color;
				mask >>= 1;
				d++;
			}
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}

//----------------------------------------------------------------------------
void CFontModule::ConvertBitmapToRGB24(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h,i,mask;
	U8	 		*s,*t;
	U8  		*d,*u;
	U32	 		color = (U16)attr_.color;
	U8			R = (color & 0xFF0000) >> 16;
	U8			G = (color & 0x00FF00) >> 8;
	U8			B = (color & 0x0000FF) >> 0;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = (source->width+7) / 8;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to mono bitmap mask
	s = t = source->buffer + y * source->pitch + x/8;
	d = u = surf->buffer + y0 * surf->pitch + x0 * 3;
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = u;
		for (x = 0; x < w; x++) 
		{
			mask = 0x80;
			for (i = 0; i < 8; i++) 
			{
				if (mask & *s)
				{
					*d++ = B;
					*d++ = G;
					*d++ = R;
				}
				else
					d+=3;
				mask >>= 1;
			}
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}

//----------------------------------------------------------------------------
void CFontModule::ConvertBitmapToRGB4444(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h,i,mask;
	U8	 		*s,*t,*u;
	U16 		*d;
	U16	 		color = attr_.color;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = (source->width+7) / 8;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to mono bitmap mask
	s = t = source->buffer + y * source->pitch + x/8;
	d = (U16*)(u = surf->buffer + y0 * surf->pitch + x0 * 2);
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = (U16*)u;
		for (x = 0; x < w; x++) 
		{
			mask = 0x80;
			for (i = 0; i < 8; i++) 
			{
				if (mask & *s)
					*d = color;
				mask >>= 1;
				d++; // U16*
			}
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}
  
//----------------------------------------------------------------------------
void CFontModule::ConvertBitmapToRGB565(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	// No pixel unpacking/repacking for 16bpp format, so same code as above
	ConvertBitmapToRGB4444(source, x0, y0, pCtx);
}
  
//----------------------------------------------------------------------------
// Convert grayscale bitmap to ARGB color buffer
//----------------------------------------------------------------------------
void CFontModule::ConvertGraymapToRGB32(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h;
	U8	 		*s,*t,*u;
	U32 		*d;
	U32	 		color = attr_.color;
	U8 			alpha;
	U8			A = (color & 0xFF000000) >> 24;
	U8			R = (color & 0xFF0000) >> 16;
	U8			G = (color & 0x00FF00) >> 8;
	U8			B = (color & 0x0000FF) >> 0;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = source->width;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to grayscale values
	s = t = source->buffer + y * source->pitch + x;
	d = (U32*)(u = surf->buffer + y0 * surf->pitch + x0 * 4);
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = (U32*)u;
		for (x = 0; x < w; x++) 
		{
#if 0 // defined(LF1000) && !defined(EMULATION)	// FIXME/dm: Enable real alpha blending
			if ((alpha = *s) != 0)
				*d = color | (alpha << 24);
			(void)R;
			(void)G;
			(void)B;
			(void)A;
#else
			if ((alpha = *s) != 0) 
			{
				U32 bgcolor = *d;
				U32 r = (bgcolor & 0xFF0000) >> 16;
				U32 g = (bgcolor & 0x00FF00) >> 8;
				U32 b = (bgcolor & 0x0000FF) >> 0;
				U32 a = (bgcolor & 0xFF000000) >> 24;
				U8  ialpha = 0xFF - alpha;
				r = (R * alpha + r * ialpha) / 0xFF;
				g = (G * alpha + g * ialpha) / 0xFF;
				b = (B * alpha + b * ialpha) / 0xFF;
				a = (A * alpha + a * ialpha) / 0xFF; 
				*d = (a << 24) | (r << 16) | (g << 8) | (b << 0);
			}
#endif
			d++;
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}

//----------------------------------------------------------------------------
void CFontModule::ConvertGraymapToRGB24(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h;
	U8	 		*s,*t;
	U8  		*d,*u;
	U32	 		color = attr_.color;
	U8 			alpha;
	U8			R = (color & 0xFF0000) >> 16;
	U8			G = (color & 0x00FF00) >> 8;
	U8			B = (color & 0x0000FF) >> 0;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = source->width;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to grayscale values
	s = t = source->buffer + y * source->pitch + x;
	d = u = surf->buffer + y0 * surf->pitch + x0 * 3;
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = u;
		for (x = 0; x < w; x++) 
		{
			if ((alpha = *s) != 0) 
			{
				U8  ialpha = 0xFF - alpha;
				*d = (B * alpha + *d * ialpha) / 0xFF; d++;
				*d = (G * alpha + *d * ialpha) / 0xFF; d++;
				*d = (R * alpha + *d * ialpha) / 0xFF; d++;
			}
			else
				d+=3;
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}
  
//----------------------------------------------------------------------------
void CFontModule::ConvertGraymapToRGB4444(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h;
	U8	 		*s,*t;
	U8  		*d,*u;
	U32	 		color = attr_.color;
	U8 			alpha;
	U8			A = (color & 0xF000) >> 12;
	U8			R = (color & 0x0F00) >> 8;
	U8			G = (color & 0x00F0) >> 4;
	U8			B = (color & 0x000F) >> 0;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = source->width;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to grayscale values
	// All ARGB components are repacked for GL_RGBA 4444 textures
	s = t = source->buffer + y * source->pitch + x;
	d = u = surf->buffer + y0 * surf->pitch + x0 * 2;
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = u;
		for (x = 0; x < w; x++) 
		{
			if ((alpha = *s) != 0) 
			{
				U8  ialpha = 0xFF - alpha;
				U16* p16 = (U16*)d;
				U16 bgcolor = *p16;
				U16 a = (bgcolor & 0xF000) >> 12;
				U16 r = (bgcolor & 0x0F00) >> 8;
				U16 g = (bgcolor & 0x00F0) >> 4;
				U16 b = (bgcolor & 0x000F) >> 0;
				b = (B * alpha + b * ialpha) / 0xFF; 
				g = (G * alpha + g * ialpha) / 0xFF;
				r = (R * alpha + r * ialpha) / 0xFF;
				a = (A * alpha + a * ialpha) / 0xFF;
				*p16 = (a << 12) | (r << 8) | (g << 4) | (b << 0); 
			}
			d+=2;
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}
  
//----------------------------------------------------------------------------
void CFontModule::ConvertGraymapToRGB565(FT_Bitmap* source, int x0, int y0, tFontSurf* pCtx)
{
	int 		x,y,w,h;
	U8	 		*s,*t;
	U8  		*d,*u;
	U32	 		color = attr_.color;
	U8 			alpha;
	U8			R = (color & 0xF800) >> 11;
	U8			G = (color & 0x07E0) >> 5;
	U8			B = (color & 0x001F) >> 0;
	tFontSurf	*surf = (tFontSurf*)pCtx;

	// Clip bounds to drawable surface area
	x = y = 0;
	w = source->width;
	h = source->rows;
	if (ClipBounds(x, y, w, h, x0, y0, surf->width, surf->height))
		return;

	// Pack RGB color into buffer according to grayscale values
	s = t = source->buffer + y * source->pitch + x;
	d = u = surf->buffer + y0 * surf->pitch + x0 * 2;
	for (y = 0; y < h; y++) 
	{
		s = t;
		d = u;
		for (x = 0; x < w; x++) 
		{
			if ((alpha = *s) != 0) 
			{
				U8  ialpha = 0xFF - alpha;
				U16* p16 = (U16*)d;
				U16 bgcolor = *p16;
				U16 r = (bgcolor & 0xF800) >> 11;
				U16 g = (bgcolor & 0x07E0) >> 5;
				U16 b = (bgcolor & 0x001F) >> 0;
				b = (B * alpha + b * ialpha) / 0xFF; 
				g = (G * alpha + g * ialpha) / 0xFF;
				r = (R * alpha + r * ialpha) / 0xFF;
				*p16 = (r << 11) | (g << 5) | (b << 0); 
			}
			d+=2;
			s++;
		}
		t += source->pitch;	// U8*
		u += surf->pitch;	// U8*
	}				  
}

//----------------------------------------------------------------------------
// Expand glyph bitmap to new width and height
//----------------------------------------------------------------------------
inline void ExpandBitmap(FT_Bitmap* source, FT_Bitmap* dest, int width, int height)
{
	CKernelMPI	kernel;
	int			w = source->pitch;
	int			h = source->rows;
	U8* 		s = source->buffer;
	U8* 		d = dest->buffer = static_cast<U8*>(kernel.Malloc(width * height));

	// Effectively use pitch for full bitmap width
	dest->pitch = dest->width = width;
	dest->rows = height;
	dest->pixel_mode = source->pixel_mode;

	memset(d, 0, width * height);
	for (int i = 0; i < h; i++)
	{
		memcpy(d, s, w);
		s += w;
		d += width;
	}
}

//----------------------------------------------------------------------------
// Free expanded glyph bitmap memory 
//----------------------------------------------------------------------------
inline void FreeBitmap(FT_Bitmap* dest)
{
	CKernelMPI	kernel;
	kernel.Free(dest->buffer);
}

//----------------------------------------------------------------------------
// Add underline bits to glyph bitmap
//----------------------------------------------------------------------------
inline void UnderlineBitmap(FT_Bitmap* source, int y, int dy)
{
	int		w = source->width;
	U8*		s = source->buffer + y * source->pitch;
	
	if (source->pixel_mode == FT_PIXEL_MODE_MONO)
		w = (w+7)/8;
	
	for (int h = y; h < y+dy && h < source->rows; h++)
	{
		memset(s, 0xFF, w);
		s += source->pitch;
	}
}

//----------------------------------------------------------------------------
// Advance glyph cursor XY position  
//----------------------------------------------------------------------------
inline void AdvanceGlyphPosition(FT_Glyph glyph, int& x, int& y)
{
	// Internal glyph cursor is in 16:16 fixed-point
	// FIXME/dm: Rounding should be applied to cummulative XY values

	x += (( glyph->advance.x + 0x8000 ) >> 16);
	y += (( glyph->advance.y + 0x8000 ) >> 16);
}

//----------------------------------------------------------------------------
// Adjust glyph position for kerning  
//----------------------------------------------------------------------------
inline void KernGlyphPosition(FT_Face face, int index, int prev, int& dx)
{
	FT_Vector 		delta; 

	// Kerning adjustments are usually negative (26.6 fixed-point) 
	FT_Get_Kerning( face, prev, index, FT_KERNING_DEFAULT, &delta ); 	
	dx = (delta.x < 0) ? (delta.x - 0x10) >> 6 : (delta.x + 0x10) >> 6;
}

//----------------------------------------------------------------------------
// Get the font face which glyphs belong to   
//----------------------------------------------------------------------------
inline void CFontModule::GetFace(FT_Face* pFace)
{
#if USE_FONT_CACHE_MGR
	FTC_Manager_LookupFace(handle_.cacheManager, handle_.imageType.face_id, pFace);
#else
	*pFace = handle_.currentFont->face;
#endif
}

//----------------------------------------------------------------------------
// Get glyph matching character code 
//----------------------------------------------------------------------------
Boolean CFontModule::GetGlyph(tWChar ch, FT_Glyph* pGlyph, int* pIndex)
{
	FT_Glyph		glyph;
	int				error = FT_Err_Ok;
	int				index;
	PFont			font = handle_.currentFont;

	// Sanity check
	if (font == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: invalid font selection\n" );
		return false;
	}
	
#if USE_FONT_CACHE_MGR
  	// For selected font, find the glyph matching the char
    index = FTC_CMapCache_Lookup( handle_.cmapCache, handle_.imageType.face_id, handle_.currentFont->cmapIndex, ch );
	if (index == 0) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to support char = %08X, index = %d\n", static_cast<unsigned int>(ch), index );
		return false;
	}

	// Get the glyph at char index
	error = FTC_ImageCache_Lookup( handle_.imageCache, &handle_.imageType, index, &glyph, NULL );
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to locate glyph for char = %08X, index = %d, error = %d\n", static_cast<unsigned int>(ch), index, error );
		return false;
	}
#else
  	// For selected font, find the glyph matching the char
	index = FT_Get_Char_Index(font->face, ch);
	if (index == 0) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to support char = %08X, index = %d\n", static_cast<unsigned int>(ch), index );
		return false;
	}

	// Load indexed glyph into face's internal glyph record slot
	error = FT_Load_Glyph(font->face, index, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH | FT_LOAD_NO_HINTING);
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to support char = %08X, index = %d, error = %d\n", static_cast<unsigned int>(ch), index, error );
		return false;
	}
	
	// Copy glyph from face's internal glyph record slot (not exact same struct)
	error = FT_Get_Glyph(font->face->glyph , &glyph);
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to locate glyph for char = %08X, error = %d\n", static_cast<unsigned int>(ch), error );
		return false;
	}
#endif		

	*pGlyph = glyph;
	*pIndex = index;
	return true;
}

//----------------------------------------------------------------------------
// Draw single glyph at XY location in rendering context buffer
//----------------------------------------------------------------------------
Boolean CFontModule::DrawGlyph(tWChar ch, int x, int y, tFontSurf* pCtx, bool isFirst)
{
	FT_Glyph		glyph;
	FT_Face			face;
	FT_BitmapGlyph  bitmap;
	FT_Bitmap*      source,clone;
	FT_Render_Mode  render_mode = FT_RENDER_MODE_MONO;
	int				error = FT_Err_Ok;
	PFont			font = handle_.currentFont;
	Boolean			rc = false;
	int				index;
	static int		prevIndex = 0;
	
	// Update XY cursor without drawing anything if newline detected
	if (ch == '\n')
	{
		curX_ = 0;
		curY_ += font->height + attr_.leading;
		return true;
	}
	
	// Get glyph matching char code
	rc = GetGlyph(ch, &glyph, &index);
	if (!rc)
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: unable to locate glyph for char = %08X\n", static_cast<unsigned int>(ch) );
		return false;
	}

	// Get font face 
	GetFace(&face);
	
	if ( glyph->format == FT_GLYPH_FORMAT_OUTLINE ) 
	{
		// Adjust font render mode as necessary
		if (attr_.antialias)
			render_mode = FT_RENDER_MODE_NORMAL;
			
		// Render the glyph to a bitmap, don't destroy original 
		error = FT_Glyph_To_Bitmap( &glyph, render_mode, NULL, false );
		if ( error )
			return false;
	}
	
	if ( glyph->format != FT_GLYPH_FORMAT_BITMAP ) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: invalid glyph format returned, error = %d\n", error );
		return false;
	}
	
	// The glyph now has bitmap data
	bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
	source = &bitmap->bitmap;
	
	// Account for bitmap XY bearing offsets and overall font ascent 
	int dx = bitmap->left;
	int dy = bitmap->top;
	int dz = font->ascent;
	int dk = 0;

	// Account for kerning adjustment for preceding glyph
	if (attr_.useKerning && !isFirst)
		KernGlyphPosition(face, index, prevIndex, dk);
	dx += dk;
	prevIndex = index;
	
	// Add underline to glyph bitmap image
	if (attr_.useUnderlining)
	{
		int du = std::min(face->underline_position >> 6, 0);
		int dt = std::max(face->underline_thickness >> 6, 1);
		int dh = std::max(face->height >> 6, source->rows);
		int dw = (( glyph->advance.x + 0x8000 ) >> 16) + attr_.spaceExtra;
			dw = std::max(dw, source->rows);
		// Expand glyph bitmap width and height to fit underline bits
		ExpandBitmap(source, &clone, dw, dh);
		source = &clone;
		UnderlineBitmap(source, dy-du, dt);
		dx = 0; // fills gaps -- affects positioning
	}
	
	// Draw mono bitmap into RGB context buffer with current color
	if (source->pixel_mode == FT_PIXEL_MODE_MONO)
	{
		switch (pCtx->format)
		{
			default:
			case kPixelFormatARGB8888:
				ConvertBitmapToRGB32(source, x+dx, y+dz-dy, pCtx); 
				break;
			case kPixelFormatRGB888:
				ConvertBitmapToRGB24(source, x+dx, y+dz-dy, pCtx); 
				break;
			case kPixelFormatRGB4444:
				ConvertBitmapToRGB4444(source, x+dx, y+dz-dy, pCtx); 
				break;
			case kPixelFormatRGB565:
				ConvertBitmapToRGB565(source, x+dx, y+dz-dy, pCtx); 
				break;
		}
	}
	else if (source->pixel_mode == FT_PIXEL_MODE_GRAY)
	{
		switch (pCtx->format)
		{
			default:
			case kPixelFormatARGB8888:
				ConvertGraymapToRGB32(source, x+dx, y+dz-dy, pCtx);
				break;
			case kPixelFormatRGB888:
				ConvertGraymapToRGB24(source, x+dx, y+dz-dy, pCtx);
				break;
			case kPixelFormatRGB4444:
				ConvertGraymapToRGB4444(source, x+dx, y+dz-dy, pCtx);
				break;
			case kPixelFormatRGB565:
				ConvertGraymapToRGB565(source, x+dx, y+dz-dy, pCtx);
				break;
		}
	}
	else
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::DrawGlyph: glyph conversion format %X not supported\n", source->pixel_mode);

	// Update the current XY glyph cursor position
	AdvanceGlyphPosition(glyph, curX_, curY_);
    curX_ += attr_.spaceExtra;
	curX_ += dk;
	
	// Release expanded bitmap memory used for underlining
	if (attr_.useUnderlining)
		FreeBitmap(&clone);
	
	return true;
}

//----------------------------------------------------------------------------
// Unpack Unicode char sequence
//----------------------------------------------------------------------------
inline void UnpackUnicode(tWChar* charcode, CString* pStr, int* i)
{
	// NOTE: Looks like glib::ustring is already parsed into 16-bit codes
	
	// Scan for UTF8 multi-byte sequences for 1, 2, or 3 additional bytes
	if ( *charcode >= 0xC0 )
	{
		int numbytes = 0;
		if ( *charcode < 0xE0 )			// U+0080 .. U+07FF
		{
		    numbytes = 1;
		    *charcode &= 0x1F;
		}
		else if ( *charcode < 0xF0 )	// U+0800 .. U+FFFF
		{
			numbytes = 2;
		    *charcode &= 0x0F;
		}
		else if ( *charcode < 0xF8 )	// U+10000 .. U+10FFFF
		{
			numbytes = 3;
			*charcode &= 0x07;
		}
		while (numbytes--)
		{
			tWChar nextchar = pStr->at(*i++);
			*charcode <<= 6;
			*charcode |= nextchar & 0x3F;
		}
	}
}

//----------------------------------------------------------------------------
Boolean CFontModule::DrawString(CString* pStr, S32 x, S32 y, tFontSurf* pCtx)
{
	const char*		ch = pStr->c_str();
	int				len = pStr->length();
	int				i;
	Boolean			rc;
	
	// C char string only used for debug output now
	dbg_.DebugOut(kDbgLvlVerbose, "FontModule::DrawString: %s, XY = %d,%d, length = %d\n", 
			ch, static_cast<int>(x), static_cast<int>(y), len);

	// Set current XY glyph cursor position
	curX_ = x;
	curY_ = y;
	
	// Draw each char glyph in the string
	for (i = 0; i < len; i++) 
	{
		tWChar charcode = pStr->at(i);
		if (prop_.encoding == kUTF8CharEncoding)
			UnpackUnicode(&charcode, pStr, &i);
		rc = DrawGlyph(charcode, curX_, curY_, pCtx, i==0);
	}

	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::DrawString(CString& str, S32& x, S32& y, tFontSurf& surf)
{
	Boolean rc = DrawString(&str, x, y, &surf);
	x = curX_;
	y = curY_;
	return rc;
}

//----------------------------------------------------------------------------
Boolean CFontModule::DrawString(CString& str, S32& x, S32& y, tFontSurf& surf, Boolean bWrap)
{
	Boolean	rc = false;
	tRect 	rect;
	CString	part;
	int		len,dy,i,p,n;
	
	// Nothing to calculate if no wrapping
	if (!bWrap)
		return DrawString(str, x, y, surf);
	
	// If entire string fits, then draw as is
	GetStringRect(&str, &rect);
	if (x + rect.right - rect.left <= surf.width)
		return DrawString(str, x, y, surf);
	
	// Parse string for space breaks to draw incrementally
	n = len = str.length();
	dy = handle_.currentFont->height + attr_.leading;
	for (i = p = 0; i < len; i++)
	{
		// Parse string for space breaks
		if (str.at(i) == ' ')
		{
			// Exclude space from wrap calculation since word may fit without it
			n = i-p;
			part = str.substr(p, n);
			// Wrap XY pre-drawing
			GetStringRect(&part, &rect);
			if (x + rect.right - rect.left > surf.width) 
			{
				x = 0;
				y += dy;
			}
			// Include space for incremental drawing and string update
			n = i+1-p;
			part = str.substr(p, n);
			p = i+1;
			n = len-p;
			rc = DrawString(part, x, y, surf);
			// Wrap XY post-drawing (in case of long words or overflow from spaces)
			if (x > surf.width)
			{
				x = 0;
				y += dy;
			}
		}
	}		

	// Draw the last part of the string, or entire string if no space breaks
	part = str.substr(p, n);
	GetStringRect(&part, &rect);
	if (x + rect.right - rect.left > surf.width) 
	{
		x = 0;
		y += dy;
	}
	rc = DrawString(part, x, y, surf);
	
	return rc;
}

//----------------------------------------------------------------------------
S32 CFontModule::GetX()
{
	return curX_;
}

//----------------------------------------------------------------------------
S32 CFontModule::GetY()
{
	return curY_;
}

//----------------------------------------------------------------------------
Boolean CFontModule::GetFontMetrics(tFontMetrics* pMtx)
{
	PFont	font = handle_.currentFont;

	if (font == NULL || pMtx == NULL)
		return false;

	pMtx->height = font->height;
	pMtx->ascent = font->ascent;
	pMtx->descent = font->descent;
	pMtx->advance = font->advance;		
	return true;
}

//----------------------------------------------------------------------------
tFontMetrics* CFontModule::GetFontMetrics()
{
	static tFontMetrics metrics;
	GetFontMetrics(&metrics);
	return &metrics;
}

//----------------------------------------------------------------------------
Boolean CFontModule::GetStringRect(CString* pStr, tRect* pRect)
{
	const char*		ch = pStr->c_str();
	int				len = pStr->length();
	PFont			font = handle_.currentFont;
	FT_Glyph		glyph;
	FT_Face			face;
	FT_BBox			bbox,gbox = {0, 0, 0, 0};
	int				dx = 0, dy = 0, dk = 0;
	int				index,prev = 0;
	
	if (font == NULL || pRect == NULL)
		return false;

	// Need font face and each preceding glyph index for kerning
	GetFace(&face);
	
	// Get bounding box for each glyph in char string
	for (int i = 0; i < len; i++)
	{
		tWChar charcode = pStr->at(i);
		if (prop_.encoding == kUTF8CharEncoding)
			UnpackUnicode(&charcode, pStr, &i);
		if (!GetGlyph(charcode, &glyph, &index))
			continue;
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
		// Adjust for kerning
		if (attr_.useKerning && prev)
		{
			KernGlyphPosition(face, index, prev, dk);
			bbox.xMin += dk;
			bbox.xMax += dk;
		}
		gbox.xMin = std::min(bbox.xMin+dx, gbox.xMin);
		gbox.yMin = std::min(bbox.yMin+dy, gbox.yMin);
		gbox.xMax = std::max(bbox.xMax+dx, gbox.xMax);
		gbox.yMax = std::max(bbox.yMax+dy, gbox.yMax);
		// Adjust for glyph position
		AdvanceGlyphPosition(glyph, dx, dy);
		dx += attr_.spaceExtra;
		// Save previous glyph index if kerning
		prev = index;
	}

	// C char string only used for debug output now
	dbg_.DebugOut(kDbgLvlVerbose, "FontModule::GetStringRect: %s, length %d, min %d,%d, max %d,%d\n", 
			ch, len, static_cast<int>(gbox.xMin), static_cast<int>(gbox.yMin), 
			static_cast<int>(gbox.xMax), static_cast<int>(gbox.yMax));

	// Pass back bounding box min/max coords as rect param
	pRect->left = gbox.xMin;
	pRect->top  = gbox.yMax;
	pRect->right = gbox.xMax;
	pRect->bottom = gbox.yMin;
	return true;
}

//----------------------------------------------------------------------------
tRect* CFontModule::GetStringRect(CString& str)
{
	static tRect rect;
	GetStringRect(&str, &rect);
	return &rect;
}

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CFontModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version; /* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CFontModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
		(void )ptr;
		//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG


// EOF
