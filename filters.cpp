#include "filters.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// --- CropFilter ---

Image CropFilter::Apply(const Image& image) const {
    const size_t new_width = std::min(width_, image.GetWidth());
    const size_t new_height = std::min(height_, image.GetHeight());
    Image result(new_width, new_height);
    for (size_t y = 0; y < new_height; ++y) {
        for (size_t x = 0; x < new_width; ++x) {
            result.At(x, y) = image.At(x, y);
        }
    }
    return result;
}

// --- GrayscaleFilter ---

Image GrayscaleFilter::Apply(const Image& image) const {
    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();
    Image result(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const Color& c = image.At(x, y);
            double gray = 0.299 * c.r + 0.587 * c.g + 0.114 * c.b;
            result.At(x, y) = Color(gray, gray, gray);
        }
    }
    return result;
}

// --- NegativeFilter ---

Image NegativeFilter::Apply(const Image& image) const {
    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();
    Image result(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const Color& c = image.At(x, y);
            result.At(x, y) = Color(1.0 - c.r, 1.0 - c.g, 1.0 - c.b);
        }
    }
    return result;
}

// --- SharpeningFilter ---

MatrixFilter::Matrix3x3 SharpeningFilter::GetMatrix() const {
    return {{{0.0, -1.0, 0.0},
             {-1.0, 5.0, -1.0},
             {0.0, -1.0, 0.0}}};
}

// --- EdgeDetectionFilter ---

MatrixFilter::Matrix3x3 EdgeDetectionFilter::GetMatrix() const {
    return {{{0.0, -1.0, 0.0},
             {-1.0, 4.0, -1.0},
             {0.0, -1.0, 0.0}}};
}

Image EdgeDetectionFilter::Apply(const Image& image) const {
    GrayscaleFilter gs;
    Image gray = gs.Apply(image);
    Image convolved = MatrixFilter::Apply(gray);

    const size_t width = convolved.GetWidth();
    const size_t height = convolved.GetHeight();
    Image result(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double val = convolved.At(x, y).r >= threshold_ ? 1.0 : 0.0;
            result.At(x, y) = Color(val, val, val);
        }
    }
    return result;
}

// --- GaussianBlurFilter ---

Image GaussianBlurFilter::Apply(const Image& image) const {
    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();

    // Kernel radius: 3*sigma is standard cutoff
    const int radius = static_cast<int>(std::ceil(3.0 * sigma_));
    const size_t kernel_size = static_cast<size_t>(2 * radius + 1);

    // Build 1D Gaussian kernel
    std::vector<double> kernel(kernel_size);
    double kernel_sum = 0.0;
    for (int i = -radius; i <= radius; ++i) {
        double val = std::exp(-(static_cast<double>(i * i)) / (2.0 * sigma_ * sigma_));
        kernel[static_cast<size_t>(i + radius)] = val;
        kernel_sum += val;
    }
    for (double& k : kernel) {
        k /= kernel_sum;
    }

    // Horizontal pass
    Image horizontal(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            for (int dx = -radius; dx <= radius; ++dx) {
                const Color& c = image.AtClamped(static_cast<int>(x) + dx, static_cast<int>(y));
                double w = kernel[static_cast<size_t>(dx + radius)];
                r += w * c.r;
                g += w * c.g;
                b += w * c.b;
            }
            horizontal.At(x, y) = Color(r, g, b);
        }
    }

    // Vertical pass
    Image result(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            for (int dy = -radius; dy <= radius; ++dy) {
                const Color& c = horizontal.AtClamped(static_cast<int>(x), static_cast<int>(y) + dy);
                double w = kernel[static_cast<size_t>(dy + radius)];
                r += w * c.r;
                g += w * c.g;
                b += w * c.b;
            }
            result.At(x, y) = Color(r, g, b).Clamped();
        }
    }

    return result;
}

// --- CrystallizeFilter ---
// Voronoi-based crystallize: each pixel gets the color of the nearest "seed" point.
// Seeds are placed on a regular grid with random offsets within each cell.

Image CrystallizeFilter::Apply(const Image& image) const {
    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();

    if (width == 0 || height == 0 || cell_size_ == 0) {
        return image;
    }

    const size_t cols = (width + cell_size_ - 1) / cell_size_;
    const size_t rows = (height + cell_size_ - 1) / cell_size_;

    // Generate seed points with a fixed pseudo-random pattern inside each cell
    std::vector<std::vector<std::pair<size_t, size_t>>> seeds(rows, std::vector<std::pair<size_t, size_t>>(cols));
    for (size_t gy = 0; gy < rows; ++gy) {
        for (size_t gx = 0; gx < cols; ++gx) {
            // Pseudo-random offset: deterministic hash based on grid position
            size_t hash = (gy * 2654435761ULL) ^ (gx * 2246822519ULL);
            size_t ox = hash % cell_size_;
            size_t oy = (hash >> 16) % cell_size_;
            size_t sx = std::min(gx * cell_size_ + ox, width - 1);
            size_t sy = std::min(gy * cell_size_ + oy, height - 1);
            seeds[gy][gx] = {sx, sy};
        }
    }

    Image result(width, height);

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            // Only check seeds in the surrounding 2x2 grid cells for efficiency
            size_t center_gx = x / cell_size_;
            size_t center_gy = y / cell_size_;

            double best_dist = std::numeric_limits<double>::max();
            size_t best_sx = 0;
            size_t best_sy = 0;

            const int search = 2;
            for (int dgy = -search; dgy <= search; ++dgy) {
                for (int dgx = -search; dgx <= search; ++dgx) {
                    int ngx = static_cast<int>(center_gx) + dgx;
                    int ngy = static_cast<int>(center_gy) + dgy;
                    if (ngx < 0 || ngy < 0 || static_cast<size_t>(ngx) >= cols || static_cast<size_t>(ngy) >= rows) {
                        continue;
                    }
                    auto [sx, sy] = seeds[static_cast<size_t>(ngy)][static_cast<size_t>(ngx)];
                    double dx = static_cast<double>(x) - static_cast<double>(sx);
                    double dy = static_cast<double>(y) - static_cast<double>(sy);
                    double dist = dx * dx + dy * dy;
                    if (dist < best_dist) {
                        best_dist = dist;
                        best_sx = sx;
                        best_sy = sy;
                    }
                }
            }

            result.At(x, y) = image.At(best_sx, best_sy);
        }
    }

    return result;
}
