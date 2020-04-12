#pragma once

#include <initializer_list>
#include <type_traits>
#include <cstdlib>
#include <vector>

namespace Random
{

void init();
void set_seed(unsigned _seed);
unsigned get_seed();

inline bool next_bool()
{
	return (double)rand() / RAND_MAX < 0.5;
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type next(T _min = 0, T _max = 1) // range is [_min, _max[
{
	return _min + (_max - _min) * (T)rand() / RAND_MAX;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type next(T _min = 0, T _max = 2) // range is [_min, _max[
{
	return _min + (rand() % (_max - _min));
}


template <typename T>
const T& element(std::initializer_list<T> _elements)
{
	return *(_elements.begin() + Random::next((size_t)0, _elements.size()));
}

template <typename T>
T& element(std::vector<T>& _elements)
{
	return _elements[Random::next((size_t)0, _elements.size())];
}

template <typename T>
const T& element(const std::vector<T>& _elements)
{
	return _elements[Random::next((size_t)0, _elements.size())];
}

}
