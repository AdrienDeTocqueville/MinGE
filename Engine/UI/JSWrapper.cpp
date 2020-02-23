#include "ui/JSWrapper.h"

#include <Ultralight/Ultralight.h>

// JSContext
JSObject JSContext::window() const
{
	return JSObject(ctx, JSContextGetGlobalObject(ctx));
}

JSObject JSContext::make_array(size_t length) const
{
	JSValueRef *elements = new JSValueRef[length];

	for (size_t i(0); i < length; i++)
		elements[i] = JSValueMakeNull(ctx);

	auto obj = JSObjectMakeArray(ctx, length, elements, NULL);

	delete[] elements;
	return JSObject(ctx, obj);
}

JSObject JSContext::make_uint32array(size_t length) const
{
	auto obj = JSObjectMakeTypedArray(ctx, kJSTypedArrayTypeUint32Array, length, NULL);
	return JSObject(ctx, obj);
}

// JSString
JSString::JSString(const char *_str):
	str(JSStringCreateWithUTF8CString(_str))
{ }

JSString::~JSString()
{
	JSStringRelease(str);
}

// JSProp
JSProp::JSProp(JSContextRef _ctx, JSObjectRef _obj, const char *_name):
	ctx(_ctx), obj(_obj), type(PropType::ByName), name(_name)
{ }

JSProp::JSProp(JSContextRef _ctx, JSObjectRef _obj, unsigned _index):
	ctx(_ctx), obj(_obj), type(PropType::ByIndex), index(_index)
{ }

void JSProp::assign(JSObject _obj)
{
	if (type == PropType::ByName)
	{
		assert(JSObjectHasProperty(ctx, obj, name.str), "Object has no such property");
		JSObjectSetProperty(ctx, obj, name.str, _obj.obj, kJSPropertyAttributeNone, NULL);
	}
	else // ByIndex
		JSObjectSetPropertyAtIndex(ctx, obj, index, _obj.obj, NULL);
}

JSValueRef JSProp::value() const
{
	if (type == PropType::ByName)
	{
		assert(JSObjectHasProperty(ctx, obj, name.str), "Object has no such property");
		return JSObjectGetProperty(ctx, obj, name.str, NULL);
	}
	else // ByIndex
	{
		return JSObjectGetPropertyAtIndex(ctx, obj, index, NULL);
	}
}

// JSObject
JSObject::JSObject(JSContextRef _ctx, JSObjectRef _obj):
	ctx(_ctx), obj(_obj)
{ }

JSObject::JSObject(JSContextRef _ctx, JSValueRef _val):
	ctx(_ctx), obj(JSValueToObject(_ctx, _val, NULL))
{ }

JSObject::JSObject(JSContextRef _ctx, int _val):
	JSObject(_ctx, JSValueMakeNumber(_ctx, (double)_val))
{ }

JSObject::JSObject(JSContextRef _ctx, const std::string& _val):
	JSObject(_ctx, JSValueMakeString(_ctx, JSString(_val.c_str()).str))
{ }

uint32_t JSObject::as_uint32() const
{
	return (uint32_t)JSValueToNumber(ctx, obj, nullptr);
}

void *JSObject::get_typedarray_temp_buffer() const
{
	return JSObjectGetTypedArrayBytesPtr(ctx, obj, nullptr);
}

JSProp JSObject::operator[](const char *prop) const
{
	return JSProp(ctx, obj, prop);
}

JSProp JSObject::operator[](unsigned index) const
{
	return JSProp(ctx, obj, index);
}

JSObject JSObject::call(std::initializer_list<JSObject> &args) const
{
	assert(JSObjectIsFunction(ctx, obj), "Object is not a function");

	JSValueRef exception;

	JSObjectRef _this = JSContextGetGlobalObject(ctx);

	size_t argc = args.size();
	JSValueRef *argv = argc ? new JSValueRef[argc] : NULL;

	size_t i(0);
	for (const JSObject& arg : args)
		argv[i++] = arg.obj;

	JSValueRef ret = JSObjectCallAsFunction(ctx, obj, _this, argc, argv, &exception);

	if (argc)
		delete[] argv;

	return JSObject(ctx, ret);
}

namespace js
{
void unprotect(JSContextRef ctx, JSValueRef value)
{
	JSValueUnprotect(ctx, value);
}
}
