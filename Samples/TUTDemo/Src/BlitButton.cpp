/*
 * BlitButton.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: lfu
 */

#include "BlitButton.h"
#include <string.h>

using namespace std;
using namespace LeapFrog::Brio;

BlitButton::BlitButton()
{
	bSelected = false;
}

BlitButton::~BlitButton()
{
	// TODO Auto-generated destructor stub
}

void BlitButton::init(CPath mainPath, CPath hilitePath, CString theString)
{
	main.surface.buffer = 0;
	main.createFromPng( mainPath );

	hilite.surface.buffer = 0;
	hilite.createFromPng(hilitePath);

	myString = theString;
}

bool BlitButton::hitTest( int px, int py)
{
	bSelected = false;
	if(px > x && px < x + main.surface.width)
	{
		if(py > y && py < y + main.surface.height)
		{
			bSelected = true;
		}
	}
	return bSelected;
}

void BlitButton::reset()
{
	bSelected = false;
}

bool BlitButton::scaleBlit(CBlitBuffer *newImg, CBlitBuffer *srcImg, int newWidth, int newHeight, int numColors) {
	//Make sure our source image exists.
	if(srcImg == NULL)
	{
		//debugMPI_.DebugOut(kDbgLvlNoteable, "BlitButton::[ERROR] Tried to scale a non-existent image!\n");

		return false;
	}

	//Setup width, height, and RGBA info.
	int srcWidth 	= srcImg->surface.width;
	int srcHeight 	= srcImg->surface.height;

	//Setup a new image.
	//delBuffer(newImg);
	//newImg = new (POOL_MALLOC(sizeof(CBlitBuffer))) CBlitBuffer;
	newImg->createNew(newWidth,newHeight);

	//Setup our buffers.
	U8* srcBuffer 	= srcImg->surface.buffer;
	U8* newBuffer 	= newImg->surface.buffer;

	//If the dimensions are identical to the source image, then simply copy the source image into the new buffer.
	if(newWidth == srcWidth && newHeight == srcHeight)
		memcpy(newBuffer,srcBuffer,newWidth * newHeight * numColors);
	else {
		int   srcPitch 	= srcWidth * numColors;
		int   newPitch 	= newWidth * numColors;
		float scaleX 	= (float) newWidth	/ srcWidth;
		float scaleY 	= (float) newHeight / srcHeight;

		//printf("\nscaleX: %f, scaleY: %f\n\n", scaleX, scaleY);

		for(int y=0; y<newHeight; y++) {
			for(int x=0; x < newWidth; x++) {
				int srcX	= x/scaleX;
				int srcY 	= y/scaleY;
				int newPos 	= (y*newPitch) + (x*numColors);
				int srcPos 	= (srcY*srcPitch) + (srcX*numColors);
				for(int i=0; i<numColors; i++)
					newBuffer[newPos+i] =  srcBuffer[srcPos+i];
			}
		}
	}

	return true;
}
