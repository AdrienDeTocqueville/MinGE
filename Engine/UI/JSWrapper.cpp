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

static void handle_exception(JSContextRef ctx, JSValueRef exception)
{
	assert(JSValueGetType(ctx, exception) == kJSTypeObject);

	JSObject obj = JSObject(ctx, exception);
	uint32_t line = JSObject(obj["line"]).as_uint32();
	uint32_t column = JSObject(obj["column"]).as_uint32();
	auto sourceURL = JSObject(obj["sourceURL"]).as_string();

	auto func = JSObject(obj["toString"]);
	JSValueRef ret = JSObjectCallAsFunction(ctx, func.object(), obj.object(), 0, NULL, NULL);
	auto error = JSObject(ctx, ret).as_string();

	printf("%s\n\tat %s:%d:%d\n", error.c_str(), sourceURL.c_str(), line, column);
}

static void debug_value(JSContextRef ctx, JSValueRef value)
{
	auto type = JSValueGetType(ctx, value);

	if (type == kJSTypeObject)
	{
		printf("Object:\n");
		JSObject obj = JSObject(ctx, value);
		auto names = JSObjectCopyPropertyNames(ctx, obj.object());
		auto count = JSPropertyNameArrayGetCount(names);
		for (size_t i = 0; i < count; i++)
		{
			JSStringRef name = JSPropertyNameArrayGetNameAtIndex(names, i);
			printf(" - %s\n", JSString(name).to_string());
		}
	}
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

std::string JSString::to_string() const
{
	size_t max_size = JSStringGetMaximumUTF8CStringSize(str);
	char* buf = new char[max_size];
	size_t size = JSStringGetUTF8CString(str, buf, max_size);
	std::string res = std::string(buf, size - 1);
	delete[] buf;

	return res;
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
	ctx(_ctx), is_value(false), obj(_obj)
{ }

JSObject::JSObject(JSContextRef _ctx, JSValueRef _val):
	ctx(_ctx), is_value(true), val(_val)
{ }

JSObject::JSObject(JSContextRef _ctx, int _val):
	JSObject(_ctx, JSValueMakeNumber(_ctx, (double)_val))
{ }

JSObject::JSObject(JSContextRef _ctx, const std::string& _val):
	JSObject(_ctx, JSValueMakeString(_ctx, JSString(_val.c_str()).str))
{ }

JSObject::JSObject(JSContextRef _ctx, JSFunc _val):
	ctx(_ctx), obj(JSObjectMakeFunctionWithCallback(ctx, NULL, js_callback))
{
	callbacks[obj] = _val;
}

JSObject::JSObject(JSProp _prop):
	JSObject(_prop.ctx, _prop.value())
{ }

uint32_t JSObject::as_uint32() const
{
	return (uint32_t)JSValueToNumber(ctx, val, nullptr);
}

std::string JSObject::as_string() const
{
	JSStringRef str = JSValueToStringCopy(ctx, val, NULL);
	return JSString(str).to_string();
}

JSObjectRef JSObject::object() const
{
	if (is_value)
		return JSValueToObject(ctx, val, NULL);
	return obj;
}

void *JSObject::get_typedarray_temp_buffer() const
{
	return JSObjectGetTypedArrayBytesPtr(ctx, object(), nullptr);
}

bool JSObject::is_number() const
{
	return JSValueIsNumber(ctx, val);
}

JSProp JSObject::operator[](const char *prop) const
{
	return JSProp(ctx, object(), prop);
}

JSProp JSObject::operator[](unsigned index) const
{
	return JSProp(ctx, object(), index);
}

JSObject JSObject::call(std::initializer_list<JSObject> &args) const
{
	assert(JSObjectIsFunction(ctx, object()), "Object is not a function");

	JSValueRef exception;

	JSObjectRef _this = JSContextGetGlobalObject(ctx);

	size_t argc = args.size();
	JSValueRef *argv = argc ? new JSValueRef[argc] : NULL;

	size_t i(0);
	for (const JSObject& arg : args)
		argv[i++] = arg.value();

	JSValueRef ret = JSObjectCallAsFunction(ctx, object(), _this, argc, argv, &exception);

	if (argc)
		delete[] argv;

	if (ret == nullptr)
		handle_exception(ctx, exception);
	return JSObject(ctx, ret);
}
