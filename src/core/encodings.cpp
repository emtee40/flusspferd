// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <iconv.h>
#include <errno.h>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include "flusspferd/create.hpp"


using namespace boost;
using namespace flusspferd;

void flusspferd::load_encodings_module(object container) {
  object exports = container.get_property_object("exports");

  // Load the binary module
  global().call("require", "binary");

  create_native_function(
    exports,
    "convertToString", &encodings::convert_to_string);

  create_native_function(
    exports,
    "convertFromString", &encodings::convert_from_string);

  create_native_function( exports, "convert", &encodings::convert);
}

// HELPER METHODS

// Actually do the conversion.
binary::vector_type do_convert(iconv_t conv, binary::vector_type const &in);

// call iconv_open, or throw an error if it cant
iconv_t open_convert(std::string const &from, std::string const &to);

// If no direct conversion is possible, do it via utf8. Helper method
object convert_via_utf8(std::string toEnc, std::string fromEnc,
                        binary const &source);


string
encodings::convert_to_string(std::string &enc, binary &source_binary) {

  binary::vector_type const &source = source_binary.get_const_data();

  to_lower(enc);
  if (enc == "utf-8") {
    binary::vector_type const &source = source_binary.get_const_data();
    return string( (char*)&source[0], source.size());
  }
  else if (enc == "utf-16") {
    // TODO: Assert on the possible alignment issue here
    binary::vector_type const &source = source_binary.get_const_data();
    return string( reinterpret_cast<char16_t const*>(&source[0]), source.size()/2);
  }
  else {
    // Not UTF-8 or UTF-16, so convert to utf-16

    // Except UTF-8 seems to suffer from a byte order issue. UTF-8 is less
    // error prone it seems. FIXME
    iconv_t conv = open_convert(enc, "utf-8");
    binary::vector_type utf16 = do_convert(conv, source);
    iconv_close(conv);
    return string( reinterpret_cast<char const*>(&utf16[0]), utf16.size());
  }
  return string();
}


object
encodings::convert_from_string(std::string &, binary const &) {
  size_t n = 0;
  return create_native_object<byte_string>(object(), (unsigned char*)"", n);
}

object encodings::convert(std::string &from, std::string &to,
    binary &source_binary)
{
  binary::vector_type const &source = source_binary.get_const_data();

  to_lower(from);
  to_lower(to);
  if ( from == to ) {
    // Encodings are the same, just return a copy of the binary
    return create_native_object<byte_string>(
      object(),
      &source[0],
      source.size()
    );
  }

  iconv_t conv = open_convert(from, to);

  binary::vector_type buf = do_convert(conv, source);
  iconv_close(conv);
  return create_native_object<byte_string>(object(), &buf[0], buf.size());
}
// End JS methods

binary::vector_type
do_convert(iconv_t conv, binary::vector_type const &source) {
  binary::vector_type outbuf;

  size_t out_left,
         in_left = source.size();

  out_left = in_left + in_left/16 + 32; // GPSEE's Wild-assed guess.

  outbuf.resize(out_left);

  const unsigned char *inbytes  = &source[0],
                      *outbytes = &outbuf[0];

  while (in_left) {
    size_t n = iconv(conv,
                     (char**)&inbytes, &in_left,
                     (char**)&outbytes, &out_left
                    );

    if (n == (size_t)(-1)) {
      switch (errno) {
        case E2BIG:
          // Not enough space in output
          // Use GPSEE's WAG again. +32 assumes no encoding needs more than 32
          // bytes(!) pre character. Probably a safe bet.
          size_t new_size = in_left + in_left/4 + 32,
                 old_size = outbytes - &outbuf[0];

          outbuf.resize(old_size + new_size);

          // The vector has probably realloced, so recalculate outbytes
          outbytes =  &outbuf[old_size];
          out_left += new_size;

          continue;

        case EILSEQ:
          // An invalid multibyte sequence has been encountered in the input.
        case EINVAL:
          // An incomplete multibyte sequence has been encountered in the input.

          // Since we have provided the entire input, both these cases are the
          // same.
          throw flusspferd::exception("convert error", "TypeError");
          break;
      }
    }

    // Else all chars got converted
    in_left -= n;
  }
  outbuf.resize(outbytes - &outbuf[0]);
  return outbuf;
}

iconv_t open_convert(std::string const &from, std::string const &to) {
  iconv_t conv = iconv_open(to.c_str(), from.c_str());

  if (conv == (iconv_t)(-1)) {
    std::stringstream ss;
    ss << "Unable to convert from \"" << from
       << "\" to \"" << to << "\"";
    throw flusspferd::exception(ss.str().c_str());
  }
  return conv;
}

object
convert_via_utf8(std::string const &to, std::string const &from, 
                 binary const &) {
  iconv_t to_utf   = iconv_open("utf-8", from.c_str()),
          from_utf = iconv_open(to.c_str(), "utf-8");

  if (to_utf == (iconv_t)(-1) || from_utf == (iconv_t)(-1)) {

    if (to_utf)
      iconv_close(to_utf);
    if (from_utf)
      iconv_close(from_utf);

    std::stringstream ss;
    ss << "Unable to convert from \"" << from
       << "\" to \"" << to << "\"";
    throw flusspferd::exception(ss.str().c_str());
  }
  return object();
}
