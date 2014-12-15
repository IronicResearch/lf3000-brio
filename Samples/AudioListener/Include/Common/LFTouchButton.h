#ifndef COMMON_INCLUDE_LFTOUCHBUTTON_H_
#define COMMON_INCLUDE_LFTOUCHBUTTON_H_

#include <CSystemData.h>
#include <DisplayMPI.h>
#include <BlitBuffer.h>

using namespace LeapFrog::Brio;

class LFTouchButtonDelegate
{
public:
	virtual void HandleTouchEvent(unsigned int id) = 0;
};
/*
 * LFTouchButton
 * Simple button manager with states and hitTest
 */
class LFTouchButton
{
 public:
    LFTouchButton(void) :
        id_(0),
        is_active_(true),
        highlight_(false){};
    ~LFTouchButton() {};

    Boolean                  is_active_;
    Boolean                  highlight_;

    CBlitBuffer button_;
    CBlitBuffer button_high_;

    //button display rectangle
    tRect frame_;
    U16 x_;
    U16 y_;

    U16  id_;

    LFTouchButtonDelegate*  delegate_;

    void createButton(LeapFrog::Brio::CPath normalState,
                      LeapFrog::Brio::CPath highState,
                      U16 x0,
                      U16 y0,
                      U16 width,
                      U16 height,
                      Boolean active,
                      LFTouchButtonDelegate* delegate = NULL)
    {
        // because the image pixel data is loaded from top-left down to bottom-right
        // and the images is rendered from bottom-left to top-right we must
        // flip the image along the y-axis, hence the bottom of the image is
        // set to height and the top is set to 0.
        frame_.left = 0;
        frame_.bottom = height;
        frame_.right = width;
        frame_.top = 0;
        x_ = x0;
        y_ = y0;

        is_active_ = active;
        delegate_ = delegate;

        button_.createFromPng(normalState);
        button_high_.createFromPng(highState);
    }

    void checkForButtonTouchEvent(U16 touch_state, int x, int y)
    {
        normal();
        // check for touch over button
        if (hitTest(x,y))
        {
            if (touch_state != 0)
            {
                highlight();
            }
            else
            {
                handleTouchEvent();
            }
        }
    }

    Boolean hitTest(U16 x, U16 y)
    {
        if(!is_active_) return false;
        if(x>x_+frame_.left && x<x_+frame_.right)
        {
            // recall the image is flipped along the y-axis, hence we need to
            // swap the top and bottom in the check for a touch event on the
            // button
            if(y>y_+frame_.top && y<y_+frame_.bottom)
            {
                return true;
            }
        }
        return false;
    }

    void render(CBlitBuffer & display_buffer)
    {
        if (highlight_)
        {
            display_buffer.blitFromBuffer(button_high_, frame_, x_, y_, true);
        } else
        {
            display_buffer.blitFromBuffer(button_, frame_, x_, y_, true);
        }
    }

    void setActive()
    {
        is_active_ = true;
    }

    void highlight()
    {
        highlight_ = true;
    }

    void normal()
    {
        highlight_ = false;
    }

    void setHighlightImage(LeapFrog::Brio::CPath & path)
    {
    	button_high_.createFromPng(path);
    }

    void handleTouchEvent(void)
    {
        if (delegate_)
        {
            delegate_->HandleTouchEvent(id_);
        }
    }
};

#endif // COMMON_INCLUDE_LFTOUCHBUTTON_H_
