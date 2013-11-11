#include <Vision/VNCompoundTrigger.h>
#include <VNCompoundTriggerPIMPL.h>

namespace LF {
namespace Vision {

	VNCompoundTrigger::VNCompoundTrigger(void) :
		pimpl_(new VNCompoundTriggerPIMPL()) {

	}

    VNCompoundTrigger::VNCompoundTrigger(VNCompoundTriggerType type,
		      	  	  	  	  	  	  	 const std::vector<const VNTrigger*>& subTriggers) :
		pimpl_(new VNCompoundTriggerPIMPL(type, subTriggers)) {

    }

    VNCompoundTrigger::~VNCompoundTrigger(void) {
	}

	bool
	VNCompoundTrigger::Triggered(const VNHotSpot& hotSpot) {
		// TODO:
		return false;
	}

    void
    VNCompoundTrigger::SetCompoundTriggerType(VNCompoundTriggerType type) {
    	pimpl_->type_ = type;
    }

    VNCompoundTriggerType
    VNCompoundTrigger::GetCompoundTriggerType(void) const {
    	return pimpl_->type_;
    }

    void
    VNCompoundTrigger::SetSubTriggers(const std::vector<const VNTrigger*>& subTriggers) {
    	pimpl_->triggers_ = subTriggers;
    }

    std::vector<const VNTrigger*>&
    VNCompoundTrigger::GetSubTriggers(void) const {
    	return pimpl_->triggers_;
    }

} // namespace Vision
} // namespace LF
