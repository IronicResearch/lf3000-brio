/*
 * BlitButton
 *
 *  Created on: Mar 1, 2012
 *      Author: lfu
 */
#include <BlitBuffer.h>
#include <math.h>
#include <CoreTypes.h>
#include <AccelerometerMPI.h>

#ifndef BLITBUTTON_H_
#define BLITBUTTON_H_

using namespace std;
using namespace LeapFrog::Brio;

class BlitButton
{
public:
	BlitButton();
	virtual ~BlitButton();

	void init(CPath mainPath, CPath hilitePath, CString theString);
	bool hitTest( int x, int y);
	void reset();
	bool scaleBlit(CBlitBuffer *newImg, CBlitBuffer *srcImg, int newWidth, int newHeight, int numColors=4);

	CBlitBuffer							main;
	CBlitBuffer							hilite;
	int									x, y;
	bool								bSelected;
	CString								myString;

private:
};

#endif /* BLITBUTTON_H_ */
