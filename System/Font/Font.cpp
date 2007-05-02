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
my_face_requester( FTC_FaceID  face_id,
					FT_Library  lib,
					FT_Pointer  request_data,
					FT_Face*    aface )
{
    int error;
    
    PFont  font = (PFont)face_id;

    FT_UNUSED( request_data );

    if ( font->file_address != NULL )
	  error = FT_New_Memory_Face( lib, (const FT_Byte*)font->file_address, font->file_size,
	                              font->face_index, aface );
    else
      error = FT_New_Face( lib,
                           font->filepathname,
                           font->face_index,
                           aface );
    if ( !error )
    {
      char*  suffix;
      char   orig[4];


      suffix = strrchr( font->filepathname, '.' );
      if ( suffix && ( strcasecmp( suffix, ".pfa" ) == 0 ||
                       strcasecmp( suffix, ".pfb" ) == 0 ) )
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
        (*aface)->charmap = (*aface)->charmaps[font->cmap_index];
    }

    return error;
}
#endif
  
//============================================================================
// Ctor & dtor
//============================================================================
CFontModule::CFontModule() : dbg_(0) // FIXME
{
	// TODO: What does this do? (Holdover from CDisplayModule...)
	//InitModule();	// delegate to platform or emulation initializer
			
	// Load FreeType library
	int error = FT_Init_FreeType(&handle_.library);
	printf("CFontModule: FT_Init_FreeType returned = %d, %p\n", error, handle_.library);
	if (error)
		return;	
	
#if USE_FONT_CACHE_MGR
	// FT samples setup FT cache manager too
	error = FTC_Manager_New( handle_.library, 0, 0, 0, my_face_requester, 0, &handle_.cache_manager );
    if ( error ) {
    	printf("CFontModule: could not initialize cache manager\n");
    	return;
    }

    error = FTC_SBitCache_New( handle_.cache_manager, &handle_.sbits_cache );
    if ( error ) {
    	printf("CFontModule: could not initialize small bitmaps cache\n" );
    	return;
    }

    error = FTC_ImageCache_New( handle_.cache_manager, &handle_.image_cache );
    if ( error ) {
    	printf("CFontModule: could not initialize glyph image cache\n" );
    	return;
    }

    error = FTC_CMapCache_New( handle_.cache_manager, &handle_.cmap_cache );
    if ( error ) {
    	printf("CFontModule: could not initialize charmap cache\n" );
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
	for (int i = 0; i < handle_.num_fonts; i++) {
		if (handle_.fonts[i])
			free(handle_.fonts[i]);
	}
	free(handle_.fonts);

#if USE_FONT_CACHE_MGR
	// Unload FreeType font cache manager
	FTC_Manager_Done( handle_.cache_manager );
#endif 
		
	// Unload FreeType library
	FT_Done_FreeType(handle_.library);
}

//----------------------------------------------------------------------------
Boolean	CFontModule::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::LoadFont(const CString* pName, tFontProp Prop)
{
	FT_Face		face;
	PFont		font;
	int			error;
	const char*	filename = pName->c_str();
	int			pixel_size;
	FTC_ScalerRec  scaler;
    FT_Size        size;
	
	// Bogus font file name?
	if (filename == NULL) {
		printf("CFontModule::LoadFont: invalid filename passed\n");
		return false;
	}
	
	// Load font file
	error = FT_New_Face(handle_.library, filename, 0, &face );
	printf("CFontModule::LoadFont: FT_New_Face (%s) returned = %d\n", filename, error);
	if (error)
		return false;
		
	// Allocate mem for font face
	font = (PFont)malloc( sizeof ( *font ) );
	
	font->filepathname = (char*)malloc( strlen( filename ) + 1 );
	strcpy( (char*)font->filepathname, filename );
	
	font->face_index = 0; // i;
	font->cmap_index = face->charmap ? FT_Get_Charmap_Index( face->charmap ) : 0;
	
	// TODO: Handle character encoding cases
	// This is default encoding case
    font->num_indices = face->num_glyphs;

#if USE_FONT_CACHE_MGR
	// Don't need any more font face info after this point
    FT_Done_Face( face );
    face = NULL;
#else    
    // Cache face for subsequent use until caching is figured out
    handle_.face = font->face = face;
#endif

	// Allocate space to load this font in our font list to date
    if ( handle_.max_fonts == 0 ) {
		handle_.max_fonts = 16;
		handle_.fonts     = (PFont*)calloc( handle_.max_fonts, sizeof ( PFont ) );
    }
    else if ( handle_.num_fonts >= handle_.max_fonts ) {
		handle_.max_fonts *= 2;
		handle_.fonts      = (PFont*)realloc( handle_.fonts, handle_.max_fonts * sizeof ( PFont ) );
		
		memset( &handle_.fonts[handle_.num_fonts], 0, ( handle_.max_fonts - handle_.num_fonts ) * sizeof ( PFont ) );
    }

	// Add this font to our list of fonts
    handle_.fonts[handle_.num_fonts++] = font;
    
#if USE_FONT_CACHE_MGR
 	// Set current font for cache manager   
    handle_.current_font = font;
    scaler.face_id = handle_.image_type.face_id = (FTC_FaceID)font;
    
    // Set selected font size
    pixel_size = (Prop.size * 72 + 36) / 72; 
    scaler.width   = handle_.image_type.width  = (FT_UShort)pixel_size;
    scaler.height  = handle_.image_type.height = (FT_UShort)pixel_size;
    scaler.pixel   = 1;
    
    error = FTC_Manager_LookupSize( handle_.cache_manager, &scaler, &size );
#else
	// FIXME: Select font size from available sizes
#endif	
		
	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::UnloadFont()
{
	// TODO
	if (handle_.face) {
		FT_Done_Face(handle_.face);
		handle_.face = NULL;
	}
	return true;
}

//----------------------------------------------------------------------------
Boolean	CFontModule::SetFontAttr(tFontAttr Attr)
{
	// TODO
	attr_ = Attr;
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

Boolean CFontModule::DrawGlyph(char ch, int X, int Y, void* pCtx)
{
	// FIXME: Until integration with Display manager is figured out,
	//        a device context argument will need to passed as argument.
	//        For now, this is actually a pointer to surface memory. 

	FT_Glyph		glyph;
	FT_BitmapGlyph  bitmap;
	FT_Bitmap*      source;
	FT_Render_Mode  render_mode = FT_RENDER_MODE_MONO;
	int				error = FT_Err_Ok;
	int				index;
	
#if USE_FONT_CACHE_MGR
  	// For selected font, find the glyph matching the char
    index = FTC_CMapCache_Lookup( handle_.cmap_cache, handle_.image_type.face_id, handle_.current_font->cmap_index, ch );
	if (index == 0) {
		printf( "CFontModule::DrawString: unable to support char = %c\n", ch );
		return false;
	}

	error = FTC_ImageCache_Lookup( handle_.image_cache, &handle_.image_type, index, &glyph, NULL );
	if (error) {
		printf( "CFontModule::DrawString: unable to locate glyph for char = %c\n", ch );
		return false;
	}
#else
  	// For selected font, find the glyph matching the char
	index = FT_Get_Char_Index(handle_.face, ch);
	if (index == 0) {
		printf( "CFontModule::DrawString: unable to support char = %c\n", ch );
		return false;
	}

	// FIXME: Error 36 = invalid size selection
	error = FT_Load_Char(handle_.face, index, FT_LOAD_DEFAULT);
	if (error) {
		printf( "CFontModule::DrawString: unable to support char index = %c\n", ch );
		return false;
	}

	error = FT_Get_Glyph(handle_.face->glyph , &glyph);
	if (error) {
		printf( "CFontModule::DrawString: unable to locate glyph for char = %c\n", ch );
		return false;
	}
#endif		

	if ( glyph->format == FT_GLYPH_FORMAT_OUTLINE ) {
		// Adjust font render mode as necessary
			
		// Render the glyph to a bitmap, don't destroy original 
		error = FT_Glyph_To_Bitmap( &glyph, render_mode, NULL, false );
		if ( error )
			return false;
	}
	
	if ( glyph->format != FT_GLYPH_FORMAT_BITMAP ) {
		printf( "CFontModule::DrawString: invalid glyph format returned!\n" );
		return false;
	}
	
	// The glyph is now a bitmap
	bitmap = (FT_BitmapGlyph)glyph;
	source = &bitmap->bitmap;

#if 0	// FIXME: Need valid surface memory location	
	// Put bitmap into device context via copy or blit
	memcpy(pCtx, source->buffer, source->pitch * source->rows);
#elif 1	// Font surf passed as context
	int 		x,y,w,h,i,mask;
	U8	 		*s,*t;
	U32 		*d,*u;
	U32	 		color = attr_.color;
	tFontSurf	*surf = (tFontSurf*)pCtx;
	// Pack RGB color into buffer according to mono bitmap mask
	w = (source->width+7) / 8;
	h = source->rows;
	s = t = (U8*)source->buffer;
	d = u = (U32*)surf->buffer + Y * surf->pitch + X * 4;
	for (y = 0; y < h; y++) {
		s = t;
		d = u;
		for (x = 0; x < w; x++) {
			mask = 0x80;
			for (i = 0; i < 8; i++) {
				if (mask & *s)
					*d = color;
				mask >>= 1;
				d++;
			}
			s++;
		}
		t += source->pitch;
		u += surf->pitch;
	}				  
#endif

	// Update the current XY glyph cursor position
	// Internal glyph cursor is in 16:16 fixed-point
	curX_ += ( glyph->advance.x + 0x8000 ) >> 16;
    curY_ += ( glyph->advance.y + 0x8000 ) >> 16;
	
	return true;
}

//----------------------------------------------------------------------------
Boolean CFontModule::DrawString(CString* pStr, int X, int Y, void* pCtx)
{
	const char*		ch = pStr->c_str();
	int				len = pStr->size();
	int				i;
	Boolean			rc;
	
	// Set current XY glyph cursor position
	curX_ = X;
	curY_ = Y;
	
	// Draw each char glyph in the string
	for (i = 0; i < len; i++) {
		rc = DrawGlyph(ch[i], curX_, curY_, pCtx);
		if (!rc)
			return false;
	}

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
