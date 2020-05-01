#pragma once

#include <cstdint>

template <typename T>
struct UID
{
	UID(): index(0) {}
	UID(T _index): index(_index) {}
	inline T id() const { return index; }

protected:
	T index;
};

template <typename T>
inline bool operator==(const UID<T> &a, const UID<T> &b)
{
	return a.id() == b.id();
}

using UID32 = UID<uint32_t>;
using UID64 = UID<uint64_t>;
