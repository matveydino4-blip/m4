#pragma once

#include <cstddef>

#include "filter.h"
#include "matrix_filter.h"

class CropFilter : public Filter {
public:
    CropFilter(size_t width, size_t height) : width_(width), height_(height) {}

    Image Apply(const Image& image) const override;

private:
    size_t width_;
    size_t height_;
};

class GrayscaleFilter : public Filter {
public:
    Image Apply(const Image& image) const override;
};

class NegativeFilter : public Filter {
public:
    Image Apply(const Image& image) const override;
};

class SharpeningFilter : public MatrixFilter {
protected:
    Matrix3x3 GetMatrix() const override;
};

class EdgeDetectionFilter : public MatrixFilter {
public:
    explicit EdgeDetectionFilter(double threshold) : threshold_(threshold) {}

    Image Apply(const Image& image) const override;

protected:
    Matrix3x3 GetMatrix() const override;

private:
    double threshold_;
};

class GaussianBlurFilter : public Filter {
public:
    explicit GaussianBlurFilter(double sigma) : sigma_(sigma) {}

    Image Apply(const Image& image) const override;

private:
    double sigma_;
};

class CrystallizeFilter : public Filter {
public:
    explicit CrystallizeFilter(size_t cell_size) : cell_size_(cell_size) {}

    Image Apply(const Image& image) const override;

private:
    size_t cell_size_;
};
