#pragma once

#include <cstdint>

namespace UID
{
struct uid32_t
{
	uint32_t id: 24;
	uint32_t gen: 8;
};

struct uid64_t
{
	uint32_t id;
	uint32_t gen;
};

static_assert(sizeof(uid32_t) == 4, "Invalid size");
static_assert(sizeof(uid64_t) == 8, "Invalid size");

template <typename T, typename U>
struct UID
{
	UID(): index(0) {}
	UID(uint32_t id, uint32_t gen): uid{id, gen} {}
	
	inline auto id() const { return uid.id; }
	inline auto gen() const { return uid.gen; }

	inline bool operator==(const UID<T, U> &other)
	{
		return index == other.index;
	}

private:
	union {
		T uid;
		U index;
	};
};
}

using UID32 = UID::UID<UID::uid32_t, uint32_t>;
using UID64 = UID::UID<UID::uid64_t, uint64_t>;
