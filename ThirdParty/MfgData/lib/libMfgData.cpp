// MfgData handler for NOR storage of Manufacturing Data on Emerald
// RDowling 7/6/2009
//

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

#include <zlib.h>

#include "libMfgData.h"

#include <sys/ioctl.h>

#include <fstream>
#include <string>

//__user means user space variables, but is not defined in angstrom so define it here
#ifndef __user
#define __user
#endif

#include <mtd/mtd-user.h>

// FIXME: Deduce this on the fly?
#define NOR_ERASE_SIZE 4096

// Some constants controlling NOR device file names and /proc/mtd output
#define FILENAME_LEN	16
#define MFG_DATA_FMT	"MfgData%d"
#define MFG_BOOT_NAME	"NOR_Boot"
#define NOR_WRITE_PROT_ADDR	"/sys/devices/platform/lf2000-nand/nor_write_addr_threshold"
#define PROC_MTD_LINE_LEN	80

// Build with -DDEBUG to embed printfs
//   If -DDEBUG, define environment var DebugMfgData=1 at run time 
//   to actually see prints
// Build with -DTEST to run under Ubuntu
#ifdef DEBUG
static int Debug()
{
#ifdef TEST
	return 1;
#else
	return getenv("DebugMfgData")!=NULL;
#endif
}
#endif

//////////////////////////////////////////////////////////////////////////////

CPartition::CPartition (const char *path)
{
	m_size=NOR_ERASE_SIZE; 
	m_path=strdup(path);
}

CPartition::~CPartition ()
{
	if (m_path)
		free (m_path);
}

// Read whole file into RAM from NOR
int CPartition::ReadAll (U8 *buf) 
{
	int fd = open (m_path, O_RDONLY);
	if (fd < 0)
		return kMfgDataErrorPartitionRead;
	int x = read (fd, buf, m_size);
	if (close (fd))
		return kMfgDataErrorPartitionRead;
	if (x != m_size)
		return kMfgDataErrorPartitionRead;
	return kMfgDataErrorNone;
}

// Erase, then Write whole file into NOR
int CPartition::WriteAll (U8 *buf)
{
	int fd = open (m_path, O_WRONLY);
	if (fd < 0)
		return kMfgDataErrorPartitionWrite;
	// Erase
	struct erase_info_user erase;
	erase.start = 0;
	erase.length = m_size;
#ifndef TEST
	if (ioctl (fd, MEMERASE, &erase) < 0)
	{
		close (fd);
		perror ("CPartition::WriteAll erase");
		return kMfgDataErrorPartitionErase;
	}
#endif
	// Write
	int x = write (fd, buf, m_size);
	if (close (fd))
		return kMfgDataErrorPartitionWrite;
	if (x != m_size)
		return kMfgDataErrorPartitionWrite;
	return kMfgDataErrorNone;
}

//////////////////////////////////////////////////////////////////////////////

CMfgData::CMfgData ()
{
	m_p0 = m_p1 = NULL;
	m_primary = m_secondary = NULL;
	m_mfg_mode = false;
	m_bad_checksum_p0 = -1;	// -1 == unknown checksum state
	m_bad_checksum_p1 = -1;	// -1 == unknown checksum state
}

CMfgData::~CMfgData ()
{
}

// Set up the Interface.  Finds a good NOR sector and reads it into
// RAM, or if it can't find one, formats a blank RAM buffer with 0's
// Does not write to NOR to repair bad data, but keeps track of
// whether it needed to go to the backup sector or not.
int CMfgData::Init ()
{
	//
	// Discover the names of the partitions
	//
	char path[FILENAME_LEN];

	// Discover name for Partition 0
	if (GetNorPartitionFilename (path, 0))
	{
		fprintf (stderr, "CMfgData::Init: GetNorPartitionFilename failed\n");
		return kMfgDataErrorMissingPartition;
	}
#ifdef DEBUG	
	if (Debug()) printf ("P0 found on %s\n", path);
#endif
	m_p0 = new CPartition (path);
	if (!m_p0)
		return kMfgDataErrorMalloc;

	// Discover Partition 1
	if (GetNorPartitionFilename (path, 1))
	{
		fprintf (stderr, "CMfgData::Init: GetNorPartitionFilename failed\n");
		return kMfgDataErrorMissingPartition;
	}
#ifdef DEBUG	
	if (Debug()) printf ("P1 found on %s\n", path);
#endif
	m_p1 = new CPartition (path);
	if (!m_p1)
		return kMfgDataErrorMalloc;

	// Make buffer
	int size = m_p0->Size ();
	m_buf = new U8[size];
	if (!m_buf)
		return kMfgDataErrorMalloc;

	// Default Primary to p0 and secondary to p1
	m_primary = m_p0;
	m_secondary = m_p1;

	//
	// Read either P0 or P1 into RAM
	//

	// Read P0 into RAM, take if valid
	if (m_p0->ReadAll (m_buf))
		return kMfgDataErrorP0Read;
	if (!ValidateCheckSum ())
	{
		// P0 is Good
#ifdef DEBUG
		if (Debug()) printf ("CMfgData::Init p0 is good\n");
#endif
		m_bad_checksum_p0 = 0;	/* mfgdata area 0 good */
		return kMfgDataErrorNone;
	}
	m_bad_checksum_p0 = 1;		/* mfgdata area 0 bad */

	// Read P1 into RAM, if valid, take
	if (m_p1->ReadAll (m_buf))
		return kMfgDataErrorP1Read;
	if (!ValidateCheckSum ())
	{
		// P1 is good, but P0 was bad
#ifdef DEBUG
		if (Debug()) printf ("CMfgData::Init p0 is bad, p1 is good\n");
#endif
		m_bad_checksum_p1 = 0;	/* mfgdata area 1 good */
	}
	else
	{
		// P1 is Bad and P0 is already bad
		// Format P0
#ifdef DEBUG
		if (Debug()) printf ("CMfgData::Init p0 and p1 bad; Formating\n");
#endif
		memset (m_buf, 0, size);
		m_bad_checksum_p1 = 1;	/* mfgdata area 1 bad */
	}
	
	// Make p1 the primary now
	m_primary = m_p1;
	m_secondary = m_p0;

	// Disable writes
	m_mfg_mode = false;

	// Happy now
	return kMfgDataErrorNone;
}

// Cause data to be written to both NOR sectors.  Write in order
// "secondary", then "primary".  Primary is determined to be a good
// sector (unless both are bad).  That way if secondary is bad, it is
// rewritten and verified before the primary is changed.
// 
// When Update finishes, both sectors are verified as good or an error
// code is returned.
int CMfgData::Update ()
{
	// Early out of not allowed
	if (!m_mfg_mode)
		return kMfgDataErrorMfgModeDisabled;

#ifdef DEBUG
	const char *primary_name   = m_primary==m_p0   ? "p0" : "p1";
	const char *secondary_name = m_secondary==m_p0 ? "p0" : "p1";
	if (Debug()) printf ("CMfgData::Update.  Write to both\n");
#endif

	// Fix checksum in RAM
	UpdateCheckSum ();

	// Write secondary
	if (m_secondary->WriteAll (m_buf))
		return m_secondary==m_p0 ? kMfgDataErrorP0Write : kMfgDataErrorP1Write;
	// Read it back in an revalidate it
	if (m_secondary->ReadAll (m_buf))
		return m_secondary==m_p0 ? kMfgDataErrorP0Read : kMfgDataErrorP1Read;
	if (ValidateCheckSum ())
	{
		// Bad
#ifdef DEBUG
		if (Debug()) printf ("CMfgData::Update: Readback of secondary %s failed validation\n", secondary_name);
#endif
		return m_secondary==m_p0 ? kMfgDataErrorP0ReadbackVerify : kMfgDataErrorP1ReadbackVerify;
	}
	
	// Write primary
	if (m_primary->WriteAll (m_buf))
		return m_primary==m_p0 ? kMfgDataErrorP0Write : kMfgDataErrorP1Write;
	// Read it back in an revalidate it
	if (m_primary->ReadAll (m_buf))
		return m_primary==m_p0 ? kMfgDataErrorP0Read : kMfgDataErrorP1Read;
	if (ValidateCheckSum ())
	{
		// Bad
#ifdef DEBUG
		if (Debug()) printf ("CMfgData::Update: Readback of primary %s failed validation\n", primary_name);
#endif
		return m_primary==m_p0 ? kMfgDataErrorP0ReadbackVerify : kMfgDataErrorP1ReadbackVerify;
	}
	return kMfgDataErrorNone;
}

// Tear down this interface
int CMfgData::Exit ()
{
	delete[] m_buf;
	delete m_p0;
	delete m_p1;
	m_p0 = m_p1 = m_primary = m_secondary = NULL;
	return kMfgDataErrorNone;
}

// Copy a chunk of the RAM copy of the NOR to a user buffer.
// Does not actually access NOR
int CMfgData::Read (U8 *buf, int offset, int len)
{
	// FIXME: Bounds check
	memcpy (buf, m_buf+offset, len);
	return len;
}

// Copy a chunk of user buffer into the RAM copy of the NOR.  Does not
// actaully write to NOR or even remember that an Update will be
// needed.
int CMfgData::Write (U8 *buf, int offset, int len)
{
	// FIXME: Bounds check
	memcpy (m_buf+offset, buf, len);
	return len;
}

// Check that checksum field and computed checksum match;
// Return 0 if they match, non-zero if they don't
int CMfgData::ValidateCheckSum (U8 *buf)
{
	tCS ecs;
	memcpy ((U8 *)&ecs, buf+kOffsetCheckSum, kSizeCheckSum);
	tCS cs = ComputeCheckSum (buf);
	return cs != ecs;
}

// Fill in checksum field with newly computed checksum
int CMfgData::UpdateCheckSum (U8 *buf)
{
	tCS cs = ComputeCheckSum (buf);
	memcpy (buf+kOffsetCheckSum, (U8 *)&cs, kSizeCheckSum);
	return kMfgDataErrorNone;
}

tCS CMfgData::ComputeCheckSum (U8 *buf)
{
	//zlib crc32 differs from kernel so we have to have this funky XOR logic
	//http://www.infradead.org/pipermail/linux-mtd/2003-February/006905.html
	return (tCS)(crc32(0xFFFFFFFF, buf, kOffsetCheckSum) ^ 0xFFFFFFFF);
}

// Find and return absolute path to /dev/mtdX corresponding to MfgDataN
// Return 0 for success and fill buf[]
// Return non-zero for failure
int CMfgData::GetNorPartitionFilename (char *buf, int partition)
{
#ifdef TEST
	// Ubuntu: Don't use device, just use a file
	sprintf (buf, "p%d", partition);
	return kMfgDataErrorNone;
#endif
	char needle[FILENAME_LEN];
	char line[PROC_MTD_LINE_LEN];
	FILE *fin = fopen ("/proc/mtd", "r");
	if (!fin)
	{
		perror ("GetNorPartitionFilename filed opening /proc/mtd");
		return kMfgDataErrorMissingPartition;
	}
	sprintf (needle, MFG_DATA_FMT, partition & 1);
	while (!feof(fin))
	{
		fgets (line, PROC_MTD_LINE_LEN, fin);
		if (strstr (line, needle))
		{
			// Found it, now pull out mtd
			char c = line[3];
			sprintf (buf, "/dev/mtd%c", c);
			fclose (fin);
			return kMfgDataErrorNone;
		}
	}
	fclose (fin);
	fprintf (stderr, "GetNorPartitionFilename: Could not find %s in /proc/mtd\n", needle);
	return kMfgDataErrorMissingPartition;
}

// Find and return address of lowest MfgData partition, for use with sysfs
// entry to unprotect the NOR from writes.
// Return 0 for success and fill addr
// Return non-zero for failure
// **** Replaced with script /usr/bin/mfgdata.sh ***
// int CMfgData::GetNorPartitionAddress (U32 &addr)
// {
// #ifdef TEST
// 	// Ubuntu: Don't use device, just use a file
// 	addr = 0xface;
// 	return kMfgDataErrorNone;
// #endif
// 	char line[PROC_MTD_LINE_LEN];
// 	FILE *fin = fopen ("/proc/mtd", "r");
// 	if (!fin)
// 	{
// 		perror ("GetNorPartitionAddress filed opening /proc/mtd");
// 		return kMfgDataErrorMissingPartition;
// 	}
// 	while (!feof(fin))
// 	{
// 		fgets (line, PROC_MTD_LINE_LEN, fin);
// 		if (strstr (line, MFG_BOOT_NAME))
// 		{
// 			// Found it, now pull out address
// 			int d, x;
// 			U32 size;
// 			x = sscanf (line, "mtd%d: %lx", &d, &size);
// 			if (x != 2)
// 			{
// 				printf ("Only found %d entries in sscanf of %s\n", x, line);
// 				return kMfgDataErrorInternal;
// 			}
// 			addr = size;
// 			fclose (fin);
// 			return kMfgDataErrorNone;
// 		}
// 	}
// 	fclose (fin);
// 	fprintf (stderr, "GetNorPartitionAddress: Could not find %s in /proc/mtd\n", MFG_BOOT_NAME);
// 	return kMfgDataErrorMissingPartition;
// }

// Get raw sector data into a user buffer.
int CMfgData::GetNORMfgSectorData (int sector, U8 *buf)
{
	CPartition *p = sector == 0 ? m_p0 : m_p1;
	return p->ReadAll (buf);
}

// Validate checksum on raw sector data
int CMfgData::ValidateNORMfgSector (int sector)
{
	CPartition *p = sector == 0 ? m_p0 : m_p1;
	U8 *buf = new U8[p->Size()];
	if (!buf)
		return kMfgDataErrorMalloc;
	if (p->ReadAll (buf))
	{
		delete[] buf;
		return kMfgDataErrorPartitionRead;
	}
	int ret = ValidateCheckSum (buf);
	delete[] buf;
	return ret;
}
int CMfgData::GetNorChecksumResults(int &bad_checksum_p0, int &bad_checksum_p1)
{
	bad_checksum_p0 = m_bad_checksum_p0;
	bad_checksum_p1 = m_bad_checksum_p1;
	return kMfgDataErrorNone;
}

// Enable driver and Update to do writes
int CMfgData::SetMfgMode (bool enable)
{
	char buf[128];
	sprintf (buf, "/usr/bin/mfgmode.sh %d", enable ? 2 : 0);
	if (system (buf))
	{
		perror ("can't exec /usr/bin/mfgmode.sh");
		return kMfgDataErrorMfgModeDisabled;
	}
 	m_mfg_mode = enable;
	return kMfgDataErrorNone;
// 	int e;
// 	U32 addr;
// 	m_mfg_mode = enable;
// 	if (e=GetNorPartitionAddress (addr))
// 		return e;
// 	if (m_mfg_mode)
// 	{
// 		// Enable, lower to addr
// 	}
// 	else
// 	{
// 		// Disable, raise 8K above addr.. or all the way to moon
// 		addr = 0x7fff0000;
// 	}
//
// 	FILE *fout = fopen (NOR_WRITE_PROT_ADDR, "w");
// 	if (!fout)
// 	{
// 		perror ("SetMfgMode failed opening " NOR_WRITE_PROT_ADDR);
// 		return kMfgDataErrorMissingPartition;
// 	}
// 	fprintf (fout, "%d\n", addr);
// 	if (fclose (fout))
// 	{
// 		perror ("SetMfgMode failed closing " NOR_WRITE_PROT_ADDR);
// 		return kMfgDataErrorMissingPartition;
// 	}
// 	return kMfgDataErrorNone;
}

int CMfgData::GetSNValidFlag (bool &v) 
{ long x, y=Read ((U8*)&x, kOffsetSNValidFlag, kSizeSNValidFlag) != kSizeSNValidFlag; v=x; return y; }

int CMfgData::SetSNValidFlag (bool v)
{ long x=v; return Write ((U8*)&x, kOffsetSNValidFlag, kSizeSNValidFlag) != kSizeSNValidFlag; }

int CMfgData::GetBLCalData (int &bl)
{ long x, y=Read ((U8*)&x, kOffsetBLCalData, kSizeBLCalData) != kSizeBLCalData; bl=x; return y; }

int CMfgData::SetBLCalData (int bl)
{ long x=bl; return Write ((U8*)&x, kOffsetBLCalData, kSizeBLCalData) != kSizeBLCalData; }

int CMfgData::GetAudioGain (int &gain)
{ long x, y=Read ((U8*)&x, kOffsetAudioGain, kSizeAudioGain) != kSizeAudioGain; gain=x; return y; }

int CMfgData::SetAudioGain (int gain)
{ long x=gain; return Write ((U8*)&x, kOffsetAudioGain, kSizeAudioGain) != kSizeAudioGain; }

int CMfgData::GetSerialNumberRaw (char *sn)
{ return Read ((U8*)sn, kOffsetSerialNumber, kSizeSerialNumber) != kSizeSerialNumber; }

int CMfgData::SetSerialNumberRaw (const char *sn)
{ return Write ((U8*)sn, kOffsetSerialNumber, kSizeSerialNumber) != kSizeSerialNumber; }

int CMfgData::GetSirasBarcode (char *sbc)
{ return Read ((U8*)sbc, kOffsetSirasBarcode, kSizeSirasBarcode) != kSizeSirasBarcode; }

int CMfgData::SetSirasBarcode (const char *sbc)
{ return Write ((U8*)sbc, kOffsetSirasBarcode, kSizeSirasBarcode) != kSizeSirasBarcode; }

int CMfgData::GetTSCalData (int *ts)
{ return Read ((U8*)ts, kOffsetTSCalData, kSizeTSCalData) != kSizeTSCalData; }

int CMfgData::SetTSCalData (int *ts)
{ return Write ((U8*)ts, kOffsetTSCalData, kSizeTSCalData) != kSizeTSCalData; }

int CMfgData::GetADCCalData (int &offset, int &gain)
{
	long x[2], y;
	y=Read ((U8*)&x[0], kOffsetADCCalData, kSizeADCCalData) != kSizeADCCalData;
	offset=x[0]; gain=x[1];
	return y;
}

int CMfgData::SetADCCalData (int offset, int gain)
{
	long x[2] = { offset, gain };
	return Write ((U8*)&x[0], kOffsetADCCalData, kSizeADCCalData) != kSizeADCCalData; 
}

int CMfgData::GetLocale (char *loc)
{ return Read ((U8*)loc, kOffsetLocale, kSizeLocale) != kSizeLocale; }

int CMfgData::SetLocale (const char *loc)
{ return Write ((U8*)loc, kOffsetLocale, kSizeLocale) != kSizeLocale; }

int CMfgData::GetTSPressure (struct ts_pressure_data *tsp)
{ return Read ((U8*)tsp, kOffsetTSPressure, kSizeTSPressure) != kSizeTSPressure; }

int CMfgData::SetTSPressure (struct ts_pressure_data *tsp)
{ return Write ((U8*)tsp, kOffsetTSPressure, kSizeTSPressure) != kSizeTSPressure; }

int CMfgData::GetSoftBoardId (int &bid)
{ long x, y=Read ((U8*)&x, kOffsetSoftBoardId, kSizeSoftBoardId) != kSizeSoftBoardId; bid=x; return y; }

int CMfgData::SetSoftBoardId (int bid)
{ long x=bid; return Write ((U8*)&x, kOffsetSoftBoardId, kSizeSoftBoardId) != kSizeSoftBoardId; }

int CMfgData::GetAclBias (int *bias)
{ return Read ((U8*)bias, kOffsetAclBias, kSizeAclBias) != kSizeAclBias; }

int CMfgData::SetAclBias (int *bias)
{ return Write ((U8*)bias, kOffsetAclBias, kSizeAclBias) != kSizeAclBias; }

int CMfgData::GetMfgVersion (int &mfgVersion)
{ long x, y=Read ((U8*)&x, kOffsetMfgVersion, kSizeMfgVersion) != kSizeMfgVersion; mfgVersion=x; return y; }

int CMfgData::SetMfgVersion (int mfgVersion)
{ long x=mfgVersion; return Write ((U8*)&x, kOffsetMfgVersion, kSizeMfgVersion) != kSizeMfgVersion; }

int CMfgData::GetBL2CalData (int &version, int &lux1, int &cal1, int &lux2, int &cal2)
{
	long x[5], y;
	y=Read ((U8*)&x[0], kOffsetBL2CalData, kSizeBL2CalData) != kSizeBL2CalData;
	version=x[0]; lux1=x[1]; cal1=x[2]; lux2=x[3]; cal2=x[4];
	return y;
}

int CMfgData::SetBL2CalData (int version, int lux1, int cal1, int lux2, int cal2)
{
	long x[5] = { version, lux1, cal1, lux2, cal2 };
	return Write ((U8*)&x[0], kOffsetBL2CalData, kSizeBL2CalData) != kSizeBL2CalData; }

int CMfgData::GetMacAddress (unsigned char *mac)
{ return Read ((U8*)mac, kOffsetMacAddress, kSizeMacAddress) != kSizeMacAddress; }

int CMfgData::SetMacAddress (const unsigned char *mac)
{ return Write ((U8*)mac, kOffsetMacAddress, kSizeMacAddress) != kSizeMacAddress; }

int CMfgData::GetIPv6Address (unsigned short *ipv6)
{ return Read ((U8*)ipv6, kOffsetIPv6Address, kSizeIPv6Address) != kSizeIPv6Address; }

int CMfgData::SetIPv6Address (const unsigned short *ipv6)
{ return Write ((U8*)ipv6, kOffsetIPv6Address, kSizeIPv6Address) != kSizeIPv6Address; }

#define kLCDM_FILE "/sys/devices/system/board/lcd_mfg"

int CMfgData::GetComponentType(int component, char *type, int len)
{
	//For file driven components
	FILE* lcd_type_file;
	size_t bytes_read;
	
	switch( component )
	{
		case kLCDM:
			lcd_type_file = fopen(kLCDM_FILE, "r");
			if( lcd_type_file == NULL )
			{
				perror("Failed to open lcd sysfs entry");
				return kMfgDataErrorSysfsRead;
			}
			bytes_read = fread(type, 1, len, lcd_type_file);
			fclose(lcd_type_file);
			//If it's smaller than the buffer, cut the newline character
			if( bytes_read < len )
				type[bytes_read - 1] = '\0';
			return 0;
		default:
			return kMfgDataErrorInvalidArguments;
	}
}

int CMfgData::SetComponentType(int component, const char *type)
{
	switch( component )
	{
		case kLCDM:
			fprintf (stderr, "LCD type is based on ADC readings and can not be set.\n");
			return kMfgDataErrorInvalidArguments;
		default:
			return kMfgDataErrorInvalidArguments;
	}
}

//////////////////////////////////////////////////////////////////////////////

// Mfg want's this hijacked a bit
#define EMERALD_PRODUCT_ID	0x00170001
#define MADRID_PRODUCT_ID	0x001E0001
#define LUCY_PRODUCT_ID		0x00210001
#define VALENCIA_PRODUCT_ID	0x00220001
#define RIO_PRODUCT_ID		0x00270001
#define CABO_PRODUCT_ID		0x002A0000
#define GLASGOW_PRODUCT_ID	0x00300000
#define XANADU_PRODUCT_ID	0x002E0000
#define PRODUCT_ID_OFFSET	4
#define PRODUCT_ID_FMT		"%06X"
#define PRODUCT_ID_LEN		6
#define PRODUCT_ID_MASK		0xffffff
#define PRODUCT_ID_FILE		"/sys/devices/system/board/platform"
#define BOARD_ID_OFFSET		10
#define BOARD_ID_FMT		"%02X"
#define BOARD_ID_LEN		2
#define BOARD_ID_MASK		0x1f
#define BOARD_ID_FILE	"/sys/devices/platform/lf1000-gpio/board_id"
#define SERIAL_NUMBER_BLANK	"                    "

int CMfgData::GetSerialNumber (char *sn)
{
	// Get raw serial number found in NOR
	int ret = GetSerialNumberRaw (sn);
	int board_id = -1;
	const int desired_length = 20;

	// Truncate length to 20
	if (desired_length < kSizeSerialNumber) // careful if someone changes kSizeSerialNumber
		sn[desired_length] = 0;

	// Paint in BOARD_ID
	char buf[10];
	
	FILE *f = fopen (BOARD_ID_FILE, "r");
	if (f)
	{
		int len = fread (buf, 1, 10, f);
		if (len > 0)
		{
			sscanf (buf, "%X", &board_id);
		}
		fclose(f);
		sprintf (buf, BOARD_ID_FMT, board_id & BOARD_ID_MASK);
		memcpy (sn+BOARD_ID_OFFSET, buf, BOARD_ID_LEN);
	}

	// Paint in PRODUCT_ID
	int product_id = 0x00000000;

#define LF1000_BOARD_MADRID		0x0B
#define LF1000_BOARD_MADRID_POP		0x0C
#define LF1000_BOARD_MADRID_LFP100	0x0D
	
	std::ifstream in;
	in.open(PRODUCT_ID_FILE);
	if(in.is_open())
	{
		std::string platform;
		in >> platform;
		in.close();
		
		if( platform == "LUCY" )
			product_id = LUCY_PRODUCT_ID;
		else if( platform == "VALENCIA" )
			product_id = VALENCIA_PRODUCT_ID;
		else if( platform == "RIO" )
			product_id = RIO_PRODUCT_ID;
		else if( platform == "CABO" )
			product_id = CABO_PRODUCT_ID;
		else if( platform == "GLASGOW" )
			product_id = GLASGOW_PRODUCT_ID;
		else if( platform == "XANADU" )
			product_id = XANADU_PRODUCT_ID;
	}
	else
	{
		/* LF1000 platform, base on board id */
		switch(board_id)
		{
			case LF1000_BOARD_MADRID:
			case LF1000_BOARD_MADRID_POP:
			case LF1000_BOARD_MADRID_LFP100:
				product_id = MADRID_PRODUCT_ID;
				break;
			default:
				product_id = EMERALD_PRODUCT_ID;
				break;
		}
	}
	
	sprintf (buf, PRODUCT_ID_FMT, product_id & PRODUCT_ID_MASK);
	memcpy (sn+PRODUCT_ID_OFFSET, buf, PRODUCT_ID_LEN);


	// Fix case to upper and map garbage characters to 0's
	int i;
	const char *folder = "0123456789ABCDEF0123456789abcdef";
	for (i=0; i<desired_length; i++)
	{
		const char *s = strchr (folder, sn[i]);
		if (s)
			sn[i] = folder[(s-folder)&15];
		else
			sn[i] = '0';
	}

	// Return original error code from NOR read
	return ret;
}

int CMfgData::SetSerialNumber (const char *sn)
{ 
	// Clear SNValid if writing the serial number
	int ret = SetSNValidFlag (false);
	if (!ret)
	{
		// Clean up length
		int i, rest=0;
		char buf[kSizeSerialNumber];
		for (i=0; i<kSizeSerialNumber; i++)
		{
			if (sn[i] == 0 || rest)
				rest=1, buf[i] = ' ';
			else
				buf[i] = sn[i];
		}
		
		// Truncate length to 20
		if (20<kSizeSerialNumber) // careful if someone changes kSizeSerialNumber
			buf[20] = 0;
		ret = SetSerialNumberRaw (buf);
	}
	return ret;
}

int CMfgData::GetSku (char *sku)
{ return Read ((U8*)sku, kOffsetSku, kSizeSku) != kSizeSku; }

int CMfgData::SetSku (const char *sku)
{ return Write ((U8*)sku, kOffsetSku, kSizeSku) != kSizeSku; }

int CMfgData::GetAccelerometerCalData (long *aCal)
{ return Read ((U8*)aCal, kOffsetAccelCalData, kSizeAccelCalData) != kSizeAccelCalData; }

int CMfgData::SetAccelerometerCalData (long *aCal)
{ return Write ((U8*)aCal, kOffsetAccelCalData, kSizeAccelCalData) != kSizeAccelCalData; }

int CMfgData::GetLineHz(int &hz)
{ long x, y=Read ((U8*)&x, kOffsetLineHz, kSizeLineHz) != kSizeLineHz; hz=x; return y; }

int CMfgData::SetLineHz(int hz)
{ long x=hz; return Write ((U8*)&x, kOffsetLineHz, kSizeLineHz) != kSizeLineHz; }
