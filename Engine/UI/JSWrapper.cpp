#include "ui/JSWrapper.h"

#include <unordered_map>
#include <Ultralight/Ultralight.h>


static std::unordered_map<JSObjectRef, JSFunc> callbacks;
static JSValueRef js_callback(JSContextRef ctx, JSObjectRef function, JSObjectRef _this,
	size_t argc, const JSValueRef argv[], JSValueRef*)
{
	auto it = callbacks.find(function);
	if (it == callbacks.end())
		return JSValueMakeNull(ctx);

	JSFunc callback(it->second);

	std::vector<JSObject> args;
	args.reserve(argc);
	for (int i(0); i < argc; i++)
		args.push_back(JSObject(ctx, argv[i]));
	return callback(JSContext(ctx), args).value();
}

// JSContext
JSObject JSContext::window() const
{
	return JSObject(ctx, JSContextGetGlobalObject(ctx));
}

JSObject JSContext::make_null() const
{
	return JSObject(ctx, JSValueMakeNull(ctx));
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
void JSProp::assign(JSObject _obj)
{
	if (type == PropType::ByName)
		JSObjectSetProperty(ctx, obj, name.str, _obj.value(), kJSPropertyAttributeNone, NULL);
	else // ByIndex
		JSObjectSetPropertyAtIndex(ctx, obj, index, _obj.value(), NULL);
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

JSObject::JSObject(JSContextRef _ctx, JSValueRef val):
	ctx(_ctx), obj(JSValueToObject(_ctx, val, NULL))
{ }

JSObject::JSObject(JSContextRef _ctx, int val):
	JSObject(_ctx, JSValueMakeNumber(_ctx, (double)val))
{ }

JSObject::JSObject(JSContextRef _ctx, const std::string& val):
	JSObject(_ctx, JSValueMakeString(_ctx, JSString(val.c_str()).str))
{ }

JSObject::JSObject(JSContextRef _ctx, JSFunc val):
	ctx(_ctx), obj(JSObjectMakeFunctionWithCallback(ctx, NULL, js_callback))
{
	callbacks[obj] = val;
}

uint32_t JSObject::as_uint32() const
{
	return (uint32_t)JSValueToNumber(ctx, obj, nullptr);
}

std::string JSObject::as_string() const
{
	JSStringRef str = JSValueToStringCopy(ctx, obj, NULL);

	size_t max_size = JSStringGetMaximumUTF8CStringSize(str);
	char* buf = new char[max_size];
	size_t size = JSStringGetUTF8CString(str, buf, max_size);
	std::string res = std::string(buf, size - 1);
	delete[] buf;

	return res;
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
