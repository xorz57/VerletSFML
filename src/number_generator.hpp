#pragma once

#include <random>

class NumberGenerator
{
public:
    NumberGenerator() : gen(rd()) {}

    std::random_device rd;
    std::mt19937 gen;
};

template <typename T>
class RealNumberGenerator : public NumberGenerator
{
private:
    std::uniform_real_distribution<T> dis{0.0f, 1.0f};

public:
    RealNumberGenerator() : NumberGenerator() {}

    float get() { return dis(gen); }

    float getUnder(T max) { return get() * max; }

    float getRange(T min, T max) { return min + get() * (max - min); }

    float getRange(T width) { return getRange(-width * 0.5f, width * 0.5f); }
};

template <typename T>
class RNG
{
public:
    static T get() { return gen.get(); }

    static float getUnder(T max) { return gen.getUnder(max); }

    static float getRange(T min, T max) { return gen.getRange(min, max); }

    static float getRange(T width) { return gen.getRange(width); }

private:
    static RealNumberGenerator<T> gen;
};

using RNGf = RNG<float>;

template <typename T>
RealNumberGenerator<T> RNG<T>::gen = RealNumberGenerator<T>();
