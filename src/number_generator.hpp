#pragma once

#include <random>

template <typename T>
class RealNumberGenerator
{
private:
    std::uniform_real_distribution<T> dis{0.0f, 1.0f};
    std::random_device rd;
    std::mt19937 gen{rd()};

public:
    float getRange(T min, T max) { return min + dis(gen) * (max - min); }
};

template <typename T>
class RNG
{
public:
    static float getRange(T min, T max) { return gen.getRange(min, max); }

private:
    static RealNumberGenerator<T> gen;
};

using RNGf = RNG<float>;

template <typename T>
RealNumberGenerator<T> RNG<T>::gen = RealNumberGenerator<T>();
