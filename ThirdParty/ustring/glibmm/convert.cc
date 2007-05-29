// Generated by gtkmmproc -- DO NOT MODIFY!

#include <glibmm/convert.h>
#include <glibmm/private/convert_p.h>

// -*- c++ -*-
/* $Id: convert.ccg,v 1.1.1.1 2003/01/07 16:58:23 murrayc Exp $ */

/* Copyright (C) 2002 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glib/gconvert.h>
#include <glib/gmessages.h>
#include <glib/gunicode.h>
#include <glibmm/utility.h>


namespace Glib
{

/**** Glib::IConv **********************************************************/

IConv::IConv(const std::string& to_codeset, const std::string& from_codeset)
:
  gobject_ (g_iconv_open(to_codeset.c_str(), from_codeset.c_str()))
{
  if(gobject_ == reinterpret_cast<GIConv>(-1))
  {
    GError* error = 0;

    // Abuse g_convert() to create a GError object.  This may seem a weird
    // thing to do, but it gives us consistently translated error messages
    // at no further cost.
    g_convert("", 0, to_codeset.c_str(), from_codeset.c_str(), 0, 0, &error);

    // If this should ever fail we're fucked.
    g_assert(error != 0);

    Error::throw_exception(error);
  }
}

IConv::IConv(GIConv gobject)
:
  gobject_ (gobject)
{}

IConv::~IConv()
{
  g_iconv_close(gobject_);
}

size_t IConv::iconv(char** inbuf, gsize* inbytes_left, char** outbuf, gsize* outbytes_left)
{
  return g_iconv(gobject_, inbuf, inbytes_left, outbuf, outbytes_left);
}

void IConv::reset()
{
  // Apparently iconv() on Solaris <= 7 segfaults if you pass in
  // NULL for anything but inbuf; work around that. (NULL outbuf
  // or NULL *outbuf is allowed by Unix98.)

  char* outbuf        = 0;
  gsize inbytes_left  = 0;
  gsize outbytes_left = 0;

  g_iconv(gobject_, 0, &inbytes_left, &outbuf, &outbytes_left);
}

std::string IConv::convert(const std::string& str)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_convert_with_iconv(
      str.data(), str.size(), gobject_, 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}


/**** charset conversion functions *****************************************/

bool get_charset()
{
  return g_get_charset(0);
}

bool get_charset(std::string& charset)
{
  const char* charset_cstr = 0;
  const bool is_utf8 = g_get_charset(&charset_cstr);

  charset = charset_cstr;
  return is_utf8;
}

std::string convert(const std::string& str,
                    const std::string& to_codeset,
                    const std::string& from_codeset)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_convert(
      str.data(), str.size(), to_codeset.c_str(), from_codeset.c_str(),
      0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}

std::string convert_with_fallback(const std::string& str,
                                  const std::string& to_codeset,
                                  const std::string& from_codeset)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_convert_with_fallback(
      str.data(), str.size(), to_codeset.c_str(), from_codeset.c_str(), 0,
      0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}

std::string convert_with_fallback(const std::string& str,
                                  const std::string& to_codeset,
                                  const std::string& from_codeset,
                                  const Glib::ustring& fallback)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_convert_with_fallback(
      str.data(), str.size(), to_codeset.c_str(), from_codeset.c_str(),
      const_cast<char*>(fallback.c_str()), 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}

Glib::ustring locale_to_utf8(const std::string& opsys_string)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_locale_to_utf8(
      opsys_string.data(), opsys_string.size(), 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  const ScopedPtr<char> scoped_buf (buf);
  return Glib::ustring(scoped_buf.get(), scoped_buf.get() + bytes_written);
}

std::string locale_from_utf8(const Glib::ustring& utf8_string)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_locale_from_utf8(
      utf8_string.data(), utf8_string.bytes(), 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}

Glib::ustring filename_to_utf8(const std::string& opsys_string)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_filename_to_utf8(
      opsys_string.data(), opsys_string.size(), 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  const ScopedPtr<char> scoped_buf (buf);
  return Glib::ustring(scoped_buf.get(), scoped_buf.get() + bytes_written);
}

std::string filename_from_utf8(const Glib::ustring& utf8_string)
{
  gsize bytes_written = 0;
  GError* error = 0;

  char *const buf = g_filename_from_utf8(
      utf8_string.data(), utf8_string.bytes(), 0, &bytes_written, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get(), bytes_written);
}

std::string filename_from_uri(const Glib::ustring& uri, Glib::ustring& hostname)
{
  char* hostname_buf = 0;
  GError* error = 0;

  char *const buf = g_filename_from_uri(uri.c_str(), &hostname_buf, &error);

  if(error)
    Error::throw_exception(error);

  // Let's take ownership at this point.
  const ScopedPtr<char> scoped_buf (buf);

  if(hostname_buf)
    hostname = ScopedPtr<char>(hostname_buf).get();
  else
    hostname.erase();

  return std::string(scoped_buf.get());
}

std::string filename_from_uri(const Glib::ustring& uri)
{
  GError* error = 0;
  char *const buf = g_filename_from_uri(uri.c_str(), 0, &error);

  if(error)
    Error::throw_exception(error);

  return std::string(ScopedPtr<char>(buf).get());
}

Glib::ustring filename_to_uri(const std::string& filename, const Glib::ustring& hostname)
{
  GError* error = 0;
  char *const buf = g_filename_to_uri(filename.c_str(), hostname.c_str(), &error);

  if(error)
    Error::throw_exception(error);

  return Glib::ustring(ScopedPtr<char>(buf).get());
}

Glib::ustring filename_to_uri(const std::string& filename)
{
  GError* error = 0;
  char *const buf = g_filename_to_uri(filename.c_str(), 0, &error);

  if(error)
    Error::throw_exception(error);

  return Glib::ustring(ScopedPtr<char>(buf).get());
}

} // namespace Glib


namespace
{
} // anonymous namespace


Glib::ConvertError::ConvertError(Glib::ConvertError::Code error_code, const Glib::ustring& error_message)
:
  Glib::Error (G_CONVERT_ERROR, error_code, error_message)
{}

Glib::ConvertError::ConvertError(GError* gobject)
:
  Glib::Error (gobject)
{}

Glib::ConvertError::Code Glib::ConvertError::code() const
{
  return static_cast<Code>(Glib::Error::code());
}

void Glib::ConvertError::throw_func(GError* gobject)
{
  throw Glib::ConvertError(gobject);
}


