#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Core/Utils.h"
#include "Core/System.h"
#include "Core/AssetDatabase.h"
#include "IO/json.hpp"

#include "Utility/stb_sprintf.h"

static const char hex_chars[] = "0123456789ABCDEF";

struct DataModel
{
	DataModel(bool writing, bool binary)
	{
		data = backup = nullptr;

		flags = 0;
		if (writing) flags |= 1 << 0;
		if (binary)  flags |= 1 << 1;
	}

	bool is_writing() const { return flags & (1 << 0); }
	bool is_reading() const { return !is_writing(); }

	template<typename T>
	void transfer(const char *name, T& value)
	{
		this->transfer(name, &value, sizeof(T));
	}

	virtual bool begin_system(const system_type_t &type) = 0;
	virtual void end_system() = 0;

	void transfer(const char *name, UID32 &value) = delete;
	virtual void transfer(const char *name, UID32 &value, const object_type_t &type) = 0;
	virtual void transfer(const char *name, char *&value) = 0;
	virtual void transfer(const char *name, std::string &value) = 0;
	virtual void transfer(const char *name, void *value, size_t size) = 0;

	nlohmann::json document;
	nlohmann::json::object_t *data;
	nlohmann::json::object_t *backup;

	private: uint32_t flags;
};

struct DataModelReader : public DataModel
{
	DataModelReader(const std::string &input): DataModel(false, false)
	{
		document = nlohmann::json::parse(input);

		types = document["types"].get_ptr<nlohmann::json::array_t*>();
		objects = document["objects"].get_ptr<nlohmann::json::array_t*>();
		current = 0;

		ASSERT(types->size() == objects->size(), "Invalid data");
	}

	bool empty() { return current >= objects->size(); }

	// Objects
	void transfer(const object_type_t **type, UID32 *object)
	{
		const char *type_name = (*types)[current].get_ptr<nlohmann::json::string_t *>()->c_str();
		data = (*objects)[current].get_ptr<nlohmann::json::object_t*>();
		current++;
		
		*type = AssetDatabase::object_type(type_name);
		ASSERT(type != nullptr, "Unknown object type '" << type_name << "'"); // assert ?

		*object = (*type)->transfer(*object, this);
	}

	// Systems
	bool begin_system(const system_type_t &type) override
	{
		const auto &sys_data = data->find(type.name);
		if (sys_data == data->end())
			return false;
		
		backup = data;
		data = sys_data->second.get_ptr<nlohmann::json::object_t *>();
		return true;
	}

	void end_system() override
	{
		data = backup;
	}

	// Data
	void transfer(const char *name, UID32 &value, const object_type_t &type) override
	{
		char *separator;
		const std::string &str = *(*data)[name].get_ptr<nlohmann::json::string_t*>();
		guid_t guid = strtoull(str.c_str(), &separator, 16);
		int id = atoi(separator + 1);

		value = AssetDatabase::get(type, guid, id);
	}

	void transfer(const char *name, char* &value) override
	{
		const std::string &str = *(*data)[name].get_ptr<nlohmann::json::string_t*>();
		if (str.size() == 0)
		{
			value = nullptr;
		}
		else
		{
			value = strdup(str.c_str());
		}
	}

	void transfer(const char *name, std::string &value) override
	{
		value = (*data)[name].get<std::string>();
	}

	void transfer(const char *name, void *value, size_t size) override
	{
		const char *encoded = (*data)[name].get_ptr<nlohmann::json::string_t*>()->c_str();
		uint8_t *bytes = (uint8_t *)value;
		for (int i = 0; i < size; i++)
		{
			int hi = strchr(hex_chars, encoded[2*i+0]) - hex_chars;
			int lo = strchr(hex_chars, encoded[2*i+1]) - hex_chars;
			*bytes++ = (hi << 4) | lo;
		}
	}

	nlohmann::json::array_t *types;
	nlohmann::json::array_t *objects;
	size_t current;
};

struct DataModelWriter : public DataModel
{
	DataModelWriter(): DataModel(true, false)
	{
		document = nlohmann::json();
		document["types"] = nlohmann::json::array();
		document["objects"] = nlohmann::json::array();
		
		types = document["types"].get_ptr<nlohmann::json::array_t*>();
		objects = document["objects"].get_ptr<nlohmann::json::array_t*>();
	}

	// Objects
	void transfer(const object_type_t *type, UID32 object)
	{
		types->push_back(type->name);
		objects->emplace_back(nlohmann::json::object());

		data = objects->back().get_ptr<nlohmann::json::object_t *>();
		type->transfer(object, this);
	}

	// Systems
	bool begin_system(const system_type_t &type) override
	{
		tmp_data = nlohmann::json::object();

		backup = data;
		data = tmp_data.get_ptr<nlohmann::json::object_t *>();
		sys_name = type.name;
		return true;
	}

	void end_system() override
	{
		data = backup;
		if (tmp_data.size() != 0)
			(*data)[sys_name] = std::move(tmp_data);
	}

	// Data
	void transfer(const char *name, UID32 &value, const object_type_t &type) override
	{
		int id = 0;
		guid_t guid = 0;
		AssetDatabase::get_guid_and_id(value, type, &guid, &id);
		
		static char buf[32];
		stbsp_snprintf(buf, sizeof(buf), "%llx:%d", guid, id);
		(*data)[name] = buf;
	}

	void transfer(const char *name, char *&value) override
	{
		int length = value == nullptr ? 0 : strlen(value);
		if (length == 0)
		{
			(*data)[name] = "";
		}
		else
		{
			(*data)[name] = value;
		}
	}

	void transfer(const char *name, std::string &value) override
	{
		(*data)[name] = value;
	}

	void transfer(const char *name, void *value, size_t size) override
	{
		char *encoded = (char*)malloc(sizeof(char) * (2 * size + 1));
		uint8_t *bytes = (uint8_t *)value;
		for (int i = 0; i < size; i++)
		{
			int lo = bytes[i] & 0xF;
			int hi = bytes[i] >> 4;
			encoded[2 * i + 0] = hex_chars[hi];
			encoded[2 * i + 1] = hex_chars[lo];
		}
		encoded[2 * size] = '\0';

		(*data)[name] = encoded;
	}

	std::string to_string(int indent = -1) const
	{
		return document.dump(indent);
	}


	nlohmann::json::array_t *types;
	nlohmann::json::array_t *objects;
	nlohmann::json tmp_data;
	const char *sys_name;
};
