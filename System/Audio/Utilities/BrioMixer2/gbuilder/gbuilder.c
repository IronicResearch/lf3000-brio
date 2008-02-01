/****************************************************************/
/* Otk_GuiBuilder.c - GUI-Builder for Otk.  (gbuilder.c)	*/
/*								*/
/* Version 0.23	- Alpha						*/
/*								*/
/* On Linux, compile with:					
     cc -O -I/usr/X11R6/include -L/usr/X11R6/lib gbuilder.c \
	-lGLU -lGL -lXmu -lXext -lX11 -lm -o gbuilder.exe

   On Sun Solaris, compile with:
     gcc -O -I/usr/dt/include -L/usr/dt/lib gbuilder.c -lGLU \
	-lGL -lXmu -lXext -lX11 -lm -o gbuilder.exe

   On Apple Mac OSx (Darwin), compile with:
     cc -O -I/sw/include -I/usr/X11R6/include -L/sw/lib \
	-L/usr/X11R6/lib gbuilder.c -lGLU -lGL -lX11 -lXmu \
	-lm -o gbuilder.exe

   On Microsoft with MinGW, compile with:
     gcc -O -Lc:\MinGW\lib -Ic:\MinGW\include gbuilder.c -lglu32 \
	-lopengl32 -lwinmm -lgdi32 -lws2_32 -lm -o gbuilder.exe

								*/
/* This is an early version !
   Still to be done:
 	Need to be able to edit some more properties of selected gadgets.
	Need to add undo.
*/
/****************************************************************/

#include "otk_lib/otk_lib.c"
#include "otk_lib/gadget_lib.h"
#include "otk_lib/gadget_lib.c"

#define gb_version 0.23
int verbose = 0;

/* Define modes. */
#define SelectMove	 0
#define PanelKind	 1
#define TextLabelKind	 2
#define FormBoxKind	 3
#define ButtonKind	 4
#define ToggleKind	 5
#define RadioButtonKind	 6
#define PullDownMenuKind 7
#define SelectionListKind 8
#define SliderKind	 9
#define SeparatorKind	 10
#define GadgetKind	 11
#define GadgetGauge2	 101
#define GadgetLedMeter	 102
#define GadgetBarMeter	 103
#define GadgetPlot	 104
#define GadgetStripChart 105
#define GadgetIndLight	 106

int tool_mode = SelectMove;
#define LeftPanelX	15
#define TopPanelY  	5
OtkWidget SelectedToggle[20];

OtkWidget rubberband[4]={0}, popupwindow=0, textform, ContainerWidget, canvas;
float mmx1, mmy1;	/* Mouse-move x,y point 1. */
float CanvasWidth, CanvasHeight;
#define rubberbandcolor Otk_Red
int objidcnt=0, snap2grid=1, mousedragged=0, mousestate=0, popupopen=0, ontick=0;
float gridsize=2.0, origmousex, origmousey;
int gadgettype=0;

struct GbuilderObject
 { int kind;				/* Kind of object. (From modes set.) */    
   int Id, layer;
   char *name, *text;
   float x1, y1, x2, y2;		/* Percent coords of canvas window. (absolute) */
   float xx1, yy1, xx2, yy2;		/* Percent coords of containing window. (relative) */
   float thickness, slant; 		/* Attributes applying to lines and characters, or panel border-thickness. */
   float fontsz;
   OtkColorVector color;
   int paneltype;
   char *imagename;
   int nrows, ncols, nentries;         /* Number of text rows and columns for form boxes. */
   int outlinestyle;
   OtkWidget RenderedObject;
   struct GbuilderObject *parent, *child, *nxtsibling, *nxt;
 } *ObjectsList = 0, *LastAddedObj = 0, *LastSelected, *PasteBuffer=0;

struct tickmarklist
{
  OtkWidget tickmarks[4];
  struct tickmarklist *nxt;
} *tickmark_freelist=0;

struct selobjectitem
 {
   struct GbuilderObject *object;
   struct tickmarklist *tickmarks;
   float origobjx, origobjy, origobjx2, origobjy2, alphax, alphay;	/* Original rendered coords. */
   struct selobjectitem *nxt;
 } *SelectedObjectsList=0;		// (Replaces SelectedObject )

 float grpminx, grpmaxx, grpminy, grpmaxy;



/*************************************************************************
 Coordinate Systems -
 There are two coordinate systems:
    1. GBuilder (recorded for eventual export)
		x1, y1, x2, y2 	   - Percent coords of canvas window. (absolute).
				     Becomes eventual xleft, ... ybottom.
		xx1, yy1, xx2, yy2 - Percent coords of containing window. (relative)
				     Actually exported.
    2. Otk (as presently rendered)
		x1, y1, x2, y2     - Percent coords of containing window. (relative)
		xleft,ytop,xright,ybottom - Absolute Percent coords of outer-window.

 At any time, the present Otk display coordinates can be calculated from the GBuilder coordinates as:
	otk->x1 = gbuilder->xx1;	// Match-check 1.
	otk->y1 = gbuilder->yy1;
	otk->x2 = gbuilder->xx2;
	otk->y2 = gbuilder->yy2;
     And,
	if (gbuilder->parent==0) gbuilder->xx1,yy1,xx2,yy2 = gbuilder->x1,y1,x2,y2
	else
	 gbuilder->xx1 = 100.0 * (gbuilder->x1 - gbparent->x1) / (gbparent->x2 - gbparent->x1)		// Match-check 2.
	 gbuilder->yy1 = 100.0 * (gbuilder->y1 - gbparent->y1) / (gbparent->y2 - gbparent->y1)
	 gbuilder->xx2 = 100.0 * (gbuilder->x2 - gbparent->x1) / (gbparent->x2 - gbparent->x1)
	 gbuilder->yy2 = 100.0 * (gbuilder->y2 - gbparent->y1) / (gbparent->y2 - gbparent->y1)
     And,
	otk->xleft =   canvas->xleft +  0.01 * gbuilder->x1 * (canvas->xright - canvas->xleft);		// Match-check 3.
	otk->ytop =    canvas->ytop  +  0.01 * gbuilder->y1 * (canvas->ybottom - canvas->ytop);
	otk->xright =  canvas->left  +  0.01 * gbuilder->x2 * (canvas->xright - canvas->xleft);
	otk->ybottom = canvas->ytop  +  0.01 * gbuilder->y2 * (canvas->ybottom - canvas->ytop);
     Or,
	otk->xleft =   container->xleft +  0.01 * otk->x1 * (container->xright - container->xleft);	// Match-check 4.
	otk->ytop =    container->ytop +   0.01 * otk->y1 * (container->ybottom - container->ytop);
	otk->xright =  container->xright + 0.01 * otk->x2 * (container->xright - container->xleft);
	otk->ybottom = container->ytop +   0.01 * otk->y2 * (container->ybottom - container->ytop);

     otk->x1 = gbuilder->xx1 = 100.0 * (otk->xleft - container->xleft) / (container->xright - container->xleft);

     gbuilder->x1 = 100.0 * (otk->xleft - canvas->xleft) / (canvas->xright - canvas->xleft);

**************************************************************************/

/* Big-Diff - Return true only when two values differ substantially. */
int bigdiff( float a, float b ) { if (fabs(a-b) > 0.01) return 1; else return 0; }

/* Check Coords - Built-in Self-Test.  Double-checks coordinates for consistency. */
void check_coords( char *msg, struct GbuilderObject *gbobj )
{
 OtkWidget renderedobj, container;
 int err=0;

 renderedobj = gbobj->RenderedObject;
 if (bigdiff(renderedobj->x1, gbobj->xx1)) {printf("%s Mismatch1_x1 %g != %g\n", msg, renderedobj->x1, gbobj->xx1 ); err++;}
 if (bigdiff(renderedobj->y1, gbobj->yy1)) {printf("%s Mismatch1_y1 %g != %g\n", msg, renderedobj->y1, gbobj->y1 ); err++;}
 if (bigdiff(renderedobj->x2, gbobj->xx2)) {printf("%s Mismatch1_x2 %g != %g\n", msg, renderedobj->x2, gbobj->xx2 ); err++;}
 if (bigdiff(renderedobj->y2, gbobj->yy2)) {printf("%s Mismatch1_y2 %g != %g\n", msg, renderedobj->y2, gbobj->yy2 ); err++;}
 if (gbobj->parent == 0)
  {
   if (bigdiff(gbobj->x1, gbobj->xx1)) {printf("%s Mismatch2a_x1 %g != %g\n", msg, gbobj->x1, gbobj->xx1 ); err++;}
   if (bigdiff(gbobj->y1, gbobj->yy1)) {printf("%s Mismatch2a_y1 %g != %g\n", msg, gbobj->y1, gbobj->yy1 ); err++;}
   if (bigdiff(gbobj->x2, gbobj->xx2)) {printf("%s Mismatch2a_x2 %g != %g\n", msg, gbobj->x2, gbobj->xx2 ); err++;}
   if (bigdiff(gbobj->y2, gbobj->yy2)) {printf("%s Mismatch2a_y2 %g != %g\n", msg, gbobj->y2, gbobj->yy2 ); err++;}
  }
 else
  {
   if (bigdiff(gbobj->xx1, 100.0 * (gbobj->x1 - gbobj->parent->x1) / (gbobj->parent->x2 - gbobj->parent->x1))) {printf("%s Mismatch2b_x1 %g != %g\n", msg, gbobj->xx1, 100.0 * (gbobj->x1 - gbobj->parent->x1) / (gbobj->parent->x2 - gbobj->parent->x1) ); err++;}
   if (bigdiff(gbobj->yy1, 100.0 * (gbobj->y1 - gbobj->parent->y1) / (gbobj->parent->y2 - gbobj->parent->y1))) {printf("%s Mismatch2b_y1 %g != %g\n", msg, gbobj->yy1, 100.0 * (gbobj->y1 - gbobj->parent->y1) / (gbobj->parent->y2 - gbobj->parent->y1) ); err++;}
   if (bigdiff(gbobj->xx2, 100.0 * (gbobj->x2 - gbobj->parent->x1) / (gbobj->parent->x2 - gbobj->parent->x1))) {printf("%s Mismatch2b_x2 %g != %g\n", msg, gbobj->xx2, 100.0 * (gbobj->x2 - gbobj->parent->x1) / (gbobj->parent->x2 - gbobj->parent->x1) ); err++;}
   if (bigdiff(gbobj->yy2, 100.0 * (gbobj->y2 - gbobj->parent->y1) / (gbobj->parent->y2 - gbobj->parent->y1))) {printf("%s Mismatch2b_y2 %g != %g\n", msg, gbobj->yy2, 100.0 * (gbobj->y2 - gbobj->parent->y1) / (gbobj->parent->y2 - gbobj->parent->y1) ); err++;}
  }
 if (bigdiff(renderedobj->xleft , canvas->xleft +  0.01 * gbobj->x1 * (canvas->xright - canvas->xleft))) {printf("%s Mismatch3_xleft %g != %g  (Diff=%g)\n", msg, renderedobj->xleft,  canvas->xleft + 0.01 * gbobj->x1 * (canvas->xright - canvas->xleft),
		  renderedobj->xleft - (canvas->xleft + 0.01 * gbobj->x1 * (canvas->xright - canvas->xleft)) ); err++;}
 if (bigdiff(renderedobj->ytop  , canvas->ytop  +  0.01 * gbobj->y1 * (canvas->ybottom - canvas->ytop))) {printf("%s Mismatch3_ytop %g != %g\n", msg, renderedobj->ytop,  canvas->ytop  +  0.01 * gbobj->y1 * (canvas->ybottom - canvas->ytop) ); err++;}
 if (bigdiff(renderedobj->xright, canvas->xleft +  0.01 * gbobj->x2 * (canvas->xright - canvas->xleft))) {printf("%s Mismatch3_xright %g != %g\n", msg, renderedobj->xright,  canvas->xleft + 0.01 * gbobj->x2 * (canvas->xright - canvas->xleft) ); err++;}
 if (bigdiff(renderedobj->ybottom, canvas->ytop +  0.01 * gbobj->y2 * (canvas->ybottom - canvas->ytop))) {printf("%s Mismatch3_ybottom %g != %g\n", msg, renderedobj->ybottom,  canvas->ytop +  0.01 * gbobj->y2 * (canvas->ybottom - canvas->ytop) ); err++;}
 if (renderedobj->parent != 0)
  {
   container = renderedobj->parent;	/* gbobj->xx1 is same as renderedobj->x1, as checked above. */
   if (bigdiff(renderedobj->xleft , container->xleft +  0.01 * gbobj->xx1 * (container->xright - container->xleft))) {printf("%s Mismatch4_xleft %g != %g\n", msg, renderedobj->xleft,   container->xleft +  0.01 * gbobj->xx1 * (container->xright - container->xleft) ); err++;}
   if (bigdiff(renderedobj->ytop  , container->ytop  +  0.01 * gbobj->yy1 * (container->ybottom - container->ytop))) {printf("%s Mismatch4_ytop %g != %g\n", msg, renderedobj->ytop,   container->ytop +  0.01 * gbobj->yy1 * (container->ybottom - container->ytop) ); err++;}
   if (bigdiff(renderedobj->xright, container->xleft +  0.01 * gbobj->xx2 * (container->xright - container->xleft))) {printf("%s Mismatch4_xright %g != %g\n", msg, renderedobj->xright,   container->xleft +  0.01 * gbobj->xx2 * (container->xright - container->xleft) ); err++;}
   if (bigdiff(renderedobj->ybottom, container->ytop +  0.01 * gbobj->yy2 * (container->ybottom - container->ytop))) {printf("%s Mismatch4_ybottom %g != %g\n", msg, renderedobj->ybottom,   container->ytop +  0.01 * gbobj->yy2 * (container->ybottom - container->ytop) ); err++;}
  }
 if (err==0) printf("Ok: %s\n", msg);
}

void check_all_objects()
{
 struct GbuilderObject *tmpobj=ObjectsList;

 printf("\nChecking All Objects:\n");
 while (tmpobj!=0)
  {
   check_coords( " Check All:", tmpobj );
   tmpobj = tmpobj->nxt;
  }
}

void check_selected_object()
{
 printf("\nChecking Selected Object:\n");
 if (SelectedObjectsList!=0)
  check_coords( "SelectedObj", SelectedObjectsList->object );
}


struct tickmarklist *new_tickmarks()	/* Return a new set of tick-marks. */
{
 struct tickmarklist *tmppt;

 if (tickmark_freelist!=0) { tmppt = tickmark_freelist;  tickmark_freelist = tickmark_freelist->nxt; }
 else
  { int j;
    tmppt = (struct tickmarklist *)malloc(sizeof(struct tickmarklist));
    for (j=0; j<4; j++)
     { 
      tmppt->tickmarks[j] = OtkMakePanel( canvas, Otk_Flat, OtkSetColor(0.8,0.1,0.1), 50.0, 50.0, 1.0, 1.0 );
      tmppt->tickmarks[j]->z = 10000.0; 
     }
  }
 return tmppt;
}


void set_tick( OtkWidget tickmark, float x, float y )
{
   tickmark->xleft = canvas->xleft + x * (canvas->xright - canvas->xleft) * 0.01;
   tickmark->ytop = canvas->ytop + y * (canvas->ybottom - canvas->ytop) * 0.01;
   tickmark->xright = canvas->xleft + (x+1.0) * (canvas->xright - canvas->xleft) * 0.01;
   tickmark->ybottom = canvas->ytop + (y+1.0) * (canvas->ybottom - canvas->ytop) * 0.01;
   tickmark->z = 245.0; 
}


void set_ticks( struct tickmarklist *ticks, float x1, float y1, float x2, float y2 )
{
   set_tick( ticks->tickmarks[0], x1, y1 );
   set_tick( ticks->tickmarks[1], x2, y1 );
   set_tick( ticks->tickmarks[2], x2, y2 );
   set_tick( ticks->tickmarks[3], x1, y2 );
}

void set_ticks1( float x1, float y1, float x2, float y2 )
{
   set_ticks( SelectedObjectsList->tickmarks, x1, y1, x2, y2 );
}


void removeticks(struct tickmarklist *ticks)
{ int j;
  for (j=0; j<4; j++) { ticks->tickmarks[j]->z = -10000.0; };
  ticks->nxt = tickmark_freelist;
  tickmark_freelist = ticks;
}


void add_selected_item( struct GbuilderObject *object )    /* Adds object to the selection-list. */
{
 struct selobjectitem *tmppt;
 OtkWidget renderedobj;

 renderedobj = object->RenderedObject;
 if (renderedobj!=0)
  {
   tmppt = (struct selobjectitem *)malloc(sizeof(struct selobjectitem));
   tmppt->object = object;
   tmppt->tickmarks = new_tickmarks();
   tmppt->nxt = SelectedObjectsList;
   SelectedObjectsList = tmppt;
   set_ticks( tmppt->tickmarks, object->x1, object->y1, object->x2, object->y2 );
   tmppt->origobjx = renderedobj->x1;   tmppt->origobjy = renderedobj->y1;
   tmppt->origobjx2 = renderedobj->x2;  tmppt->origobjy2 = renderedobj->y2;
   tmppt->alphax = (canvas->xright - canvas->xleft) / (renderedobj->parent->xright - renderedobj->parent->xleft);
   tmppt->alphay = (canvas->ybottom - canvas->ytop) / (renderedobj->parent->ybottom - renderedobj->parent->ytop);
  }
}


void discard_obj_list()		/* Frees list of selected objects. */
{
 struct selobjectitem *tmppt;
 while (SelectedObjectsList!=0)
  {
   tmppt = SelectedObjectsList;
   removeticks(tmppt->tickmarks);
   SelectedObjectsList = SelectedObjectsList->nxt;
   free( tmppt );
  }
}


void update_selected_object_positions()
{
 struct selobjectitem *tmppt=SelectedObjectsList;
 OtkWidget renderedobj;

 while (tmppt!=0)
  {
   renderedobj = tmppt->object->RenderedObject;
   set_ticks( tmppt->tickmarks, tmppt->object->x1, tmppt->object->y1, tmppt->object->x2, tmppt->object->y2 );
   tmppt->origobjx = renderedobj->x1;   tmppt->origobjy = renderedobj->y1;
   tmppt->origobjx2 = renderedobj->x2;  tmppt->origobjy2 = renderedobj->y2;
   tmppt->alphax = (canvas->xright - canvas->xleft) / (renderedobj->parent->xright - renderedobj->parent->xleft);
   tmppt->alphay = (canvas->ybottom - canvas->ytop) / (renderedobj->parent->ybottom - renderedobj->parent->ytop);
   tmppt = tmppt->nxt;
  }
}


void convert_mouse2coords( float mousex, float mousey, float *x, float *y );


void determine_selected_enclosed_objects( float x1, float y1, float x2, float y2 )	/* Determine objects within rubberband box. */
{  /* x1, y1, x2, y2 are percent coords of the drawing canvas (0 - 100%). (Not the whole window.) */
 struct GbuilderObject *tmpobj=ObjectsList, *closestobject=0;
 float x, y, dx, dy, grav = 0.5;

 discard_obj_list();
 while (tmpobj!=0)	/* Search through all objects. */
  {
    if ((x1 <= tmpobj->x1) && (x2 >= tmpobj->x2) && (y1 <= tmpobj->y1) && (y2 >= tmpobj->y2) )
     if ((tmpobj->parent==0) || (x1 > tmpobj->parent->x1) || (x2 < tmpobj->parent->x2) || (y1 > tmpobj->parent->y1) || (y2 < tmpobj->parent->y2))
     {
      if (verbose) printf(" SelectedObj (%g, %g) - (%g, %g)\n", tmpobj->x1, tmpobj->y1, tmpobj->x2, tmpobj->y2 );
      add_selected_item( tmpobj );
     }
   tmpobj = tmpobj->nxt;
  }

 if (SelectedObjectsList==0) printf("No enclosed objects.\n");
 ontick = 0;
}


void determine_selected_object()	/* Determine closest and highest object near mouse click. */
{
 struct GbuilderObject *tmpobj=ObjectsList, *closestobject=0;
 struct selobjectitem *selobj=SelectedObjectsList, *prv=0, *bestobj=0, *bestprv;
 float x, y, dx, dy, grav = 1.5, smallest=9e9, area;
 int closestlayer=0;

 convert_mouse2coords( Otk_MouseX, Otk_MouseY, &x, &y );
 ontick = 0;
 if ((SelectedObjectsList!=0) && (SelectedObjectsList->nxt!=0))
  {
   grpminx = 9e9;  grpmaxx = -9e9;  grpminy = 9e9;  grpmaxy = -9e9;
   while (selobj!=0)	 /* Check if click is within a presently selected object. */
    {
     if ((x+grav >= selobj->object->x1) && (x-grav <= selobj->object->x2) && (y+grav >= selobj->object->y1) && (y-grav <= selobj->object->y2))
      { /* Clicked within presently selected region, so keep selection-list. */
        origmousex = x;  origmousey = y;
	area = (selobj->object->x2 - selobj->object->x1) * (selobj->object->y2 - selobj->object->y1);
	if (area < smallest)
	 {
	  smallest = area;
	  bestobj = selobj;
	  bestprv = prv;
	 }
      }
     /* Check if on a group vertex for a group-stretch. */
     if (selobj->object->x1 < grpminx) grpminx = selobj->object->x1;
     if (selobj->object->y1 < grpminy) grpminy = selobj->object->y1;
     if (selobj->object->x2 > grpmaxx) grpmaxx = selobj->object->x2;
     if (selobj->object->y2 > grpmaxy) grpmaxy = selobj->object->y2;
     prv = selobj;
     selobj = selobj->nxt;
    }
   if (bestobj!=0)
    {
       if (Otk_Get_Shift_Key()) 
	{ /* Remove object from selection-list. */
	  if (bestprv==0) SelectedObjectsList = SelectedObjectsList->nxt; else bestprv->nxt = bestobj->nxt;
	  removeticks(bestobj->tickmarks);
	  free(bestobj);
	  return;
	}
       else
	{ /* Check if on a group vertex for a group-stretch. */
	  dx = 0.05 * (grpmaxx - grpminx) + 0.5;
	  dy = 0.05 * (grpmaxy - grpminy) + 0.5;
	  if ((fabs(x - grpminx) < dx) &&  (fabs(y - grpminy) < dy)) ontick = 11; else
	  if ((fabs(x - grpmaxx) < dx) &&  (fabs(y - grpminy) < dy)) ontick = 12; else
	  if ((fabs(x - grpminx) < dx) &&  (fabs(y - grpmaxy) < dy)) ontick = 13; else
	  if ((fabs(x - grpmaxx) < dx) &&  (fabs(y - grpmaxy) < dy)) ontick = 14; else ontick = 0;
	}
       return;
    }
  }
 if (Otk_Get_Shift_Key()==0) discard_obj_list();
 while (tmpobj!=0)	/* Search through all objects. */
  {
    area = (tmpobj->x2 - tmpobj->x1) * (tmpobj->y2 - tmpobj->y1);
    if ((x+grav >= tmpobj->x1) && (x-grav <= tmpobj->x2) && (y+grav >= tmpobj->y1) && (y-grav <= tmpobj->y2) && (closestlayer <= tmpobj->layer)
	&& (area < smallest))
     {
      smallest = area;
      closestlayer = tmpobj->layer;
      closestobject = tmpobj;
     }
   tmpobj = tmpobj->nxt;
  }

 if (closestobject==0) printf("No close objects.\n");
 else
  { OtkWidget renderedobj;
   printf("Closest object = %s   selectedobj_x=%g, mousex=%g\n", closestobject->name, closestobject->x1, x);
   add_selected_item( closestobject );
   origmousex = x;
   origmousey = y;

   /* Determine if tick-mark was selected, such as for stretching. */
   dx = 0.1 * (closestobject->x2 - closestobject->x1) + 0.5;
   dy = 0.1 * (closestobject->y2 - closestobject->y1) + 0.5;
   if ((fabs(x - closestobject->x1) < dx) && (fabs(y - closestobject->y1) < dy)) ontick = 1;  else
   if ((fabs(x - closestobject->x2) < dx) && (fabs(y - closestobject->y1) < dy)) ontick = 2;  else
   if ((fabs(x - closestobject->x1) < dx) && (fabs(y - closestobject->y2) < dy)) ontick = 3;  else
   if ((fabs(x - closestobject->x2) < dx) && (fabs(y - closestobject->y2) < dy)) ontick = 4;  else  ontick = 0;

   if (verbose) printf("SelectedObj (%g, %g) - (%g, %g)   ontick=%d\n", closestobject->x1, closestobject->y1, closestobject->x2, closestobject->y2, ontick );
  }
}



void set_container( struct GbuilderObject *obj )	/* Determine an object's containing panel (parent) from its absolute position. */
{
 struct GbuilderObject *tmpobj=ObjectsList, *closestcontainer=0, *prv=0;
 int closestlayer = 0;

 while (tmpobj!=0)	/* Search through all objects for panels. */
  {			/* (Only panels can be containers.) */
   if (((tmpobj->kind == PanelKind) || ((obj->kind==PullDownMenuKind) && (tmpobj->kind==PullDownMenuKind)))
	&& (tmpobj->layer > closestlayer) && (tmpobj!=obj))
    { /* See if object fits on (within) this panel. */
      if ((obj->x1 >= tmpobj->x1) && (obj->x1 <= tmpobj->x2) && (obj->y1 >= tmpobj->y1) && (obj->y1 <= tmpobj->y2))
       {
	closestlayer = tmpobj->layer;
	closestcontainer = tmpobj;
       }
    }
   tmpobj = tmpobj->nxt;
  }

 obj->layer = closestlayer + 1;
 obj->parent = closestcontainer;
 if (closestcontainer==0)	/* Not contained on any panel. */
  {
   ContainerWidget = canvas;
   obj->xx1 = obj->x1;
   obj->yy1 = obj->y1;
   obj->xx2 = obj->x2;
   obj->yy2 = obj->y2;
  }
 else
  {	/* Contained within a panel. */
    ContainerWidget = closestcontainer->RenderedObject;
    /* Set realtive coordinates within the panel.  (Based on absolute OuterWindowBased position.) */
    obj->xx1 = 100.0 * (obj->x1 - closestcontainer->x1) / (closestcontainer->x2 - closestcontainer->x1);
    obj->xx2 = 100.0 * (obj->x2 - closestcontainer->x1) / (closestcontainer->x2 - closestcontainer->x1);
    obj->yy1 = 100.0 * (obj->y1 - closestcontainer->y1) / (closestcontainer->y2 - closestcontainer->y1);
    obj->yy2 = 100.0 * (obj->y2 - closestcontainer->y1) / (closestcontainer->y2 - closestcontainer->y1);
  }
}



/*----------------------------------------------*/
/* Add_Object - 				*/
/*----------------------------------------------*/
struct GbuilderObject *Add_object( int kind, float x1, float y1, float x2, float y2, float *xx1, float *yy1, float *xx2, float *yy2 )
{
 struct GbuilderObject *newobj;
 char newobjname[100];

 newobj = (struct GbuilderObject *)calloc(1,sizeof(struct GbuilderObject));
 newobj->kind = kind;
 newobj->Id = objidcnt;
 sprintf(newobjname,"obj%d", objidcnt++);
 newobj->name = strdup(newobjname);
 newobj->x1 = x1;
 newobj->y1 = y1;
 newobj->x2 = x2;
 newobj->y2 = y2;

 if (ObjectsList==0) ObjectsList = newobj;
 else LastAddedObj->nxt = newobj;
 LastAddedObj = newobj;
 newobj->nxt = 0;

 set_container( newobj );
 *xx1 = newobj->xx1;
 *yy1 = newobj->yy1;
 *xx2 = newobj->xx2;
 *yy2 = newobj->yy2;
 if (newobj->parent==0) newobj->nxtsibling = 0;
 else 
  {
   newobj->nxtsibling = newobj->parent->child;
   newobj->parent->child = newobj;
  }
 newobj->child = 0;
printf("NewObject Added at (%g,%g) Rel (%g,%g)\n", x1, y1, *xx1, *yy1 );
 add_selected_item( newobj );
 return newobj;
}



void Set_Focus( OtkWidget frmbx )
{
 Otk_keyboard_focus = frmbx->children; 
 Otk_Keyboard_state.textform_insertion_column = 0;
}


void Set_object( OtkWidget newobj )
{
 LastAddedObj->RenderedObject = newobj;
 LastAddedObj->RenderedObject->mouse_sensitive = 0;
}



void dismisspopup(void *x)
{
 OtkWidget pw=(OtkWidget)x;
 Otk_RemoveObject( pw );
 popupopen = 0;
}

void killedpopup(void *x)
{ 
 popupopen = 0;
}

void registerpopup()
{
 popupopen = 1;  
 Otk_RegisterWindowKillEventFunction( popupwindow, killedpopup, popupwindow );
}



void radiocallback( void *x )
{
 char *sel=(char *)x;

 sscanf(sel,"%d",&tool_mode);
 printf("Radio button %d selected.\n", tool_mode );
}

void radiobuttoncallback( void *x )
{
 char *sel=(char *)x;

 sscanf(sel,"%d",&tool_mode);
 printf("Radio button %d selected.\n", tool_mode );
 Otk_SetRadioButton( SelectedToggle[tool_mode] );
 discard_obj_list();
}


void enforce_minsize( float x1, float *x2, float minsz )
{ if (*x2 - x1 < minsz) *x2 = x1 + minsz; }

void enforce_minsize_xy( float x1, float *x2, float minxsz , float y1, float *y2, float minysz )
{
 enforce_minsize( x1, x2, minxsz );
 enforce_minsize( y1, y2, minysz );
}



void resize_button( struct GbuilderObject *buttonobj )
{ 
 float w, h, horiz_size, vert_size;

  buttonobj->RenderedObject->children->scale = 1.0;
  buttonobj->RenderedObject->children->sqrtaspect = 1.0;
  Otk_Get_Text_Size( buttonobj->RenderedObject->children, &w, &h );
  horiz_size = buttonobj->RenderedObject->xright - buttonobj->RenderedObject->xleft;
  vert_size = buttonobj->RenderedObject->ybottom - buttonobj->RenderedObject->ytop;
  buttonobj->RenderedObject->children->sqrtaspect = sqrt( (h * horiz_size) / (w * 0.8 * vert_size) );
  buttonobj->RenderedObject->children->scale = 0.925 * horiz_size / (w * buttonobj->RenderedObject->children->sqrtaspect);
  Otk_center_text( buttonobj->RenderedObject->children );           /* FONTS insertion */
}


void adjust_guagepointer( OtkWidget container )
{
 float phi, r1=0.04, r2=0.37, w=0.9;
 OtkWidget tmppt, child=0;

 tmppt = container->children;
 while (tmppt!=0) 
  { if ((tmppt->children!=0) && (tmppt->children->superclass==Otk_SC_TextLabel)) Otk_FitTextInPanel(tmppt->children);
    if (tmppt->object_class == Otk_class_triangle) child = tmppt;
    tmppt = tmppt->nxt;
  }
 if (child==0) return;
 phi = 2.0 * 3.14159 * 0.01 * 33.4;
 child->x1 = container->xleft + (0.5 + r2*cos(phi)) * (container->xright - container->xleft);
 child->y1 = container->ytop + (0.5 + r2*sin(phi)) * (container->ybottom - container->ytop);
 child->x2 = container->xleft + (0.5 + r1*cos(phi-w)) * (container->xright - container->xleft);
 child->y2 = container->ytop + (0.5 + r1*sin(phi-w)) * (container->ybottom - container->ytop);
 child->xleft = container->xleft + (0.5 + r1*cos(phi+w)) * (container->xright - container->xleft);
 child->ytop = container->ytop + (0.5 + r1*sin(phi+w)) * (container->ybottom - container->ytop);

}




struct Etta	/* Hold original mouse coordinates before being lost. */
 {
   int tool_mode;
   char txt[1000];
   float x1, y1, x2, y2;
 } etta;


void accept_text1( char *s, void *x )
{
 float xx1, yy1, xx2, yy2, dx;	/* Native coords translated to gbuilder's absolute coords. */
 float fontsz;

 Add_object( etta.tool_mode, etta.x1, etta.y1, etta.x2, etta.y2, &xx1, &yy1, &xx2, &yy2 );
 dx = etta.x2 - etta.x1;
 fontsz = dx / ((float)strlen(s)+1.0);
 if (fontsz < 2.0)
   fontsz = (int)(5.0 * fontsz + 0.5) * 0.2;	/* Stratify the estimated font size to yield more uniform fonts. */
 else
 if (fontsz < 4.0)
   fontsz = (int)(2.0 * fontsz + 0.5) * 0.5;
 else
   fontsz = (int)(fontsz + 0.5);
 if (fontsz==0.0) fontsz = 1.0;
 if (fontsz<0.2) fontsz = 0.2;
 printf("fontsz=%g\n", fontsz);
 Set_object( OtkMakeTextLabel( ContainerWidget, s, Otk_Black, fontsz, 1.0 , xx1, yy1 ) );
 LastAddedObj->text = strdup(s);
 LastAddedObj->fontsz = fontsz;
 LastAddedObj->RenderedObject->x2 = etta.x2;
 LastAddedObj->RenderedObject->y2 = etta.y2;
// LastAddedObj->color =
 dismisspopup(popupwindow);
}

void accept_text( void *x )
{
 Otk_Get_Text( textform, etta.txt, 1000 );
 accept_text1( etta.txt, 0 );
}

void Enter_Text_To_Add( int tool_mode, float x1, float y1, float x2, float y2 )
{
 etta.tool_mode = tool_mode;
 etta.x1 = x1;
 etta.y1 = y1;
 etta.x2 = x2;
 etta.y2 = y2;
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 75, 30 );
 OtkMakeTextLabel( popupwindow, "Enter Text:", Otk_Black, 2.0, 1.5, 3, 8 );
 textform = OtkMakeTextFormBox( popupwindow, "", 30, 6, 30, 85, 25, accept_text1, popupwindow );
 Set_Focus( textform );
 OtkMakeButton( popupwindow, 15, 70, 20, 18, " Ok ", accept_text, popupwindow );  
 OtkMakeButton( popupwindow, 70, 70, 20, 18, "Cancel", dismisspopup, popupwindow );  
 registerpopup();
}



void accept_buttonlabel1( char *s, void *x )
{
 float xx1, yy1, xx2, yy2;      /* Native coords translated to gbuilder's absolute coords. */

 enforce_minsize_xy( etta.x1, &(etta.x2), 3.0, etta.y1, &(etta.y2), 2.0 );
 Add_object( etta.tool_mode, etta.x1, etta.y1, etta.x2, etta.y2, &xx1, &yy1, &xx2, &yy2 );
 Set_object( OtkMakeButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, s, 0, 0 ) );
 LastAddedObj->text = strdup(s);
 dismisspopup(popupwindow);
}

void accept_buttonlabel( void *x )
{
 Otk_Get_Text( textform, etta.txt, 1000 );
 accept_buttonlabel1( etta.txt, 0 );
}

void Enter_Button_Label( int tool_mode, float x1, float y1, float x2, float y2 )
{
 etta.tool_mode = tool_mode;
 etta.x1 = x1;
 etta.y1 = y1;
 etta.x2 = x2;
 etta.y2 = y2;
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 75, 30 );
 OtkMakeTextLabel( popupwindow, "Enter Button Label:", Otk_Black, 2.0, 1.5, 3, 8 );
 textform = OtkMakeTextFormBox( popupwindow, "", 30, 6, 30, 85, 25, accept_buttonlabel1, popupwindow );
 Set_Focus( textform );
 OtkMakeButton( popupwindow, 15, 70, 20, 18, " Ok ", accept_buttonlabel, popupwindow );  
 OtkMakeButton( popupwindow, 70, 70, 20, 18, "Cancel", dismisspopup, popupwindow );  
 registerpopup();
}




void accept_menulabel1( char *s, void *x )
{
 float xx1, yy1, xx2, yy2;      /* Native coords translated to gbuilder's absolute coords. */

 enforce_minsize_xy( etta.x1, &(etta.x2), 4.0, etta.y1, &(etta.y2), 2.0 );
 Add_object( etta.tool_mode, etta.x1, etta.y1, etta.x2, etta.y2, &xx1, &yy1, &xx2, &yy2 );
 if ((LastAddedObj->parent==0) || (LastAddedObj->parent->kind!=PullDownMenuKind))
  Set_object( Otk_Make_Menu( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, s ) );
 else
  { OtkWidget obj;
   obj = ContainerWidget;
   while ((obj!=0) && (obj->superclass!=Otk_SC_Menu_DropDown_Button)) { LastAddedObj->parent = LastAddedObj->parent->parent;  obj = obj->parent; }
   if (obj==0) {printf("Unexpected error. No Dropdown parent button.\n"); return;}
printf("Adding to a %d\n\n", obj->superclass);
   Set_object( Otk_Add_Menu_Item( obj, s, 0, 0 ) );
  }
 LastAddedObj->text = strdup(s);
 dismisspopup(popupwindow);
}

void accept_menulabel( void *x )
{
 Otk_Get_Text( textform, etta.txt, 1000 );
 accept_menulabel1( etta.txt, 0 );
}

void Enter_Menu_Label( int tool_mode, float x1, float y1, float x2, float y2 )
{
 etta.tool_mode = tool_mode;
 etta.x1 = x1;
 etta.y1 = y1;
 etta.x2 = x2;
 etta.y2 = y2;
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 75, 30 );
 OtkMakeTextLabel( popupwindow, "Enter Menu Label:", Otk_Black, 2.0, 1.5, 3, 8 );
 textform = OtkMakeTextFormBox( popupwindow, "", 30, 6, 30, 85, 25, accept_menulabel1, popupwindow );
 Set_Focus( textform );
 OtkMakeButton( popupwindow, 15, 70, 20, 18, " Ok ", accept_menulabel, popupwindow );  
 OtkMakeButton( popupwindow, 70, 70, 20, 18, "Cancel", dismisspopup, popupwindow );  
 registerpopup();
}





void AddGadgetType( void *x )
{
 float xx1, yy1, xx2, yy2, r;      /* Native coords translated to gbuilder's absolute coords. */
 char orientation;

 sscanf((char *)x,"%d",&gadgettype);

 enforce_minsize_xy( etta.x1, &(etta.x2), 5.0, etta.y1, &(etta.y2), 5.0 );
 Add_object( gadgettype, etta.x1, etta.y1, etta.x2, etta.y2, &xx1, &yy1, &xx2, &yy2 );

 switch( gadgettype )
  {
   case GadgetGauge2:
	Set_object( Otk_MakeGauge2( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, "Gauge " ) );
	break;
   case GadgetLedMeter:
	if (etta.x2 - etta.x1 > etta.y2 - etta.y1) orientation = 'h';  else  orientation = 'v';  
	Set_object( Otk_MakeLEDmeter( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, 10, orientation, Otk_Red ) );
	break;
   case GadgetBarMeter:
	if (etta.x2 - etta.x1 > etta.y2 - etta.y1) orientation = 'h';  else  orientation = 'v'; 
	Set_object( Otk_MakeBarMeter( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, orientation, Otk_Green ) );
	Otk_SetBarMeter( LastAddedObj->RenderedObject, 50.0 );
	break;
   case GadgetPlot:
   case GadgetStripChart:
	Set_object( Otk_Plot_Init( ContainerWidget, "X-axis", "Y-axis", xx1, yy1, xx2 - xx1, yy2 - yy1, Otk_Green, Otk_DarkGray ) );
	break;
   case GadgetIndLight:
	r = 0.5 * (xx2 - xx1);
//	Set_object( Otk_AddIndicatorLight( ContainerWidget, xx1+r, yy1+r, r, Otk_Green ) );
	break;
  }

 dismisspopup(popupwindow);
}

void dismissaddgadget( void *x )
{
 tool_mode = SelectMove;
 Otk_SetRadioButton( SelectedToggle[tool_mode] );
 dismisspopup(x); 
}

void BringUpGadgetsWindow( int tool_mode, float x1, float y1, float x2, float y2 )
{
 float xa=20, y=20, dy=10.5, bwdth=60, bht=9.0;

 etta.tool_mode = tool_mode;
 etta.x1 = x1;
 etta.y1 = y1;
 etta.x2 = x2;
 etta.y2 = y2;
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 60, 60 );
 OtkMakeTextLabel( popupwindow, "Add Gadget", Otk_Black, 2.0, 1.5, 3, 3 );
 OtkMakeTextLabel( popupwindow, "Select Type of Gadget to Add:", Otk_Black, 1.6, 1.4, 5, 13 );
 // OtkMakeTextLabel( popupwindow, "(Then click or drag-box to place.)", Otk_Black, 1.4, 1.1, 15, 20 );

 OtkMakeButton( popupwindow, xa, y, bwdth, bht, "LED Meter ", AddGadgetType, "102" );
 y = y + dy;
 OtkMakeButton( popupwindow, xa, y, bwdth, bht, "Bar Meter", AddGadgetType, "103" );
 y = y + dy;
 OtkMakeButton( popupwindow, xa, y, bwdth, bht, "Analog Gauge ", AddGadgetType, "101" );
 y = y + dy;
 OtkMakeButton( popupwindow, xa, y, bwdth, bht, "XY Graph ", AddGadgetType, "104" );
 y = y + dy;
 OtkMakeButton( popupwindow, xa, y, bwdth, bht, "Strip Chart", AddGadgetType, "105" );
 y = y + dy;
// OtkMakeButton( popupwindow, xa, y, bwdth, bht, "Indicator Light", AddGadgetType, "106" );
 y = y + dy;

 OtkMakeButton( popupwindow, 40, 85, 20, 10, "Cancel", dismissaddgadget, popupwindow );  
 registerpopup();
}



void AddSomething( float x1, float y1, float x2, float y2 )
{  /* x1, y1, x2, y2 are percent coords of the drawing canvas (0 - 100%). (Not the whole window.) */
 float xx1, yy1, xx2, yy2;	/* Native coords translated to gbuilder's absolute whole-window coords. */
 OtkWidget newobj;

 if (snap2grid)
  {
   x1 = (int)(x1/gridsize+0.5) * gridsize;
   y1 = (int)(y1/gridsize+0.5) * gridsize;
   x2 = (int)(x2/gridsize+0.5) * gridsize;
   y2 = (int)(y2/gridsize+0.5) * gridsize;
  }
 // printf("Adding a %d:\n", tool_mode);
 switch (tool_mode)
  {
   case SelectMove:  
printf("mousedragged=%d\n", mousedragged);
          if (mousedragged)
		determine_selected_enclosed_objects( x1, y1, x2, y2 );
	 return;
	break;
   case PanelKind:
	enforce_minsize_xy( x1, &x2, 2.0, y1, &y2, 2.0 );
	Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	enforce_minsize_xy( xx1, &xx2, 2.0, yy1, &yy2, 2.0 );
	printf("Making Panel at: (%g, %g) of size (%g, %g)\n", xx1, yy1, xx2-xx1, yy2-yy1 );
	Set_object( OtkMakePanel( ContainerWidget, Otk_Raised, Otk_LightGray, xx1, yy1, xx2-xx1, yy2-yy1 ) );
	break;
   case TextLabelKind:
	enforce_minsize_xy( x1, &x2, 2.0, y1, &y2, 2.0 );
	Enter_Text_To_Add( tool_mode, x1, y1, x2, y2 );
	break;
   case FormBoxKind:
	enforce_minsize_xy( x1, &x2, 4.0, y1, &y2, 2.0 );
	Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeTextFormBox( ContainerWidget, "", (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;
   case ButtonKind:
	Enter_Button_Label( tool_mode, x1, y1, x2, y2 );
	break;
   case ToggleKind:
	enforce_minsize_xy( x1, &x2, 3.0, y1, &y2, 2.0 );
	Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeToggleButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;
   case RadioButtonKind:
	enforce_minsize_xy( x1, &x2, 3.0, y1, &y2, 2.0 );
	Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( OtkMakeRadioButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;
   case PullDownMenuKind:
	Enter_Menu_Label( tool_mode, x1, y1, x2, y2 );
	break;
   case SelectionListKind:
	enforce_minsize_xy( x1, &x2, 6.0, y1, &y2, 6.0 );
	Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	Set_object( Otk_Make_Selection_List( ContainerWidget, (int)(0.5*(yy2-yy1)+2.0), (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1 ) );
	break;
   case SliderKind:
	enforce_minsize_xy( x1, &x2, 6.0, y1, &y2, 6.0 );
	if (x2-x1 >= y2 - y1)
	 {
	  y2 = y1 + 3.0;
	  Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	  Set_object( OtkMakeSliderHorizontal( ContainerWidget, xx1, yy1, xx2-xx1, 0, 0 ) );
	  LastAddedObj->yy2 = LastAddedObj->RenderedObject->y2;
	  LastAddedObj->y2 = 100.0 * (LastAddedObj->RenderedObject->ybottom - canvas->ytop) / CanvasHeight;
	 }
	else
	 {
	  x2 = x1 + 3.0;
	  Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	  Set_object( OtkMakeSliderVertical( ContainerWidget, xx1, yy1, yy2-yy1, 0, 0 ) );
	  LastAddedObj->xx2 = LastAddedObj->RenderedObject->x2;
	  LastAddedObj->x2 = 100.0 * (LastAddedObj->RenderedObject->xright - canvas->xleft) / CanvasWidth;
	 }
	break;
   case SeparatorKind:
	if (fabs(x1-x2)<1.5) 
	 { /*Vert.-line*/
	  x1 = 0.5 * (x1+x2);
	  x2 = x1;
	  Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	  Set_object( Otk_Add_Line( ContainerWidget, Otk_Black, 1.0, xx1, yy1, xx2, yy2 ) );
	 }
	else
	if (fabs(y1-y2)<1.5)
	 { /*Horiz.-line*/
	  y1 = 0.5 * (y1+y2);
	  y2 = y1;
	  Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	  Set_object( Otk_Add_Line( ContainerWidget, Otk_Black, 1.0, xx1, yy1, xx2, yy2 ) );
	 }
	else
	 { /*Rect.-box*/
	  Add_object( tool_mode, x1, y1, x2, y2, &xx1, &yy1, &xx2, &yy2 );
	  printf("Adding BoundingBox at (%g,%g) to (%g,%g)    Absolute: (%g, %g) to (%g, %g)\n", x1, y1, x2, y2, xx1, yy1, xx2, yy2 );
	  Set_object( Otk_Add_BoundingBox( ContainerWidget, Otk_Black, 1.0, xx1, yy1, xx2, yy2 ) );
	  newobj = 0;
	 }
	break;
    case GadgetKind:
         BringUpGadgetsWindow( tool_mode, x1, y1, x2, y2 );
	break;
  }

 tool_mode = SelectMove;
 Otk_SetRadioButton( SelectedToggle[tool_mode] );
}





OtkWidget answrform, answrform2, answrform3, slider1, slider2, slider3, slider4, afrm[5], radbutton[4];
float sliderv[5];

void get_slider( float v, void *x )
{
 int j, k;
 char s[10];

 sscanf((char *)x,"%d",&k);
 if (k<3)
  { /*Color*/
   if (verbose) printf("Setting color = %g %g %g\n",  0.01 * sliderv[0], 0.01 * sliderv[1], 0.01 * sliderv[2] );
   Otk_Set_Button_Color( LastSelected->RenderedObject, OtkSetColor( 0.01 * sliderv[0], 0.01 * sliderv[1], 0.01 * sliderv[2] ) );
   for (j=0; j<3; j++) LastSelected->color[j] = 0.01 * sliderv[j];
   sliderv[k] = 100.0 * v;
   sprintf(s,"%3d",(int)sliderv[k]);
   Otk_Modify_Text( afrm[k], s );
  }
 else
 if (k==3)
  { /*Thickness*/
    sliderv[k] = (int)(5.0 * (9.1 * v + 1.0))/5.0;
    sprintf(s,"%1.1f",sliderv[k]);
    Otk_Modify_Text( afrm[k], s );
    Otk_Set_Line_Thickness( LastSelected->RenderedObject, sliderv[k] );
    LastSelected->thickness = sliderv[k];
  }
 else
 if (k==4)
  { /*Thickness*/
    if (v>=0.5) sliderv[k] = 0.2 * (int)(25.0 * (v - 0.3));
    else        sliderv[k] = 0.2 * (int)(5.0 * (v + 0.5));
    sprintf(s,"%1.1f",sliderv[k]);
    Otk_Modify_Text( afrm[k], s );
    Otk_Set_Line_Thickness( LastSelected->RenderedObject, sliderv[k] );
    LastSelected->thickness = sliderv[k];
  }
 else
 if ((k>=30) && (k<=32))
  { /*Color*/
   for (j=0; j<3; j++) LastSelected->color[j] = 0.01 * sliderv[j];
   sliderv[k-30] = 100.0 * v;
   Otk_SetLEDmeter( LastSelected->RenderedObject, sliderv[4], Otk_LightGray, OtkSetColor( 0.01 * sliderv[0], 0.01 * sliderv[1], 0.01 * sliderv[2] ) );
   sprintf(s,"%3d",(int)sliderv[k-30]);
   Otk_Modify_Text( afrm[k-30], s );
  }
 else
 if (k==34)
  {
   sliderv[4] = 100.0 * v;
   Otk_SetLEDmeter( LastSelected->RenderedObject, sliderv[4], Otk_LightGray, OtkSetColor( 0.01 * sliderv[0], 0.01 * sliderv[1], 0.01 * sliderv[2] ) );
   sprintf(s,"%3d",(int)sliderv[4]);
   Otk_Modify_Text( afrm[4], s );
  }
 else
 if ((k>=40) && (k<=42))
  { /*Color*/
   for (j=0; j<3; j++) LastSelected->color[j] = 0.01 * sliderv[j];
   sliderv[k-40] = 100.0 * v;
   for (j=0; j<3; j++) LastSelected->RenderedObject->children->color[j] = 0.01 * sliderv[j];
   sprintf(s,"%3d",(int)sliderv[k-40]);
   Otk_Modify_Text( afrm[k-40], s );
  }
 else
 if (k==44)
  {
   sliderv[4] = 100.0 * v;
   Otk_SetBarMeter( LastSelected->RenderedObject, sliderv[4] );
   sprintf(s,"%3d",(int)sliderv[4]);
   Otk_Modify_Text( afrm[4], s );
  }
 else printf("Unexpected slider\n");
}


float origfontsz;

void get_fontsz_slider( float v, void *origsz )
{
 float fontsz;
 char s[30];

// fontsz = (0.3 + 2.7 * v) * origfontsz;
 fontsz = 3.08 * (v*v + 0.075) * origfontsz;
 if (fontsz < 2.0)
   fontsz = (int)(5.0 * fontsz + 0.5) * 0.2;	/* Stratify the estimated font size to yield more uniform fonts. */
 else
 if (fontsz < 4.0)
   fontsz = (int)(2.0 * fontsz + 0.5) * 0.5;
 else
   fontsz = (int)(fontsz + 0.5);
 if (fontsz<0.1) fontsz = 0.1;

 sprintf(s,"%g",fontsz);
 s[5] = '\0';
 Otk_Modify_Text( answrform2, s );
 Otk_Modify_Text_Scale(  LastSelected->RenderedObject, fontsz );
 LastSelected->fontsz = fontsz;
}



char wildcards2[100]=".ppm", filename2[1500]="", directory2[500]=".";

void pickimage( char *fname )
{
 if (LastSelected==0) return;
 LastSelected->RenderedObject->object_subtype = 20;
 LastSelected->RenderedObject->image = Otk_Read_Image_File( fname );
}


void setpaneltype( void *x )
{
 int j;

 sscanf((char *)x,"%d",&j);
 printf("Setting panel type to %d = %s\n", j, (char *)x );
 if (j==20)
  { /* Pickimage*/
   Otk_Browse_Files( "Image File:", 1500, directory2, wildcards2, filename2, pickimage );   
   return;
  }
 LastSelected->RenderedObject->object_subtype = j;
}



void  accept_properties( void *x )
{
 char s[2000];
 float scale;

 switch (LastSelected->kind)
  {
   case PanelKind:
	Otk_Set_Button_Color( LastSelected->RenderedObject, OtkSetColor( 0.01 * sliderv[0], 0.01 * sliderv[1], 0.01 * sliderv[2] ) );
	break;
   case TextLabelKind:
	Otk_Get_Text( answrform2, s, 200 );
        printf("Setting font size to be '%s'\n", s );
	sscanf( s, "%f", &(LastSelected->fontsz) );
	Otk_Modify_Text_Scale(  LastSelected->RenderedObject, LastSelected->fontsz );
	Otk_Get_Text( answrform, s, 200 );
	printf("Setting text to be '%s'\n", s );
	LastSelected->text = strdup(s);
	Otk_Modify_Text( LastSelected->RenderedObject, s );
	LastSelected->x2 = LastSelected->x1 + (LastSelected->x2 - LastSelected->x1) * LastSelected->fontsz / origfontsz;
	LastSelected->y2 = LastSelected->y1 + (LastSelected->y2 - LastSelected->y1) * LastSelected->fontsz / origfontsz;
	break;
   case ButtonKind:
	Otk_Get_Text( answrform, s, 200 );
	printf("Setting text to be '%s'\n", s );
	LastSelected->text = strdup(s);
	Otk_Modify_Text( LastSelected->RenderedObject->children, s );	
	resize_button( LastSelected );
	break;
   case FormBoxKind:
   case PullDownMenuKind:
	Otk_Get_Text( answrform, s, 200 );
	printf("Setting text to be '%s'\n", s );
	LastSelected->text = strdup(s);
	Otk_Modify_Text( LastSelected->RenderedObject, s );	
	if (LastSelected->kind==PullDownMenuKind) resize_button( LastSelected );
      break;
  }
 dismisspopup(x);
}

void accept_textprop( char *s, void *x ) { accept_properties( x ); }



void  EditProperties( void *x )
{ 
 char tstrng[100], tstrng2[100], wrd[100];
 float xa, xb, y, dx, dy, v, winwth=75, winht=40;

 printf("Edit Properties.\n");
 if (SelectedObjectsList==0)
  printf(" Nothing Selected.\n");
 else
 if (SelectedObjectsList->nxt != 0)
  printf(" No Single Object Selected.\n");
 else
  {
   popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, winwth, winht );
   LastSelected = SelectedObjectsList->object;

   switch (LastSelected->kind)
    {
     case PanelKind:
	{ 
	xa = 13;  xb = 30;  y = 20;  dy = 9;
        sprintf(tstrng,"Edit Properties of:  Panel  %s", LastSelected->name);
	OtkMakeTextLabel( popupwindow, "Color:", Otk_Black, 1.8, 1.0, 4, y );  
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Red:", Otk_Black, 1.5, 1.0, xa, y );
	slider1 = OtkMakeSliderHorizontal( popupwindow, xb, y+1, 30, get_slider, "0" );
	sliderv[0] = 100.0 * LastSelected->RenderedObject->color[0];
	Otk_SetSlider( slider1, 0.01 * sliderv[0], 3.0 );
	sprintf(wrd,"%3g",sliderv[0]);
	afrm[0] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Green:", Otk_Black, 1.5, 1.0, xa, y );
	slider2 = OtkMakeSliderHorizontal( popupwindow, xb, y+1, 30, get_slider, "1" );
	sliderv[1] = 100.0 * LastSelected->RenderedObject->color[1];
	Otk_SetSlider( slider2, 0.01 * sliderv[1], 3.0 );
	sprintf(wrd,"%3g",sliderv[1]);
	afrm[1] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Blue:", Otk_Black, 1.5, 1.0, xa, y );
	slider3 = OtkMakeSliderHorizontal( popupwindow, xb, y+1, 30, get_slider, "2" );
	sliderv[2] = 100.0 * LastSelected->RenderedObject->color[2];
	Otk_SetSlider( slider3, 0.01 * sliderv[2], 3.0 );
	sprintf(wrd,"%3g",sliderv[2]);
	afrm[2] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );

	y = y + dy + 3.5;
        xa = 30.0;  dx = 14.0;
        OtkMakeTextLabel( popupwindow, "Panel Type:", Otk_Black, 1.6, 1.0, 4, y );  
        radbutton[1] = OtkMakeRadioButton( popupwindow, xa, y, 4, 4, setpaneltype, "1" );
        OtkMakeTextLabel( popupwindow, "Raised", Otk_Black, 1.1, 1.0, xa+4, y );	xa = xa + dx;  
        radbutton[0] = OtkMakeRadioButton( radbutton[1], xa, y, 4, 4, setpaneltype, "0" );
        OtkMakeTextLabel( popupwindow, "Flat", Otk_Black, 1.1, 1.0, xa+4, y );  	xa = xa + dx - 3.0;
        radbutton[2] = OtkMakeRadioButton( radbutton[1], xa, y, 4, 4, setpaneltype, "2" );
        OtkMakeTextLabel( popupwindow, "Recessed", Otk_Black, 1.1, 1.0, xa+4, y );  	xa = xa + dx + 4.0; 
        radbutton[3] = OtkMakeRadioButton( radbutton[1], xa, y, 4, 4, setpaneltype, "20" );
        OtkMakeTextLabel( popupwindow, "Image", Otk_Black, 1.1, 1.0, xa+4, y );  
	if (LastSelected->RenderedObject->object_subtype>2) Otk_SetRadioButton( radbutton[3] );  else  Otk_SetRadioButton( radbutton[LastSelected->RenderedObject->object_subtype] );

        y = y + dy + 2.0;
        xa = 38.0;
        OtkMakeTextLabel( popupwindow, "Border Thickness:", Otk_Black, 1.6, 1.0, 4, y );  
	slider4 = OtkMakeSliderHorizontal( popupwindow, xa, y+2.0, 22, get_slider, "4" );
	sliderv[4] = LastSelected->RenderedObject->thickness;
	if (sliderv[4] >= 1.0)  v = 0.2 * sliderv[4] + 0.3;  else  v = sliderv[4] - 0.5;
	Otk_SetSlider( slider4, v, 3.0 );
	sprintf(wrd,"%3g",sliderv[4]);
	afrm[4] = OtkMakeTextFormBox( popupwindow, wrd, 5, xa+27, y, 10, 8, 0, 0 );
 	}
	break;
     case TextLabelKind:
        sprintf(tstrng,"Edit Properties of:  Text-Label  %s", LastSelected->name);
	answrform = OtkMakeTextFormBox( popupwindow, LastSelected->text, 45, 5, 20, 90, 15, accept_textprop, popupwindow );
        Set_Focus( answrform );
	OtkMakeTextLabel( popupwindow, "Font Size:", Otk_Black, 1.4, 1.1, 5, 42 );
	sprintf(tstrng2,"%3.2f",LastSelected->fontsz);
	answrform2 = OtkMakeTextFormBox( popupwindow, tstrng2, 5, 25, 40, 20, 10, accept_textprop, popupwindow );
        origfontsz = LastSelected->fontsz;
	slider4 = OtkMakeSliderHorizontal( popupwindow, 50, 45, 30, get_fontsz_slider, &origfontsz );

	y = 55;  dy = 7;   xa = 13;  xb = 25;
	OtkMakeTextLabel( popupwindow, "Color:", Otk_Black, 1.4, 1.0, 5, y );  
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Red:", Otk_Black, 1.2, 1.0, xa, y );
	slider1 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "0" );
	sliderv[0] = 100.0 * LastSelected->RenderedObject->color[0];
	Otk_SetSlider( slider1, 0.01 * sliderv[0], 3.0 );
	sprintf(wrd,"%3g",sliderv[0]);
	afrm[0] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 6, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Green:", Otk_Black, 1.2, 1.0, xa, y );
	slider2 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "1" );
	sliderv[1] = 100.0 * LastSelected->RenderedObject->color[1];
	Otk_SetSlider( slider2, 0.01 * sliderv[1], 3.0 );
	sprintf(wrd,"%3g",sliderv[1]);
	afrm[1] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 6, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Blue:", Otk_Black, 1.2, 1.0, xa, y );
	slider3 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "2" );
	sliderv[2] = 100.0 * LastSelected->RenderedObject->color[2];
	Otk_SetSlider( slider3, 0.01 * sliderv[2], 3.0 );
	sprintf(wrd,"%3g",sliderv[2]);
	afrm[2] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 6, 0, 0 );

	break;
     case FormBoxKind:
        sprintf(tstrng,"Edit Properties of:  Form Box  %s", LastSelected->name);
	if (LastSelected->text==0) LastSelected->text = strdup("");
	answrform = OtkMakeTextFormBox( popupwindow, LastSelected->text, 40, 5, 30, 80, 15, accept_textprop, popupwindow );
        Set_Focus( answrform );
	break;
     case ButtonKind:
        sprintf(tstrng,"Edit Properties of:  Push Button  %s", LastSelected->name);
	answrform = OtkMakeTextFormBox( popupwindow, LastSelected->text, 40, 5, 30, 80, 15, accept_textprop, popupwindow );
        Set_Focus( answrform );
	break;
     case ToggleKind:
        sprintf(tstrng,"Edit Properties of:  Toggle Button  %s", LastSelected->name);
	break;
     case RadioButtonKind:
        sprintf(tstrng,"Edit Properties of:  Radio Button  %s", LastSelected->name);
	break;
     case PullDownMenuKind:
        sprintf(tstrng,"Edit Properties of:  Pull-down Menu  %s", LastSelected->name);
	answrform = OtkMakeTextFormBox( popupwindow, LastSelected->text, 40, 5, 30, 80, 15, accept_textprop, popupwindow );
        Set_Focus( answrform );
	break;
     case SelectionListKind:
        sprintf(tstrng,"Edit Properties of:  Selection List  %s", LastSelected->name);
	break;
     case SliderKind:
        sprintf(tstrng,"Edit Properties of:  Slider  %s", LastSelected->name);
	break;
     case SeparatorKind:
        sliderv[0] = 50;  sliderv[1] = 50;  sliderv[2] = 50;
	xa = 13;  xb = 30;  y = 20;  dy = 12;
        sprintf(tstrng,"Edit Properties of:  Separator  %s", LastSelected->name);
	OtkMakeTextLabel( popupwindow, "Color:", Otk_Black, 1.6, 1.0, 4, y );  
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Red:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[0] = 100.0 * LastSelected->RenderedObject->color[0];
	slider1 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "0" );
	Otk_SetSlider( slider1, 0.01 * sliderv[0], 3.0 );
	sprintf(wrd,"%3g",sliderv[0]);
	afrm[0] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Green:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[1] = 100.0 * LastSelected->RenderedObject->color[1];
	slider2 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "1" );
	Otk_SetSlider( slider2, 0.01 * sliderv[1], 3.0 );
	sprintf(wrd,"%3g",sliderv[1]);
	afrm[1] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Blue:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[2] = 100.0 * LastSelected->RenderedObject->color[2];
	slider3 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "2" );
	Otk_SetSlider( slider3, 0.01 * sliderv[2], 3.0 );
	sprintf(wrd,"%3g",sliderv[2]);
	afrm[2] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Thickness:", Otk_Black, 1.6, 1.0, 4, y );
	sliderv[3] = LastSelected->RenderedObject->thickness;
	slider4 = OtkMakeSliderHorizontal( popupwindow, xb, y+1, 30, get_slider, "3" );
	Otk_SetSlider( slider4, 0.1 * (sliderv[3]-1.0), 3.0 );
	sprintf(wrd,"%3g",sliderv[3]);
	afrm[3] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y, 10, 8, 0, 0 );

	break;
     
     case GadgetGauge2:
        sprintf(tstrng,"Edit Properties of:  Analog Gauge  %s", LastSelected->name);
	break;
     case GadgetLedMeter:
        sprintf(tstrng,"Edit Properties of:  LED Meter  %s", LastSelected->name);
        xa = 13;  xb = 30;  y = 20;  dy = 12;
	OtkMakeTextLabel( popupwindow, "Color:", Otk_Black, 1.6, 1.0, 4, y );  
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Red:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[0] = 100.0 * LastSelected->RenderedObject->children->color[0];
	slider1 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "30" );
	Otk_SetSlider( slider1, 0.01 * sliderv[0], 3.0 );
	sprintf(wrd,"%3g",sliderv[0]);
	afrm[0] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Green:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[1] = 100.0 * LastSelected->RenderedObject->children->color[1];
	slider2 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "31" );
	Otk_SetSlider( slider2, 0.01 * sliderv[1], 3.0 );
	sprintf(wrd,"%3g",sliderv[1]);
	afrm[1] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Blue:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[2] = 100.0 * LastSelected->RenderedObject->children->color[2];
	slider3 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "32" );
	Otk_SetSlider( slider3, 0.01 * sliderv[2], 3.0 );
	sprintf(wrd,"%3g",sliderv[2]);
	afrm[2] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;   
        OtkMakeTextLabel( popupwindow, "Value:", Otk_Black, 1.6, 1.0, 4, y );  
	slider4 = OtkMakeSliderHorizontal( popupwindow, xb, y+2.0, 30, get_slider, "34" );
	sliderv[4] = 0.5;
	Otk_SetSlider( slider4, sliderv[4], 3.0 );
	sprintf(wrd,"%3g",sliderv[4]);
	afrm[4] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y, 10, 8, 0, 0 );
	break;
     case GadgetBarMeter:
        sprintf(tstrng,"Edit Properties of:  Bar Meter  %s", LastSelected->name);
        xa = 13;  xb = 30;  y = 20;  dy = 12;
	OtkMakeTextLabel( popupwindow, "Color:", Otk_Black, 1.6, 1.0, 4, y );  
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Red:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[0] = 100.0 * LastSelected->RenderedObject->children->color[0];
	slider1 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "40" );
	Otk_SetSlider( slider1, 0.01 * sliderv[0], 3.0 );
	sprintf(wrd,"%3g",sliderv[0]);
	afrm[0] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Green:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[1] = 100.0 * LastSelected->RenderedObject->children->color[1];
	slider2 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "41" );
	Otk_SetSlider( slider2, 0.01 * sliderv[1], 3.0 );
	sprintf(wrd,"%3g",sliderv[1]);
	afrm[1] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
	OtkMakeTextLabel( popupwindow, "Blue:", Otk_Black, 1.5, 1.0, xa, y );
	sliderv[2] = 100.0 * LastSelected->RenderedObject->children->color[2];
	slider3 = OtkMakeSliderHorizontal( popupwindow, xb, y, 30, get_slider, "42" );
	Otk_SetSlider( slider3, 0.01 * sliderv[2], 3.0 );
	sprintf(wrd,"%3g",sliderv[2]);
	afrm[2] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y-1, 10, 8, 0, 0 );
	y = y + dy;
        OtkMakeTextLabel( popupwindow, "Value:", Otk_Black, 1.6, 1.0, 4, y );  
	slider4 = OtkMakeSliderHorizontal( popupwindow, xb, y+2.0, 30, get_slider, "44" );
	sliderv[4] = 50.0;
	Otk_SetSlider( slider4, 0.01 * sliderv[4], 3.0 );
	sprintf(wrd,"%3g",sliderv[4]);
	afrm[4] = OtkMakeTextFormBox( popupwindow, wrd, 5, xb + 35, y, 10, 8, 0, 0 );
	break;
     case GadgetPlot:
        sprintf(tstrng,"Edit Properties of:  XY Graph  %s", LastSelected->name);
     case GadgetStripChart:
        sprintf(tstrng,"Edit Properties of:  Strip-Chart  %s", LastSelected->name);
	break;
     default: sprintf(tstrng,"Error: obj-type %d unknown to properties dialog.", LastSelected->kind);
    }

   OtkMakeTextLabel( popupwindow, tstrng, Otk_Black, 1.7, 1.1, 3, 8 );
   OtkMakeButton( popupwindow, 15, 87, 20, 9, " Ok ", accept_properties, popupwindow );  
   OtkMakeButton( popupwindow, 70, 87, 20, 9, "Cancel", dismisspopup, popupwindow );  
   registerpopup();
  }
}



char *otk_lib_location=0;


void GenerateExport( void *x )
{
 char *fname=(char *)x, *exname, *container, *firstwindow="OtkOuterWindow", orientation, colorspec[100];
 struct GbuilderObject *obj;
 float xx1, yy1, xx2, yy2, thickness;
 int j, gadgets=0;
 FILE *outfile;

 printf("Exporting GUI '%s'.\n", fname );
 if (otk_lib_location==0)
  {
   otk_lib_location = getenv("OTK_LIB_LOCATION");
   if (otk_lib_location==0) 
    {
     printf("OTK_LIB_LOCATION enviornment variable not set.  Assuming otk_lib is under current directory.\n");
     otk_lib_location = strdup("otk_lib");  /* If env-var not set, assume otk_lib is under user's current dir. */
    }
  }
 outfile = fopen(fname,"w");
 if (outfile==0) {printf("Error: Cannot open '%s'\n", fname);  return; }
 fprintf(outfile,"/* Created by OTK GUI-Builder v%2.2f */\n", gb_version);
 fprintf(outfile,"/* Compile on Linux with:\n");
 fprintf(outfile,"     cc -I/usr/X11R6/include -L/usr/X11R6/lib %s \\\n", fname);
 exname = strdup(fname);
 j = strlen(exname) - 1;
 while ((j>0) && (exname[j]!='.')) j--;
 exname[j] = '\0';
 fprintf(outfile,"	-lGLU -lGL -lXmu -lXext -lX11 -lm -o %s.exe\n", exname);
 fprintf(outfile,"*/\n\n");
 fprintf(outfile,"#include \"%s/otk_lib.c\"\n", otk_lib_location);
 /* Scan for any gadgets. */
 obj = ObjectsList;  while (obj!=0) {if (obj->kind >= GadgetGauge2) gadgets = 1;  obj = obj->nxt;}
 if (gadgets)
  fprintf(outfile,"#include \"%s/gadget_lib.h\"\n#include \"%s/gadget_lib.c\"\n", otk_lib_location, otk_lib_location);
 if (ObjectsList!=0)
  {
   fprintf(outfile,"OtkWidget ");
   obj = ObjectsList;  j = 0;
   while (obj != 0)
    {
     if (obj->nxt != 0) fprintf(outfile,"%s, ", obj->name);
     else fprintf(outfile,"%s;", obj->name);
     j++; if (j % 10 == 0) fprintf(outfile,"\n	");
     obj = obj->nxt;
    }
   fprintf(outfile,"\n");
  }
 fprintf(outfile,"\nmain( int argc, char **argv )\n");
 fprintf(outfile,"{\n");
 fprintf(outfile,"  OtkInitWindow( 610, 600, argc, argv );\n");

 obj = ObjectsList;
 while (obj!=0)
  {
   xx1 = obj->xx1;
   yy1 = obj->yy1;
   xx2 = obj->xx2;
   yy2 = obj->yy2;
   if (obj->parent == 0) container = firstwindow;
   else container = obj->parent->name;
   switch (obj->kind)
    {
     case PanelKind:
	if ((obj->RenderedObject->color[0]==0.75) && (obj->RenderedObject->color[1]==0.75) &&(obj->RenderedObject->color[2]==0.75))
	 strcpy(colorspec,"Otk_LightGray");
	else
	 sprintf(colorspec,"OtkSetColor( %g, %g, %g)", obj->RenderedObject->color[0], obj->RenderedObject->color[1], obj->RenderedObject->color[2]);
	switch (obj->RenderedObject->object_subtype)
	 {
	  case Otk_Raised: fprintf(outfile,"  %s = OtkMakePanel( %s, Otk_Raised, %s, %g, %g, %g, %g );\n", obj->name, container, colorspec, xx1, yy1, xx2-xx1, yy2-yy1);
		break;
	  case Otk_Flat: fprintf(outfile,"  %s = OtkMakePanel( %s, Otk_Flat, %s, %g, %g, %g, %g );\n", obj->name, container, colorspec, xx1, yy1, xx2-xx1, yy2-yy1);
		break;
	  case Otk_Recessed: fprintf(outfile,"  %s = OtkMakePanel( %s, Otk_Recessed, %s, %g, %g, %g, %g );\n", obj->name, container, colorspec, xx1, yy1, xx2-xx1, yy2-yy1);
		break;
	  case Otk_ImagePanel: fprintf(outfile,"  %s = OtkMakeImagePanel( %s, \"%s\", %g, %g, %g, %g );\n", obj->name, container,  obj->RenderedObject->image->filename, xx1, yy1, xx2-xx1, yy2-yy1);
		break;
	 }
        if (bigdiff(obj->RenderedObject->thickness,1.0))
	 fprintf(outfile,"  Otk_SetBorderThickness( %s, %g );\n", obj->name, obj->RenderedObject->thickness );
	break;
     case TextLabelKind:
	if ((obj->RenderedObject->color[0]==0.0) && (obj->RenderedObject->color[1]==0.0) &&(obj->RenderedObject->color[2]==0.0))
	 strcpy(colorspec,"Otk_Black");
	else
	 sprintf(colorspec,"OtkSetColor( %g, %g, %g)", obj->RenderedObject->color[0], obj->RenderedObject->color[1], obj->RenderedObject->color[2]);
	fprintf(outfile,"  %s = OtkMakeTextLabel( %s, \"%s\", %s, %g, 1.0, %g, %g );\n", obj->name, container, obj->text, colorspec, obj->fontsz, xx1, yy1);
	break;
     case FormBoxKind:
	fprintf(outfile,"  %s = OtkMakeTextFormBox( %s, \"\", %d, %g, %g, %g, %g, 0, 0 );\n", obj->name, container, (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1);
	break;
     case ButtonKind:
	fprintf(outfile,"  %s = OtkMakeButton( %s, %g, %g, %g, %g, \"%s\", 0, 0 );\n", obj->name, container, xx1, yy1, xx2-xx1, yy2-yy1, obj->text);
	break;
     case ToggleKind:
	fprintf(outfile,"  %s = OtkMakeToggleButton( %s, %g, %g, %g, %g, 0, 0 );\n", obj->name, container, xx1, yy1, xx2-xx1, yy2-yy1);
	break;
     case RadioButtonKind:
	fprintf(outfile,"  %s = OtkMakeRadioButton( %s, %g, %g, %g, %g, 0, 0 );\n", obj->name, container, xx1, yy1, xx2-xx1, yy2-yy1);
	break;
     case PullDownMenuKind:
	if ((obj->parent==0) || (obj->parent->kind!=PullDownMenuKind))
	 fprintf(outfile,"  %s = Otk_Make_Menu( %s, %g, %g, %g, %g, \"%s  \" );\n", obj->name, container, xx1, yy1, xx2-xx1, yy2-yy1, obj->text);
	else
	 fprintf(outfile,"  %s = Otk_Add_Menu_Item( %s, \"%s  \", 0, 0 );\n", obj->name, container, obj->text);
	break;
     case SelectionListKind:
	fprintf(outfile,"  %s = Otk_Make_Selection_List( %s, %d, %d, %g, %g, %g, %g);\n", obj->name, container, 
			(int)(0.5*(yy2-yy1)+2.0), (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1 );
	break;
     case SliderKind:
	if (xx2-xx1 >= yy2 - yy1)
	 fprintf(outfile,"  %s = OtkMakeSliderHorizontal( %s, %g, %g, %g, 0, 0 );\n", obj->name, container, xx1, yy1, xx2-xx1);
	else
	 fprintf(outfile,"  %s = OtkMakeSliderVertical( %s, %g, %g, %g, 0, 0 );\n", obj->name, container, xx1, yy1, yy2-yy1);
	break;

     case SeparatorKind:
	thickness = obj->RenderedObject->thickness;
	if ((obj->RenderedObject->color[0]==0.0) && (obj->RenderedObject->color[1]==0.0) &&(obj->RenderedObject->color[2]==0.0))
	 strcpy(colorspec,"Otk_Black");
	else
	 sprintf(colorspec,"OtkSetColor( %g, %g, %g)", obj->RenderedObject->color[0], obj->RenderedObject->color[1], obj->RenderedObject->color[2]);
	if (fabs(xx1-xx2)<1.0) 
	 { /*Vert.-line*/
	  fprintf(outfile,"  %s = Otk_Add_Line( %s, %s, %g, %g, %g, %g, %g );\n", obj->name, container, colorspec, thickness, xx1, yy1, xx2, yy2 );
	 }
	else
	if (fabs(yy1-yy2)<1.0)
	 { /*Horiz.-line*/
	  fprintf(outfile,"  %s = Otk_Add_Line( %s, %s, %g, %g, %g, %g, %g );\n", obj->name, container, colorspec, thickness, xx1, yy1, xx2, yy2 );
	 }
	else
	 { /*Rect.-box*/
	  fprintf(outfile,"  %s = Otk_Add_BoundingBox( %s, %s, %g, %g, %g, %g, %g );\n", obj->name, container, colorspec, thickness, xx1, yy1, xx2, yy2);
	 }
	break;

     case GadgetGauge2:
	fprintf(outfile,"  %s = Otk_MakeGauge2( %s, %g, %g, %g, %g, \"Gauge\" );\n", obj->name, container, xx1, yy1, xx2 - xx1, yy2 - yy1 );
	break;
     case GadgetLedMeter:
	if ((obj->RenderedObject->children->color[0]==1.0) && (obj->RenderedObject->children->color[1]==0.0) && (obj->RenderedObject->children->color[2]==0.0))
	 strcpy(colorspec,"Otk_Red" );
	else sprintf(colorspec,"OtkSetColor( %g, %g, %g)", obj->RenderedObject->children->color[0], obj->RenderedObject->children->color[1], obj->RenderedObject->children->color[2] );
	if (xx2 - xx1 > yy2 - yy1) orientation = 'h';  else  orientation = 'v';  
	fprintf(outfile,"  %s = Otk_MakeLEDmeter( %s,  %g, %g, %g, %g, %d, '%c', %s );\n", obj->name, container, xx1, yy1, xx2 - xx1, yy2 - yy1, 10, orientation, colorspec );
	break;
     case GadgetBarMeter:
	if ((obj->RenderedObject->children->color[0]==1.0) && (obj->RenderedObject->children->color[1]==0.0) && (obj->RenderedObject->children->color[2]==0.0))
	 strcpy(colorspec,"Otk_Green" );
	else sprintf(colorspec,"OtkSetColor( %g, %g, %g)", obj->RenderedObject->children->color[0], obj->RenderedObject->children->color[1], obj->RenderedObject->children->color[2] );
	if (xx2 - xx1 > yy2 - yy1) orientation = 'h';  else  orientation = 'v'; 
	fprintf(outfile,"  %s = Otk_MakeBarMeter( %s,  %g, %g, %g, %g, '%c', %s );\n", obj->name, container, xx1, yy1, xx2 - xx1, yy2 - yy1, orientation, colorspec );
	break;
     case GadgetPlot:
     case GadgetStripChart:
	fprintf(outfile,"  %s = Otk_Plot_Init( %s, \"X-axis\", \"Y-axis\",  %g, %g, %g, %g, Otk_Green, Otk_DarkGray );\n", obj->name, container, xx1, yy1, xx2 - xx1, yy2 - yy1 );
	break;

    }
   obj = obj->nxt;
  }

 fprintf(outfile,"  OtkMainLoop();\n");
 fprintf(outfile,"}\n");
 fclose(outfile);
}





char directory[1500]=".", wildcards[1500]="", filename[1500]="newgui.c";

void my_file_answer( char *filename )
{ printf("To open file %s\n", filename);
  read_gui_file( filename );
}

void fileopener()
{ Otk_Browse_Files( "File to Open:", 1500, directory, wildcards, filename, my_file_answer ); }

void saveas_answer( char *fname )
{ printf("To saveas file %s\n", fname);
  GenerateExport( fname );
  strcpy_safe(filename,fname,1500);
}

void filesaver()
{ Otk_Browse_Files( "File to Save:", 1500, directory, wildcards, filename, saveas_answer ); }


void GenerateExport1( void *x )
{ GenerateExport( filename ); }





struct GbuilderObject *CopyObject( struct GbuilderObject *obj )
{
 struct GbuilderObject *newobj;
 char newobjname[100];

 newobj = (struct GbuilderObject *)calloc(1,sizeof(struct GbuilderObject));
 newobj->kind = obj->kind;
 newobj->name = strdup("");
 newobj->x1 = obj->x1;
 newobj->y1 = obj->y1;
 newobj->x2 = obj->x2;
 newobj->y2 = obj->y2;
 if (obj->text != 0)  newobj->text = strdup(obj->text);
 newobj->thickness = obj->RenderedObject->thickness;
 newobj->slant = obj->slant;
 newobj->fontsz = obj->fontsz;
 { int j; for (j=0; j<3; j++) newobj->color[j] = obj->RenderedObject->color[j]; }
 newobj->paneltype = obj->RenderedObject->object_subtype;
 if (obj->imagename != 0) newobj->imagename = obj->imagename;
 newobj->nrows = obj->nrows;
 newobj->ncols = obj->ncols;
 newobj->nentries = obj->nentries;
 newobj->outlinestyle = obj->outlinestyle;
 newobj->child = 0;
 newobj->nxtsibling = 0;
 return newobj;
}



void CopyObjects( void *x )	/* Copy selected objects to paste-buffer. */
{				/* ... But first clear the previous paste-buffer. */
 struct GbuilderObject *obj, *newobj=0;
 struct selobjectitem *tmppt;

 if (SelectedObjectsList==0) return;

 /* First, 'actually' free any objects previously in the paste-buffer. */
 while (PasteBuffer != 0)
  {
   obj = PasteBuffer;
   PasteBuffer = PasteBuffer->nxt;
   free(obj);
  }

 /* Copy, move the selected objects onto paste-buffer. */
 tmppt = SelectedObjectsList;
 while (tmppt != 0)
  {
   newobj = CopyObject( tmppt->object );
   newobj->nxt = PasteBuffer;
   PasteBuffer = newobj;
   tmppt = tmppt->nxt;
  }
}



void removeobj( struct GbuilderObject *rmobj, int descendent )
{
 struct GbuilderObject *obj, *prvobj=0;

 /* Remove myself from the objects list. */
 obj = ObjectsList;
 while ((obj!=0) && (obj!=rmobj)) { prvobj = obj;  obj = obj->nxt; }
 if (obj==0) { return;  /* Non-existent object. */ }
 if (prvobj==0) ObjectsList = ObjectsList->nxt;  else  prvobj->nxt = obj->nxt;

 if (rmobj==LastAddedObj) LastAddedObj = prvobj;

 /* Remove my children, if I have any. */
 if (rmobj->child != 0) removeobj( rmobj->child, 1 );
 /* Remove my sibling, if I have any, and I am a descendent of one being removed. */
 if ((descendent) && (rmobj->nxtsibling != 0)) removeobj( rmobj->nxtsibling, 1 );

 /* Finally, release my rendered Otk-object. */
 Otk_RemoveObject( rmobj->RenderedObject );   rmobj->RenderedObject = 0;
}


void CutObjects( void *x )	/* Move selected objects to paste-buffer. */
{				/* ... But first clear the previous paste-buffer. */
 struct GbuilderObject *SelectedObject, *obj, *prvobj=0;
 struct selobjectitem *tmppt;

 /* First, copy the selected items into the paste buffer. */
 CopyObjects( 0 );

 /* Now, remove all selected objects. */
 tmppt = SelectedObjectsList;
 while (tmppt != 0)
  {
   SelectedObject = tmppt->object;
   if (SelectedObject->parent != 0)
    { /* Sever from parent. */
     obj = SelectedObject->parent->child;
     while ((obj!=SelectedObject) && (obj!=0)) { prvobj = obj;  obj = obj->nxtsibling; }
     if (prvobj!=0) prvobj->nxtsibling = SelectedObject->nxtsibling;
     else SelectedObject->parent->child = SelectedObject->nxtsibling;
    }
   removeobj( SelectedObject, 0 );
   tmppt = tmppt->nxt;
  }
 discard_obj_list();
}




void PasteObjects( void *x )	/* Paste the paste-buffer. */
{
 struct GbuilderObject *obj, *newobj=0;
 float xx1, yy1, xx2, yy2;
 char orientation;

 /* Copy, the objects from paste-buffer. */
 discard_obj_list();
 obj = PasteBuffer;
 while (obj != 0)
  {

   Add_object( obj->kind, obj->x1, obj->y1, obj->x2, obj->y2, &xx1, &yy1, &xx2, &yy2 );
   switch (obj->kind)
   {
   case PanelKind:
	Set_object( OtkMakePanel( ContainerWidget, obj->paneltype, OtkSetColor(obj->color[0],obj->color[1],obj->color[2]), xx1, yy1, xx2-xx1, yy2-yy1 ) );
	Otk_Set_Line_Thickness( LastAddedObj->RenderedObject, obj->thickness );
	break;
   case TextLabelKind:
	 Set_object( OtkMakeTextLabel( ContainerWidget, obj->text, OtkSetColor(obj->color[0],obj->color[1],obj->color[2]), obj->fontsz, 1.0 , xx1, yy1 ) );
	 LastAddedObj->text = strdup(obj->text);
	 LastAddedObj->fontsz = obj->fontsz;
	 LastAddedObj->RenderedObject->x2 = obj->x2;
	 LastAddedObj->RenderedObject->y2 = obj->y2;
	break;
   case FormBoxKind:
   	Set_object( OtkMakeTextFormBox( ContainerWidget, "", (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;
   case ToggleKind:
	Set_object( OtkMakeToggleButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;
   case RadioButtonKind:
	Set_object( OtkMakeRadioButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, 0, 0 ) );
	break;

   case ButtonKind:
	Set_object( OtkMakeButton( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, obj->text, 0, 0 ) );
	LastAddedObj->text = strdup(obj->text);
	break;
   case PullDownMenuKind:
	if ((LastAddedObj->parent==0) || (LastAddedObj->parent->kind!=PullDownMenuKind))
	 Set_object( Otk_Make_Menu( ContainerWidget, xx1, yy1, xx2-xx1, yy2-yy1, obj->text ) );
	else
	 { OtkWidget tmpobj;
	   tmpobj = ContainerWidget;
	   while ((tmpobj!=0) && (tmpobj->superclass!=Otk_SC_Menu_DropDown_Button)) { LastAddedObj->parent = LastAddedObj->parent->parent;  tmpobj = tmpobj->parent; }
	   if (tmpobj==0) {printf("Unexpected error. No Dropdown parent button.\n"); return;}
	   Set_object( Otk_Add_Menu_Item( tmpobj, obj->text, 0, 0 ) );
	 }
	LastAddedObj->text = strdup(obj->text);
	break;
   case SelectionListKind:
	Set_object( Otk_Make_Selection_List( ContainerWidget, (int)(0.5*(yy2-yy1)+2.0), (int)(0.5*(xx2-xx1)+2.0), xx1, yy1, xx2-xx1, yy2-yy1 ) );
	break;

   case SliderKind:
	if (obj->x2 - obj->x1 >= obj->y2 - obj->y1)
	 {
	  Set_object( OtkMakeSliderHorizontal( ContainerWidget, xx1, yy1, xx2-xx1, 0, 0 ) );
	  LastAddedObj->yy2 = LastAddedObj->RenderedObject->y2;
	  LastAddedObj->y2 = 100.0 * (LastAddedObj->RenderedObject->ybottom - canvas->ytop) / CanvasHeight;
	 }
	else
	 {
	  Set_object( OtkMakeSliderVertical( ContainerWidget, xx1, yy1, yy2-yy1, 0, 0 ) );
	  LastAddedObj->xx2 = LastAddedObj->RenderedObject->x2;
	  LastAddedObj->x2 = 100.0 * (LastAddedObj->RenderedObject->xright - canvas->xleft) / CanvasWidth;
	 }
	break;
   case SeparatorKind:
	if ((fabs(obj->x1 - obj->x2) < 1.5) || (fabs(obj->y1 - obj->y2) < 1.5))
	 { /*line*/
	  Set_object( Otk_Add_Line( ContainerWidget, OtkSetColor(obj->color[0],obj->color[1],obj->color[2]), obj->thickness, xx1, yy1, xx2, yy2 ) );
	 }
	else
	 { /*Rect.-box*/
	  Set_object( Otk_Add_BoundingBox( ContainerWidget, OtkSetColor(obj->color[0],obj->color[1],obj->color[2]), obj->thickness, xx1, yy1, xx2, yy2 ) );
	 }
	break;

   case GadgetGauge2:
		Set_object( Otk_MakeGauge2( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, "Gauge " ) );
		break;
   case GadgetLedMeter:
		if (obj->x2 - obj->x1 > obj->y2 - obj->y1) orientation = 'h';  else  orientation = 'v';  
		Set_object( Otk_MakeLEDmeter( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, 10, orientation, Otk_Red ) );
		break;
   case GadgetBarMeter:
		if (obj->x2 - obj->x1 > obj->y2 - obj->y1) orientation = 'h';  else  orientation = 'v'; 
		Set_object( Otk_MakeBarMeter( ContainerWidget, xx1, yy1, xx2 - xx1, yy2 - yy1, orientation, Otk_Green ) );
		Otk_SetBarMeter( LastAddedObj->RenderedObject, 50.0 );
		break;
   case GadgetPlot:
   case GadgetStripChart:
		Set_object( Otk_Plot_Init( ContainerWidget, "X-axis", "Y-axis", xx1, yy1, xx2 - xx1, yy2 - yy1, Otk_Green, Otk_DarkGray ) );
		break;

    }
   add_selected_item(LastAddedObj);
   obj = obj->nxt;
  }
}




OtkWidget snapindicator, gridindicator;

void snaptoggle( int state, void *x )
{
 if (snap2grid) 
  {
   snap2grid = 0;
   Otk_Modify_Text( snapindicator, "Snap is now Off" );   
  }
 else
  {
   snap2grid = 1;
   Otk_Modify_Text( snapindicator, "Snap is now On" );   
  }
}

void enlargegrid(float v, void *x)
{ char gstrng[50];
  if (v < 0.1) gridsize = 0.25; else
  if (v < 0.25) gridsize = 0.5;  else
  if (v < 0.4) gridsize = 1.0;  else
  if (v < 0.7) gridsize = 2.0;  else
  if (v < 0.9) gridsize = 4.0;  else  gridsize = 8.0;  
  sprintf(gstrng, "Grid size = %2.1f %%", gridsize );
  Otk_Modify_Text( gridindicator, gstrng );
}

void SetSnap( void *x )
{ char gstrng[50];
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 75, 60 );
 OtkMakeTextLabel( popupwindow, "Set Snap Options:", Otk_Black, 1.8, 1.5, 3, 5 );
 OtkMakeTextLabel( popupwindow, "Snap to Grid (On/Off):", Otk_Black, 1.2, 1.2, 6, 20 );
 OtkMakeToggleButton( popupwindow, 40, 20, 8, 6, snaptoggle, 0 );
 if (snap2grid) snapindicator = OtkMakeTextLabel( popupwindow, "Snap is now On", Otk_Black, 1.2, 1.2, 50, 20 );
 else snapindicator = OtkMakeTextLabel( popupwindow, "Snap is now Off", Otk_Black, 1.2, 1.2, 50, 20 );
 OtkMakeTextLabel( popupwindow, "Alter Grid Size:", Otk_Black, 1.2, 1.2, 6, 40 );
 OtkMakeSliderHorizontal( popupwindow, 40, 40, 40, enlargegrid, 0 );
 sprintf(gstrng, "Grid size = %2.1f %%", gridsize );
 gridindicator = OtkMakeTextLabel( popupwindow, gstrng, Otk_Black, 1.1, 1.0, 20, 48 );

 OtkMakeButton( popupwindow, 40, 85, 20, 10, "Dismiss", dismisspopup, popupwindow );
 registerpopup();
}


void About( void *x )
{
 char title[100];
 popupwindow = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 10, 10, 60, 40 );
 sprintf(title,"Otk GUI Builder - v%2.2f", gb_version);
 OtkMakeTextLabel( popupwindow, title, Otk_Black, 1.8, 1.5, 3, 5 );
 OtkMakeTextLabel( popupwindow, "See:   http://otk.sourceforge.net", Otk_Black, 1.8, 1.3, 5, 40 );
 OtkMakeButton( popupwindow, 40, 85, 20, 10, "Dismiss", dismisspopup, popupwindow );  
 registerpopup();
}



void quit()
{ exit(0); }




void convert_mouse2coords( float mousex, float mousey, float *x, float *y )
{
   *x = 100.0 * (mousex - LeftPanelX)/ (100.0-LeftPanelX);
   *y = 100.0 * (mousey - TopPanelY)/ (100.0-TopPanelY);
}




/* Move an object's children with it. */
void move_my_children( struct GbuilderObject *obj, float obj_dx, float obj_dy, int final_mouse_up )
{
 OtkWidget renderedobj, container, child, tmpobj;
 float wdth, hgt;

 /* Adjust the rendered (presently displayed) object's coords. */
 renderedobj = obj->RenderedObject;

 container = renderedobj->parent;
 wdth = renderedobj->xright - renderedobj->xleft;
 hgt = renderedobj->ybottom - renderedobj->ytop;
 renderedobj->xleft   = container->xleft + renderedobj->x1 * (container->xright - container->xleft) * 0.01;
 renderedobj->xright  =  renderedobj->xleft + wdth;  
 renderedobj->ytop    = container->ytop  + renderedobj->y1 * (container->ybottom - container->ytop) * 0.01;
 renderedobj->ybottom = renderedobj->ytop + hgt;

 if ((obj->kind == SliderKind) || (obj->kind == ButtonKind) || (obj->kind == PullDownMenuKind))
  { 
    container = renderedobj;
    tmpobj = renderedobj->children;
    wdth = container->xright - container->xleft;
    hgt = container->ybottom - container->ytop;
    tmpobj->xleft =   container->xleft + tmpobj->x1 * wdth * 0.01;
    tmpobj->xright =  container->xleft + tmpobj->x2 * wdth * 0.01;   
    tmpobj->ytop =    container->ytop + tmpobj->y1 * hgt * 0.01;   
    tmpobj->ybottom = container->ytop + tmpobj->y2 * hgt * 0.01; 
  }

 if ((obj->kind >= GadgetGauge2) && (obj->kind<=GadgetStripChart))
  { /*movegadget*/
    wdth = obj->RenderedObject->xright - obj->RenderedObject->xleft;
    hgt = obj->RenderedObject->ybottom - obj->RenderedObject->ytop;
    child = obj->RenderedObject->children;
    while (child!=0)
     {
      child->xleft = obj->RenderedObject->xleft + child->x1 * wdth * 0.01;
      child->xright = obj->RenderedObject->xleft + child->x2 * wdth * 0.01;
      child->ytop = obj->RenderedObject->ytop + child->y1 * hgt * 0.01;
      child->ybottom = obj->RenderedObject->ytop + child->y2 * hgt * 0.01;
      if (child->children!=0)
       { OtkWidget tmpobj;
	tmpobj = child->children;
	tmpobj->xleft = child->xleft + tmpobj->x1 * (child->xright - child->xleft) * 0.01;
	tmpobj->xright = child->xleft + tmpobj->x2 * (child->xright - child->xleft) * 0.01;
	tmpobj->ytop = child->ytop + tmpobj->y1 * (child->ybottom - child->ytop) * 0.01;
	tmpobj->ybottom = child->ytop + tmpobj->y2 * (child->ybottom - child->ytop) * 0.01;
       }
      child = child->nxt;
     }
   if (obj->kind==GadgetGauge2) adjust_guagepointer(obj->RenderedObject);
  } /*movegadget*/

 if (final_mouse_up)
  {
   /* Adjust the object's absolute window coords. */
   obj->x1 = obj->x1 + obj_dx;
   obj->y1 = obj->y1 + obj_dy;
   obj->x2 = obj->x2 + obj_dx;
   obj->y2 = obj->y2 + obj_dy;
   // set_container( obj );
   if (verbose) check_coords("Moved child", obj);
  }


 /* Container-relative coords. (xx1, etc.) do not change, since obj moves with container. */

 /* Move my children, if I have any. */
 if (obj->child != 0) move_my_children( obj->child, obj_dx, obj_dy, final_mouse_up );
 /* Move my sibling, if I have any, and I am a descendent of one being moved. */
 if (obj->nxtsibling != 0) move_my_children( obj->nxtsibling, obj_dx, obj_dy, final_mouse_up );
}




struct GbuilderObject *stretched_object;



/* Stretch an object's children with it. */
void stretch_my_children( struct GbuilderObject *obj )
{
 OtkWidget renderedobj, childobj, parentobj;
 float wdth, ht;

 /* Situation is this:
	The child's Otk: x1,y1,x2,y2 and the gbuilder: xx1,yy1,xx2,yy2 are not changed by stretch of a parent (ancestral) container!
	 (They are always *relative* *their* *parent* (container) panel. Not Absolute.)

	Only the Otk:  Xleft,Ytop,Xright,Ybottom, and gbuilder: x1,y1,x2,y2  change.
	But they change by the pro-rated amount of the mouse move based on their
	distance from the moving corner to the non-moving sides.

	The outer-window absolute coords of the box being stretched are:
	  For gbuilder: stretched_object->x1, y1,x2,y2.
	  For Otk:      stretched_object->RenderedObject->Xleft, Ytop,Xright,Ybottom.
	The corner being stretched is known by the global:  ontick

	Child vertices can always be calculated by:
 */
 if (obj->parent==0) stretch_my_children( obj->child );

 wdth = 0.01 * (obj->parent->x2 - obj->parent->x1);
 ht = 0.01 * (obj->parent->y2 - obj->parent->y1);
 obj->x1 = obj->parent->x1 + wdth * obj->xx1;
 obj->y1 = obj->parent->y1 + ht * obj->yy1;
 obj->x2 = obj->parent->x1 + wdth * obj->xx2;
 obj->y2 = obj->parent->y1 + ht * obj->yy2;

 renderedobj = obj->RenderedObject;
 parentobj = renderedobj->parent;
 wdth = 0.01 * (parentobj->xright - parentobj->xleft);
 ht = 0.01 * (parentobj->ybottom - parentobj->ytop);
 renderedobj->xleft = parentobj->xleft + renderedobj->x1 * wdth;
 renderedobj->ytop  = parentobj->ytop + renderedobj->y1 * ht;
 renderedobj->xright = parentobj->xleft + renderedobj->x2 * wdth;
 renderedobj->ybottom = parentobj->ytop + renderedobj->y2 * ht;

 if ((renderedobj->xright <= renderedobj->xleft + 0.5) && (obj->kind != SeparatorKind))
  {
   printf("Warning: negative obj size. (tick=%d stretchedobjects_width=%g)\n", ontick, wdth);
   renderedobj->xright = renderedobj->xleft + 0.5;
  }

 if ((renderedobj->ybottom <= renderedobj->ytop + 0.5) && (obj->kind != SeparatorKind))
  {
   printf("Warning: negative obj size. (tick=%d stretchedobjects_ht=%g)\n", ontick, ht);
   renderedobj->ybottom = renderedobj->ytop + 0.5;
  }

 /* Adjust the object's sub-child parts, if any, as rendered (presently displayed). */
 if ((obj->kind == SliderKind) || (obj->kind == ButtonKind) || (obj->kind == PullDownMenuKind))
  { /*childparts*/
   wdth = 0.01 * (renderedobj->xright - renderedobj->xleft);
   ht   = 0.01 * (renderedobj->ybottom - renderedobj->ytop);
   childobj = obj->RenderedObject->children;
   childobj->xleft  = renderedobj->xleft + childobj->x1 * wdth;
   childobj->ytop  = renderedobj->ytop + childobj->y1 * ht;
   childobj->xright  = renderedobj->xleft + childobj->x2 * wdth;
   childobj->ybottom  = renderedobj->ytop + childobj->y2 * ht;

   if ((obj->kind == ButtonKind) || (obj->kind == PullDownMenuKind)) resize_button( obj );
  } /*childparts*/

 if ((obj->kind >= GadgetGauge2) && (obj->kind<=GadgetStripChart))
  { /*movegadget*/  
    wdth = 0.01 * (obj->RenderedObject->xright - obj->RenderedObject->xleft);
    ht   = 0.01 * (obj->RenderedObject->ybottom - obj->RenderedObject->ytop);
    childobj = obj->RenderedObject->children;
    while (childobj!=0)
     {
     childobj->xleft = obj->RenderedObject->xleft + childobj->x1 * wdth;
     childobj->xright = obj->RenderedObject->xleft + childobj->x2 * wdth;
     childobj->ytop = obj->RenderedObject->ytop + childobj->y1 * ht;
     childobj->ybottom = obj->RenderedObject->ytop + childobj->y2 * ht;
     if (childobj->children!=0)
      {
       renderedobj = childobj->children;
       renderedobj->xleft = childobj->xleft + renderedobj->x1 * (childobj->xright - childobj->xleft) * 0.01;
       renderedobj->xright = childobj->xleft + renderedobj->x2 * (childobj->xright - childobj->xleft) * 0.01;
       renderedobj->ytop = childobj->ytop + renderedobj->y1 * (childobj->ybottom - childobj->ytop) * 0.01;
       renderedobj->ybottom = childobj->ytop + renderedobj->y2 * (childobj->ybottom - childobj->ytop) * 0.01;
      }
     childobj = childobj->nxt;
    }
   if (obj->kind==GadgetGauge2) adjust_guagepointer(obj->RenderedObject);
  } /*movegadget*/

 /* Move my children, if I have any. */
 if (obj->child != 0) stretch_my_children( obj->child );
 /* Move my sibling, if I have any, and I am a descendent of one being moved. */
 if (obj->nxtsibling != 0) stretch_my_children( obj->nxtsibling );
}




float prevmouseclickx=-1.0, prevmouseclicky=-1.0;


void MouseClicked( int state )
{
 float x1, y1, x2, y2, a, objwdth, objht;
 struct GbuilderObject *SelectedObject;
 struct selobjectitem *objlistitem;

 if (popupopen) return;
 if (state==0) 	/* Mouse-clicked-down. */
  {
   mousestate = 1;
   if (verbose) printf("MouseClickedDOWN\n");
   if (tool_mode==SelectMove)
    {
     determine_selected_object();
     if ((Otk_MouseX==prevmouseclickx) && (Otk_MouseY==prevmouseclicky))
      { /*doubleclick*/
	EditProperties(0);
        prevmouseclickx = -1.0;  prevmouseclicky = -1.0;
      } /*doubleclick*/
     else { prevmouseclickx = Otk_MouseX;  prevmouseclicky = Otk_MouseY; }
    }
   else
    {
     discard_obj_list();
     prevmouseclickx = -1.0;  prevmouseclicky = -1.0;
    }
   return;	/* Don't do anything on the down-click. */
  }
 /* Else Mouse-Up. */
 mousestate = 0;
 if (rubberband[0]!=0) 
  { /*rubberbandbox*/
    int k; 
    for (k=0; k<4; k++) Otk_RemoveObject( rubberband[k] );  
    rubberband[0] = 0;
    // printf("End of rubberband box at (%g,%g) to (%g, \%g)\n", mmx1, mmy1, Otk_MouseX, Otk_MouseY );
    convert_mouse2coords( mmx1, mmy1, &x1, &y1 );
    convert_mouse2coords( Otk_MouseX, Otk_MouseY, &x2, &y2 );
    if (x2 < x1 ) { a = x1;  x1 = x2;  x2 = a; }
    if (y2 < y1 ) { a = y1;  y1 = y2;  y2 = a; }
    if (x1 < 0.0) x1 = 0.0;
    if (y1 < 0.0) y1 = 0.0;
    if (x2 < 0.0) { /* printf("Out of Bounds x\n"); */  return; }
    if (y2 < 0.0) { /* printf("Out of Bounds y\n"); */  return; }
    if (x2 > 100.0) x2 = 100.0;
    if (y2 > 100.0) y2 = 100.0;
    // printf("	Canvas point = (%g,%g) to (%g,%g)\n", x1, y1, x2, y2 );
    mousedragged = 1;
    AddSomething( x1, y1, x2, y2 );
    mousedragged = 0;
  } /*rubberbandbox*/
 else
  { /*clicked_or_dragged*/
   OtkWidget renderedobj, container, tmpobj;
   float obj_dx, obj_dy, xa1, ya1;

   // printf("	Mouse clicked %d	(%d,%d)/(%d,%d)	(%g,%g)\n", state, Otk_MousePixX, Otk_MousePixY, OtkWindowSizeX, OtkWindowSizeY, Otk_MouseX, Otk_MouseX );
   convert_mouse2coords( Otk_MouseX, Otk_MouseY, &xa1, &ya1 );
   if (mousedragged)
    { /*dragged*/
     float x0, y0;

     if (SelectedObjectsList == 0) { printf("Error: dragged without selection.\n");  return; }
     if (fabs(xa1-origmousex)+fabs(ya1-origmousey)==0.0) { mousedragged = 0;  return; }
     objlistitem = SelectedObjectsList;
     while (objlistitem!=0)
      { /*objlistitem*/
        SelectedObject = objlistitem->object;
        if (ontick==0)
         { /*obj_moved*/
           objwdth = SelectedObject->x2 - SelectedObject->x1;
           objht = SelectedObject->y2 - SelectedObject->y1;
           x1 = xa1 - (origmousex - SelectedObject->x1);
           y1 = ya1 - (origmousey - SelectedObject->y1);
           if (snap2grid)
            {
             x1 = (int)(x1/gridsize+0.5) * gridsize;
             y1 = (int)(y1/gridsize+0.5) * gridsize;
            }
           x2 = x1 + objwdth;
           y2 = y1 + objht;
           obj_dx = x1 - SelectedObject->x1;   /* Record amount object is moved. (absolute, relative base-canvas/window.) */
           obj_dy = y1 - SelectedObject->y1;

           SelectedObject->x1 = x1; 	/* Set object's new absolute window coords. */
           SelectedObject->y1 = y1; 
           SelectedObject->x2 = x2; 
           SelectedObject->y2 = y2;
	   // set_container( SelectedObject );

           if (SelectedObject->parent==0)
            {
             // printf("Dragged around base window.\n");
             SelectedObject->xx1 = SelectedObject->x1;             SelectedObject->yy1 = SelectedObject->y1;
             SelectedObject->xx2 = SelectedObject->x2;             SelectedObject->yy2 = SelectedObject->y2;
            }
           else
            {
             /* Scale to live within parent panel.  (Base coords are OuterWindowBased.) */
	     if (SelectedObject->x1 < SelectedObject->parent->x1) 	/* Force to stay within parent. */
	      { SelectedObject->x2 = SelectedObject->x2 - SelectedObject->x1 + SelectedObject->parent->x1;  SelectedObject->x1 = SelectedObject->parent->x1;  obj_dx = x1 - SelectedObject->x1;}
	     if (SelectedObject->x2 > SelectedObject->parent->x2) 	/* Force to stay within parent. */
	      { SelectedObject->x1 = SelectedObject->parent->x2 - (SelectedObject->x2 - SelectedObject->x1);  SelectedObject->x2 = SelectedObject->parent->x2;  obj_dx = x1 - SelectedObject->x1;}
             SelectedObject->xx1 = 100.0 * (SelectedObject->x1 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
             SelectedObject->xx2 =  100.0 * (SelectedObject->x2 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
	     if (SelectedObject->y1 < SelectedObject->parent->y1) 	/* Force to stay within parent. */
	      { SelectedObject->y2 = SelectedObject->y2 - SelectedObject->y1 + SelectedObject->parent->y1;  SelectedObject->y1 = SelectedObject->parent->y1;  obj_dy = y1 - SelectedObject->y1;}
	     if (SelectedObject->y2 > SelectedObject->parent->y2) 	/* Force to stay within parent. */
	      { SelectedObject->y1 = SelectedObject->parent->y2 - (SelectedObject->y2 - SelectedObject->y1);  SelectedObject->y2 = SelectedObject->parent->y2;  obj_dy = y1 - SelectedObject->y1;}
             SelectedObject->yy1 = 100.0 * (SelectedObject->y1 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
             SelectedObject->yy2 = 100.0 * (SelectedObject->y2 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
            }

           renderedobj = SelectedObject->RenderedObject;  
           renderedobj->x1 = SelectedObject->xx1;           renderedobj->y1 = SelectedObject->yy1;	/* By definition. */
           renderedobj->x2 = SelectedObject->xx2;           renderedobj->y2 = SelectedObject->yy2;
           container = renderedobj->parent;
           x0 = renderedobj->xleft;
           y0 = renderedobj->ytop;
           objwdth = renderedobj->xright - renderedobj->xleft;
           objht = renderedobj->ybottom - renderedobj->ytop;
           renderedobj->xleft   = container->xleft + renderedobj->x1 * (container->xright - container->xleft) * 0.01;
           renderedobj->xright  =  renderedobj->xleft + objwdth;  //container->xleft + renderedobj->x2 * (container->xright - container->xleft) * 0.01;
           renderedobj->ytop    = container->ytop  + renderedobj->y1 * (container->ybottom - container->ytop) * 0.01;
           renderedobj->ybottom = renderedobj->ytop + objht;   // container->ytop  + renderedobj->y2 * (container->ybottom - container->ytop) * 0.01;

           if ((SelectedObject->kind == SliderKind) || (SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind))
            { 
              tmpobj = renderedobj->children;
              container = tmpobj->parent;
              objwdth = container->xright - container->xleft;
              objht = container->ybottom - container->ytop;
              tmpobj->xleft =   container->xleft + tmpobj->x1 * objwdth * 0.01;
              tmpobj->xright =  container->xleft + tmpobj->x2 * objwdth * 0.01;   
              tmpobj->ytop =    container->ytop + tmpobj->y1 * objht * 0.01;   
              tmpobj->ybottom = container->ytop + tmpobj->y2 * objht * 0.01; 
            }

 	   if (verbose) check_coords("Moved Obj", SelectedObject );

           /* Move all the object's children with it. */
           if (SelectedObject->child != 0) move_my_children( SelectedObject->child, obj_dx, obj_dy, 1 );
         } /*obj_moved*/

         else

	 if (ontick<=4)
          { /*StretchedObject*/
            OtkWidget tmpobj, container, rendobj, child;
            float x0, y0, dx, dy;
            int okh=1, okv=1;

            tmpobj = SelectedObject->RenderedObject;
            if ((SelectedObject->kind == SliderKind) || ((SelectedObject->kind == SeparatorKind) && ((SelectedObject->x2 - SelectedObject->x1 == 0.0) || (SelectedObject->y2 - SelectedObject->y1 == 0.0))))
             {
              if (SelectedObject->x2 - SelectedObject->x1 > SelectedObject->y2 - SelectedObject->y1 ) okv = 0; else okh = 0;
             }

	    if (verbose) printf("Completing stretch: (%g, %g) (%g, %g ) to ", SelectedObject->x1, SelectedObject->y1, SelectedObject->x2, SelectedObject->y2 );
            switch (ontick)
             {
              case 1:
	        if (okh) SelectedObject->x1 = xa1 - origmousex + SelectedObject->x1;
	        if (okv) SelectedObject->y1 = ya1 - origmousey + SelectedObject->y1;
		break;
              case 2:
	        if (okh) SelectedObject->x2 = xa1 - origmousex + SelectedObject->x2;
	        if (okv) SelectedObject->y1 = ya1 - origmousey + SelectedObject->y1;
		break;
              case 3:
	        if (okh) SelectedObject->x1 = xa1 - origmousex + SelectedObject->x1;
	        if (okv) SelectedObject->y2 = ya1 - origmousey + SelectedObject->y2;
		break;
              case 4:
	        if (okh) SelectedObject->x2 = xa1 - origmousex + SelectedObject->x2;
	        if (okv) SelectedObject->y2 = ya1 - origmousey + SelectedObject->y2;
		break;
             }

            if (snap2grid)
             {
              if (okh) SelectedObject->x1 = (int)(SelectedObject->x1/gridsize+0.5) * gridsize;
              if (okv) SelectedObject->y1 = (int)(SelectedObject->y1/gridsize+0.5) * gridsize;
              if (okh) SelectedObject->x2 = (int)(SelectedObject->x2/gridsize+0.5) * gridsize;
              if (okv) SelectedObject->y2 = (int)(SelectedObject->y2/gridsize+0.5) * gridsize;
             }
            set_ticks1( SelectedObject->x1, SelectedObject->y1, SelectedObject->x2, SelectedObject->y2 );
	    if (verbose) printf(" (%g, %g) (%g, %g )\n", SelectedObject->x1, SelectedObject->y1, SelectedObject->x2, SelectedObject->y2 );

            if (SelectedObject->parent==0)
             {
              // printf("Dragged around base window.\n");
              SelectedObject->xx1 = SelectedObject->x1;
              SelectedObject->yy1 = SelectedObject->y1;
              SelectedObject->xx2 = SelectedObject->x2;
              SelectedObject->yy2 = SelectedObject->y2;
             }
            else
             {
              /* Scale to live within new panel.  (Base coords are OuterWindowBased.) */
              SelectedObject->xx1 = 100.0 * (SelectedObject->x1 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
              SelectedObject->xx2 =  100.0 * (SelectedObject->x2 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
              SelectedObject->yy1 = 100.0 * (SelectedObject->y1 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
              SelectedObject->yy2 = 100.0 * (SelectedObject->y2 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
             }

	    tmpobj->x1 = SelectedObject->xx1;	  tmpobj->y1 = SelectedObject->yy1;	/* By definition. */
	    tmpobj->x2 = SelectedObject->xx2;	  tmpobj->y2 = SelectedObject->yy2;	/* By definition. */
    
            container = tmpobj->parent;
            x0 = tmpobj->xleft;
            y0 = tmpobj->ytop;
            objwdth = (container->xright - container->xleft) * 0.01;
            objht = (container->ybottom - container->ytop) * 0.01;
            tmpobj->xleft =   container->xleft + tmpobj->x1 * objwdth;
            tmpobj->xright =  container->xleft + tmpobj->x2 * objwdth;
            tmpobj->ytop =    container->ytop  + tmpobj->y1 * objht;
            tmpobj->ybottom = container->ytop  + tmpobj->y2 * objht;
    
            if ((SelectedObject->kind == SliderKind) || (SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind))
             {
	      tmpobj = tmpobj->children;
              container = tmpobj->parent;
              objwdth = 0.01 * (container->xright - container->xleft);
              objht = 0.01 * (container->ybottom - container->ytop);
              tmpobj->xleft =   container->xleft + tmpobj->x1 * objwdth;
              tmpobj->xright =  container->xleft + tmpobj->x2 * objwdth;
              tmpobj->ytop =    container->ytop + tmpobj->y1 * objht;
              tmpobj->ybottom = container->ytop + tmpobj->y2 * objht;
	      if ((SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind)) resize_button( SelectedObject );
             }

	    if ((SelectedObject->kind >= GadgetGauge2) && (SelectedObject->kind<=GadgetStripChart))
	     { /*movegadget*/  
	      dx = SelectedObject->RenderedObject->xright - SelectedObject->RenderedObject->xleft;
	      dy = SelectedObject->RenderedObject->ybottom - SelectedObject->RenderedObject->ytop;
	      child = SelectedObject->RenderedObject->children;
	      while (child!=0)
	       {
		child->xleft = SelectedObject->RenderedObject->xleft + child->x1 * dx * 0.01;
		child->xright = SelectedObject->RenderedObject->xleft + child->x2 * dx * 0.01;
		child->ytop = SelectedObject->RenderedObject->ytop + child->y1 * dy * 0.01;
		child->ybottom = SelectedObject->RenderedObject->ytop + child->y2 * dy * 0.01;
		if (child->children!=0)
		 {
		  rendobj = child->children;
		  rendobj->xleft = child->xleft + rendobj->x1 * (child->xright - child->xleft) * 0.01;
		  rendobj->xright = child->xleft + rendobj->x2 * (child->xright - child->xleft) * 0.01;
		  rendobj->ytop = child->ytop + rendobj->y1 * (child->ybottom - child->ytop) * 0.01;
		  rendobj->ybottom = child->ytop + rendobj->y2 * (child->ybottom - child->ytop) * 0.01;
		 }
	       child = child->nxt;
	      }
	      if (SelectedObject->kind==GadgetGauge2) adjust_guagepointer(SelectedObject->RenderedObject);
	     } /*movegadget*/

            /* Finally, stretch all the object's children with it. */
            if (SelectedObject->child != 0) stretch_my_children( SelectedObject->child );

          } /*StretchedObject*/
	else
	 { /*group_stretch*/
            OtkWidget tmpobj, container, rendobj, child;
            float x0, y0, dx, dy, mousemovex, mousemovey, okh, okv;

	    mousemovex = xa1 - origmousex;	mousemovey = ya1 - origmousey;
	    switch (ontick)	/* Do not allow stretching group inside out or vanisingly small. */
	     {
	      case 11:	if (mousemovex + 5.0 > grpmaxx-grpminx) mousemovex = grpmaxx - grpminx - 5.0;
			if (mousemovey + 5.0 > grpmaxy-grpminy) mousemovey = grpmaxy - grpminy - 5.0;	break;
	
	      case 12:	if (mousemovex - 5.0 < grpminx-grpmaxx) mousemovex = grpminx - grpmaxx + 5.0;
			if (mousemovey + 5.0 > grpmaxy-grpminy) mousemovey = grpmaxy - grpminy - 5.0;	break;
	
	      case 13:	if (mousemovex + 5.0 > grpmaxx-grpminx) mousemovex = grpmaxx - grpminx - 5.0;
			if (mousemovey - 5.0 < grpminy-grpmaxy) mousemovey = grpminy - grpmaxy + 5.0;	break;
	
	      case 14:	if (mousemovex - 5.0 < grpminx-grpmaxx) mousemovex = grpminx - grpmaxx + 5.0;
			if (mousemovey - 5.0 < grpminy-grpmaxy) mousemovey = grpminy - grpmaxy + 5.0;	break;

	     }
	   okh = 0.0;	okv = 0.0;
	   if (SelectedObject->kind == SliderKind) 
	    {
	     if (SelectedObject->x2 - SelectedObject->x1 > SelectedObject->y2 - SelectedObject->y1 ) 
	     okv = SelectedObject->y2 - SelectedObject->y1; else okh = SelectedObject->x2 - SelectedObject->x1;
	    }
            switch (ontick)
             {
	      case 11:
		    SelectedObject->x1 = SelectedObject->x1 + mousemovex * (grpmaxx - SelectedObject->x1) / (grpmaxx - grpminx);
		    SelectedObject->x2 = SelectedObject->x2 + mousemovex * (grpmaxx - SelectedObject->x2) / (grpmaxx - grpminx);
		    SelectedObject->y1 = SelectedObject->y1 + mousemovey * (grpmaxy - SelectedObject->y1) / (grpmaxy - grpminy);
		    SelectedObject->y2 = SelectedObject->y2 + mousemovey * (grpmaxy - SelectedObject->y2) / (grpmaxy - grpminy);
		   break;
	      case 12:
		    SelectedObject->x1 = SelectedObject->x1 + mousemovex * (SelectedObject->x1 - grpminx) / (grpmaxx - grpminx);
		    SelectedObject->x2 = SelectedObject->x2 + mousemovex * (SelectedObject->x2 - grpminx) / (grpmaxx - grpminx);
		    SelectedObject->y1 = SelectedObject->y1 + mousemovey * (grpmaxy - SelectedObject->y1) / (grpmaxy - grpminy);
		    SelectedObject->y2 = SelectedObject->y2 + mousemovey * (grpmaxy - SelectedObject->y2) / (grpmaxy - grpminy);
		   break;
	      case 13:
		    SelectedObject->x1 = SelectedObject->x1 + mousemovex * (grpmaxx - SelectedObject->x1) / (grpmaxx - grpminx);
		    SelectedObject->x2 = SelectedObject->x2 + mousemovex * (grpmaxx - SelectedObject->x2) / (grpmaxx - grpminx);
		    SelectedObject->y1 = SelectedObject->y1 + mousemovey * (SelectedObject->y1 - grpminy) / (grpmaxy - grpminy);
	 	    SelectedObject->y2 = SelectedObject->y2 + mousemovey * (SelectedObject->y2 - grpminy) / (grpmaxy - grpminy);
		   break;
	      case 14:
		    SelectedObject->x1 = SelectedObject->x1 + mousemovex * (SelectedObject->x1 - grpminx) / (grpmaxx - grpminx);
		    SelectedObject->x2 = SelectedObject->x2 + mousemovex * (SelectedObject->x2 - grpminx) / (grpmaxx - grpminx);
		    SelectedObject->y1 = SelectedObject->y1 + mousemovey * (SelectedObject->y1 - grpminy) / (grpmaxy - grpminy);
		    SelectedObject->y2 = SelectedObject->y2 + mousemovey * (SelectedObject->y2 - grpminy) / (grpmaxy - grpminy);
             }
//	    if (okh>0.0) SelectedObject->x2 = SelectedObject->x1 + okh;
//	    if (okv>0.0) SelectedObject->y2 = SelectedObject->y1 + okv;

            tmpobj = SelectedObject->RenderedObject;
            if (SelectedObject->parent==0)
             {
              // printf("Dragged around base window.\n");
              SelectedObject->xx1 = SelectedObject->x1;
              SelectedObject->yy1 = SelectedObject->y1;
              SelectedObject->xx2 = SelectedObject->x2;
              SelectedObject->yy2 = SelectedObject->y2;
             }
            else
             {
              /* Scale to live within new panel.  (Base coords are OuterWindowBased.) */
              SelectedObject->xx1 = 100.0 * (SelectedObject->x1 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
              SelectedObject->xx2 =  100.0 * (SelectedObject->x2 - SelectedObject->parent->x1) / (SelectedObject->parent->x2 - SelectedObject->parent->x1);
              SelectedObject->yy1 = 100.0 * (SelectedObject->y1 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
              SelectedObject->yy2 = 100.0 * (SelectedObject->y2 - SelectedObject->parent->y1) / (SelectedObject->parent->y2 - SelectedObject->parent->y1);
             }

	    tmpobj->x1 = SelectedObject->xx1;	  tmpobj->y1 = SelectedObject->yy1;	/* By definition. */
	    tmpobj->x2 = SelectedObject->xx2;	  tmpobj->y2 = SelectedObject->yy2;	/* By definition. */

            /* Finally, stretch all the object's children with it. */
            if (SelectedObject->child != 0) stretch_my_children( SelectedObject->child );

	 } /*group_stretch*/
 	if (verbose) check_coords("Stretched Obj", SelectedObject );

        objlistitem = objlistitem->nxt;
      } /*objlistitem*/

     update_selected_object_positions();
     mousedragged = 0;
    } /*dragged*/
   else
    { /*clicked*/
     if (xa1 < 0.0) { /* printf("Out of Bounds x\n"); */  return; }
     if (ya1 < 0.0) { /* printf("Out of Bounds y\n"); */  return; }
     // printf("	Canvas point = (%g,%g)\n", xa1, ya1 );
     AddSomething( xa1, ya1, xa1, ya1 );
    } /*clicked*/
  } /*clicked_or_dragged*/
}




void MouseMoved()	/* Mouse Drag. */
{ int j;
  OtkWidget rendobj, container, child;
  struct GbuilderObject *SelectedObject;
  struct selobjectitem *objlistitem;

 if (!mousestate) return;
 if (popupopen) return;
 // printf("Mouse moved,  Last=(%g,%g)\n", Otk_MouseX, Otk_MouseX );
 if (rubberband[0]!=0) 
  { int k;  for (k=0; k<4; k++) Otk_RemoveObject( rubberband[k] ); }
 else
  {
    mmx1 = Otk_MouseX;
    mmy1 = Otk_MouseY; 
  } 
 if (SelectedObjectsList==0)
  {
   rubberband[0] = Otk_Add_Line( OtkOuterWindow, rubberbandcolor, 1.0, mmx1, mmy1, Otk_MouseX, mmy1 );
   rubberband[1] = Otk_Add_Line( OtkOuterWindow, rubberbandcolor, 1.0, Otk_MouseX, mmy1, Otk_MouseX, Otk_MouseY );
   rubberband[2] = Otk_Add_Line( OtkOuterWindow, rubberbandcolor, 1.0, Otk_MouseX, Otk_MouseY, mmx1, Otk_MouseY );
   rubberband[3] = Otk_Add_Line( OtkOuterWindow, rubberbandcolor, 1.0, mmx1, Otk_MouseY, mmx1, mmy1 );
   for (j=0; j<4; j++) rubberband[j]->z = 400.0;
  }
 else
 if (ontick==0)
  { /*DraggingObject*/
    float x1, y1, orig_wdth, orig_ht, dx, dy, x0, y0;

    if (mousestate==0) return;
    convert_mouse2coords( Otk_MouseX, Otk_MouseY, &x1, &y1 );
    mousedragged = 1;
    objlistitem = SelectedObjectsList;
    while (objlistitem!=0)
     { /*objlistitem*/
       SelectedObject = objlistitem->object;
       // printf("Dragging object\n");
       rendobj = SelectedObject->RenderedObject;
       container = rendobj->parent;
       orig_wdth = rendobj->x2 - rendobj->x1;
       orig_ht = rendobj->y2 - rendobj->y1;

       rendobj->x1 = objlistitem->alphax * (x1 - origmousex) + objlistitem->origobjx;
       rendobj->y1 = objlistitem->alphay * (y1 - origmousey) + objlistitem->origobjy;
       rendobj->x2 = rendobj->x1 + orig_wdth;
       rendobj->y2 = rendobj->y1 + orig_ht;

       set_ticks( objlistitem->tickmarks, x1 - origmousex + SelectedObject->x1, y1 - origmousey + SelectedObject->y1, x1 - origmousex + SelectedObject->x2, y1 - origmousey + SelectedObject->y2 );

       x0 = rendobj->xleft;
       y0 = rendobj->ytop;
       dx = rendobj->xright - rendobj->xleft;
       dy = rendobj->ybottom - rendobj->ytop;
       rendobj->xleft =   container->xleft + rendobj->x1 * (container->xright - container->xleft) * 0.01;
       rendobj->xright =  rendobj->xleft + dx;		// container->xleft + rendobj->x2 * (container->xright - container->xleft) * 0.01;   
       rendobj->ytop =    container->ytop  + rendobj->y1 * (container->ybottom - container->ytop) * 0.01;   
       rendobj->ybottom = rendobj->ytop + dy;		// container->ytop  + rendobj->y2 * (container->ybottom - container->ytop) * 0.01; 

       if ((SelectedObject->kind == SliderKind) || (SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind))
        {
           container = rendobj;
           rendobj = rendobj->children;
	   if (rendobj==0) {printf("Object of type %d had no children\n", SelectedObject->kind); exit(0);}
           dx = container->xright - container->xleft;
           dy = container->ybottom - container->ytop;
           rendobj->xleft =   container->xleft + rendobj->x1 * dx * 0.01;
           rendobj->xright =  container->xleft + rendobj->x2 * dx * 0.01;   
           rendobj->ytop =    container->ytop + rendobj->y1 * dy * 0.01;   
           rendobj->ybottom = container->ytop + rendobj->y2 * dy * 0.01; 
        }

       if ((SelectedObject->kind >= GadgetGauge2) && (SelectedObject->kind<=GadgetStripChart))
        { /*movegadget*/
          dx = SelectedObject->RenderedObject->xright - SelectedObject->RenderedObject->xleft;
          dy = SelectedObject->RenderedObject->ybottom - SelectedObject->RenderedObject->ytop;
          child = SelectedObject->RenderedObject->children;
          while (child!=0)
           {
            child->xleft = SelectedObject->RenderedObject->xleft + child->x1 * dx * 0.01;
            child->xright = SelectedObject->RenderedObject->xleft + child->x2 * dx * 0.01;
            child->ytop = SelectedObject->RenderedObject->ytop + child->y1 * dy * 0.01;
            child->ybottom = SelectedObject->RenderedObject->ytop + child->y2 * dy * 0.01;
	    if (child->children!=0)
	     {
	      rendobj = child->children;
              rendobj->xleft = child->xleft + rendobj->x1 * (child->xright - child->xleft) * 0.01;
              rendobj->xright = child->xleft + rendobj->x2 * (child->xright - child->xleft) * 0.01;
              rendobj->ytop = child->ytop + rendobj->y1 * (child->ybottom - child->ytop) * 0.01;
              rendobj->ybottom = child->ytop + rendobj->y2 * (child->ybottom - child->ytop) * 0.01;
	    }
           child = child->nxt;
          }
	 if (SelectedObject->kind==GadgetGauge2) adjust_guagepointer(SelectedObject->RenderedObject);
        } /*movegadget*/
   
       /* Finally, move all the object's children with it. */
       if (SelectedObject->child != 0) move_my_children( SelectedObject->child, x1-origmousex, y1-origmousey, 0 );

       objlistitem = objlistitem->nxt;
     } /*objlistitem*/
  } /*DraggingObject*/
 else
 if (ontick<=4)
  { /*StretchingObject*/
    OtkWidget container;
    float x1, y1, dx, dy, x0, y0;
    int okh=1, okv=1;

    if (mousestate==0) return;
    if (verbose) printf("Stretching object (%d)\n", ontick);
    convert_mouse2coords( Otk_MouseX, Otk_MouseY, &x1, &y1 );

    mousedragged = 1;
    objlistitem = SelectedObjectsList;
    while (objlistitem!=0)
     { /*objlistitem*/
       SelectedObject = objlistitem->object;
       rendobj = SelectedObject->RenderedObject;
       if ((SelectedObject->kind == SliderKind) || ((SelectedObject->kind == SeparatorKind) && ((SelectedObject->x2 - SelectedObject->x1 == 0.0) || (SelectedObject->y2 - SelectedObject->y1 == 0.0))))
        {
         if (SelectedObject->x2 - SelectedObject->x1 > SelectedObject->y2 - SelectedObject->y1 ) okv = 0; else okh = 0;
        }
       container = rendobj->parent;

       /* Stretch protection. (Do not allow going inside-out.) */
       switch (ontick)
        {
         case 1:
	    if (SelectedObject->x1 + x1 - origmousex > SelectedObject->x2 - 2.0 ) x1 = SelectedObject->x2 - SelectedObject->x1 - 2.0 + origmousex; 
	    if (SelectedObject->y1 + y1 - origmousey > SelectedObject->y2 - 2.0 ) y1 = SelectedObject->y2 - SelectedObject->y1 - 2.0 + origmousey;
	break;
         case 3:
	    if (SelectedObject->x1 + x1 - origmousex > SelectedObject->x2 - 2.0 ) x1 = SelectedObject->x2 - SelectedObject->x1 - 2.0 + origmousex; 
	    if (SelectedObject->y2 + y1 - origmousey < SelectedObject->y1 + 2.0 ) y1 = SelectedObject->y1 - SelectedObject->y2 + 2.0 + origmousey; 
	break;
         case 2:
	    if (SelectedObject->x2 + x1 - origmousex < SelectedObject->x1 + 2.0 ) x1 = SelectedObject->x1 - SelectedObject->x2 + 2.0 + origmousex; 
	    if (SelectedObject->y1 + y1 - origmousey > SelectedObject->y2 - 2.0 ) y1 = SelectedObject->y2 - SelectedObject->y1 - 2.0 + origmousey;
	break;
         case 4:
	    if (SelectedObject->x2 + x1 - origmousex < SelectedObject->x1 + 2.0 ) x1 = SelectedObject->x1 - SelectedObject->x2 + 2.0 + origmousex; 
	    if (SelectedObject->y2 + y1 - origmousey < SelectedObject->y1 + 2.0 ) y1 = SelectedObject->y1 - SelectedObject->y2 + 2.0 + origmousey; 
	break;
        }

       switch (ontick)
        {
         case 1:
	    if (SelectedObject->x1 + x1 - origmousex > SelectedObject->x2 - 2.0 ) x1 = SelectedObject->x2 - SelectedObject->x1 - 2.0 + origmousex; 
	    if (okh) rendobj->x1 = objlistitem->alphax * (x1 - origmousex) + objlistitem->origobjx;
    	    if (okv) rendobj->y1 = objlistitem->alphay * (y1 - origmousey) + objlistitem->origobjy;
            set_ticks1( x1 - origmousex + SelectedObject->x1, y1 - origmousey + SelectedObject->y1, SelectedObject->x2, SelectedObject->y2 );
	   break;
         case 2:
	    if (okh) rendobj->x2 = objlistitem->alphax * (x1 - origmousex) + objlistitem->origobjx2;
    	    if (okv) rendobj->y1 = objlistitem->alphay * (y1 - origmousey) + objlistitem->origobjy;
	    set_ticks1( SelectedObject->x1, y1 - origmousey + SelectedObject->y1, x1 - origmousex + SelectedObject->x2, SelectedObject->y2 );
	   break;
         case 3:
	    if (okh) rendobj->x1 = objlistitem->alphax * (x1 - origmousex) + objlistitem->origobjx;
	    if (okv) rendobj->y2 = objlistitem->alphay * (y1 - origmousey) + objlistitem->origobjy2;
	    set_ticks1( x1 - origmousex + SelectedObject->x1, SelectedObject->y1, SelectedObject->x2, y1 - origmousey + SelectedObject->y2 );
	   break;
         case 4:
	    if (okh) rendobj->x2 = objlistitem->alphax * (x1 - origmousex) + objlistitem->origobjx2;
	    if (okv) rendobj->y2 = objlistitem->alphay * (y1 - origmousey) + objlistitem->origobjy2;
	    set_ticks1( SelectedObject->x1, SelectedObject->y1, x1 - origmousex + SelectedObject->x2, y1 - origmousey + SelectedObject->y2 );
	   break;
        }

       x0 = rendobj->xleft;
       y0 = rendobj->ytop;
       dx = (container->xright - container->xleft) * 0.01;
       dy = (container->ybottom - container->ytop) * 0.01;
       rendobj->xleft =   container->xleft + rendobj->x1 * dx;
       rendobj->xright =  container->xleft + rendobj->x2 * dx;
       rendobj->ytop =    container->ytop  + rendobj->y1 * dy;
       rendobj->ybottom = container->ytop  + rendobj->y2 * dy;

       if ((SelectedObject->kind == SliderKind) || (SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind))
        {
         rendobj = rendobj->children;
         container = rendobj->parent;
         dx = 0.01 * (container->xright - container->xleft);
         dy = 0.01 * (container->ybottom - container->ytop);
         rendobj->xleft =   container->xleft + rendobj->x1 * dx;
         rendobj->xright =  container->xleft + rendobj->x2 * dx;
         rendobj->ytop =    container->ytop + rendobj->y1 * dy;
         rendobj->ybottom = container->ytop + rendobj->y2 * dy;
         if ((SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind)) resize_button( SelectedObject );
        }

       if ((SelectedObject->kind >= GadgetGauge2) && (SelectedObject->kind<=GadgetStripChart))
        { /*movegadget*/
          dx = SelectedObject->RenderedObject->xright - SelectedObject->RenderedObject->xleft;
          dy = SelectedObject->RenderedObject->ybottom - SelectedObject->RenderedObject->ytop;
          child = SelectedObject->RenderedObject->children;
          while (child!=0)
           {
            child->xleft = SelectedObject->RenderedObject->xleft + child->x1 * dx * 0.01;
            child->xright = SelectedObject->RenderedObject->xleft + child->x2 * dx * 0.01;
            child->ytop = SelectedObject->RenderedObject->ytop + child->y1 * dy * 0.01;
            child->ybottom = SelectedObject->RenderedObject->ytop + child->y2 * dy * 0.01;
	    if (child->children!=0)
	     {
	      rendobj = child->children;
              rendobj->xleft = child->xleft + rendobj->x1 * (child->xright - child->xleft) * 0.01;
              rendobj->xright = child->xleft + rendobj->x2 * (child->xright - child->xleft) * 0.01;
              rendobj->ytop = child->ytop + rendobj->y1 * (child->ybottom - child->ytop) * 0.01;
              rendobj->ybottom = child->ytop + rendobj->y2 * (child->ybottom - child->ytop) * 0.01;
	    }
            child = child->nxt;
           }
	 if (SelectedObject->kind==GadgetGauge2) adjust_guagepointer(SelectedObject->RenderedObject);
        } /*movegadget*/
   
       /* Finally, stretch all the object's children with it. */
       stretched_object = SelectedObject;
       if (SelectedObject->child != 0) stretch_my_children( SelectedObject->child );

       objlistitem = objlistitem->nxt;
     } /*objlistitem*/
  } /*StretchingObject*/
 else
  { /*GroupStretch*/
    float y1, x1, x2, y2, dx, dy, x0, y0, mousemovex, mousemovey, okv, okh;
    if (mousestate==0) return;
    if (verbose) printf("GroupStretch (%d)\n", ontick);
    convert_mouse2coords( Otk_MouseX, Otk_MouseY, &x1, &y1 );
    mousedragged = 1;
    mousemovex = x1 - origmousex;	mousemovey = y1 - origmousey;
    switch (ontick)	/* Do not allow stretching group inside out or vanisingly small. */
     {
      case 11:	if (mousemovex + 5.0 > grpmaxx-grpminx) mousemovex = grpmaxx - grpminx - 5.0;
		if (mousemovey + 5.0 > grpmaxy-grpminy) mousemovey = grpmaxy - grpminy - 5.0;	break;

      case 12:	if (mousemovex - 5.0 < grpminx-grpmaxx) mousemovex = grpminx - grpmaxx + 5.0;
		if (mousemovey + 5.0 > grpmaxy-grpminy) mousemovey = grpmaxy - grpminy - 5.0;	break;

      case 13:	if (mousemovex + 5.0 > grpmaxx-grpminx) mousemovex = grpmaxx - grpminx - 5.0;
		if (mousemovey - 5.0 < grpminy-grpmaxy) mousemovey = grpminy - grpmaxy + 5.0;	break;

      case 14:	if (mousemovex - 5.0 < grpminx-grpmaxx) mousemovex = grpminx - grpmaxx + 5.0;
		if (mousemovey - 5.0 < grpminy-grpmaxy) mousemovey = grpminy - grpmaxy + 5.0;	break;

     }
    objlistitem = SelectedObjectsList;
    while (objlistitem!=0)
     { /*objlistitem*/
       SelectedObject = objlistitem->object;

       okh = 0.0;	okv = 0.0;
       if (SelectedObject->kind == SliderKind) 
        {
         if (SelectedObject->x2 - SelectedObject->x1 > SelectedObject->y2 - SelectedObject->y1 ) 
	  okv = SelectedObject->y2 - SelectedObject->y1; else okh = SelectedObject->x2 - SelectedObject->x1;
        }
       switch (ontick)
        {
         case 11:
	    x1 = SelectedObject->x1 + mousemovex * (grpmaxx - SelectedObject->x1) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xleft = canvas->xleft + 0.01 * x1 * CanvasWidth;
	    if (okh>0.0) x2 = x1 + okh; else
	     x2 = SelectedObject->x2 + mousemovex * (grpmaxx - SelectedObject->x2) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xright = canvas->xleft + 0.01 * x2 * CanvasWidth;
	    y1 = SelectedObject->y1 + mousemovey * (grpmaxy - SelectedObject->y1) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ytop = canvas->ytop + 0.01 * y1 * CanvasHeight;
	    if (okv>0.0) y2 = y1 + okv; else
	     y2 = SelectedObject->y2 + mousemovey * (grpmaxy - SelectedObject->y2) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ybottom = canvas->ytop + 0.01 * y2 * CanvasHeight;
	   break;
         case 12:
	    x1 = SelectedObject->x1 + mousemovex * (SelectedObject->x1 - grpminx) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xleft = canvas->xleft + 0.01 * x1 * CanvasWidth;
	    if (okh>0.0) x2 = x1 + okh; else
	     x2 = SelectedObject->x2 + mousemovex * (SelectedObject->x2 - grpminx) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xright = canvas->xleft + 0.01 * x2 * CanvasWidth;
	    y1 = SelectedObject->y1 + mousemovey * (grpmaxy - SelectedObject->y1) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ytop = canvas->ytop + 0.01 * y1 * CanvasHeight;
	    if (okv>0.0) y2 = y1 + okv; else
	     y2 = SelectedObject->y2 + mousemovey * (grpmaxy - SelectedObject->y2) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ybottom = canvas->ytop + 0.01 * y2 * CanvasHeight;
	   break;
         case 13:
	    x1 = SelectedObject->x1 + mousemovex * (grpmaxx - SelectedObject->x1) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xleft = canvas->xleft + 0.01 * x1 * CanvasWidth;
	    if (okh>0.0) x2 = x1 + okh; else
	     x2 = SelectedObject->x2 + mousemovex * (grpmaxx - SelectedObject->x2) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xright = canvas->xleft + 0.01 * x2 * CanvasWidth;
	    y1 = SelectedObject->y1 + mousemovey * (SelectedObject->y1 - grpminy) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ytop = canvas->ytop + 0.01 * y1 * CanvasHeight;
	    if (okv>0.0) y2 = y1 + okv; else
	     y2 = SelectedObject->y2 + mousemovey * (SelectedObject->y2 - grpminy) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ybottom = canvas->ytop + 0.01 * y2 * CanvasHeight;
	   break;
         case 14:
	    x1 = SelectedObject->x1 + mousemovex * (SelectedObject->x1 - grpminx) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xleft = canvas->xleft + 0.01 * x1 * CanvasWidth;
	    if (okh>0.0) x2 = x1 + okh; else
	     x2 = SelectedObject->x2 + mousemovex * (SelectedObject->x2 - grpminx) / (grpmaxx - grpminx);
	    SelectedObject->RenderedObject->xright = canvas->xleft + 0.01 * x2 * CanvasWidth;
	    y1 = SelectedObject->y1 + mousemovey * (SelectedObject->y1 - grpminy) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ytop = canvas->ytop + 0.01 * y1 * CanvasHeight;
	    if (okv>0.0) y2 = y1 + okv; else
	     y2 = SelectedObject->y2 + mousemovey * (SelectedObject->y2 - grpminy) / (grpmaxy - grpminy);
	    SelectedObject->RenderedObject->ybottom = canvas->ytop + 0.01 * y2 * CanvasHeight;
	   break;
        }
       set_ticks( objlistitem->tickmarks, x1, y1, x2, y1 );

       if ((SelectedObject->kind == SliderKind) || (SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind))
        {
         rendobj = SelectedObject->RenderedObject->children;
         container = SelectedObject->RenderedObject;
         dx = 0.01 * (container->xright - container->xleft);
         dy = 0.01 * (container->ybottom - container->ytop);
         rendobj->xleft =   container->xleft + rendobj->x1 * dx;
         rendobj->xright =  container->xleft + rendobj->x2 * dx;
         rendobj->ytop =    container->ytop + rendobj->y1 * dy;
         rendobj->ybottom = container->ytop + rendobj->y2 * dy;
         if ((SelectedObject->kind == ButtonKind) || (SelectedObject->kind == PullDownMenuKind)) resize_button( SelectedObject );
        }

       if ((SelectedObject->kind >= GadgetGauge2) && (SelectedObject->kind<=GadgetStripChart))
        { /*movegadget*/
          dx = SelectedObject->RenderedObject->xright - SelectedObject->RenderedObject->xleft;
          dy = SelectedObject->RenderedObject->ybottom - SelectedObject->RenderedObject->ytop;
          child = SelectedObject->RenderedObject->children;
          while (child!=0)
           {
            child->xleft = SelectedObject->RenderedObject->xleft + child->x1 * dx * 0.01;
            child->xright = SelectedObject->RenderedObject->xleft + child->x2 * dx * 0.01;
            child->ytop = SelectedObject->RenderedObject->ytop + child->y1 * dy * 0.01;
            child->ybottom = SelectedObject->RenderedObject->ytop + child->y2 * dy * 0.01;
	    if (child->children!=0)
	     {
	      rendobj = child->children;
              rendobj->xleft = child->xleft + rendobj->x1 * (child->xright - child->xleft) * 0.01;
              rendobj->xright = child->xleft + rendobj->x2 * (child->xright - child->xleft) * 0.01;
              rendobj->ytop = child->ytop + rendobj->y1 * (child->ybottom - child->ytop) * 0.01;
              rendobj->ybottom = child->ytop + rendobj->y2 * (child->ybottom - child->ytop) * 0.01;
	    }
            child = child->nxt;
           }
	 if (SelectedObject->kind==GadgetGauge2) adjust_guagepointer(SelectedObject->RenderedObject);
        } /*movegadget*/

       if (SelectedObject->child != 0) stretch_my_children( SelectedObject->child );
       objlistitem = objlistitem->nxt;
     } /*objlistitem*/
  } /*GroupStretch*/

} /*MouseDragged*/




int Keyclicked( char key )
{
 int ctrl;
 if (popupopen) return 0;
 ctrl = Otk_Get_Control_Key();
 if ((SelectedObjectsList!=0) && ((key=='\b') || ((key=='x') && (ctrl))))	/* Delete key (backspace) cuts objects. */
  {
   CutObjects(0);
   return 1;
  }
 else
 if ((key=='c') && (ctrl))	/* ^c Copies objects. */
  {
   CopyObjects(0);
   return 1;
  }
 else
 if ((key=='v') && (ctrl))	/* ^v Pastes objects. */
  {
   PasteObjects(0);
   return 1;
  }
 else return 0;
}




void compile_gui(void *x)
{
 char tstr[1000], *exname;
 int j;

 exname = strdup(filename);
 j = strlen(exname) - 1;
 while ((j>0) && (exname[j]!='.')) j--;
 exname[j] = '\0';
 sprintf(tstr,"cc -I/usr/X11R6/include -L/usr/X11R6/lib %s -lGLU -lGL -lXmu -lXext -lX11 -lm -o %s.exe", filename, exname);
 printf("Issuing: %s\n", tstr);
 system(tstr);
}

void run_gui(void *x)
{
 char tstr[1000], *exname;
 int j;

 exname = strdup(filename);
 j = strlen(exname) - 1;
 while ((j>0) && (exname[j]!='.')) j--;
 exname[j] = '\0';
 sprintf(tstr,"./%s.exe &", exname);
 printf("Issuing: %s\n", tstr);
 system(tstr);
}


void export_build_run(void *x)
{
 GenerateExport1( filename );
 compile_gui( 0 );
 run_gui( 0 );
}



#include "readguifile.c"


int main( int argc, char **argv )
{
 OtkWidget menubar, filemenu, editmenu, viewmenu, toolsmenu, optionsmenu, helpmenu, sbmenu, rbutton1;
 float x, y, fontsz, hsz, vsz, bht, dx, dy;
 char vstrng[30];
 int j, tog=0;

 printf("GBuilder %1.2f\n", gb_version );
 if (Otk_version<0.53) printf("Warning: This version of GBuilder needs Otk_lib 0.53 or newer.\n");
 OtkInitWindow( 650, 640, argc, argv );

 /**********************/
 /* Make top tool-bar. */
 /**********************/
 menubar = OtkMakePanel( OtkOuterWindow, Otk_Flat, OtkSetColor(0.68,0.7,0.75), 0, 0, 100, TopPanelY );
 Otk_Set_Default_Button_Color( 0.68, 0.7, 0.75 );
 x = 0.0;  dx = 13.0;
 filemenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " File " );
 Otk_Add_Menu_Item( filemenu, "Open", fileopener, 0 );
 Otk_Add_Menu_Item( filemenu, "Save", GenerateExport1, filename );
 Otk_Add_Menu_Item( filemenu, "Save As", filesaver, 0 );
 Otk_Add_Menu_Item( filemenu, "Print", 0, 0 );
 Otk_Add_Menu_Item( filemenu, "Exit", quit, 0 );
 x = x + dx;
 editmenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " Edit " );
 Otk_Add_Menu_Item( editmenu, "Cut        ^x", CutObjects, 0 );
 Otk_Add_Menu_Item( editmenu, "Copy      ^c", CopyObjects, 0 );
 Otk_Add_Menu_Item( editmenu, "Paste     ^v", PasteObjects, 0 );
 Otk_Add_Menu_Item( editmenu, "------------", 0, 0 );
 Otk_Add_Menu_Item( editmenu, "Properties", EditProperties, 0 );
 x = x + dx;
 viewmenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " View " );
 Otk_Add_Menu_Item( viewmenu, "Normal", 0, 0 );
 sbmenu = Otk_Add_SubMenu( viewmenu, "Detail" );	/* Add a Sub-Menu. */
 Otk_Add_Menu_Item( sbmenu, "Verbose", 0, 0 );
 Otk_Add_Menu_Item( sbmenu, "Quiet", 0, 0 );
 x = x + dx;
 optionsmenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " Options " );
 Otk_Add_Menu_Item( optionsmenu, "Snap Options", SetSnap, 0 );
 Otk_Add_Menu_Item( optionsmenu, "Check Selected Objects", check_selected_object, 0 );
 Otk_Add_Menu_Item( optionsmenu, "Check All Objects", check_all_objects, 0 );
 x = x + dx;
 toolsmenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " Tools " );
 Otk_Add_Menu_Item( toolsmenu, "Export New GUI", GenerateExport1, filename );
 Otk_Add_Menu_Item( toolsmenu, "Compile New GUI", compile_gui, 0 );
 Otk_Add_Menu_Item( toolsmenu, "Run New GUI", run_gui, 0 );
 Otk_Add_Menu_Item( toolsmenu, "---------------------", 0, 0 );
 Otk_Add_Menu_Item( toolsmenu, "Export, Build, + Run", export_build_run, 0 );
 x = x + dx;
 helpmenu = Otk_Make_Menu( menubar, x, 0, dx, 100, " Help " );
 Otk_Add_Menu_Item( helpmenu, "About", About, 0 );

 sprintf(vstrng,"GBuilder %2.2f", gb_version);
 OtkMakeTextLabel( menubar, vstrng, Otk_Black, 1.0, 1.0, 88.0, 55.0 );

 /***********************/
 /* Make side tool-bar. */
 /***********************/
 menubar = OtkMakePanel( OtkOuterWindow, Otk_Flat, OtkSetColor(0.68,0.7,0.75), 0, 5.5, LeftPanelX, 94 );

 x = 3.0;    y = 2.0;
 hsz = 8.0;  vsz=2.0;	bht = 4.0;  dy = 5.5;  fontsz = 1.4;
 Otk_Set_Button_Outline_Style( Otk_Flat );	/* ?? Is this working? */
 OtkMakeTextLabel( menubar, "Point:", Otk_Black, 1.5, 2.0, 1.0, y );
 y = y + 0.75 * dy;
 rbutton1 = OtkMakeRadioButton( menubar, 1.0, y, hsz, vsz, radiocallback, "0" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, 4.0, "Select/Move", radiobuttoncallback, "0" );
 SelectedToggle[tog++] = rbutton1;
 y = y + dy;
 OtkMakeTextLabel( menubar, "Add:", Otk_Black, 1.5, 2.0, 1.0, y );
 y = y + 0.75 * dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "1" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Panel", radiobuttoncallback, "1" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "2" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Text Label", radiobuttoncallback, "2" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "3" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Text Form Box", radiobuttoncallback, "3" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "4" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Push Button", radiobuttoncallback, "4" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "5" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Toggle Button", radiobuttoncallback, "5" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "6" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Radio Button", radiobuttoncallback, "6" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "7" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Pull-Down Menu", radiobuttoncallback, "7" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "8" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Selection List", radiobuttoncallback, "8" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "9" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Slider  ", radiobuttoncallback, "9" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "10" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Separator", radiobuttoncallback, "10" );
 y = y + dy;
 SelectedToggle[tog++] = OtkMakeRadioButton( rbutton1, x, y, hsz, vsz, radiocallback, "11" );
 OtkMakeButton( menubar, x+9.0, y, 85.0-x, bht, "Other Gadget", radiobuttoncallback, "11" );

 y = y + 1.5 * dy;
 OtkMakeButton( menubar, x, y, 95.0-x, bht, "Properties", EditProperties, 0 );
 OtkMakeButton( menubar, 5.0, 92.0, 90.0, 4.0, "Exit", quit, 0 );

 CanvasWidth = 100.0 - LeftPanelX;
 CanvasHeight = 100.0 - TopPanelY;
 canvas = OtkMakePanel( OtkOuterWindow, Otk_Flat, OtkSetColor(0.5,0.5,0.5), LeftPanelX, TopPanelY, CanvasWidth, CanvasHeight );

 /* Make a faint grid. */
 for (y=10.0;  y < 99.0;  y = y + 10.0)
  Otk_Add_Line( canvas, OtkSetColor(0.45,0.45,0.55), 1.0, 1.0, y, 99.0, y );
 for (x=10.0;  x < 99.0;  x = x + 10.0)
  Otk_Add_Line( canvas, OtkSetColor(0.45,0.45,0.55), 1.0, x, 1.0, x, 99.0 );

 Otk_Register_MouseClick_Callback( MouseClicked );
 Otk_Register_MouseMove_Callback( MouseMoved );
 Otk_Register_Keyboard_Callback( Keyclicked );

 Otk_Set_Default_Button_Color( 0.7, 0.7, 0.7 );

 j = 1;
 while (j<argc)
  {
   // printf("arg[%d] = '%s'\n", j, argv[j]);
   if (argv[j][0] == '-')
    { /*option*/
      if (strcmp(argv[j],"-v")==0)
	{
	 verbose = 1;
	}
      else
       printf("Unknown option '%s'\n", argv[j]);
    } /*option*/
   else
    read_gui_file(argv[j]);

   j++;
  }

 check_all_objects();

 OtkMainLoop();
 return 0;
}

