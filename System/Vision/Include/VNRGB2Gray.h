#ifndef __VNRGB2Gray_h__
#define __VNRGB2Gray_h__
#include <opencv2/opencv.hpp>
namespace LF {
    namespace Vision {

        void RGB2Gray(const cv::Mat& input, cv::Mat& output);
    }
}
#endif
