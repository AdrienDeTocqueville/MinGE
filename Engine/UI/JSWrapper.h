#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <initializer_list>

typedef const struct OpaqueJSContext* JSContextRef;
typedef const struct OpaqueJSValue* JSValueRef;
typedef struct OpaqueJSValue* JSObjectRef;
typedef struct OpaqueJSString* JSStringRef;

struct JSProp;
struct JSObject;
typedef const std::vector<JSObject>& JSArgs;
typedef std::function<JSObject(struct JSContext, JSArgs)> JSFunc;
#define MethodAsJSFunc(func) ((JSFunc)std::bind(func, this, std::placeholders::_1, std::placeholders::_2))

struct JSString
{
	JSString(const char *_str);
	JSString(JSStringRef _str): str(_str) {}
	~JSString();

	std::string to_string() const;

	JSStringRef str;
};

struct JSObject
{
	// Constructors
	JSObject(JSContextRef _ctx, JSObjectRef _obj);
	JSObject(JSContextRef _ctx, JSValueRef _val);
	JSObject(JSContextRef _ctx, int _val);
	JSObject(JSContextRef _ctx, const std::string& _val);
	JSObject(JSContextRef _ctx, JSFunc _val);
	JSObject(JSProp _prop);

	~JSObject() {}

	// Conversions
	uint32_t as_uint32() const;
	std::string as_string() const;

	void *get_typedarray_temp_buffer() const;

	JSObjectRef object() const;
	JSValueRef value() const { return val; }

	bool is_number() const;

	// Operators
	JSProp operator[](const char *prop) const;
	JSProp operator[](unsigned index) const;

	template<typename... Args>
	JSObject operator()(Args&&... args) const
	{ return call(std::initializer_list<JSObject>{args...}); }

private:
	JSObject call(std::initializer_list<JSObject> &args) const;

	JSContextRef ctx;

	bool is_value;
	union
	{
		JSObjectRef obj;
		JSValueRef val;
	};
};

struct JSProp
{
	friend JSObject;

	JSProp(JSContextRef _ctx, JSObjectRef _obj, const char *_name):
		ctx(_ctx), obj(_obj), type(PropType::ByName), name(_name)
	{ }

	JSProp(JSContextRef _ctx, JSObjectRef _obj, unsigned _index):
		ctx(_ctx), obj(_obj), type(PropType::ByIndex), index(_index)
	{ }

	~JSProp() {}

	template<typename T>
	JSProp& operator=(T value) { assign(JSObject(ctx, value)); return *this; }

	template<>
	JSProp& operator=(JSObject value) { assign(value); return *this; }

	template<typename... Args>
	JSObject operator()(Args&&... args) const { return JSObject(ctx, value())(args...); }

private:
	JSValueRef value() const;
	void assign(JSObject _obj);

	enum PropType { ByName, ByIndex };

	JSContextRef ctx;
	JSObjectRef obj;

	PropType type;
	union {
		JSString name;
		unsigned index;
	};
};

struct JSContext
{
	JSContext(JSContextRef _ctx): ctx(_ctx) {}

	JSObject window() const;

	template<typename T>
	JSObject make(T value) const { return JSObject(ctx, value); }

	JSObject make_null() const;
	JSObject make_array(size_t length) const;
	JSObject make_uint32array(size_t length) const;

private:
	JSContextRef ctx;
};
