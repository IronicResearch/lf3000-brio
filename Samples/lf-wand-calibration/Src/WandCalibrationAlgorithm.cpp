#include "WandCalibrationAlgorithm.h"
#include "VNCoordinateTranslator.h"
#include "VNInRange3.h"
#include "VNYUYV2RGB.h"
#include "VNRGB2HSV.h"
#include "VNIntegralImage.h"
#include <EventMPI.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNPointTrigger.h>
#include <Vision/VNOcclusionTrigger.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <Hardware/HWController.h>
#include <fstream>
#include <stdio.h>

#define USE_VIRTUAL_TOUCH_TO_CAPTURE 1

static const int kCalibrationHotspotTag            = 0x47477474;
static const float kLearningRate                   = 0.09f;
static const int kGrayIntensityThreshold           = 77;
static const float kOcclusionTriggerPercentage     = 0.13;
static const int kWarmUpFrameCount                 = 55;
static const int kHueRange                         = 70;
static const int kHueStep                          = 2;
//sum: 121380 umin: 184 umax: 254 vmin: 139 vmax: 209
// sum: 35700 umin: 41 umax: 111 vmin: 57 vmax: 127
// sum: 58395 umin: 58 umax: 128 vmin: 184 vmax: 254
// sum: 15810 umin: 9 umax: 79 vmin: 153 vmax: 223
static const char kWandColorConfigurationFile[] = "/LF/Bulk/Data/Local/All/VNWandColorCalibration.json";
  static const char kWandDefaultColorConfiguration[] = "{\n"
                                                        "    \"controllers\" : {\n"
                                                        "        \"default\" : {\n"
                                                        "            \"green\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 184,\n"
                                                        "                \"umax\" : 254,\n"
                                                        "                \"vmin\" : 144,\n"
                                                        "                \"vmax\" : 214\n"
                                                        "            },\n"
                                                        "            \"red\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 32,\n"
                                                        "                \"umax\" : 102,\n"
                                                        "                \"vmin\" : 56,\n"
                                                        "                \"vmax\" : 126\n"
                                                        "            },\n"
                                                        "            \"blue\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 36,\n"
                                                        "                \"umax\" : 106,\n"
                                                        "                \"vmin\" : 178,\n"
                                                        "                \"vmax\" : 248\n"
                                                        "            },\n"
                                                        "            \"yellow\" : {\n"
                                                        "                \"ymin\" : 40,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 8,\n"
                                                        "                \"umax\" : 78,\n"
                                                        "                \"vmin\" : 17,\n"
                                                        "                \"vmax\" : 87\n"
                                                        "            },\n"
                                                        "            \"cyan\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 0,\n"
                                                        "                \"umax\" : 70,\n"
                                                        "                \"vmin\" : 173,\n"
                                                        "                \"vmax\" : 243\n"
                                                        "            },\n"
                                                        "            \"magenta\" : {\n"
                                                        "                \"ymin\" : 1,\n"
                                                        "                \"ymax\" : 255,\n"
                                                        "                \"umin\" : 37,\n"
                                                        "                \"umax\" : 107,\n"
                                                        "                \"vmin\" : 38,\n"
                                                        "                \"vmax\" : 108\n"
                                                        "            },\n"
                                                        "            \"off\" : {\n"
                                                        "                \"ymin\" : 0,\n"
                                                        "                \"ymax\" : 0,\n"
                                                        "                \"umin\" : 0,\n"
                                                        "                \"umax\" : 0,\n"
                                                        "                \"vmin\" : 0,\n"
                                                        "                \"vmax\" : 0\n"
                                                        "            }\n"
                                                        "\n"
                                                        "\n"
                                                        "        }\n"
                                                        "    }\n"
                                                        "\n"
                                                        "}\n";

VNCalibrationColor calibrationColors[] = {
	{std::string("green"),	    cv::Scalar(77, 0, 0),	cv::Scalar(255, kHueRange, kHueRange)},
	{std::string("red"),	    cv::Scalar(77, 0, 0),	cv::Scalar(255, kHueRange, kHueRange)},
	{std::string("blue"),	    cv::Scalar(77, 0, 0),   cv::Scalar(255, kHueRange, kHueRange)},
    {std::string("yellow"),	    cv::Scalar(77, 0, 0),   cv::Scalar(255, kHueRange, kHueRange)},
    {std::string("cyan"),	    cv::Scalar(77, 0, 0),   cv::Scalar(255, kHueRange, kHueRange)},
    {std::string("magenta"),	cv::Scalar(77, 0, 0),   cv::Scalar(255, kHueRange, kHueRange)}
};


int calculatePixelSumForHueRange( VNCalibrationColor& color, const cv::Mat& yuv, cv::Rect& roi, cv::Mat& rangeMaskOutput ) {
    cv::Mat rangeMask;
    cv::Mat yuvROI = yuv(roi);

	cv::inRange(yuvROI, color.min, color.max, rangeMask);

    cv::Mat imageRoi = rangeMask;
    int sum = 0;
    for( int i = 0; i < imageRoi.total(); i++ ) {
        sum += imageRoi.at<uint8_t>(i);
    }
    return sum;
}

void calibrateColorInRange( VNCalibrationColor& bestcolor, const cv::Mat& yuv, cv::Rect& roi, cv::Scalar& min, cv::Scalar& max, cv::Mat& outmask ) {
    int maxPixelCount = 0;

    cv::Mat bestRangeMask;

    VNCalibrationColor color = bestcolor;

    printf("calibrateColorInRange: min(%f, %f) max(%f, %f)\n", min[0], min[1], max[0], max[1] );


    for( int umin = min[0], umax = min[0] + kHueRange; umax < max[0]; umin+=kHueStep, umax+=kHueStep ) {
        for( int vmin = min[1], vmax = min[1] + kHueRange; vmax < max[1]; vmin+=kHueStep, vmax+=kHueStep ) {
            color.min[1] = umin;
            color.max[1] = umax;
            color.min[2] = vmin;
            color.max[2] = vmax;

            int sum = calculatePixelSumForHueRange( color, yuv, roi, bestRangeMask );
            // bestRangeMask.copyTo(outmask);
            //std::cout << "sum: " << sum << " umin: " << umin << " umax: " << umax << " vmin: " << vmin  << " vmax: " << vmax << std::endl;
            if( sum > maxPixelCount ) {
                bestcolor = color;
                maxPixelCount = sum;
                bestcolor = color;
                std::cout << "\asum: " << sum << " umin: " << umin << " umax: " << umax << " vmin: " << vmin  << " vmax: " << vmax << std::endl;
            }
        }
    }

    // assert(0);

}


static LeapFrog::Brio::tControlInfo* FindCameraControl(const LeapFrog::Brio::tCameraControls &controls, const LeapFrog::Brio::tControlType type) {
    LeapFrog::Brio::tControlInfo *c = NULL;
    for(LeapFrog::Brio::tCameraControls::const_iterator i = controls.begin(); i != controls.end(); ++i) {

        c = *i;
        if (!c)
            continue;

        if (c->type == type)
            break;
    }

    return c;
}


WandCalibrationAlgorithm::WandCalibrationAlgorithm()
   :    virtualTouch_( 0 )
    ,    wandTracker_( 0 )
    ,    state_(kNoState)
    ,    color_(kGreen)
    ,    stateChangedCallback_(0)
    ,    stateChangedCallbackUserData_(0)
    ,    postExecuteCallback_(0)
    ,    postExecuteCallbackUserData_(0)
    ,    stateChanged_(false)
    ,    configurationFile_( "/LF/Bulk/Data/Local/All/VNWandColorCalibration.json" )
    ,    controller_(0)
{

	// create and overwrite defaulat configuration file
	std::ofstream defaultConfigurationFile( kWandColorConfigurationFile );
	defaultConfigurationFile << std::string( kWandDefaultColorConfiguration );
	defaultConfigurationFile.close();

	LF::Vision::VNInputParameters touchParams;
	touchParams.push_back(LF::Vision::VNInputParameter(LF::Vision::kVNVirtualTouchLearningRateKey, kLearningRate));
	touchParams.push_back(LF::Vision::VNInputParameter(LF::Vision::kVNVirtualTouchThresholdKey, kGrayIntensityThreshold));

	virtualTouch_ = new LF::Vision::VNVirtualTouch(&touchParams);

    // setup vision MPI
    LF::Vision::VNInputParameters params;
    // The VNWTMinWandArea represents the wand area, in pixels, at which the input domain is
    // smallest.
    params.push_back(LF::Vision::VNInputParameter("VNWTMinWandArea", 15.f));

    wandTracker_ = new LF::Vision::VNWandTracker(&params);
}

WandCalibrationAlgorithm::~WandCalibrationAlgorithm() {
    // clean hot spot listener
    LeapFrog::Brio::CEventMPI eventMPI;
    eventMPI.UnregisterEventListener(&hotspotListener_);
}

void WandCalibrationAlgorithm::Initialize(LeapFrog::Brio::U16 frameProcessingWidth, LeapFrog::Brio::U16 frameProcessingHeight) {

    configurationFile_.Load();

    // // get the controller bluetooth address
    // assert( controller_ && "controller not set");
    const char* bluetoothAddress = controller_->GetBluetoothAddress();




    // set to default configuration
    defaultWandConfiguration_ = configurationFile_.mRoot["controllers"]["default"];
    if( configurationFile_.mRoot["controllers"].isMember(bluetoothAddress)) {
        wandConfiguration_ = configurationFile_.mRoot["controllers"][bluetoothAddress];
    } else {
        wandConfiguration_ = defaultWandConfiguration_;
    }

    // virtualTouch_->Initialize( frameProcessingWidth, frameProcessingHeight );
    wandTracker_->Initialize( frameProcessingWidth, frameProcessingHeight );

/*
    // // setup the camera for low exposure
    LeapFrog::Brio::tCameraControls controls;
    LeapFrog::Brio::CCameraMPI cameraMPI;

    LeapFrog::Brio::Boolean err = cameraMPI.GetCameraControls(controls);

    //turn off autowhitebalance
    LeapFrog::Brio::tControlInfo *awb = FindCameraControl(controls, LeapFrog::Brio::kControlTypeAutoWhiteBalance);
    if (awb) {
        printf("AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
        cameraMPI.SetCameraControl(awb, 0); // is a boolean, set to 0 for false
        printf("New AutoWhiteBalance (min,max,preset,current): %li %li %li %li\n", awb->min, awb->max, awb->preset, awb->current);
    }

    // turn off auto exposure
    LeapFrog::Brio::tControlInfo *ae = FindCameraControl(controls,
    LeapFrog::Brio::kControlTypeAutoExposure);
    if (ae) {
        printf("AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
        cameraMPI.SetCameraControl(ae, 1); // V4L2_EXPOSURE_MANUAL == 1
        printf("New AutoExposure (min,max,preset,current): %li %li %li %li\n", ae->min, ae->max, ae->preset, ae->current);
    }

    // set exposure to minimum
    LeapFrog::Brio::tControlInfo *e = FindCameraControl(controls,
    LeapFrog::Brio::kControlTypeExposure);
    if (e) {
        printf("Exposure (min,max,preset,current): %li %li %li %li\n", e->min, e->max, e->preset, e->current);
        cameraMPI.SetCameraControl(e, e->min);
		//cameraMPI.SetCameraControl(e, 100);
        printf("New Exposure(min,max,preset,current) : %li %li %li %li\n", e->min, e->max, e->preset, e->current);
    }
*/
	// Register for hotspot events
	LeapFrog::Brio::CEventMPI eventMPI;
	eventMPI.RegisterEventListener(&hotspotListener_);
	hotspotListener_.parent_ = this;


}

void* WandCalibrationAlgorithm::CalibrateThread(void* arg) {

    WandCalibrationAlgorithm* me = (WandCalibrationAlgorithm*)arg;

    //me->bestColor_ = me->calibrationColor_;
    int searchRange = kHueRange/3;
    cv::Scalar min( me->bestColor_.min[1] - searchRange, me->bestColor_.min[2] - searchRange );
    if( min[0] < 0 )
        min[0] = 0;
    if( min[1] < 0 )
        min[1] = 0;

    cv::Scalar max( me->bestColor_.max[1] + searchRange, me->bestColor_.max[2] + searchRange );
    if( max[0] > 255 )
        max[0] = 255;
    if( max[1] > 255 )
        max[1] = 255;

    calibrateColorInRange( me->bestColor_, me->yuv_, me->roi_, min, max, me->foregroundMask_ );

    printf("\a\a\aBEST COLOR: min(%f,%f) max(%f, %f)\n", me->bestColor_.min[1], me->bestColor_.min[2], me->bestColor_.max[1], me->bestColor_.max[2] );


    me->wandConfiguration_[me->bestColor_.name]["ymin"] = me->bestColor_.min[0];
    me->wandConfiguration_[me->bestColor_.name]["ymax"] = me->bestColor_.max[0];
    me->wandConfiguration_[me->bestColor_.name]["umin"] = me->bestColor_.min[1];
    me->wandConfiguration_[me->bestColor_.name]["umax"] = me->bestColor_.max[1];
    me->wandConfiguration_[me->bestColor_.name]["vmin"] = me->bestColor_.min[2];
    me->wandConfiguration_[me->bestColor_.name]["vmax"] = me->bestColor_.max[2];

    me->GotoState( kWandColorFinishedCalibration, me->color_ );

    /*
    // me->bestColor_ = me->calibrationColor_;

    me->maxPixelCount_ = 0;

    cv::Mat bestRangeMask;


    for( int umin = 0, umax = kHueRange; umax < 255; umin++, umax++ ) {
        for( int vmin = 0, vmax = kHueRange; vmax < 255; vmin++, vmax++ ) {
            me->calibrationColor_.min[1] = umin;
            me->calibrationColor_.max[1] = umax;
            me->calibrationColor_.min[2] = vmin;
            me->calibrationColor_.max[2] = vmax;

            int sum = calculatePixelSumForHueRange( me->calibrationColor_, me->yuv_, me->roi_, me->foregroundMask_ );
            //std::cout << "sum: " << sum << " umin: " << umin << " umax: " << umax << " vmin: " << vmin  << " vmax: " << vmax << std::endl;
            if( sum > me->maxPixelCount_ ) {
                me->bestColor_ = me->calibrationColor_;
                std::cout << "\asum: " << sum << " umin: " << umin << " umax: " << umax << " vmin: " << vmin  << " vmax: " << vmax << std::endl;
            }
        }
    }
    */

    // assert(0);
    return kNull;
}

void WandCalibrationAlgorithm::Execute(cv::Mat &frame, cv::Mat &foregroundMask) {
    LF::Vision::VNVisionMPI  visionMPI;
    switch(state_) {
        case kWaitForWandTrigger: {
#if USE_VIRTUAL_TOUCH_TO_CAPTURE
            virtualTouch_->Execute(frame, foregroundMask);
#else
            wandTracker_->Execute(frame, foregroundMask);
#endif
			frame.copyTo(frame_);
			if( foregroundMask.total() > 0 )
				foregroundMask.copyTo(foregroundMask_);
            warmUpFrameCount_--;
        } break;


        case kWandVerifyColor: {
	    std::cout << ".";
            wandTracker_->Execute(frame, foregroundMask);
            frame.copyTo(frame_);
            if( foregroundMask.total() > 0 )
                foregroundMask.copyTo(foregroundMask_);
        } break;

        case kCalibratingColor: {
            foregroundMask = cv::Scalar::all(0);
        } break;

        default:
            frame.copyTo(frame_);



    }

    if( postExecuteCallback_ ) {
        (*postExecuteCallback_)(frame_, foregroundMask_, postExecuteCallbackUserData_);
    }

    if( stateChanged_ ) {
        switch( state_ ) {
            case kWandTriggered:
                visionMPI.RemoveHotSpot( hotSpots_[0] );
            break;

			case kWandVerifyColor: {
				if( previousState_ != kWandVerifyHotspotTriggered ) {    // only setup hotspots once

					triggeredVerifyHotspot_ = 0;
					visionMPI.RemoveAllHotSpots();

                    SetHotSpotRects( hotSpotRects_ );

					// // note +1 to skip first hotspot reserved for wand trigger
					for( std::vector<LF::Vision::VNRectHotSpot*>::iterator it = hotSpots_.begin()+1; it != hotSpots_.end(); it++ ) {
					 	visionMPI.AddHotSpot(*it);
					}
				} else {
					visionMPI.RemoveHotSpot(triggeredVerifyHotspot_);
				}

			} break;



        }
		stateChanged_ = false;
		previousState_ = state_;

        if( stateChangedCallback_ ) {
            (*stateChangedCallback_)( state_, color_, stateChangedCallbackUserData_ );
        }
    }

}

void WandCalibrationAlgorithm::GotoState(WandCalibrationAlgorithm::CalibrationState gotostate, WandCalibrationAlgorithm::CalibrationColorState gotocolor) {

    LF::Vision::VNVisionMPI  visionMPI;

    std::cout << "gotostate: " << gotostate << " gotocolor: " << gotocolor << std::endl;

    if( gotostate == state_ && gotocolor == color_ ) {
        std::cout << "already in state and color" << std::endl;
        return;
    }

    WandCalibrationAlgorithm::CalibrationState previousState = state_;

    switch( previousState ) {
        case kWaitForWandTrigger: {
            warmUpFrameCount_ = kWarmUpFrameCount;

            // un register for hotspot events
            // LeapFrog::Brio::CEventMPI eventMPI;
            // eventMPI.UnregisterEventListener(&hotspotListener_);

        } break;

    }

    switch( gotostate ) {
        case kWaitForWandTrigger: {
            warmUpFrameCount_ = kWarmUpFrameCount;

            // // Register for hotspot events
            // LeapFrog::Brio::CEventMPI eventMPI;
            // eventMPI.RegisterEventListener(&hotspotListener_);
            // hotspotListener_.parent_ = this;

            visionMPI.AddHotSpot(hotSpots_[0]);    // add just the first hotspot


        } break;

        case kWandTriggered: {


            cv::Mat rgb;
            //LF::Vision::YUYV2RGB( frame_, rgb );
            //LF::Vision::RGBToHSV( rgb, hsv_ );
            //cv::cvtColor(frame_, rgb, CV_YUV2BGR_YUYV);
            //cv::cvtColor(rgb, hsv_, CV_BGR2HSV);
            LF::Vision::YUYV2YUV( frame_, yuv_ );
            maxPixelCount_ = 0;
            calibrationColor_ = calibrationColors[color_];


            std::cout << "Setting Color: " << calibrationColor_.name << " color idx: " << color_ << std::endl;
            bestColor_.name = calibrationColor_.name;
            bestColor_.min[0] = defaultWandConfiguration_[calibrationColor_.name]["ymin"].asInt();
            bestColor_.max[0] = defaultWandConfiguration_[calibrationColor_.name]["ymax"].asInt();
            bestColor_.min[1] = defaultWandConfiguration_[calibrationColor_.name]["umin"].asInt();
            bestColor_.max[1] = defaultWandConfiguration_[calibrationColor_.name]["umax"].asInt();
            bestColor_.min[2] = defaultWandConfiguration_[calibrationColor_.name]["vmin"].asInt();
            bestColor_.max[2] = defaultWandConfiguration_[calibrationColor_.name]["vmax"].asInt();

        	tTaskHndl 		hndl;
        	tTaskProperties	prop;


        	// Setup task properties
        	prop.TaskMainFcn			= (void* (*)(void*))WandCalibrationAlgorithm::CalibrateThread;
        	prop.taskMainArgCount		= 1;
        	prop.pTaskMainArgValues		= this;

            CKernelMPI kernel;
        	tErrType error = kernel.CreateTask( hndl, prop, NULL );

            gotostate = kCalibratingColor;

        } break;


        case kWandVerifyHotspotTriggered: {
            visionMPI.RemoveHotSpot(triggeredVerifyHotspot_);
        } break;


    }

	previousState_ = previousState;
    state_ = gotostate;
    color_ = gotocolor;
    stateChanged_ = true;


}

void WandCalibrationAlgorithm::SetHotSpotRects( std::vector<LeapFrog::Brio::tRect>& rects ) {
    LF::Vision::VNVisionMPI  visionMPI;
    hotSpots_.clear();
    hotSpotRects_ = rects;
    visionMPI.RemoveAllHotSpots();

#if USE_VIRTUAL_TOUCH_TO_CAPTURE
    LF::Vision::VNOcclusionTrigger* occlusionTrigger = new LF::Vision::VNOcclusionTrigger();
    occlusionTrigger->SetOcclusionTriggerPercentage(kOcclusionTriggerPercentage);
#endif

    LF::Vision::VNPointTrigger* pointTrigger = new LF::Vision::VNPointTrigger();

    int i = 0;
    for( std::vector<LeapFrog::Brio::tRect>::iterator it = rects.begin(); it != rects.end(); it++, i++ ) {
        LF::Vision::VNRectHotSpot* hotspot = new LF::Vision::VNRectHotSpot(*it);
        hotspot->SetTag(kCalibrationHotspotTag + i);

        hotSpots_.push_back(hotspot);


#if USE_VIRTUAL_TOUCH_TO_CAPTURE
        if( i == 0 ) {    // only calculate rect from first hotspot.  it is assumed to be the calibration hotspot
            hotspot->SetTrigger(occlusionTrigger);
            // get the cv::Rect version
            LeapFrog::Brio::tRect r = LF::Vision::VNCoordinateTranslator::Instance()->FromDisplayToVision(*it);

            // store in rect_
            roi_.x = r.left;
            roi_.y = r.top;
            roi_.width = fabs(r.right - r.left);
            roi_.height = fabs(r.bottom - r.top);

        } else {    // the verification hotspots
            hotspot->SetTrigger(pointTrigger);
        }
#else // USE_VIRTUAL_TOUCH_TO_CAPTURE
		hotspot->SetTrigger(pointTrigger);

		if( i == 0 ) {    // only calculate rect from first hotspot.  it is assumed to be the calibration hotspot
			// get the cv::Rect version
			LeapFrog::Brio::tRect r = LF::Vision::VNCoordinateTranslator::Instance()->FromDisplayToVision(*it);

			// store in rect_
			roi_.x = r.left;
			roi_.y = r.top;
			roi_.width = fabs(r.right - r.left);
			roi_.height = fabs(r.bottom - r.top);

		}

#endif // USE_VIRTUAL_TOUCH_TO_CAPTURE

    }
}

void WandCalibrationAlgorithm::ResetHotSpots() {
    // LF::Vision::VNVisionMPI  visionMPI;
    // for( std::vector<LF::Vision::VNRectHotSpot*>::iterator it = hotSpots_.end(); it != hotSpots_.end(); it++ ) {
    //     visionMPI.AddHotSpot(*it);
    // }
}

void WandCalibrationAlgorithm::SaveCalibration() {
    // get the controller bluetooth address
    const char* bluetoothAddress = controller_->GetBluetoothAddress();

    configurationFile_.mRoot["controllers"][bluetoothAddress] = wandConfiguration_; // bugbug hardwired controller
    configurationFile_.Save();
}



const tEventType
gHotSpotEventKey[] = {LF::Vision::kVNHotSpotTriggeredEvent};

WandCalibrationAlgorithm::HotSpotListener::HotSpotListener()
    : IEventListener(gHotSpotEventKey, 5) {
}

LeapFrog::Brio::tEventStatus WandCalibrationAlgorithm::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg) {

    LF::Vision::VNVisionMPI  visionMPI;
    const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
    const LF::Vision::VNHotSpot* hs = hsMessage.GetHotSpot();

    if( parent_->state_ == kWaitForWandTrigger ) {
        if ( parent_->warmUpFrameCount_ < 0 && hs->IsTriggered()) {

            std::cout << "\aHotSpot:" << hs->GetTag() << std::endl;
            if( hs->GetTag() == kCalibrationHotspotTag ) {
                parent_->GotoState( kWandTriggered, parent_->color_ );
            }

        }
    } else if( parent_->state_ == kWandVerifyColor ) {

        if( hs->IsTriggered() && hs->GetTag() > kCalibrationHotspotTag && hs != parent_->triggeredVerifyHotspot_) {
			std::cout << "\aHotSpot:" << hs->GetTag() << std::endl;
            parent_->GotoState( kWandVerifyHotspotTriggered, parent_->color_ );
            parent_->triggeredVerifyHotspot_ = (LF::Vision::VNHotSpot*)hs;
        }
    }


    return kEventStatusOK;
}
