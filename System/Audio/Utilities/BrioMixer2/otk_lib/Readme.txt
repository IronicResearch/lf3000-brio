/****************************************************************************************/
/* Otk_lib.c - A general purpose GUI environment based on OpenGL and GLUT or X-Windows. */
/*  (GUI = Graphical User Interface.)                                                   */
/*                                                                                      */
/* For Documentation and Usage Notes, see:    http://otk.sourceforge.net/               */
/*                                                                                      */
/* To Compile:   Directive in code should detect environment and do the right thing.    */
/*  Unix/Linux:                                                                         */
/*        cc -O -c -I/usr/X11R6/include otk_lib.c -o otk.o                              */
/*    Link with:    -lGLU -lGL -lXmu -lXext -lX11                                       */
/*  Microsoft with MinGW compiler:                                                      */
/*        gcc -O -c otk_lib.c -o otk.o                                                  */
/*    Link with:    -lglu32 -lopengl32 -lwinmm -lgdi32                                  */
/*                                                                                      */
/* Carl Kindman 6-17-2004     carlkindman@yahoo.com                                     */
/****************************************************************************************/
