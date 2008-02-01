/****************************************************************************************/
/* Readguifile.c - Reads a previousely exported Otk GUI back into Otk-GuiBuilder.	*/
/*											*/
/* Presently this is a weak parser that will read GUI's exported from gbuilder,	but may	*/
/* not necessary handle modified files properly.					*/
/*											*/
/* Edit or view with tabs being std eight-characters.					*/
/****************************************************************************************/

#include <stdio.h>
#define MaxLine 20000


int test( char *a, char *b )
{
 printf("Comparing '%s' to '%s'\n", a, b );
 return strcmp(a,b);
}



/*.......................................................................
  .     NEXT_WORD - accepts a line of text, and returns with the        .
  . next word in that text in the third parameter, the original line    .
  . is shortened from the beginning so that the word is removed.        .
  . If the line encountered is empty, then the word returned will be    .
  . empty.                                                              .
  . NEXTWORD can parse on an arbitrary number of delimiters, and it 	.
  . returns everthing that was cut away in the trash (2nd) parameter.	.
  .......................................................................*/
void Next_Word( char *line, char *trash, char *word, char *delim )
{
 int i=0, j=0, k=0, m=0, flag=1;

 /* Eat away preceding garbage */
 while ((line[i]!='\0') && (flag))
  {
   j = 0;
   while ((delim[j]!='\0') && (line[i]!=delim[j])) j = j + 1;
   if (line[i]==delim[j]) 
    { trash[k] = line[i];  k = k + 1;  i = i + 1; }
   else  flag = 0;
  }

 /* Copy the word until the next delimiter. */
 while ((line[i]!='\0') && (!flag))
  {
   word[m] = line[i];
   m = m + 1;
   i = i + 1;
   if (line[i]!='\0')
    {
     j = 0;
     while ((delim[j]!='\0') && (line[i]!=delim[j])) j = j + 1;
     if (line[i]==delim[j]) flag = 1;
    }
  }

 /* Shorten line. */
 j = 0;
 while (line[i]!='\0')
  { line[j] = line[i];  j = j + 1;  i = i + 1; }

 /* Terminate the char-strings. */
 line[j] = '\0';
 trash[k] = '\0';
 word[m] = '\0';
}



#if (0)
 // Expect GUI files to be in the form of:

	/* Created by OTK GUI-Builder v0.06 */
	/* Compile on Linux with:
	     cc -I/usr/X11R6/include -L/usr/X11R6/lib newgui.c \
		-lGLU -lGL -lXmu -lXext -lX11 -lm -o newgui.exe
	*/
	#include "otk_lib/otk_lib.c"
	OtkWidget obj0, obj1, obj2, obj3, obj4, obj5;
	
	main( int argc, char **argv )
	{
	  OtkInitWindow( 610, 600, argc, argv );
	  obj0 = OtkMakePanel( OtkOuterWindow, Otk_Raised, Otk_LightGray, 12, 22, 82, 38 );
	  obj1 = OtkMakeTextLabel( OtkOuterWindow, "hellow world", Otk_Black, 2.15385, 1.0 , 10, 10 );
	  obj2 = OtkMakeButton( OtkOuterWindow, 10, 26, 22, 6, "SA button", 0, 0 );
	  obj3 = OtkMakeTextFormBox( OtkOuterWindow, "", 11, 12, 40, 18, 4, 0, 0 );
	  obj4 = OtkMakeToggleButton( OtkOuterWindow, 12, 52, 10, 6, 0, 0 );
	  obj2 = OtkMakeSliderVertical( obj0, 51.2195, 26.3158, 68.421, 0, 0 );
	  obj3 = OtkMakeSliderHorizontal( obj0, 7.40741, 8.33333, 74.0741, 0, 0 );
	  obj4 = Otk_Add_BoundingBox( obj0, Otk_Black, 1.0, 4.87805, 21.0526, 46.3415, 105.263 );
	  obj5 = Otk_Add_Line( obj0, Otk_Black, 1.0, 85.1852, 16.6667, 85.1852, 75 );
	  OtkMainLoop();
	}

//  obj5 = OtkMakeRadioButton( OtkOuterWindow, 8, 64, 4, 4, 0, 0 );
//  obj6 = Otk_Make_Menu( obj5, 8, 74, 18, 6, "Pulldown  " );
//  obj7 = Otk_Make_Selection_List( OtkOuterWindow, 8, 9, 10, 84, 14, 12);
//  obj8 = OtkMakeSliderHorizontal( OtkOuterWindow, 52, 14, 40, 0, 0 );
//  obj11 = Otk_MakeLEDmeter( OtkOuterWindow,  54, 56, 6, 14, 10, 'v', Otk_Red );
//  obj12 = Otk_MakeBarMeter( OtkOuterWindow,  64, 56, 6, 16, 'v', Otk_Green );
//  obj13 = Otk_MakeGauge2( OtkOuterWindow, 74, 56, 12, 16, "Gauge" );
//  obj14 = Otk_Plot_Init( OtkOuterWindow, "X-axis", "Y-axis",  56, 78, 16, 16, Otk_Green, Otk_DarkGray );
//  obj15 = Otk_Plot_Init( OtkOuterWindow, "X-axis", "Y-axis",  78, 78, 14, 14, Otk_Green, Otk_DarkGray );
//
//  So be sensitive to the lines looking like:
//	objxx = Otkxxxxx( xxxx );

#endif



struct objects_list_item
 {
   char *name, *container;
   float x1, y1, x2, y2;
   struct objects_list_item *nxt;
 } *read_objects_list=0;


void add_read_object( char *name, char *container, float x1, float y1, float x2, float y2 )
{
 struct objects_list_item *newpt;

 newpt = (struct objects_list_item *)malloc(sizeof(struct objects_list_item));
 newpt->name = strdup(name);
 newpt->container = strdup(container);
 newpt->x1 = x1;
 newpt->y1 = y1;
 newpt->x2 = x2;
 newpt->y2 = y2;
 newpt->nxt = read_objects_list;
 read_objects_list = newpt;
}


void adjust_coords( char *objname, char *parent, float *x1, float *y1, float *x2, float *y2 )
{ /* Convert inside container coords to absolute window coords. */
 struct objects_list_item *tmppt, *tmppt2;

 tmppt = read_objects_list;
 while ((tmppt!=0) && (strcmp(tmppt->name,parent)!=0)) tmppt = tmppt->nxt;
 while (tmppt!=0)
  {
   *x1 = 0.01 * *x1 * (tmppt->x2 - tmppt->x1) + tmppt->x1;
   *y1 = 0.01 * *y1 * (tmppt->y2 - tmppt->y1) + tmppt->y1;
   *x2 = 0.01 * *x2 * (tmppt->x2 - tmppt->x1) + tmppt->x1;
   *y2 = 0.01 * *y2 * (tmppt->y2 - tmppt->y1) + tmppt->y1;

   //tmppt2 = read_objects_list;
   //while (tmppt2!=0) && (strcmp(tmppt2->name,tmppt->container)!=0)) tmppt2 = tmppt2->nxt;
   //tmppt = tmpp2;
   tmppt = 0;
  }
} 




int read_gui_file( char *fname )
{
 FILE *infile=0;
 char line[MaxLine], word[2048], trash[2048], objname[500], parent[500];
 char param[10][100];
 int j;
 float xx1, xx2, yy1, yy2, x1, x2, y1, y2, thickness, fontsz;
 OtkColor color;

 infile = fopen(fname,"r");
 if (infile==0) {printf("ERROR: Cannot open input file '%s'.\n", fname);  return 0;}

 fgets(line, MaxLine, infile);
 while (!feof(infile))
  { /*parseline*/
    if (verbose) printf("\nLine='%s'\n", line);
    Next_Word(line, trash, objname, " \t=");

    if (strstr(objname,"obj")==objname)
     { /*possible_object*/
      int ptype=Otk_Raised;
      Next_Word(line, trash, word, " \t=(");

      if (strcmp(word,"OtkMakePanel")==0)
       { /*MakePanel*/  // OtkMakePanel( OtkOuterWindow, Otk_Raised, Otk_LightGray, 12, 22, 82, 38 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	if (strcmp(param[1],"Otk_Flat")==0) ptype = Otk_Flat; else
	if (strcmp(param[1],"Otk_Recessed")==0) ptype = Otk_Recessed;
	Next_Word(line, trash, param[2], " \t=,()");
        if (strcmp(param[2],"Otk_LightGray")==0) { color = Otk_LightGray; } else
        if (strstr(param[2],"OtkSetColor")==param[2]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	add_read_object( objname, param[0], x1, y1, x2, y2 );
	printf("Making Panel at: (%g, %g) of size (%g, %g)\n", x1, y1, x2-x1, y2-y1 );
        Add_object( PanelKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
        Set_object( OtkMakePanel( ContainerWidget, ptype, color, xx1, yy1, xx2-xx1, yy2-yy1 ) );
       } /*MakePanel*/

      else

      if (strcmp(word,"OtkMakeImagePanel")==0)
       { /*MakeImagePanel*/  // OtkMakeImagePanel( OtkOuterWindow, image_file, 12, 22, 82, 38 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)\"");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
printf("Putting image at (%g,%g) (%g,%g)\n", x1, y1, x2, y2 );
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	add_read_object( objname, param[0], x1, y1, x2, y2 );
	printf("Making ImagePanel at: (%g, %g) of size (%g, %g)\n", x1, y1, x2-x1, y2-y1 );
        Add_object( PanelKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
        Set_object( OtkMakeImagePanel( ContainerWidget, param[1], xx1, yy1, xx2-xx1, yy2-yy1 ) );

	OtkTranslateColor( Otk_LightGray, LastAddedObj->RenderedObject->color );
        LastAddedObj->RenderedObject->thickness = 1.0;
#if(0)
	tmpobj->x1 = left;
	tmpobj->y1 = top;
	tmpobj->x2 = left + horiz_size;
	tmpobj->y2 = top + vert_size;
	tmpobj->outlinestyle = Otk_Default_Button_Outline_Style;
	if (container==0)
	 {
	  tmpobj->xleft =     0.0;
	  tmpobj->xright =  100.0;
	  tmpobj->ytop =      0.0;
	  tmpobj->ybottom = 100.0;
	  tmpobj->z =         0.0;
	 }
	else
	 {
	  tmpobj->xleft =   container->xleft + tmpobj->x1 * (container->xright - container->xleft) * 0.01;
	  tmpobj->xright =  container->xleft + tmpobj->x2 * (container->xright - container->xleft) * 0.01;   
	  tmpobj->ytop =    container->ytop + tmpobj->y1 * (container->ybottom - container->ytop) * 0.01;   
	  tmpobj->ybottom = container->ytop + tmpobj->y2 * (container->ybottom - container->ytop) * 0.01;   
	  if (panel_subtype==Otk_Invisible) tmpobj->z = container->z;
	  else tmpobj->z = container->z + 0.5 * Otk_DZ;
	 }
#endif

       } /*MakePanel*/

      else

      if (strcmp(word,"Otk_Add_BoundingBox")==0) 
       { /*BoundingBox*/  // Otk_Add_BoundingBox( obj0, Otk_Black, 1.0, 4.87805, 21.0526, 46.3415, 105.263 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,()");
        if (strcmp(param[1],"Otk_Black")==0) { color = Otk_Black; } else
        if (strstr(param[1],"OtkSetColor")==param[1]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[2],"%f",&thickness);
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
        Add_object( SeparatorKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
        Set_object( Otk_Add_BoundingBox( ContainerWidget,  color, thickness, xx1, yy1, xx2, yy2 ) );
       } /*BoundingBox*/

      else

      if (strcmp(word,"Otk_Add_Line")==0)
       { /*AddLine*/  // Otk_Add_Line( obj0, Otk_Black, 1.0, 85.1852, 16.6667, 85.1852, 75 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,()");
        if (strcmp(param[1],"Otk_Black")==0) { color = Otk_Black; } else
        if (strstr(param[1],"OtkSetColor")==param[1]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[2],"%f",&thickness);
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
        Add_object( SeparatorKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
        Set_object( Otk_Add_Line( ContainerWidget, color, thickness, xx1, yy1, xx2, yy2 ) );
       } /*AddLine*/

      else

      if (strcmp(word,"OtkMakeTextLabel")==0)
       { /*TextLabel*/  // OtkMakeTextLabel( OtkOuterWindow, "hellow world", Otk_Black, 2.15385, 1.0 , 10, 10 );
	Next_Word(line, trash, param[0], " \t=,()");
        j = 0; while (line[j]!='"') j++;
	Next_Word( &(line[j+1]), trash, param[1], "\"");
	Next_Word(line, trash, param[2], "\" \t=,()");
        if (strcmp(param[2],"Otk_Black")==0) { color = Otk_Black; } else
        if (strstr(param[2],"OtkSetColor")==param[2]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[3],"%f",&fontsz);
	sscanf(param[4],"%f",&thickness);
	sscanf(param[5],"%f",&x1);
	sscanf(param[6],"%f",&y1);
	x2 = x1 + 1.0 + sqrt(fontsz) * strlen(param[1]);
	y2 = y1 + 5.0 * sqrt(fontsz);
	printf("Text label '%s' fontsz=%g, x1=%g y1=%g, x2=%g y2=%g\n", param[1], fontsz, x1, y1, x2, y2 );
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
        Add_object( TextLabelKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeTextLabel( ContainerWidget, param[1], color, fontsz, thickness , xx1, yy1 ) );
	LastAddedObj->text = strdup(param[1]);
	LastAddedObj->fontsz = fontsz;
	LastAddedObj->RenderedObject->x2 = x2;
	LastAddedObj->RenderedObject->y2 = y2;
       } /*TextLabel*/

      else

      if (strcmp(word,"OtkMakeButton")==0)
       { /*Button*/  // OtkMakeButton( OtkOuterWindow, 10, 26, 22, 6, "SA button", 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
        j = 0; while (line[j]!='"') j++;
	Next_Word(&(line[j+1]), trash, param[5], "\"");
	printf("Button '%s'\n", param[5]);
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
        Add_object( ButtonKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, param[5], 0, 0 ) );
	LastAddedObj->text = strdup(param[5]);
       } /*Button*/

      else

      if (strcmp(word,"OtkMakeTextFormBox")==0)
       { /*FormBox*/  // OtkMakeTextFormBox( OtkOuterWindow, "", 11, 12, 40, 18, 4, 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], ",");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( FormBoxKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeTextFormBox( ContainerWidget, "", (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
       } /*FormBox*/

      else

      if (strcmp(word,"OtkMakeToggleButton")==0)
       { /*Toggle*/  // OtkMakeToggleButton( OtkOuterWindow, 12, 52, 10, 6, 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( ToggleKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeToggleButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
       } /*Toggle*/

    else

      if (strcmp(word,"OtkMakeRadioButton")==0)
       { /*RadioButton*/  // OtkMakeRadioButton( OtkOuterWindow, 8, 64, 4, 4, 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( RadioButtonKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeRadioButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
       } /*RadioButton*/

    else

      if (strcmp(word,"Otk_Make_Menu")==0)
       { /*MenuButtin*/  // Otk_Make_Menu( OtkOuterWindow, 8, 74, 18, 6, "Pulldown  " );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
        j = 0; while (line[j]!='"') j++;
	Next_Word(&(line[j+1]), trash, param[5], "\"");
	j = strlen(param[5]);
	if ((j>2) && (param[5][j-1]==' ')) param[5][j-1] = '\0';
	if ((j>3) && (param[5][j-2]==' ')) param[5][j-2] = '\0';
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( PullDownMenuKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_Make_Menu( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, param[5] ) );
	LastAddedObj->text = strdup(param[5]);
       } /*MenuButton*/

    else

      if (strcmp(word,"Otk_Make_Selection_List")==0)
       { /*Make_Selection_List*/  // Otk_Make_Selection_List( OtkOuterWindow, 8, 9, 10, 84, 14, 12);
	int rows, cols;
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
	sscanf(param[1],"%d",&rows);
	sscanf(param[2],"%d",&cols);
	sscanf(param[3],"%f",&x1);
	sscanf(param[4],"%f",&y1);
	sscanf(param[5],"%f",&x2);
	sscanf(param[6],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( SelectionListKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_Make_Selection_List( ContainerWidget, rows, cols, xx1, yy1, xx2-xx1, yy2-yy1 ) );
       } /*Make_Selection_List*/

    else

      if (strcmp(word,"OtkMakeSliderHorizontal")==0)
       { /*OtkMakeSliderHorizontal*/  // OtkMakeSliderHorizontal( OtkOuterWindow, 52, 14, 40, 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	x2 = x1 + x2;
	y2 = y1 + 0.1 * (x2 - x1);
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( SliderKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeSliderHorizontal( ContainerWidget, xx1, yy1, xx2-xx1, 0, 0 ) );
	LastAddedObj->yy2 = LastAddedObj->RenderedObject->y2;
	LastAddedObj->y2 = 100.0 * (LastAddedObj->RenderedObject->ybottom - canvas->ytop) / CanvasHeight;
       } /*OtkMakeSliderHorizontal*/

    else

      if (strcmp(word,"OtkMakeSliderVertical")==0)
       { /*OtkMakeSliderVertical*/  // OtkMakeSliderHorizontal( OtkOuterWindow, 52, 14, 40, 0, 0 );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&y2);
	y2 = y1 + y2;
	x2 = x1 + 0.1 * (y2 - y1);
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( SliderKind, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeSliderVertical( ContainerWidget, xx1, yy1, yy2-yy1, 0, 0 ) );
	LastAddedObj->xx2 = LastAddedObj->RenderedObject->x2;
	LastAddedObj->x2 = 100.0 * (LastAddedObj->RenderedObject->xright - canvas->xleft) / CanvasWidth;
       } /*OtkMakeSliderVertical*/

     else

      if (strcmp(word,"Otk_MakeLEDmeter")==0)
       { /*Otk_MakeLEDmeter*/  // Otk_MakeLEDmeter( OtkOuterWindow,  54, 56, 6, 14, 10, 'v', Otk_Red );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,()");
	Next_Word(line, trash, param[7], " \t=,()");
        if (strcmp(param[7],"Otk_Red")==0) { color = Otk_Red; } else
        if (strstr(param[7],"OtkSetColor")==param[7]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	sscanf(param[5],"%d",&j);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( GadgetLedMeter , x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_MakeLEDmeter( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, j, param[6][1], color ) );
       } /*Otk_MakeLEDmeter*/

     else

      if (strcmp(word,"Otk_MakeBarMeter")==0)
       { /*Otk_MakeBarMeter*/  // Otk_MakeBarMeter( OtkOuterWindow,  64, 56, 6, 16, 'v', Otk_Green );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
	Next_Word(line, trash, param[5], " \t=,)");
	Next_Word(line, trash, param[6], " \t=,)");
        if (strcmp(param[6],"Otk_Green")==0) { color = Otk_Green; } else
        if (strstr(param[6],"OtkSetColor")==param[6]) 
	 { float r, g, b;
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&r)!=1) printf("Error:  Reading color red '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&g)!=1) printf("Error:  Reading color green '%s'\n", word);
	   Next_Word(line, trash, word, " \t,()");
	   if (sscanf(word,"%f",&b)!=1) printf("Error:  Reading color blue '%s'\n", word);
	   color = OtkSetColor( r, g, b );
	 } else printf("Error:  Unknown color specifier '%s' in Otk_Add_Line.\n", param[3]);
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( GadgetBarMeter, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_MakeBarMeter( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, param[5][1], color ) );
	Otk_SetBarMeter( LastAddedObj->RenderedObject, 50.0 );
       } /*Otk_MakeBarMeter*/

/*
  obj13 = Otk_MakeGauge2( OtkOuterWindow, 74, 56, 12, 16, "Gauge" );
  obj14 = Otk_Plot_Init( OtkOuterWindow, "X-axis", "Y-axis",  56, 78, 16, 16, Otk_Green, Otk_DarkGray );
  obj15 = Otk_Plot_Init( OtkOuterWindow, "X-axis", "Y-axis",  78, 78, 14, 14, Otk_Green, Otk_DarkGray );
#define AddGadget        11
#define GadgetGauge2     101
#define GadgetPlot       104
#define GadgetStripChart 105
*/

     else

      if (strcmp(word,"Otk_MakeGauge2")==0)
       { /*Otk_MakeGauge2*/  // Otk_MakeGauge2( OtkOuterWindow, 74, 56, 12, 16, "Gauge" );
	Next_Word(line, trash, param[0], " \t=,()");
	Next_Word(line, trash, param[1], " \t=,)");
	Next_Word(line, trash, param[2], " \t=,)");
	Next_Word(line, trash, param[3], " \t=,)");
	Next_Word(line, trash, param[4], " \t=,)");
        j = 0; while (line[j]!='"') j++;
	Next_Word(&(line[j+1]), trash, param[5], "\"");
	sscanf(param[1],"%f",&x1);
	sscanf(param[2],"%f",&y1);
	sscanf(param[3],"%f",&x2);
	sscanf(param[4],"%f",&y2);
	x2 = x1 + x2;
	y2 = y1 + y2;
	adjust_coords( objname, param[0], &x1, &y1, &x2, &y2 );
	Add_object( GadgetGauge2, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_MakeGauge2( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, param[5] ) );
       } /*Otk_MakeGauge2*/



     } /*possible_object*/

    else

    if (strcmp(objname,"Otk_SetBorderThickness(")==0)
     { /*Otk_SetBorderThickness*/
       Next_Word(line, trash, word, " \t,(");
       Next_Word(line, trash, word, " \t,)");
       sscanf(word,"%f",&x1);
printf("Setting border thickness to %g\n", x1 );
       Otk_SetBorderThickness( LastAddedObj->RenderedObject, x1 );
     } /*Otk_SetBorderThickness*/

    fgets(line, MaxLine, infile);
  } /*parseline*/
 return 1;
}
