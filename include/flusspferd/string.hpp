// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_STRING_HPP
#define FLUSSPFERD_STRING_HPP

#include "convert.hpp"
#include "implementation/string.hpp"
#include <string>

namespace flusspferd {

class value;

/**
 * A Javascript string.
 *
 * @ingroup value_types
 */
class string : public Impl::string_impl {
public:
  /// Construct an empty string.
  string();

  /**
   * Construct a string from a value.
   *
   * @param v The value to convert to a string.
   */
  string(value const &v);

  /**
   * Construct a string from a UTF-8 input string.
   *
   * @param str The UTF-8 string.
   * @param length The length in bytes. If this is <code>0</code>, std::strlen
   *            will be used to determine the length.
   */
  string(char const *str, std::size_t length = 0);
  string(char16_t const *str, std::size_t length);
  string(std::string const &s);
  string(std::basic_string<char16_t> const &s);
  string(Impl::string_impl const &s)
    : Impl::string_impl(s)
  { }

  ~string();

  string &operator=(string const &o);

  std::size_t length() const;

  std::size_t size() const { return length(); }

  bool empty() const {
    return !length();
  }

  std::string to_string() const;
  char const *c_str() const;

  std::basic_string<char16_t> to_utf16_string() const;
  char16_t const *data() const;

  string substr(size_t start, size_t length);

  static string concat(string const &a, string const &b);
};

/**
 * Compare two string%s for equality.
 *
 * @relates string
 */
bool operator==(string const &lhs, string const &rhs);

/**
 * Compare two string%s lexicographically.
 *
 * @return Whether @p lhs is lexicographically smaller than @p rhs.
 *
 * @relates string
 */
bool operator<(string const &lhs, string const &rhs);

template<>
struct detail::convert<string> {
  typedef to_value_helper<string> to_value;

  struct from_value {
    root_value root;

    string perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s;
    }
  };
};

template<>
struct detail::convert<char const *> {
  typedef to_value_helper<char const *, string> to_value;

  struct from_value {
    root_value root;

    char const *perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s.c_str();
    }
  };
};

template<>
struct detail::convert<std::string> {
  typedef to_value_helper<std::string, string> to_value;

  struct from_value {
    std::string perform(value const &v) {
      return v.to_string().to_string();
    }
  };
};

template<>
struct detail::convert<std::basic_string<char16_t> > {
  typedef std::basic_string<char16_t> string_t;

  typedef to_value_helper<string_t, string> to_value;

  struct from_value {
    string_t perform(value const &v) {
      return v.to_string().to_utf16_string();
    }
  };
};

}

#endif /* FLUSSPFERD_STRING_HPP */