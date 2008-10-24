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

#include "flusspferd/native_function_base.hpp"
#include "flusspferd/call_context.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/spidermonkey/function.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <boost/foreach.hpp>
#include <js/jsapi.h>

using namespace flusspferd;

class native_function_base::impl {
public:
  impl(unsigned arity, std::string const &name)
  : arity(arity), name(name)
  {}

  static JSBool call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
  static void finalize(JSContext *, JSObject *);

  unsigned arity;
  std::string name;

  static JSClass function_parent_class;
};

native_function_base::native_function_base(unsigned arity)
: p(new impl(arity, std::string()))
{}

native_function_base::native_function_base(unsigned arity, std::string const &name)
: p(new impl(arity, name))
{}

native_function_base::~native_function_base() { }

void native_function_base::set_arity(unsigned arity) {
  p->arity = arity;
}

unsigned native_function_base::get_arity() const {
  return p->arity;
}

void native_function_base::set_name(std::string const &name) {
  p->name = name;
}

std::string const &native_function_base::get_name() const {
  return p->name;
}

JSClass native_function_base::impl::function_parent_class = {
  "FunctionParent",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
  &native_function_base::impl::finalize,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

function native_function_base::create_function() {
  JSContext *ctx = Impl::current_context();

  JSFunction *function;

  {
    local_root_scope scope;

    JSObject *parent = JS_NewObject(ctx, &impl::function_parent_class, 0, 0);

    if (!parent)
      throw exception("Could not create native function");

    JS_SetPrivate(ctx, parent, this);

    function = JS_NewFunction(
        ctx, &impl::call_helper,
        p->arity, 0, parent, p->name.c_str());
  }

  return Impl::wrap_function(function);
}

JSBool native_function_base::impl::call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    JSObject *function = JSVAL_TO_OBJECT(argv[-2]);
    JSObject *parent = JS_GetParent(ctx, function);

    native_function_base *self =
      (native_function_base *) JS_GetInstancePrivate(ctx, parent, &function_parent_class, 0);

    if (!self)
      throw exception("Could not call native function");

    call_context x;

    x.self = Impl::wrap_object(obj);
    x.arg = Impl::arguments_impl(argc, argv);
    x.result.bind(Impl::wrap_jsvalp(rval));
    x.function = Impl::wrap_object(function);

    self->call(x);
  } FLUSSPFERD_CALLBACK_END;
}

void native_function_base::impl::finalize(JSContext *ctx, JSObject *parent) {
  current_context_scope scope(Impl::wrap_context(ctx));

  native_function_base *self =
    (native_function_base *) JS_GetInstancePrivate(ctx, parent, &function_parent_class, 0);

  if (!self)
    throw exception("Could not finalize native function");

  delete self;
}