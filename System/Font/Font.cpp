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
#include <SystemResourceMPI.h>

#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>
#include <ftglyph.h>

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Font";


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
CFontModule::CFontModule() : dbg_(kGroupDisplay) // FIXME: new enum?
{
	// Load FreeType library
	int error = FT_Init_FreeType(&handle_.library);
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule: FT_Init_FreeType returned = %d, %p\n", error, handle_.library);
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

    error = FTC_SBitCache_New( handle_.cacheManager, &handle_.sbitsCache );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize small bitmaps cache\n" );
    	return;
    }

    error = FTC_ImageCache_New( handle_.cacheManager, &handle_.imageCache );
    if ( error ) 
    {
    	dbg_.DebugOut(kDbgLvlCritical, "CFontModule: could not initialize glyph image cache\n" );
    	return;
    }

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
	if (!handle_.library)
		return;

	// Release memory used by fonts and font list
	for (int i = 0; i < handle_.numFonts; i++) 
	{
		if (handle_.fonts[i])
			free(handle_.fonts[i]);
	}
	free(handle_.fonts);

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
tFontHndl CFontModule::LoadFontInt(const CString* pName, tFontProp prop, void* pFileImage, int fileSize)
{
	FT_Face			face;
	PFont			font;
	int				error;
	const char*		filename = pName->c_str();
	int				pixelSize;
	FTC_ScalerRec	scaler;
    FT_Size      	size;
	
	// Bogus font file name?
	if (filename == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::LoadFont: invalid filename passed\n");
		return false;
	}
	
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
		
	// Allocate mem for font face
	font = (PFont)malloc( sizeof ( *font ) );
	
	font->filepathname = (char*)malloc( strlen( filename ) + 1 );
	strcpy( (char*)font->filepathname, filename );

	font->fileAddress = pFileImage;
	font->fileSize = fileSize;
	
	font->faceIndex = 0; // i;
	font->cmapIndex = face->charmap ? FT_Get_Charmap_Index( face->charmap ) : 0;
	
	// TODO: Handle character encoding cases
	// This is default encoding case
    font->numIndices = face->num_glyphs;

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
		handle_.fonts     = (PFont*)calloc( handle_.maxFonts, sizeof ( PFont ) );
    }
    else if ( handle_.numFonts >= handle_.maxFonts ) 
    {
		handle_.maxFonts *= 2;
		handle_.fonts      = (PFont*)realloc( handle_.fonts, handle_.maxFonts * sizeof ( PFont ) );
		
		memset( &handle_.fonts[handle_.numFonts], 0, ( handle_.maxFonts - handle_.numFonts ) * sizeof ( PFont ) );
    }

	// Add this font to our list of fonts
    handle_.fonts[handle_.numFonts++] = font;
    
#if USE_FONT_CACHE_MGR
 	// Set current font for cache manager   
    handle_.currentFont = font;
    scaler.face_id = handle_.imageType.face_id = (FTC_FaceID)font;
    
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
    // Get font metrics for all glyphs (26:6 fixed-point)
	font->height = size->metrics.height >> 6;
	font->ascent = size->metrics.ascender >> 6;
	font->descent = size->metrics.descender >> 6;
	font->advance = size->metrics.max_advance >> 6;
	dbg_.DebugOut(kDbgLvlVerbose, "CFontModule::LoadFont: font size = %d, pixels = %d, height = %d, ascent = %d, descent = %d\n", prop.size, pixelSize, font->height, font->ascent, font->descent);
#else
	// FIXME: Select font size from available sizes
#endif	
		
	return (tFontHndl)font; //true;
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(const CString* pName, tFontProp prop)
{
	return LoadFontInt(pName, prop, NULL, 0);
}

//----------------------------------------------------------------------------
tFontHndl CFontModule::LoadFont(tRsrcHndl hRsrc, tFontProp prop)
{
	CSystemResourceMPI	rsrcmgr;
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
		dbg_.DebugOut(kDbgLvlCritical, "FontModule::LoadFont: LoadRsrc failed for %0X\n", hRsrc);
		return kInvalidFontHndl;
	}
	 
	// Use file image loaded in memory by resource manager
	fontname = rsrcmgr.GetURI(hRsrc); // GetName(hRsrc);
	filesize = rsrcmgr.GetUnpackedSize(hRsrc);
	fileimage = rsrcmgr.GetPtr(hRsrc);
	dbg_.DebugOut(kDbgLvlVerbose, "FontModule::LoadFont: resource font name = %s, file size = %d\n", fontname->c_str(), filesize);
	
	hFont = LoadFontInt(fontname, prop, fileimage, filesize);

	if (hFont == kInvalidFontHndl)
		return kInvalidFontHndl;

	// Save resource handle for unloading
	pFont = (PFont)hFont;		
	pFont->hRsrcFont = hRsrc;
	return hFont;
}

//----------------------------------------------------------------------------
Boolean CFontModule::UnloadFont(tFontHndl hFont)
{
	PFont	pFont = (PFont)hFont;

	// Unload font if loaded via resource manager 
	if (pFont)
	{
		if (pFont->hRsrcFont != kInvalidRsrcHndl)
		{
			CSystemResourceMPI	rsrcmgr;
			rsrcmgr.UnloadRsrc(pFont->hRsrcFont);
			pFont->hRsrcFont = kInvalidRsrcHndl;
		}
	}

	if (handle_.face) 
	{
		FT_Done_Face(handle_.face);
		handle_.face = NULL;
	}
	return true;
}

//----------------------------------------------------------------------------
Boolean	CFontModule::SetFontAttr(tFontAttr attr)
{
	// TODO
	attr_ = attr;
	return true;
}

//----------------------------------------------------------------------------
Boolean	CFontModule::GetFontAttr(tFontAttr* pAttr)
{
	// TODO
	*pAttr = attr_;
	return true;
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

	// FIXME/dm: Handle general case:
	// 2D surface = rightside-up bitmap for downward Y window coords
	// 3D surface = upside-down bitmap for upward Y OpenGL coords

	// Pack RGB color into buffer according to mono bitmap mask
	w = (source->width+7) / 8;
	h = source->rows;
	s = t = source->buffer;
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
// Draw single glyph at XY location in rendering context buffer
//----------------------------------------------------------------------------
Boolean CFontModule::DrawGlyph(char ch, int x, int y, tFontSurf* pCtx)
{
	// FIXME: Until integration with Display manager is figured out,
	//        a device context argument will need to passed as argument.
	//        For now, this is a pointer to tFontSurf surface descriptor. 

	FT_Glyph		glyph;
	FT_BitmapGlyph  bitmap;
	FT_Bitmap*      source;
	FT_Render_Mode  render_mode = FT_RENDER_MODE_MONO;
	int				error = FT_Err_Ok;
	int				index;
	PFont			font;
	
#if USE_FONT_CACHE_MGR
	// Sanity check
	if (handle_.currentFont == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: invalid font selection\n" );
		return false;
	}
	font = handle_.currentFont;
	
  	// For selected font, find the glyph matching the char
    index = FTC_CMapCache_Lookup( handle_.cmapCache, handle_.imageType.face_id, handle_.currentFont->cmapIndex, ch );
	if (index == 0) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: unable to support char = %c, index = %d\n", ch, index );
		return false;
	}

	error = FTC_ImageCache_Lookup( handle_.imageCache, &handle_.imageType, index, &glyph, NULL );
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: unable to locate glyph for char = %c, index = %d, error = %d\n", ch, index, error );
		return false;
	}
#else
	// Sanity check
	if (handle_.face == NULL) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: invalid font selection\n" );
		return false;
	}
	font = handle_.fonts[handle_.curFont];
	
  	// For selected font, find the glyph matching the char
	index = FT_Get_Char_Index(handle_.face, ch);
	if (index == 0) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: unable to support char = %c, index = %d\n", ch, index );
		return false;
	}

	// FIXME: Error 36 = invalid size selection
	error = FT_Load_Char(handle_.face, index, FT_LOAD_DEFAULT);
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: unable to support char = %c, index = %d, error = %d\n", ch, index, error );
		return false;
	}

	error = FT_Get_Glyph(handle_.face->glyph , &glyph);
	if (error) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: unable to locate glyph for char = %c, error = %d\n", ch, error );
		return false;
	}
#endif		

	// Special handling for spaces: (and other non-visible glyphs?)
	// No bitmaps are created, though XY cursor still needs to advance.
	if ( ch == ' ' ) 
	{
		curX_ += ( glyph->advance.x + 0x8000 ) >> 16;
	    curY_ += ( glyph->advance.y + 0x8000 ) >> 16;
		return true;		
	}

	if ( glyph->format == FT_GLYPH_FORMAT_OUTLINE ) 
	{
		// Adjust font render mode as necessary
			
		// Render the glyph to a bitmap, don't destroy original 
		error = FT_Glyph_To_Bitmap( &glyph, render_mode, NULL, false );
		if ( error )
			return false;
	}
	
	if ( glyph->format != FT_GLYPH_FORMAT_BITMAP ) 
	{
		dbg_.DebugOut(kDbgLvlCritical, "CFontModule::DrawString: invalid glyph format returned, error = %d\n", error );
		return false;
	}
	
	// The glyph is now a bitmap
	bitmap = (FT_BitmapGlyph)glyph;
	source = &bitmap->bitmap;

	// Account for bitmap XY bearing offsets and overall font ascent 
	int dx = bitmap->left;
	int dy = bitmap->top;
	int dz = font->ascent;

	// TODO: Handle other source bitmap cases besides 1bpp mono
	// TODO: Handle other destination buffer cases besides 32bpp RGB

	// Draw mono bitmap into RGB context buffer with current color
	ConvertBitmapToRGB32(source, x+dx, y+dz-dy, pCtx);

	// Update the current XY glyph cursor position
	// Internal glyph cursor is in 16:16 fixed-point
	curX_ += ( glyph->advance.x + 0x8000 ) >> 16;
    curY_ += ( glyph->advance.y + 0x8000 ) >> 16;
	
	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::DrawString(CString* pStr, int x, int y, tFontSurf* pCtx)
{
	const char*		ch = pStr->c_str();
	int				len = pStr->size();
	int				i;
	Boolean			rc;
	
	dbg_.DebugOut(kDbgLvlVerbose, "FontModule::DrawString: %s, XY = %d,%d, length = %d\n", ch, x, y, len);

	// Set current XY glyph cursor position
	curX_ = x;
	curY_ = y;
	
	// Draw each char glyph in the string
	for (i = 0; i < len; i++) 
	{
		rc = DrawGlyph(ch[i], curX_, curY_, pCtx);
		if (!rc)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------
U32 CFontModule::GetX()
{
	return curX_;
}

//----------------------------------------------------------------------------
U32 CFontModule::GetY()
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

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

LF_USING_BRIO_NAMESPACE()

static CFontModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if( sinst == NULL )
		sinst = new CFontModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}


// EOF
