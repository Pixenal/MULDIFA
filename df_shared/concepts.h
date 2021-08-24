#pragma once
#include <concepts>

template <typename T>
concept convertible_to_int = std::convertible_to<T, int>;

template <typename T>
concept floating_point = std::floating_point<T>;

template <typename T>
concept signed_integral = std::signed_integral<T>;

template <typename T>
concept unsigned_integral = std::unsigned_integral<T>;

template <typename T>
concept integral = std::integral<T>;

template <typename T>
concept not_convertible_to_int = !std::convertible_to<T, int>;