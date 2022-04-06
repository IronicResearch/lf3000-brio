#ifndef LF_BRIO_USTRING_H
#define LF_BRIO_USTRING_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		UString.h
//
// Description:
//		Defines string class compatible with Glib::ustring type.
//
//==============================================================================

#include <string>

class ustring : public std::string
{
public:
	ustring();
	~ustring();
	ustring(const ustring& other)				{ _ustring = other; };
	ustring(const std::string src)				{ _ustring = src; };
	ustring(const char* src)					{ _ustring = std::string(src); };

	size_type			bytes() const 			{ return _ustring.size(); };
	const std::string&	raw() const				{ return _ustring; };
	ustring				uppercase() const		{ return _ustring; };
	ustring				lowercase() const		{ return _ustring; };
	std::string			collate_key() const		{ return _ustring; };

private:
	std::string			_ustring;
};

#endif // LF_BRIO_USTRING_H
