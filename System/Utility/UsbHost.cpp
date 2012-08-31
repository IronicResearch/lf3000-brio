#include <UsbHost.h>
#include <Utility.h>
#if !defined(EMULATION) && defined(LF1000)
#include <linux/lf1000/gpio_ioctl.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <sys/statvfs.h>

LF_BEGIN_BRIO_NAMESPACE()

boost::weak_ptr<CUsbHost> CUsbHost::mSingleton;

boost::shared_ptr<CUsbHost> CUsbHost::Instance()
{
	boost::shared_ptr<CUsbHost> result = mSingleton.lock();
	if(!result)
	{
		result.reset(new CUsbHost());
		mSingleton = result;
	}
	return result;
}

CUsbHost::CUsbHost()
{
	// USB host power option on Madrid only
	if (GetPlatformName() != "Madrid")
		return;
	// Set USB host enable via GPIO
	int fd = open("/dev/gpio", O_RDWR | O_SYNC);
	if (fd > -1) {
		int r;
		union gpio_cmd c;
		c.outvalue.port  = 2;
		c.outvalue.pin 	 = 0;
		c.outvalue.value = 0;
		r = ioctl(fd, GPIO_IOCSOUTVAL, &c);
		close(fd);
	}
}

CUsbHost::~CUsbHost()
{
	// USB host power option on Madrid only
	if (GetPlatformName() != "Madrid")
		return;
	// Set USB host enable via GPIO
	int fd = open("/dev/gpio", O_RDWR | O_SYNC);
	if (fd > -1) {
		int r;
		union gpio_cmd c;
		c.outvalue.port  = 2;
		c.outvalue.pin 	 = 0;
		c.outvalue.value = 1;
		r = ioctl(fd, GPIO_IOCSOUTVAL, &c);
		close(fd);
	}
}

LF_END_BRIO_NAMESPACE()
