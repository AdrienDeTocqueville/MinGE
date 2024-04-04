#pragma once

#include <stdint.h>

namespace UID
{
struct uid32_t
{
	uint32_t id: 24;
	uint32_t gen: 8;
	static const uint32_t invalid = (1 << 24) - 1;
};

struct uid64_t
{
	uint32_t id;
	uint32_t gen;
	static const uint32_t invalid = -1;
};

static_assert(sizeof(uid32_t) == 4, "Invalid size");
static_assert(sizeof(uid64_t) == 8, "Invalid size");

template <typename T, typename U>
struct UID
{
	UID(uint32_t id, uint32_t gen): uid{id, gen} {}
	UID(): UID(T::invalid, -1) {}

	inline auto id()   const { return uid.id; }
	inline auto gen()  const { return uid.gen; }
	inline auto uint() const { return 0; } // TODO: remove

	inline bool operator==(const UID<T, U> &other) const
	{
		return index == other.index;
	}
	inline bool operator!=(const UID<T, U> &other) const
	{
		return index != other.index;
	}

	inline bool valid() const { return uid.id != T::invalid; }

private:
	union {
		T uid;
		U index;
	};
};
}

using UID32 = UID::UID<UID::uid32_t, uint32_t>;
using UID64 = UID::UID<UID::uid64_t, uint64_t>;
