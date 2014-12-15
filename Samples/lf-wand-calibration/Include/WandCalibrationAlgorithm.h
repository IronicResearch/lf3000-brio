#ifndef __WANDCALIBRATIONALGORITHM_H__
#define __WANDCALIBRATIONALGORITHM_H__

#include "Vision/VNVirtualTouch.h"
#include <CameraMPI.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNWandTracker.h>
#include <Hardware/HWControllerMPI.h>
#include <EventListener.h>
#include <LTM.h>
#include <vector>


struct VNCalibrationColor {
    std::string name;
    cv::Scalar min;
    cv::Scalar max;
};


class WandCalibrationAlgorithm : public LF::Vision::VNAlgorithm {
public:


    enum CalibrationState {
        kNoState                         = 0,
        kWaitForWandTrigger              = 1,
        kWandTriggered                   = 2,
        kCalibratingColor                = 3,
        kWandColorFinishedCalibration    = 4,
        kWandVerifyColor                 = 5,
        kWandVerifyHotspotTriggered      = 6,
        kCalibrationComplete             = 7
    };


    enum CalibrationColorState {
        kGreen		= 0,
        kRed		= 1,
        kBlue		= 2,
        kYellow		= 3,
        kCyan		= 4,
        kMagenta	= 5
    };

    typedef void (*StateChangedCallback)(WandCalibrationAlgorithm::CalibrationState, WandCalibrationAlgorithm::CalibrationColorState, void* user);
    typedef void (*PostExecuteCallback)(cv::Mat&, cv::Mat&, void* user);


public:


    WandCalibrationAlgorithm();
    ~WandCalibrationAlgorithm();
    virtual void Initialize(LeapFrog::Brio::U16 frameProcessingWidth, LeapFrog::Brio::U16 frameProcessingHeight);

    virtual void Execute(cv::Mat &input, cv::Mat &output);

    void GotoState(WandCalibrationAlgorithm::CalibrationState state, WandCalibrationAlgorithm::CalibrationColorState color);
    WandCalibrationAlgorithm::CalibrationState GetState() const { return state_; }
    WandCalibrationAlgorithm::CalibrationColorState GetColor() const { return color_; }

    void SetHotSpotRects( std::vector<LeapFrog::Brio::tRect>& rects );
    std::vector<LF::Vision::VNRectHotSpot*> GetHotSpots() { return hotSpots_; }
    void ResetHotSpots();

    void SetStateChangedCallback( StateChangedCallback cb, void* user ) { stateChangedCallback_ = cb; stateChangedCallbackUserData_ = user; }
    void SetPostExecuteCallback( PostExecuteCallback cb, void* user ) { postExecuteCallback_ = cb; postExecuteCallbackUserData_ = user; }

    void SaveCalibration();

    void SetController( LF::Hardware::HWController* controller ) { controller_ = controller; }


private:

    class HotSpotListener : public LeapFrog::Brio::IEventListener {
    public:
      HotSpotListener(void);
      LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage& msg);
      WandCalibrationAlgorithm* parent_;
    };

    HotSpotListener hotspotListener_;

private:

    static void* CalibrateThread(void* arg);


private:
    LF::Vision::VNVirtualTouch*                        virtualTouch_;
    LF::Vision::VNWandTracker*                         wandTracker_;
    LF::Hardware::HWController*                        controller_;

    WandCalibrationAlgorithm::CalibrationState         state_;
    WandCalibrationAlgorithm::CalibrationState         previousState_;
    WandCalibrationAlgorithm::CalibrationColorState    color_;
    bool                                               stateChanged_;
    cv::Rect                                           roi_;
    std::vector<LF::Vision::VNRectHotSpot*>            hotSpots_;
    std::vector<LeapFrog::Brio::tRect>                 hotSpotRects_;
    int                                                warmUpFrameCount_;
    StateChangedCallback                               stateChangedCallback_;
    void*                                              stateChangedCallbackUserData_;
    PostExecuteCallback                                postExecuteCallback_;
    void*                                              postExecuteCallbackUserData_;
    cv::Mat                                            frame_;
    cv::Mat                                            foregroundMask_;
    cv::Mat                                            hsv_;
    cv::Mat                                            yuv_;

    VNCalibrationColor                                 bestColor_;
    VNCalibrationColor                                 calibrationColor_;
    int                                                maxPixelCount_;

    LTM::CJSonFile                                     configurationFile_;
    LTM::Value                                         wandConfiguration_;
    LTM::Value                                         defaultWandConfiguration_;
    LF::Vision::VNHotSpot*                             triggeredVerifyHotspot_;
};

#endif // __WANDCALIBRATIONALGORITHM_H__
