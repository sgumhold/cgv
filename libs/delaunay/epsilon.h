#pragma once

template <typename T> struct epsilon {};

template <> struct epsilon<float> { inline static float get_eps() { return 1e-5f; } };
template <> struct epsilon<double> { inline static double get_eps() { return 1e-15; } };
