#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "color.h"

class Image {
public:
    Image() = default;
    Image(size_t width, size_t height) : width_(width), height_(height), pixels_(height, std::vector<Color>(width)) {}

    size_t GetWidth() const {
        return width_;
    }

    size_t GetHeight() const {
        return height_;
    }

    Color& At(size_t x, size_t y) {
        return pixels_[y][x];
    }

    const Color& At(size_t x, size_t y) const {
        return pixels_[y][x];
    }

    // Clamps coordinates to image boundaries (for border handling in filters)
    const Color& AtClamped(int x, int y) const {
        size_t cx = static_cast<size_t>(std::clamp(x, 0, static_cast<int>(width_) - 1));
        size_t cy = static_cast<size_t>(std::clamp(y, 0, static_cast<int>(height_) - 1));
        return pixels_[cy][cx];
    }

private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<std::vector<Color>> pixels_;
};
