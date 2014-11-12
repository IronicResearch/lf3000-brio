#include <AudioMPI.h>
#include <AudioTypes.h>
#include <BrioOpenGLConfig.h>
#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <FontMPI.h>
#include <KernelMPI.h>
#include <SystemTypes.h>
#include <math.h>

LF_USING_BRIO_NAMESPACE()
#include "3d.h"
#include "cube.h"
#include "font.h"

//============================================================================
// RenderCube functions
//============================================================================
//----------------------------------------------------------------------------
void RenderCube()
{
  static int rotation = 0;
    
  // We are going to draw a cube centered at origin, and with an edge of 10 units
  /*
           7                      6
            +--------------------+
          / |                   /|
        /   |                 /  |
   3  /     |             2 /    |
    +---------------------+      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |             |      |
    |       |4            |      |5
    |       +-------------|------+
    |     /               |     /
    |   /                 |   /
    | /                   | /
    +---------------------+
   0                      1
  
  */
  

  static GLubyte front[]  = {2,1,3,0}; //front face
  static GLubyte back[]   = {5,6,4,7}; //back face
  static GLubyte top[]    = {6,2,7,3}; //top face
  static GLubyte bottom[] = {1,5,0,4}; //bottom face    
  static GLubyte left[]   = {3,0,7,4}; //left face
  static GLubyte right[]  = {6,5,2,1}; //right face

  
  static GLshort vertices[] = {-5,-5,-5,  5,-5,-5,  5,5,-5, -5,5,-5,  
                               -5,-5,5,   5,-5,5,   5,5,5,  -5,5,5};

  // Texture map coordinates
  static GLshort texmap[] = { 0,1,  1,1,  1,0,  0,0,   
  							  0,1,  1,1,  1,0,  0,0 };
  							 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

  glLoadIdentity();
  glPushMatrix();
  if(drawInOrtho)    
    glTranslatex(0, 0, FixedFromInt(-30));

  glRotatex(FixedFromInt(45), ONE, 0, 0);
  glRotatex(FixedFromInt(rotation++), 0, ONE,0); 
  

  //Enable the vertices array  
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_SHORT, 0, vertices);
  //3 = XYZ coordinates, GL_SHORT = data type, 0 = 0 stride bytes
 
  if (draw3dText) 
  {
    // Enable texture map array for selected sides = 1st two
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_SHORT, 0, texmap); //2 = UV coordinates per vertex
	glEnable(GL_TEXTURE_2D);
  }
  
  /*We are going to draw the cube, face by face, with different colors for 
  each face, using indexed vertex arrays and triangle strips*/
  glColor4x(ONE,0,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, front);

  glColor4x(0,ONE,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, back);
  glDisable(GL_TEXTURE_2D);

  glColor4x(0,0,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, top);

  glColor4x(ONE,ONE,0,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, bottom);

  glColor4x(0,ONE,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, left);

  glColor4x(ONE,0,ONE,0);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, right);

  // Render textured quad in normalized modelview coordinates
  glPopMatrix();

  // FIXME/dm: We really need normalized orthograophic matrix
  //		   to render flat texture tile at 1:1 scale.
  if (draw3dText) 
    {
  static GLshort quad[] = { -5,-5,-20,  5,-5,-20,  5,5,-20,  -5,5,-20 }; 

	  glVertexPointer(3, GL_SHORT, 0, quad);
	  glTexCoordPointer(2, GL_SHORT, 0, texmap);
	  glEnable(GL_TEXTURE_2D);
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_ONE, GL_ONE);
	  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	  }
		
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);		
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();   	
  eglSwapBuffers(glesDisplay, glesSurface);
}

