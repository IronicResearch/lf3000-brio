// MfgData handler for NOR storage of Manufacturing Data on Emerald
// RDowling 7/6/2009
//

#ifndef _libMfgData_h_
#define _libMfgData_h_

typedef unsigned char U8;
typedef unsigned long U32;

// Checksum type
typedef U32 tCS;

// Fields in NOR partition
#define kOffsetTSCalData	0
#define kSizeTSCalData		(7*sizeof(int))
#define kOffsetBLCalData	28
#define kSizeBLCalData		(sizeof(int))
#define kOffsetADCCalData	32
#define kSizeADCCalData		(2*sizeof(int))
#define kOffsetAudioGain	40
#define kSizeAudioGain		(sizeof(int))
#define kOffsetSerialNumber	44
#define kSizeSerialNumber	24
#define kOffsetSNValidFlag	68
#define kSizeSNValidFlag	(sizeof(int))
#define kOffsetSirasBarcode	72
#define kSizeSirasBarcode	32
#define kOffsetLocale		(kOffsetSirasBarcode+kSizeSirasBarcode) // 104
#define kSizeLocale		8
#define kOffsetTSPressure	(kOffsetLocale+kSizeLocale) // 112
#define kSizeTSPressure		(24*sizeof(int))
#define kOffsetSoftBoardId	(kOffsetTSPressure+kSizeTSPressure) // 208
#define kSizeSoftBoardId	(sizeof(int))
#define kOffsetAclBias		(kOffsetSoftBoardId+kSizeSoftBoardId) // 212
#define kSizeAclBias		(3*sizeof(int))
#define kOffsetMfgVersion	(kOffsetAclBias+kSizeAclBias) // 224
#define	kSizeMfgVersion		(sizeof(int))
#define	kOffsetBL2CalData	(kOffsetMfgVersion+kSizeMfgVersion) // 228
#define kSizeBL2CalData		(5*sizeof(int))
#define kOffsetMacAddress	(kOffsetBL2CalData+kSizeBL2CalData) // 248
#define kSizeMacAddress		(20*sizeof(char))
#define kOffsetIPv6Address	(kOffsetMacAddress+kSizeMacAddress) // 268
#define kSizeIPv6Address	(8*sizeof(unsigned short))
#define kOffsetLCDMType		(kOffsetIPv6Address+kSizeIPv6Address) //284
#define kSizeLCDMType		(sizeof(int))
#define kOffsetSku		(kOffsetLCDMType+kSizeLCDMType)	//288
#define kSizeSku		(11*sizeof(char))
#define kOffsetAccelCalData	(kOffsetSku+kSizeSku) //299
#define kSizeAccelCalData	(10*sizeof(long))
#define kOffsetCheckSum		(4096-kSizeCheckSum)
#define kSizeCheckSum		(sizeof(tCS))
#define kOffsetLineHz		(kOffsetAccelCalData+kSizeAccelCalData)
#define kSizeLineHz			(sizeof(int))

// Error codes returns by MfgData methods
enum MfgDataError {
	kMfgDataErrorNone = 0,
	kMfgDataErrorInternal, 
	kMfgDataErrorMalloc, 
	kMfgDataErrorPartitionRead, 
	kMfgDataErrorPartitionWrite, 
	kMfgDataErrorPartitionErase, 
	kMfgDataErrorMissingPartition, 
	kMfgDataErrorP0ReadbackVerify, 
	kMfgDataErrorP1ReadbackVerify, 
	kMfgDataErrorP0Read, 
	kMfgDataErrorP1Read, 
	kMfgDataErrorP0Write, 
	kMfgDataErrorP1Write, 
	kMfgDataErrorP0Erase, 
	kMfgDataErrorP1Erase, 
	kMfgDataErrorMfgModeDisabled,
	kMfgDataErrorInvalidArguments,
	kMfgDataErrorSysfsRead,
};

// CPartition:  Not a public class
//	Manipulate a file as a whole chunk.
//	Used by CMfgData on /dev/mtdX partition files to do raw I/O from NOR.
class CPartition
{
 public:
	CPartition (const char *path);
	~CPartition ();
	int ReadAll (U8 *buf);
	int WriteAll (U8 *buf);
	int Size () { return m_size; }
 private:
	int m_size;
	char *m_path;
};

// Version 1 of the ts pressure data
struct ts_pressure_data1
{
	int max_tnt_down;
	int min_tnt_up;
	int max_delta_tnt;
	int delay_in_us;
	int y_delay_in_us;
	int tnt_delay_in_us;
	int pressure_curve[9];
	int reserved[8];
};

// Version 2 of the ts pressure data
struct ts_pressure_data2
{
	int max_tnt_down;
	int min_tnt_up;
	int max_delta_tnt;
	int delay_in_us;
	int y_delay_in_us;
	int tnt_delay_in_us;
	int pressure_curve[9];
	int tnt_mode;
	int averaging;
	int reserved[6];
};

// Version 3 of the ts pressure data
struct ts_pressure_data3
{
	int max_tnt_down;
	int min_tnt_up;
	int max_delta_tnt;
	int delay_in_us;
	int y_delay_in_us;
	int tnt_delay_in_us;
	int pressure_curve[9];
	int tnt_mode;
	int averaging;
	int tnt_plane[3];
	int reserved[3];
};

// Generic ts pressure data has version field first
struct ts_pressure_data
{
	int version;
	union {
		struct ts_pressure_data1 data1;
		struct ts_pressure_data2 data2;
		struct ts_pressure_data3 data3;
	} u;
};

//Component listing
#define kLCDM 1

// CMfgData
//	Public interface to NOR MfgData sectors
class CMfgData
{
 public:
	CMfgData ();
	~CMfgData ();

	// Call Init before using any other public functions.
	// Call Exit after, but before destroying object.
	int Init ();
	int Exit ();

	// Only call Update if you are willing to write data to NOR
	// You would need to call this if you want effect of any SetXYZ calls
	// to persist.
	int Update ();

	// Lock/Unlock Driver so that Update call will work
	// Call with true before calling Update (but after Init)
	int SetMfgMode (bool enable);

	int GetSerialNumberRaw (char *sn);
	int SetSerialNumberRaw (const char *sn);
	int GetSerialNumber (char *sn);
	int SetSerialNumber (const char *sn);
	int GetSNValidFlag (bool &v);
	int SetSNValidFlag (bool v=true);
	int GetSirasBarcode (char *sbc);
	int SetSirasBarcode (const char *sbc);
	int GetBLCalData (int &bl);
	int SetBLCalData (int bl);
	int GetAudioGain (int &gain);
	int SetAudioGain (int gain);
	int GetADCCalData (int &offset, int &gain);
	int SetADCCalData (int offset, int gain);
	int GetTSCalData (int *ts);
	int SetTSCalData (int *ts);
	int GetLocale (char *loc);
	int SetLocale (const char *loc);
	int GetTSPressure (struct ts_pressure_data *tsp);
	int SetTSPressure (struct ts_pressure_data *tsp);
	int GetSoftBoardId (int &bid);
	int SetSoftBoardId (int bid);
	int GetAclBias (int *bias);
	int SetAclBias (int *bias);
	int GetMfgVersion(int &mfgVersion);
	int SetMfgVersion(int mfgVersion);
	int GetBL2CalData (int &version, int &lux1, int &cal1, int &lux2, int &cal2);
	int SetBL2CalData (int version, int lux1, int cal1, int lux2, int cal2);
	int GetMacAddress(unsigned char *mac);
	int SetMacAddress(const unsigned char *mac);
	int GetIPv6Address(unsigned short *ipv6);
	int SetIPv6Address(const unsigned short *ipv6);
	int GetComponentType(int component, char *type, int len);
	int SetComponentType(int component, const char *type);
	int GetSku(char *sku);
	int SetSku(const char *sku);
	int GetAccelerometerCalData(long *aCal);
	int SetAccelerometerCalData(long *aCal);
	int GetLineHz(int &hz);
	int SetLineHz(int hz);

	// Raw access to MfgData sectors; does not interact with any of above
	// functions and operates directly on NOR.
	// But you still must call Init before and Exit afterwards.
	int GetNORMfgSectorData (int sector, U8 *buf);
	int ValidateNORMfgSector (int sector);

	// Access to internal library results
	int GetNorChecksumResults(int &checksum_p0, int &checksum_p1);

 private:
	int Read (U8 *buf, int offset, int len);
	int Write (U8 *buf, int offset, int len);
	int GetNorPartitionFilename (char *buf, int partition);
	// int GetNorPartitionAddress (U32 &addr);
	int ValidateCheckSum (U8 *buf);
	int UpdateCheckSum (U8 *buf);
	tCS ComputeCheckSum (U8 *buf);
	int ValidateCheckSum () { return ValidateCheckSum (m_buf); }
	int UpdateCheckSum () { return UpdateCheckSum (m_buf); }
	tCS ComputeCheckSum () { return ComputeCheckSum (m_buf); }
	CPartition *m_p0, *m_p1;
	CPartition *m_primary, *m_secondary;
	U8 *m_buf;
	bool m_mfg_mode;
	int m_bad_checksum_p0, m_bad_checksum_p1;
};
#endif // _libMfgData_h_
