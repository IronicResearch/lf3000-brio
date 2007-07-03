#ifndef LF_BRIO_STRINGTYPES_H
#define LF_BRIO_STRINGTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		StringTypes.h
//
// Description:
//		Defines the string types of the system.
//
// NOTE: On Linux, get the "libglib2.0-dev" and "libglibmm-2.4-dev" packages 
//		using the Synaptic Package Manager to get the header files for the
//		"Glib::ustring" class.
//
//============================================================================== 

//#ifdef EMULATION //FIXME/BSK
	#include <glibmm/ustring.h>
//#endif

#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================== 
// Character encodings
//============================================================================== 

//------------------------------------------------------------------------------
// Type:
//		tCharEncoding
//
//	Definition:
//		Character encodings extend character and string support beyond
//		Clib ASCII to include Windows codepages, ISO-8859 parts, and Unicode 
//		among other common forms & standards.
//
//		tCharEncoding stores a character encoding value, comprised of three elements:
//
//			MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//
//		- the domain, which segments system, product and application encodings 
//		- the group, which specifies the group to which the encoding belongs
//		- the tag - a key number that identifies the specific sub-encoding 
//			within its group
//
//		Application code may use any product or System-defined types, but 
//		can only define new application types -- not product or System
//		types.  Similarly, product code may use any System-defined types,
//		but can only define new product types -- not application or System types.
//------------------------------------------------------------------------------

typedef tU16NumSpace	tCharEncoding;

#define MakeCharEncoding(domain, group, tag) ((tCharEncoding)MakeU16NumSpace(domain, group, tag))

#define kUndefinedCharEncoding ((tCharEncoding)0)

//============================================================================== 
// System-defined character encoding groups
//============================================================================== 

enum {
	kSystemCharEncodingGroupSystem = kFirstNumSpaceGroup,		
												// special System/LeapFrog encodings
	kSystemCharEncodingGroupLegacy,				// legacy encodings (e.g. ASCII)
	kSystemCharEncodingGroupISO,				// International Standards Org (ISO)
	kSystemCharEncodingGroupWindows,			// Windows codepages
	kSystemCharEncodingGroupMac,				// Mac encodings
	kSystemCharEncodingGroupUnicode				// Unicode standard
};

#define MakeFirstSystemGroupCharEncoding(group)  \
				MakeCharEncoding(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)

//==============================================================================
// System-defined character encodings
//==============================================================================

enum {
	//------------------------------------------------------------------------------
	// System encodings
	//------------------------------------------------------------------------------
	// <firstGroupEncoding>			= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupSystem),

	//------------------------------------------------------------------------------
	// Legacy encodings
	//------------------------------------------------------------------------------
	kASCIICharEncoding				= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupLegacy),					
										// ISO 646-1991; standard C 7-bit encoding -- values 0-127

	//------------------------------------------------------------------------------
	// ISO encodings
	//------------------------------------------------------------------------------
	kISOLatin1CharEncoding			= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupISO),
										// ISO 8859-1 - Western European; 8-bit encoding
	kISOLatin2CharEncoding,				// ISO 8859-2 - Central European; 8-bit encoding
	kISOLatin3CharEncoding,				// ISO 8859-3 - South European; 8-bit encoding
	kISOLatin4CharEncoding,				// ISO 8859-4 - North European; 8-bit encoding
	kISOCyrillicCharEncoding,			// ISO 8859-5 - Cyrillic; 8-bit encoding
	kISOArabicCharEncoding,				// ISO 8859-6 - Arabic; 8-bit encoding
	kISOGreekCharEncoding,				// ISO 8859-7 - Greek; 8-bit encoding
	kISOHebrewCharEncoding,				// ISO 8859-8 - Hebrew; 8-bit encoding
	kISOLatin5CharEncoding,				// ISO 8859-9 - Turkish; 8-bit encoding
	kISOLatin6CharEncoding,				// ISO 8859-10 - Nordic; 8-bit encoding
	kISOThaiCharEncoding,				// ISO 8859-11 - Thai; 8-bit encoding
	kISOPart12CharEncoding,				// ISO 8859-12 - <not used>; 8-bit encoding
	kISOLatin7CharEncoding,				// ISO 8859-13 - Baltic Rim; 8-bit encoding
	kISOLatin8CharEncoding,				// ISO 8859-14 - Celtic; 8-bit encoding
	kISOLatin9CharEncoding,				// ISO 8859-15 - Latin1 variant; 8-bit encoding
	kISOLatin10CharEncoding,			// ISO 8859-16 - South-Eastern Europe; 8-bit encoding

	//------------------------------------------------------------------------------
	// Windows encodings
	//------------------------------------------------------------------------------
	kWindowsUSCharEncoding			= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupWindows),
										// Windows code page 437 - United States; 8-bit encoding
	kWindowsLatin2CharEncoding,			// Windows code page 1250 - Central European; 8-bit encoding							
	kWindowsCyrillicCharEncoding,		// Windows code page 1251 - Cyrillic; 8-bit encoding
	kWindowsLatin1CharEncoding,			// Windows code page 1252 - Western European; 8-bit encoding
	kWindowsGreekCharEncoding,			// Windows code page 1253 - Greek; 8-bit encoding
	kWindowsTurkishCharEncoding,		// Windows code page 1254 - Turkish; 8-bit encoding
	kWindowsHebrewCharEncoding,			// Windows code page 1255 - Hebrew; 8-bit encoding
	kWindowsArabicCharEncoding,			// Windows code page 1256 - Arabic; 8-bit encoding
	kWindowsBalticRimCharEncoding,		// Windows code page 1257 - Baltic region; 8-bit encoding
	kWindowsVietnameseCharEncoding,		// Windows code page 1258 - Vietnamese; 8-bit encoding
	kWindowsThaiCharEncoding,			// Windows code page 874 - Thai; 8-bit encoding

	//------------------------------------------------------------------------------
	// Mac encodings
	//------------------------------------------------------------------------------
	kMacRomanCharEncoding			= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupMac),
										// MacRoman; 8-bit encoding

	//------------------------------------------------------------------------------
	// Unicode encodings
	//------------------------------------------------------------------------------
	kUTF8CharEncoding				= MakeFirstSystemGroupCharEncoding(kSystemCharEncodingGroupUnicode),					
										// Unicode: variable 8 to 32-bits/character; "8-bit safe" for use w/ Clib strxxx fcns
	kUTF16CharEncoding,					// Unicode: mostly 16-bits/character (with some obscure exceptions); 
										//			leading BOM ("byte order mark") value indicates endianness
	kUTF16BECharEncoding,				// Unicode: UTF16 - guaranteed big endian
	kUTF16LECharEncoding,				// Unicode: UTF16 - guaranteed little endian
	kUTF32CharEncoding,					// Unicode: uniform 32-bits/character; 
										//		leading BOM ("byte order mark") value indicates endianness
	kUTF32BECharEncoding,				// Unicode: UTF32 - guaranteed big endian
	kUTF32LECharEncoding				// Unicode: UTF32 - guaranteed little endian
};


//==============================================================================
// Type:
//		tWChar
//
//	Definition:
//		This 4-byte type is big enough to hold any single character regardless of  
//		the particular text encoding.  It provides international & multi-byte  
//		character support in lieu of the C-library wchar_t type.  We don't 
//		use wchar_t or related Clib string functionality because the Clib 
//		"wide character" definition can vary in size from 8 to 32 bits depending 
//		on the compiler and library settings, and wchar_t string processing
//		typically depends on a single text encoding scheme.
//
//		tWChars are related to tCharEncoding-encoded strings in that any single
//		character, when represented outside of an encoded string, is of type tWChar.  
//		However, within the encoded string, individual character storage size & 
//		format is entirely determined by the character encoding.
//
//		tWChars themselves provide no information about their character set, 
//		format or byte-size.  No assumptions can be made about any of these attributes.  
//		tWChar objects must be paired with a tCharEncoding type that defines the 
//		format of the character.
//==============================================================================

typedef U32 		tWChar;				// storage type large enough for a single character of any encoding	

//==============================================================================
// Type:
//		tUniChar
//
//	Definition:
//		FIXME/dg: fill in
//==============================================================================

typedef U8			tUTF8Char;
typedef U16			tUTF16Char;
typedef U32			tUTF32Char;

typedef tUTF16Char	tUniChar;		

typedef Glib::ustring	CString;
typedef Glib::ustring	CPath;
typedef Glib::ustring	CURI;
	
typedef const CString*	ConstPtrCString;
typedef const CURI*		ConstPtrCURI;

extern const CString	kNullString;
extern const CURI		kNullURI;



//==============================================================================
// Utility functions
//==============================================================================
//------------------------------------------------------------------------------
inline CString AppendPathSeparator(const CString& path)
{
	if( path.empty() || path.at(path.length()-1) != '/' )
		return path + '/';
	return path;
}

//------------------------------------------------------------------------------
inline CString AppendPathSeparator(const char* pathIn)
{
	CString path(pathIn);
	if( path.empty() || path.at(path.length()-1) != '/' )
		path += '/';
	return path;
}


LF_END_BRIO_NAMESPACE()	
#endif	// LF_BRIO_STRINGTYPES_H

// EOF
