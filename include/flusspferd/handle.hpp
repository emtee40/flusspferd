// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009, 2010 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

#ifndef FLUSSPFERD_HANDLE_HPP
#define FLUSSPFERD_HANDLE_HPP

namespace flusspferd {

namespace detail {
  class handle_base {
  public:
  };

  template<typename T>
  class handle : public handle_base {
  public:
    T *get() {
      return &value;
    }

  private:
    T value;
  };
}

template<typename T>
class handle {
private:
  typedef T &reference;
  typedef T const &const_reference;
  typedef T *pointer;
  typedef T const *const_pointer;

  typedef detail::handle<T> internal_type;
  typedef boost::shared_ptr<internal_type> pointer_type;

public:
  handle(T const &value)
    : p(new internal_type(value))
    {}

  ~handle()
    {}

  reference operator*() const {
    return *p->get();
  }

  pointer operator->() const {
    return p->get();
  }

private:
  pointer_type p;
};

}

#endif
