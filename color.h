#pragma once

#include <algorithm>
#include <cstdint>

struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    Color() = default;
    Color(double r, double g, double b) : r(r), g(g), b(b) {}

    Color Clamped() const {
        return {std::clamp(r, 0.0, 1.0), std::clamp(g, 0.0, 1.0), std::clamp(b, 0.0, 1.0)};
    }
};
