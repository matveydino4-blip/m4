#pragma once

#include <array>

#include "filter.h"
#include "image.h"

// Base class for 3x3 convolution matrix filters
class MatrixFilter : public Filter {
public:
    using Matrix3x3 = std::array<std::array<double, 3>, 3>;

    Image Apply(const Image& image) const override;

protected:
    virtual Matrix3x3 GetMatrix() const = 0;
};
