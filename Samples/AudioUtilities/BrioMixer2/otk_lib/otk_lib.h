/*------------------------------------------------------------*/
/* OTK Macros.						      */
/*------------------------------------------------------------*/

#ifndef OTK_DEFS
#define OTK_DEFS 

#ifndef PLATFORM_KIND
 #define Posix_Platform  0 
 #define Mingw_Platform  1
 #define MsVisC_Platform 2
 #ifdef __CYGWIN32__
  #ifndef __CYGWIN__
   #define __CYGWIN__ __CYGWIN32__
  #endif
 #endif
 #if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MSYS__)
  #define PLATFORM_KIND Mingw_Platform /* MinGW or like platform */
 #elif defined(__WIN32) || defined(WIN32)
  #define PLATFORM_KIND MsVisC_Platform /* microsoft visual C */
 #else
  #define PLATFORM_KIND Posix_Platform    /* Posix/Linux/Unix */
 #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#if (PLATFORM_KIND==Posix_Platform)
 // #include <unistd.h>
 #define GlutEnabled 0  /* Choice: 0=Use X-windows, 1=Use Glut. */
 #define WinGLEnabled 0
 #include <GL/gl.h>
 #include <GL/glx.h>
 #include <sys/time.h>
#else
 #ifndef GlutEnabled
  #define GlutEnabled 0
 #endif
 #if (GlutEnabled==0)
  #define WinGLEnabled 1
 #else
  #define WinGLEnabled 0
 #endif
 #include <windows.h>
 #if (PLATFORM_KIND!=MsVisC_Platform)
  #include <windowsx.h>
 #endif
 #include <GL/gl.h>
#endif

#include <GL/glu.h>

#if (WinGLEnabled==0)
 #if (GlutEnabled==0)
  #include <X11/Xlib.h>
  #include <X11/Xatom.h>
  #include <X11/Xmu/StdCmap.h>
  #include <X11/keysym.h>
 #else
  #include <GL/glut.h>
 #endif
#endif

#define Otk_White  OtkSetColor( 1.0, 1.0, 1.0 )		/* Pre-defined basic colors. */
#define Otk_LightGray   OtkSetColor( 0.8, 0.8, 0.8 )
#define Otk_Gray   	OtkSetColor( 0.5, 0.5, 0.5 )
#define Otk_DarkGray	OtkSetColor( 0.25, 0.25, 0.25 )
#define Otk_Black  OtkSetColor( 0.0, 0.0, 0.0 )
#define Otk_Red    OtkSetColor( 1.0, 0.0, 0.0 )
#define Otk_Green  OtkSetColor( 0.0, 1.0, 0.0 )
#define Otk_Blue   OtkSetColor( 0.0, 0.0, 1.0 )
#define Otk_Cyan   OtkSetColor( 0.0, 1.0, 1.0 )
#define Otk_Yellow  OtkSetColor( 1.0, 1.0, 0.0 )
#define Otk_Violet  OtkSetColor( 1.0, 0.0, 1.0 )

#define Otk_Flat     0				/* Panel Types. */
#define Otk_Raised   1
#define Otk_Recessed 2
#define Otk_Invisible 10
#define Otk_ImagePanel 20

#define Otk_SC_Panel      1			/* Super-Classes. */
#define Otk_SC_TextLabel  2
#define Otk_SC_Button     3
#define Otk_SC_FormBox    4
#define Otk_SC_Line       5
#define Otk_SC_Box        500
#define Otk_SC_hSlider    6
#define Otk_SC_vSlider    7
#define Otk_SC_textscrollbar 8
#define Otk_SC_Window 	    10
#define Otk_SC_Menu_DropDown_Button 12
#define Otk_SC_Menu_Item 	    14
#define Otk_SC_Menu_Submenu 	    15
#define Otk_SC_RadioButton  	    20
#define Otk_SC_ToggleButton 	    30
#define Otk_SC_Select_List 	    40
#define Otk_SC_Select_List_Item     41

#define Otk_class_panel         1		/* Base Classes. */
#define Otk_class_text          2
#define Otk_class_button        3
#define Otk_class_line          5
#define Otk_class_box           500
#define Otk_class_radiobutton1 20
#define Otk_class_radiobutton2 21
#define Otk_class_togglebutton 30
#define Otk_class_triangle    100
#define Otk_class_disk        200
#define Otk_class_circle      250

#define Otk_subtype_plane            0		/* Sub-Types. */
#define Otk_subtype_raised           1
#define Otk_subtype_recessed         2
#define Otk_subtype_moveable         3
#define Otk_subtype_recessed_diamond 4
#define Otk_subtype_raised_diamond   5

/*------------------------------------------------------------*/
/* OTK Object Structures.				      */
/*------------------------------------------------------------*/
 
 struct OtkColor_record
  {
   float r, g, b;
  };
 typedef struct OtkColor_record OtkColor;

 typedef float OtkColorVector[4];

/* FONTS begin */
 struct OtkKernPair
  {
   char chr[2];
   float kerning;
  };

 typedef struct OtkGlyph_record OtkGlyph;
 struct OtkGlyph_record
  {
   int code;
   char *path;
   char orientation;
   float x_adv;
   float y_adv;
  };

 typedef struct OtkGlyphMetrics_record OtkGlyphMetrics;
 struct OtkGlyphMetrics_record 	/* predetermined data about glyph size */
  {
   float width; 	/* of the drawn glyph */
   float ascent; 	/* drawn above the baseline */
   float descent; 	/* drawn below the baseline */
   struct OtkKernPair **pairs;
  };

 typedef struct OtkFont_record OtkFont;
 struct OtkFont_record 	/* predetermined data about glyph size */
  {
   char *family;
   float weight;
   float units_per_em;
   float bbox[4];
   float ascent;
   float descent;
   float x_height;
   float underline_thickness;
   float underline_position;
   int start_glyph, end_glyph;
   unsigned int missing_glyph;  /* glyph to use for undefined glyphs */
   unsigned int glyphs;	  /* start list of the (96) glyphs */
   OtkGlyphMetrics *metrics;
   OtkGlyph **glyph_defs;
   OtkGlyph *missing_glyph_def;
  };

#define Otk_Font_Vect 0		/* Very basic vector built-in font. */
#define Otk_Font_Helv 1		/* Second and nicer built-in font. */
#define Otk_FontDefault Otk_Font_Helv

#define Otk_FontSpacing_Mono 0
#define Otk_FontSpacing_Proportional 1
#define Otk_FontSpacingDefault Otk_FontSpacing_Mono

/* FONTS end */


 struct OtkObjectInstance
  { int superclass,			/* Super-Class designator.  What object is part-of, or its purpose. */
	object_class, 			/* Base-Class designator.   The Drawable type. */
	object_subtype;			/* Kind of object within a class. */
    int Id;
    char *text;
    OtkFont *font;	/* FONTS insertion */
    float x1, y1, x2, y2, scale;	/* Percent coords of containing window. */
    float thickness, slant, sqrtaspect;	/* (Applies to lines and characters.) */
    float xleft, xright, ytop, ybottom;	/* Actual current absolute coordinates. (Related to x1, x2, y1, y2.) */
    float z;				/* Depth (background/foreground). */
    OtkColorVector color;
    float xscroll, yscroll;		/* Amount this object's contents are scrolled */
    int horiztextscroll;		/* Text form window state (left-right viewing). */
    int verttextscroll;			/* Text form window state (up-down viewing). */
    int nrows, ncols, nentries;		/* Number of text rows and columns for form boxes. */
    int outlinestyle;
    int mouse_sensitive;		/* True/false. */
    int state;
    int invisible;
    int scissor_to_parent;		/* Boolean flag to limit display to parent's footprint. */
    struct Otk_image *image;
    void (*callback)(void *x);		 /* Callback function for buttons, kill-callback for windows. */
    void (*functval1)(char *s, void *x); /* Callback function for textform-boxes which return charstrings. */
    void (*functval2)(float v, void *x); /* Callback function for sliders which return values. */
    void (*functval3)(int s, void *x);   /* Callback function for toggle which returns state. */
    void *callback_param;
    struct OtkObjectInstance *parent, *children, *child_tail, *hidden_children, *hidden_tail, *nxt;
  };

 typedef struct OtkObjectInstance *OtkWidget;

 struct OtkObjectList
  {
   OtkWidget object;
   struct OtkObjectList *nxt;
  };


 typedef struct OtkClipList OtkClipList;
 struct OtkClipList		/* Object clippling list. */
  {
   float xleft, ytop, xright, ybottom;
   OtkClipList *nxt;
  } ;


 /* Structures to support Tabbed-Window-panels. */
 typedef struct OtkTabbedPanel OtkTabbedPanel;
 typedef struct OtkTabbedPanelSelect OtkTabbedPanelSelect;

 struct OtkTabbedPanel
 {
  OtkWidget top;
  int num;
  char **names;
  OtkWidget panel_top;
  OtkWidget *panels;
  OtkWidget *buttons;
  OtkTabbedPanelSelect *selects;
  int selection;
  float panel_height;
  float button_height;
 };

 struct OtkTabbedPanelSelect
 {
  OtkTabbedPanel *tp;
  int selection;
 };

 /* Structures to support images. */
 struct Otk_image_rec { unsigned int r, g, b; };

 struct Otk_image
  {
   int cols, rows;
   struct Otk_image_rec *image;
   char *filename;
   int texturesize, texturerows, texturecols;
   #if (PLATFORM_KIND != MsVisC_Platform)
   GLuint  texturename;
   GLubyte *textureimage;
   #endif
   int calllist_num;
   struct Otk_image *nxt;
  };


/********************************************************************************/
/* Otk - Functions								*/
/********************************************************************************/

void OtkSetBorderThickness( float thickness );
OtkColor OtkSetColor( float r, float g, float b );

OtkWidget OtkMakePanel( OtkWidget container, int panel_type, OtkColor panel_color, float left, float top, float horiz_size, float vert_size );
void Otk_SetBorderThickness( OtkWidget container, float thickness );
void OtkResizePanel( OtkWidget panel, float left, float top, float horiz_size, float vert_size );
OtkWidget OtkMakeImagePanel( OtkWidget container, char *file_name, float left, float top, float horiz_size, float vert_size );

OtkWidget Otk_Add_Line( OtkWidget container, OtkColor tmpcolor, float thickness, float x1, float y1, float x2, float y2 );
OtkWidget Otk_Add_BoundingBox( OtkWidget container, OtkColor tmpcolor, float thickness, float x1, float y1, float x2, float y2 );
void Otk_Set_Line_Thickness( OtkWidget tmpobj, float thickness );

OtkWidget OtkMakeTextLabel( OtkWidget container, char *text, OtkColor text_color, float scale, float thickness, float x, float y );
void Otk_Modify_Text_Slant( OtkWidget tmpobj, float slant );
void Otk_Modify_Text_Color( OtkWidget tmpobj, OtkColor text_color );
void Otk_Modify_Text_Thickness( OtkWidget tmpobj, float thickness );
void Otk_Modify_Text_Scale( OtkWidget tmpobj, float scale );
void Otk_Modify_Text_Aspect( OtkWidget tmpobj, float aspect );
void Otk_Set_Text_Aspect( float aspect );
void Otk_Modify_Text_Position( OtkWidget tmpobj, float x, float y );
void Otk_Get_Text_Size( OtkWidget tmpobj, float *width, float *height );
void Otk_Get_Character_Size( OtkWidget tmpobj, float *width, float *height );
void Otk_Modify_Text( OtkWidget tmpobj, char *text );
void Otk_FitTextInPanel( OtkWidget txtobj );

OtkWidget OtkMakeButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			 char *text, void (*callback)(void *x), void *parameter );
void Otk_Set_Default_Button_Color( float r, float g, float b );
void Otk_Set_Button_Color( OtkWidget container, OtkColor panel_color );
void Otk_Set_Panel_Color( OtkWidget container, OtkColor panel_color );
void Otk_Set_Default_Button_BorderThickness( float x );
void Otk_Set_Button_Outline_Style( int style );
void Otk_Set_Button_State( OtkWidget container, int state );
int Otk_Get_Button_State( OtkWidget container );
OtkWidget OtkMakeToggleButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			       void (*callback)(int state, void *x), void *parameter );
OtkWidget OtkMakeRadioButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			      void (*callback)(void *x), void *parameter );
void Otk_SetRadioButton( OtkWidget topobj );

OtkWidget OtkMakeWindow( int panel_type, OtkColor tab_color, OtkColor panel_color, 
				 float left, float top, float horiz_size, float vert_size );
void OtkSetWindowTitle( OtkWidget window, OtkColor text_color, char *title );

OtkWidget OtkMakeTextFormBox( OtkWidget container, char *text, int ncols, 
                                       float x, float y, float horiz_size, float vert_size,
                                       void (*callback)(char *s, void *x), void *parameter );
void Otk_Get_Text( OtkWidget tmpobj, char *text, int n );
OtkWidget OtkMakeTextEditBox( OtkWidget container, char *text, int nrows, int ncols, 
                                float left, float top, float horiz_size, float vert_size );
void OtkAddTextScrollbar( OtkWidget container, float width );

OtkWidget Otk_Browse_Files( char *prompt, int maxlength, char *directory, char *wildcards, 
			    char *filename, void (*callback)(char *fname) );


int Otk_handle_key_input( int ks );
void Otk_handle_key_release( int ks );
int Otk_UpdateMouse( int x, int y, int state );
int Otk_handle_mouse_move( int MouseDx, int MouseDy );

OtkWidget Otk_Make_Menu( OtkWidget container, float left, float top, float horiz_size, float vert_size, char *text );
OtkWidget Otk_Add_Menu_Item( OtkWidget container, char *text, void (*callback)(void *x), void *parameter );
OtkWidget Otk_Add_SubMenu( OtkWidget container, char *text );

OtkWidget Otk_Make_Selection_List( OtkWidget container, int rows, int cols, 
                                   float left, float top, float horiz_size, float vert_size );
void Otk_Frame_Selection_List( OtkWidget container );
OtkWidget Otk_Add_Selection_Item( OtkWidget container, char *text, void (*callback)(void *x), void *parameter );

OtkWidget Otk_MakeDisk( OtkWidget container, float x, float y, float radius, OtkColor disk_color );
OtkWidget Otk_MakeCircle( OtkWidget container, float x, float y, float radius, OtkColor circ_color, float thickness );


OtkWidget OtkMakeSliderVertical( OtkWidget container, float left, float top, float vertical_size, 
				 void (*callback)(float v, void *x), void *parameter );
OtkWidget OtkMakeSliderHorizontal( OtkWidget container, float left, float top, float horiz_size,
				   void (*callback)(float v, void *x), void *parameter );
void Otk_SetSlider( OtkWidget slider, float position, float sz );
void Otk_Set_Default_Slider_Width( float x );

void OtkInitWindow( int WinWidth, int WinHeight, int argc, char **argv );
void OtkMakeOuterWindow();
void OtkUpdateWindow( int WinWidth, int WinHeight );

OtkWidget Otk_RemoveObject( OtkWidget objt );
void Otk_ClearAll();
void Set_PermanentObjs();
void OtkDrawObjectTree();
void Otk_ReDraw_Display();

OtkTabbedPanel *Otk_Tabbed_Panel_New( OtkWidget parent, int num, char **names, OtkColor color, 
				      float left, float top, float width, float height, float button_height );


OtkWidget Special_Otk_Add_Selection_Item_TO_LAST( OtkWidget container, char *text, void (*callback)(void *x), void *parameter, OtkWidget *lastitem );


OtkFont *Otk_Read_SVG_Font( char *filename );  /* FONTS insertion */
int Otk_Get_Shift_Key();
int Otk_Get_Control_Key();


/* Otk Internal or Special Functions. */
void Otk_object_attach( OtkWidget parent, OtkWidget child );
void Otk_object_attach_at_end( OtkWidget parent, OtkWidget child );
void Otk_object_attach_hidden( OtkWidget parent, OtkWidget child );
void Otk_object_attach_hidden_at_front( OtkWidget parent, OtkWidget child );
void Otk_object_detach( OtkWidget child );
void Otk_object_detach_hidden( OtkWidget child );
void Otk_object_detach_any( OtkWidget child );
void Otk_object_correct_position( OtkWidget obj, int descend );
void Otk_calculate_object_position( OtkWidget container, OtkWidget obj );
void Otk_position_object( OtkWidget objpt, int absolute, float x, float y );
void Otk_move_object( OtkWidget objpt, int absolute, float x, float y );

void Otk_Register_MouseClick_Callback( void (*callback)(int state) );
void Otk_Register_MouseMove_Callback( void (*callback)() );

#endif
