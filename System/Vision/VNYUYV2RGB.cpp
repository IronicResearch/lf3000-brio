#include "VNYUYV2RGB.h"

namespace LF {
namespace Vision {
#define SAT(c) if (c & (~255)) { if (c < 0) c = 0; else c = 255; }
	void YUYV2RGB( const uint8_t* src, const int width, const int height, cv::Mat& dst ) {
	
		if( dst.empty() ) { 	// initialize rgb image if not already initialized
			dst.create(width, height, CV_8UC3);
		}

		const uint8_t *s = src;
		uint8_t *d = dst.data;
		int l, c;
		int r, g, b, cr, cg, cb, y1, y2;

		l = height;
		while (l--) {
			c = width >> 1;
			while (c--) {
				y1 = *s++;
				cb = ((*s - 128) * 454) >> 8;
				cg = (*s++ - 128) * 88;
				y2 = *s++;
				cr = ((*s - 128) * 359) >> 8;
				cg = (cg + (*s++ - 128) * 183) >> 8;

				r = y1 + cr;
				b = y1 + cb;
				g = y1 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b;

				r = y2 + cr;
				b = y2 + cb;
				g = y2 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b; 
			}
		}
	}
}
}
