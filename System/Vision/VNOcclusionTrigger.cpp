#include <Vision/VNOcclusionTrigger.h>
#include <VNOcclusionTriggerPIMPL.h>
#include <opencv2/opencv.hpp>
#include <Vision/VNHotSpot.h>
#include <VNHotSpotPIMPL.h>

namespace LF {
namespace Vision {

	VNOcclusionTrigger::VNOcclusionTrigger(void) :
		pimpl_(new VNOcclusionTriggerPIMPL()) {

	}

	VNOcclusionTrigger::VNOcclusionTrigger(float percentOccluded) :
		pimpl_(new VNOcclusionTriggerPIMPL(percentOccluded)) {

	}

	VNOcclusionTrigger::~VNOcclusionTrigger(void) {

	}

	bool
	VNOcclusionTrigger::Triggered(const VNHotSpot& hotSpot) {
		assert(hotSpot.pimpl_->triggerImage_.type() == CV_8U);
		int numDiffPixels = 0;
		for (int i = 0; i < hotSpot.pimpl_->triggerImage_.rows; ++i) {
			uchar* rowData = hotSpot.pimpl_->triggerImage_.ptr<uchar>(i);
			for (int j = 0; j < hotSpot.pimpl_->triggerImage_.cols; ++j) {
				if (rowData[j] == static_cast<uchar>(0)) {
					numDiffPixels++;
				}
			}
		}

		int numPixels = hotSpot.pimpl_->triggerImage_.rows*hotSpot.pimpl_->triggerImage_.cols;
		// if the percentage of occluded pixels is greater than the threshold to trigger, return true.
		return ((1.0f - (static_cast<float>(numPixels - numDiffPixels)/static_cast<float>(numPixels))) > pimpl_->percentOccludedToTrigger_);
	}

	void
	VNOcclusionTrigger::SetOcclusionPercentage(float percentOccluded) {
		pimpl_->percentOccludedToTrigger_ = percentOccluded;
	}

	float
	VNOcclusionTrigger::GetOcclusionPercentage(void) const {
		return pimpl_->percentOccludedToTrigger_;
	}

} // namespace Vision
} // namespace LF
