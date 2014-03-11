//============================================================================
// Name        : lf-input-daemon.cpp
// Author      : Micah Pearlman
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <cwiid.h>
#include <bluetooth/bluetooth.h>
#include <unistd.h>

using namespace std;

#define ACCELEROMETER_DEVICE_NAME 	"Accelerometer"
#define BUTTON_DEVICE_NAME 			"gpio-keys"
#define ANALOG_STICK_DEVICE_NAME 	"analog-stick"
#define ACCELEROMETER_FD_INDEX 		0
#define BUTTON_FD_INDEX 			1
#define ANALOG_STICK_FD_INDEX 		2
#define INPUT_FD_COUNT 				3

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

volatile sig_atomic_t stop = 0;

void
inthand(int signum)
{
    stop = 1;
}

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
	int i, j;
	int valid_source;

	int* input_fds = (int*)cwiid_get_data( wiimote );

	static uint16_t previous_buttons = 0;

	for (i=0; i < mesg_count; i++) {

		switch (mesg[i].type) {
		case CWIID_MESG_STATUS:
			printf("Status Report: battery=%d extension=",
			       mesg[i].status_mesg.battery);
			switch (mesg[i].status_mesg.ext_type) {
			case CWIID_EXT_NONE:
				printf("none");
				break;
			case CWIID_EXT_NUNCHUK:
				printf("Nunchuk");
				break;
			case CWIID_EXT_CLASSIC:
				printf("Classic Controller");
				break;
			case CWIID_EXT_BALANCE:
				printf("Balance Board");
				break;
			case CWIID_EXT_MOTIONPLUS:
				printf("MotionPlus");
				break;
			default:
				printf("Unknown Extension");
				break;
			}
			printf("\n");
			break;
		case CWIID_MESG_BTN: {
			printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			// press down
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_UP ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_UP;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_DOWN ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_DOWN;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_LEFT ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_LEFT;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_RIGHT ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_RIGHT;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_HOME ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_M;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_A ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_A;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));

			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_B ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_B;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));

			}
			if( mesg[i].btn_mesg.buttons & CWIID_BTN_1 ) {
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_KEY;
				ev.code = KEY_H;
				ev.value = 1;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
			}

			// BUGBUG this won't work with multi button press should go through all previous buttons.
			// this only works when all buttons are up!
			if( mesg[i].btn_mesg.buttons == 0 && previous_buttons ) {	// button up
				if( previous_buttons & CWIID_BTN_UP ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_UP;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_DOWN ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_DOWN;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_LEFT ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_LEFT;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_RIGHT ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_RIGHT;
					ev.value =0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_HOME ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_M;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_A ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_A;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_B ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_B;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
				if( previous_buttons & CWIID_BTN_1 ) {
					struct input_event ev;
					memset( &ev, 0, sizeof(ev) );
					gettimeofday( &ev.time, NULL );
					ev.type = EV_KEY;
					ev.code = KEY_H;
					ev.value = 0;
					write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev));
				}
			}

			previous_buttons = mesg[i].btn_mesg.buttons;

			{
				struct input_event ev;
				memset( &ev, 0, sizeof(ev) );
				gettimeofday( &ev.time, NULL );
				ev.type = EV_SYN;
				ev.code = SYN_REPORT;
				write( input_fds[BUTTON_FD_INDEX], &ev, sizeof(ev) );

			}

		} break;
		case CWIID_MESG_ACC: {
			struct input_event ev;

			memset( &ev, 0, sizeof(ev) );
			gettimeofday( &ev.time, NULL );
			ev.type = EV_ABS;
			ev.code = ABS_X;
			ev.value = 128 - mesg[i].acc_mesg.acc[CWIID_X];
			write( input_fds[ACCELEROMETER_FD_INDEX], &ev, sizeof(struct input_event) );

			ev.code = ABS_Y;
			ev.value = 128 - mesg[i].acc_mesg.acc[CWIID_Y];
			write( input_fds[ACCELEROMETER_FD_INDEX], &ev, sizeof(struct input_event) );

			ev.code = ABS_Z;
			ev.value = 128 - mesg[i].acc_mesg.acc[CWIID_Z];
			write( input_fds[ACCELEROMETER_FD_INDEX], &ev, sizeof(struct input_event) );

			memset( &ev, 0, sizeof(ev) );
			gettimeofday( &ev.time, NULL );
			ev.type = EV_SYN;
			write( input_fds[ACCELEROMETER_FD_INDEX], &ev, sizeof(struct input_event) );


//			printf("Acc Report: x=%d, y=%d, z=%d\n",
//                   mesg[i].acc_mesg.acc[CWIID_X],
//			       mesg[i].acc_mesg.acc[CWIID_Y],
//			       mesg[i].acc_mesg.acc[CWIID_Z]);
		} break;
		case CWIID_MESG_IR:
			printf("IR Report: ");
			valid_source = 0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source = 1;
					printf("(%d,%d) ", mesg[i].ir_mesg.src[j].pos[CWIID_X],
					                   mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
				}
			}
			if (!valid_source) {
				printf("no sources detected");
			}
			printf("\n");
			break;
		case CWIID_MESG_NUNCHUK: {

			struct input_event ev;

			memset( &ev, 0, sizeof(ev) );
			gettimeofday( &ev.time, NULL );

			ev.type = EV_ABS;
			ev.code = ABS_HAT0X;
			ev.value = mesg[i].nunchuk_mesg.stick[CWIID_X];
			write( input_fds[ANALOG_STICK_FD_INDEX], &ev, sizeof(struct input_event) );

			ev.type = EV_ABS;
			ev.code = ABS_HAT0Y;
			ev.value = mesg[i].nunchuk_mesg.stick[CWIID_Y];
			write( input_fds[ANALOG_STICK_FD_INDEX], &ev, sizeof(struct input_event) );

			memset( &ev, 0, sizeof(ev) );
			gettimeofday( &ev.time, NULL );
			ev.type = EV_SYN;
			write( input_fds[ANALOG_STICK_FD_INDEX], &ev, sizeof(struct input_event) );

			//printf("Nunchuk Report: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
			 //      "acc.z=%d\n", mesg[i].nunchuk_mesg.buttons,
			 //      mesg[i].nunchuk_mesg.stick[CWIID_X],
			 //      mesg[i].nunchuk_mesg.stick[CWIID_Y],
			 //      mesg[i].nunchuk_mesg.acc[CWIID_X],
			 //      mesg[i].nunchuk_mesg.acc[CWIID_Y],
			 //      mesg[i].nunchuk_mesg.acc[CWIID_Z]);
		}
			break;
		case CWIID_MESG_CLASSIC:
			printf("Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
			       "l=%d r=%d\n", mesg[i].classic_mesg.buttons,
			       mesg[i].classic_mesg.l_stick[CWIID_X],
			       mesg[i].classic_mesg.l_stick[CWIID_Y],
			       mesg[i].classic_mesg.r_stick[CWIID_X],
			       mesg[i].classic_mesg.r_stick[CWIID_Y],
			       mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
			break;
		case CWIID_MESG_BALANCE:
			printf("Balance Report: right_top=%d right_bottom=%d "
			       "left_top=%d left_bottom=%d\n",
			       mesg[i].balance_mesg.right_top,
			       mesg[i].balance_mesg.right_bottom,
			       mesg[i].balance_mesg.left_top,
			       mesg[i].balance_mesg.left_bottom);
			break;
		case CWIID_MESG_MOTIONPLUS:
			// BUGBUG: low_speed field does not look like it is implemented
//			printf("MotionPlus Report: angle_rate=(%d,%d,%d) low_speed=(%d,%d,%d)\n",
//			       mesg[i].motionplus_mesg.angle_rate[0],
//			       mesg[i].motionplus_mesg.angle_rate[1],
//			       mesg[i].motionplus_mesg.angle_rate[2],
//			       mesg[i].motionplus_mesg.low_speed[0],
//			       mesg[i].motionplus_mesg.low_speed[1],
//			       mesg[i].motionplus_mesg.low_speed[2]);
			break;
		case CWIID_MESG_ERROR:
			if (cwiid_close(wiimote)) {
				fprintf(stderr, "Error on wiimote disconnect\n");
				exit(-1);
			}
			exit(0);
			break;
		default:
			printf("Unknown Report");
			break;
		}
	}
}

// see: https://linuxlink.timesys.com/files/webinars/code/wiisensor
// see: http://www.cmpe.boun.edu.tr/medialab/VW/Wiimote_Linux_integration.pdf
cwiid_wiimote_t* setup_wiimote( int* input_fds ) {

	cwiid_wiimote_t* wiimote;

	//Do this little dance to fix broken BDADDR_ANY in C++
	bdaddr_t bdaddr;
	memset(&bdaddr, 0, sizeof(bdaddr_t));

	printf( "#### PRESS 1 + 2 ON THE WIIMOTE NOW!!! ####\n");
	wiimote = cwiid_open( &bdaddr, CWIID_FLAG_MESG_IFC | CWIID_FLAG_CONTINUOUS );
	if( !wiimote ) {
		fprintf( stderr, "Could not initialize Wiimote\n" );
		return wiimote;
	}

	if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
		fprintf(stderr, "Unable to set Wiimote message callback\n");
	}

	unsigned char rpt_mode = CWIID_RPT_ACC | CWIID_RPT_BTN | CWIID_RPT_STATUS | CWIID_RPT_NUNCHUK;

	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}

	// set our input file descriptors as our user data so we can write in the callback above
	cwiid_set_data( wiimote, input_fds );

	//Turn on all LEDS on the remote
	cwiid_set_led(wiimote, CWIID_LED1_ON | CWIID_LED2_ON | CWIID_LED3_ON | CWIID_LED4_ON);

	//Vibrate the remote for 1 sec
	if (cwiid_set_rumble(wiimote, 1)) {
		fprintf(stderr, "Error setting rumble\n");
	}
	sleep(1);

	if (cwiid_set_rumble(wiimote, 0)) {
		fprintf(stderr, "Error setting rumble\n");
	}

	//Turn on all LEDS on the remote
	cwiid_set_led(wiimote, CWIID_LED2_ON | CWIID_LED3_ON );

	return wiimote;
}

int setup_analog_stick_userdevice() {
	int fd = open( "/dev/uinput", O_WRONLY );
	if( fd < 0 ) {
		fprintf(stderr, "Could not open /dev/uinput.  Check the /dev/uinput group and make sure you are a group member or run as root\n" );
		return fd;
	}

	int ret = 0;
	ret = ioctl( fd, UI_SET_EVBIT, EV_ABS );
	ret = ioctl( fd, UI_SET_ABSBIT, ABS_HAT0X );
	ret = ioctl( fd, UI_SET_ABSBIT, ABS_HAT0Y );
	ret = ioctl( fd, UI_SET_EVBIT, EV_SYN );

	// setup accelerometer
	struct uinput_user_dev userdevice;
	memset( &userdevice, 0, sizeof( struct uinput_user_dev) );

	snprintf( userdevice.name, UINPUT_MAX_NAME_SIZE, ANALOG_STICK_DEVICE_NAME );
	userdevice.id.bustype = BUS_BLUETOOTH;
	userdevice.id.vendor = 0x1234;
	userdevice.id.product = 0xfedc;
	userdevice.id.version = 1;
	userdevice.absmin[ABS_X]=0;
	userdevice.absmax[ABS_X]=255;
	userdevice.absfuzz[ABS_X]=0;
	userdevice.absflat[ABS_X ]=0;
	userdevice.absmin[ABS_Y]=0;
	userdevice.absmax[ABS_Y]=255;
	userdevice.absfuzz[ABS_Y]=0;
	userdevice.absflat[ABS_Y]=0;
	userdevice.absmin[ABS_Z]=0;
	userdevice.absmax[ABS_Z]=255;
	userdevice.absfuzz[ABS_Z]=0;
	userdevice.absflat[ABS_Z]=0;


	ret = write( fd, &userdevice, sizeof(userdevice) );

	ret = ioctl( fd, UI_DEV_CREATE );

	return fd;

}
int setup_accelerometer_userdevice() {
	int fd = open( "/dev/uinput", O_WRONLY );
	if( fd < 0 ) {
		fprintf(stderr, "Could not open /dev/uinput.  Check the /dev/uinput group and make sure you are a group member or run as root\n" );
		return fd;
	}

	/// uinput info: http://thiemonge.org/getting-started-with-uinput

	// turn on absolute events for accelerometer
	int ret = 0;
	ret = ioctl( fd, UI_SET_EVBIT, EV_ABS );
	ret = ioctl( fd, UI_SET_ABSBIT, ABS_X );
	ret = ioctl( fd, UI_SET_ABSBIT, ABS_Y );
	ret = ioctl( fd, UI_SET_ABSBIT, ABS_Z );
	ret = ioctl( fd, UI_SET_EVBIT, EV_SYN );

	// setup accelerometer
	struct uinput_user_dev userdevice;
	memset( &userdevice, 0, sizeof( struct uinput_user_dev) );

	snprintf( userdevice.name, UINPUT_MAX_NAME_SIZE, ACCELEROMETER_DEVICE_NAME );
	userdevice.id.bustype = BUS_BLUETOOTH;
	userdevice.id.vendor = 0x1234;
	userdevice.id.product = 0xfedc;
	userdevice.id.version = 1;
	userdevice.absmin[ABS_X]=0;
	userdevice.absmax[ABS_X]=255;
	userdevice.absfuzz[ABS_X]=0;
	userdevice.absflat[ABS_X ]=0;
	userdevice.absmin[ABS_Y]=0;
	userdevice.absmax[ABS_Y]=255;
	userdevice.absfuzz[ABS_Y]=0;
	userdevice.absflat[ABS_Y]=0;
	userdevice.absmin[ABS_Z]=0;
	userdevice.absmax[ABS_Z]=255;
	userdevice.absfuzz[ABS_Z]=0;
	userdevice.absflat[ABS_Z]=0;


	ret = write( fd, &userdevice, sizeof(userdevice) );

	ret = ioctl( fd, UI_DEV_CREATE );

	return fd;

}

int setup_button_userdevice() {
	int fd = open( "/dev/uinput", O_WRONLY | O_NDELAY);
	if( fd < 0 ) {
		fprintf(stderr, "Could not open /dev/uinput.  Check the /dev/uinput group and make sure you are a group member or run as root\n" );
		return fd;
	}

	/// uinput info: http://thiemonge.org/getting-started-with-uinput

	// turn on absolute key events
	int ret = 0;
	ret = ioctl( fd, UI_SET_EVBIT, EV_KEY );
	ret = ioctl( fd, UI_SET_EVBIT, EV_SYN );

	// button maps
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_UP );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_DOWN );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_RIGHT );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_LEFT );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_A );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_B );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_L );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_R );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_M );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_H );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_P );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_X );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_VOLUMEDOWN );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_VOLUMEUP );
	ret = ioctl( fd, UI_SET_KEYBIT, KEY_ESC );

	// setup keyboard
	struct uinput_user_dev userdevice;
	memset( &userdevice, 0, sizeof( struct uinput_user_dev) );

	snprintf( userdevice.name, UINPUT_MAX_NAME_SIZE, BUTTON_DEVICE_NAME );
	userdevice.id.bustype = BUS_BLUETOOTH;
	userdevice.id.vendor = 0x1234;
	userdevice.id.product = 0xfedc;
	userdevice.id.version = 1;

	ret = write( fd, &userdevice, sizeof(userdevice) );

	ret = ioctl( fd, UI_DEV_CREATE );

	return fd;

}

int main( int argc, char* argv[] ) {

	// setup accelerometer
	int input_fds[INPUT_FD_COUNT];
	input_fds[ACCELEROMETER_FD_INDEX] = setup_accelerometer_userdevice();
	if( input_fds[ACCELEROMETER_FD_INDEX]  < 0 ) {
		fprintf( stderr, "Could not setup userdevice for accelerometer input" );
		exit(1);
	}

	// setup buttons
	input_fds[BUTTON_FD_INDEX] = setup_button_userdevice();
	if( input_fds[BUTTON_FD_INDEX]  < 0 ) {
		fprintf( stderr, "Could not setup userdevice for button input" );
		exit(1);
	}

	// setup analog stick (nunchuck)
	// setup buttons
	input_fds[ANALOG_STICK_FD_INDEX] = setup_analog_stick_userdevice();
	if( input_fds[ANALOG_STICK_FD_INDEX]  < 0 ) {
		fprintf( stderr, "Could not setup userdevice for analog stick (nunchuck) input" );
		exit(1);
	}


	// setup wiimote
	cwiid_wiimote_t* wiimote = setup_wiimote( input_fds );

	if ( !wiimote ) {
		fprintf( stderr, "Could not setup Wiimote" );
		exit(2);
	}

	printf( "Wiimote Connected!\n" );


	signal(SIGINT, inthand);
	while(!stop) {
		usleep( 1000*10 );
	}
	cwiid_close( wiimote );

	ioctl( input_fds[ACCELEROMETER_FD_INDEX], UI_DEV_DESTROY );
	close( input_fds[ACCELEROMETER_FD_INDEX] );

	ioctl( input_fds[BUTTON_FD_INDEX], UI_DEV_DESTROY );
	close( input_fds[BUTTON_FD_INDEX] );

	ioctl( input_fds[ANALOG_STICK_FD_INDEX], UI_DEV_DESTROY );
	close( input_fds[ANALOG_STICK_FD_INDEX] );

	printf( "Goodbye!\n" );

	return 0;
}
