#include "matrix_filter.h"

#include <cstddef>

Image MatrixFilter::Apply(const Image& image) const {
    const Matrix3x3 matrix = GetMatrix();
    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();
    Image result(width, height);

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const Color& c = image.AtClamped(static_cast<int>(x) + dx, static_cast<int>(y) + dy);
                    double weight = matrix[static_cast<size_t>(dy + 1)][static_cast<size_t>(dx + 1)];
                    r += weight * c.r;
                    g += weight * c.g;
                    b += weight * c.b;
                }
            }
            result.At(x, y) = Color(r, g, b).Clamped();
        }
    }

    return result;
}
