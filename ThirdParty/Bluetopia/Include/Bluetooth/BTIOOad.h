#ifndef __BTIO_OAD_H__
#define __BTIO_OAD_H__

#include <stdint.h>

//Constants for the Controller binary image file
#define BASE_CONTROLLER_FW_PATH 		"/LF/Bulk/Data/Controller/"
#define CONTROLLER_FW_FILE_EXTENSION	".bin"
#define CONTROLLER_VERSION_NUM_LENGTH	4
#define BASE_CONTROLLER_FW_A_BASE_NAME "ControllerA_"
#define BASE_CONTROLLER_FW_B_BASE_NAME "ControllerB_"

//OAD Profile Information
#define ATT_UUID_SIZE 			128
#define KEY_BLENGTH    			16
#define OAD_IMG_CRC_OSET      	0x0000
#define OAD_IMG_HDR_OSET      	0x0002
#define OAD_CHAR_CNT          	2

#define OAD_CHAR_IMG_NOTIFY   	0
#define OAD_CHAR_IMG_BLOCK    	1

#define OAD_LOCAL_CHAR        	0 			// Local OAD characteristics
#define OAD_DISC_CHAR         	1 			// Discovered OAD characteristics

#define OAD_IMG_ID_SIZE       	4 			// Image Identification size
#define OAD_IMG_HDR_SIZE      	( 2 + 2 + OAD_IMG_ID_SIZE )	// Image header size (version + length + image id size)
#define OAD_BLOCK_SIZE        	16 			// The Image is transporte in 16-byte blocks in order to avoid using blob operations.
#define OAD_BLOCKS_PER_PAGE  	(HAL_FLASH_PAGE_SIZE / OAD_BLOCK_SIZE)
#define OAD_BLOCK_MAX        	(OAD_BLOCKS_PER_PAGE * OAD_IMG_D_AREA)

typedef struct {
  uint16_t crc1;       						// CRC-shadow must be 0xFFFF.  
  uint16_t ver;								// User-defined Image Version Number
  uint16_t len;        						// Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks).
  uint8_t  uid[4];     						// User-defined Image Identification bytes.
  uint8_t  res[4];     						// Reserved space for future use.
} img_hdr_t;


// OAD Parameter IDs
#define OAD_LOCAL_CHAR_NOTIFY 	1 			// Handle for local Image Notify characteristic. Read only. size uint16.
#define OAD_LOCAL_CHAR_BLOCK  	2 			// Handle for local Image Block characteristic. Read only. size uint16.
#define OAD_DISC_CHAR_NOTIFY  	3 			// Handle for discovered Image Notify characteristic. Read/Write. size uint16.
#define OAD_DISC_CHAR_BLOCK   	4 			// Handle for discovered Image Block characteristic. Read/Write. size uint16.

//BTIO OAD Condition codes - this enumeration is intended to generally mirror LF::Hardware::HWFwUpdateResult
//for simplicity, not all codes necessarily used at the BTIO layer
enum OadResult {
	kOadSuccess = 0, 						//The operation was successful
	kOadFailure,							//The operation was not done
	kOadMissingImage, 						//The Controller binary image needed was not found
	kOadCorruptImage,						//The Controller binary image needed was found to be corrupt in some way
	kOadLowBattery,							//The Controller battery is too low to complete the update process reliably
	kOadExcessiveNoise,						//The update process failed as there is too much RF noise in the environment
	kOadPrematureDisconnect,				//The Controller disconnected unexpectedly before the update process was finished
	kOadAlreadyInProgress					//There is an update already in progress, only one allowed currently
};

//Function pointer prototypes
typedef void (*pFnOadUpdateCallback)( OadResult, unsigned int );
typedef OadResult (*pFnUpdateControllerFw)(char *, unsigned int, pFnOadUpdateCallback);
typedef OadResult (*pFnAbortUpdateControllerFw)(void);

#ifdef USE_EXPORTS

extern pFnUpdateControllerFw 			pBTIO_UpdateControllerFw;
extern pFnAbortUpdateControllerFw 		pBTIO_AbortUpdateControllerFw;

#define BTIO_UpdateControllerFw 		pBTIO_UpdateControllerFw
#define BTIO_AbortUpdateControllerFw 	pBTIO_AbortUpdateControllerFw

#else

extern "C" {

OadResult BTIO_UpdateControllerFw(char* btaddr, unsigned int version, pFnOadUpdateCallback);
OadResult BTIO_AbortUpdateControllerFw(void);

}

#endif // USE_EXPORTS

#endif //__BTIO_OAD_H__
