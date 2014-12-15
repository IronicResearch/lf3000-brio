/*
 * LFDisplayRenderer.h
 *
 *  Created on: Dec 20, 2012
 *      Author: lfu
 */

#ifndef COMMON_INCLUDE_LFDISPLAYRENDERER_H_
#define COMMON_INCLUDE_LFDISPLAYRENDERER_H_

#include <BlitBuffer.h>
#include <DisplayMPI.h>

using LeapFrog::Brio::CDisplayMPI;
using LeapFrog::Brio::tDisplayHandle;
using LeapFrog::Brio::tDisplayScreenStats;
using LeapFrog::Brio::kPixelFormatARGB8888;
using LeapFrog::Brio::kDisplayOnTop;
using LeapFrog::Brio::kDisplayScreenAllScreens;
using LeapFrog::Brio::tDisplayOrientation;
using LeapFrog::Brio::kOrientationPortrait;
using LeapFrog::Brio::U16;
using LeapFrog::Brio::tRect;

using namespace LeapFrog::Brio;  // needed for kNoErr

class LFDisplayRenderer
{
private:
    CDisplayMPI                 display_mpi_;         ///< the display manager
    CBlitBuffer                 display_buffers_[2];  ///< two buffers for double buffering
    tDisplayHandle              display_handles_[2];  ///< handles to the display buffers
    unsigned int                current_buffer_;      ///< which buffer we are currently rendering to
    const tDisplayScreenStats*  screen_stats_;        ///< screen size information
    bool                        display_ready_;       ///< flag to indicate displays are registered and ready

public:
	void InitDisplay(void)
	{
	    display_handles_[0] = display_mpi_.CreateHandle(screen_stats_->height,screen_stats_->width,kPixelFormatARGB8888,NULL);
	    display_handles_[1] = display_mpi_.CreateHandle(screen_stats_->height,screen_stats_->width,kPixelFormatARGB8888,NULL);
	    display_buffers_[0].setFromDisplayHandle(display_handles_[0]);
	    display_buffers_[1].setFromDisplayHandle(display_handles_[1]);
	    current_buffer_ = 0;
	    assert(display_mpi_.Register(display_handles_[0], 0, 0, kDisplayOnTop, kDisplayScreenAllScreens) == kNoErr);
	    assert(display_mpi_.Register(display_handles_[1], 0, 0, kDisplayOnTop, kDisplayScreenAllScreens) == kNoErr);
	    display_ready_ = true;
	}

	void TearDownDisplay(void)
	{
		if (display_ready_)
		{
			display_mpi_.UnRegister(display_handles_[0], kDisplayScreenAllScreens);
			display_mpi_.DestroyHandle(display_handles_[0], false);
			display_mpi_.UnRegister(display_handles_[1], kDisplayScreenAllScreens);
			display_mpi_.DestroyHandle(display_handles_[1], false);
			display_ready_ = false;
		}
	}

	LFDisplayRenderer() :
		display_ready_(false)
	{
		// default is portrait
		screen_stats_ = display_mpi_.GetScreenStats(kOrientationPortrait);
	}

	~LFDisplayRenderer()
	{
		TearDownDisplay();
	}

	CDisplayMPI* GetDisplayManager(void)
	{
		return &display_mpi_;
	}

	const tDisplayScreenStats* GetScreenStats()
	{
		return screen_stats_;
	}

	U16 Width(void)
	{
		return screen_stats_->width;
	}

	U16 Height(void)
	{
		return screen_stats_->height;
	}

	void SetOrientation(tDisplayOrientation orientation)
	{
		screen_stats_ = display_mpi_.GetScreenStats(orientation);
		TearDownDisplay();
		InitDisplay();
	}

	CBlitBuffer& CurrentBuffer(void)
	{
		return display_buffers_[current_buffer_];
	}

	void SetupFrameRender(void)
	{
	    // swap buffer index and create the display rectangle
	    if (++current_buffer_ > 1) current_buffer_ = 0;
	    tRect rect = {0, screen_stats_->width, 0, screen_stats_->height};
	    display_buffers_[current_buffer_].blitRect(rect, 0, 0, 0, 0, false);
	}

	void SwapBuffers(void)
	{
	    display_mpi_.SwapBuffers(display_handles_[current_buffer_], true);
	}

};


#endif /* COMMON_INCLUDE_LFDISPLAYRENDERER_H_ */
