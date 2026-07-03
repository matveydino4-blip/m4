#include "bmp_io.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#pragma pack(push, 1)
struct BmpFileHeader {
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t data_offset;
};

struct BmpInfoHeader {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};
#pragma pack(pop)

static const uint16_t BMP_SIGNATURE = 0x4D42;
static const uint16_t BITS_PER_PIXEL_24 = 24;
static const uint32_t BI_RGB = 0;
static const uint32_t BITMAPINFOHEADER_SIZE = 40;

Image BmpReader::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    BmpFileHeader file_header;
    file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    if (!file) {
        throw std::runtime_error("Failed to read BMP file header: " + path);
    }
    if (file_header.signature != BMP_SIGNATURE) {
        throw std::runtime_error("Not a BMP file: " + path);
    }

    BmpInfoHeader info_header;
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    if (!file) {
        throw std::runtime_error("Failed to read BMP info header: " + path);
    }
    if (info_header.header_size != BITMAPINFOHEADER_SIZE) {
        throw std::runtime_error("Unsupported BMP header type: " + path);
    }
    if (info_header.bits_per_pixel != BITS_PER_PIXEL_24) {
        throw std::runtime_error("Only 24-bit BMP is supported: " + path);
    }
    if (info_header.compression != BI_RGB) {
        throw std::runtime_error("Only uncompressed BMP is supported: " + path);
    }
    if (info_header.width <= 0 || info_header.height == 0) {
        throw std::runtime_error("Invalid BMP dimensions: " + path);
    }

    const size_t width = static_cast<size_t>(info_header.width);
    const size_t height = static_cast<size_t>(std::abs(info_header.height));
    const bool is_bottom_up = info_header.height > 0;

    // Row size is padded to 4-byte boundary
    const size_t row_size = ((width * 3 + 3) / 4) * 4;

    file.seekg(file_header.data_offset, std::ios::beg);

    std::vector<std::vector<std::vector<uint8_t>>> raw_rows(height, std::vector<std::vector<uint8_t>>(1, std::vector<uint8_t>(row_size)));

    for (size_t row = 0; row < height; ++row) {
        file.read(reinterpret_cast<char*>(raw_rows[row][0].data()), static_cast<std::streamsize>(row_size));
        if (!file) {
            throw std::runtime_error("Unexpected end of BMP data: " + path);
        }
    }

    Image image(width, height);
    for (size_t row = 0; row < height; ++row) {
        size_t img_y = is_bottom_up ? (height - 1 - row) : row;
        for (size_t x = 0; x < width; ++x) {
            uint8_t blue = raw_rows[row][0][x * 3];
            uint8_t green = raw_rows[row][0][x * 3 + 1];
            uint8_t red = raw_rows[row][0][x * 3 + 2];
            image.At(x, img_y) = Color(red / 255.0, green / 255.0, blue / 255.0);
        }
    }

    return image;
}

void BmpWriter::Save(const std::string& path, const Image& image) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }

    const size_t width = image.GetWidth();
    const size_t height = image.GetHeight();
    const size_t row_size = ((width * 3 + 3) / 4) * 4;
    const uint32_t pixel_data_size = static_cast<uint32_t>(row_size * height);
    const uint32_t file_size = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + pixel_data_size;

    BmpFileHeader file_header;
    file_header.signature = BMP_SIGNATURE;
    file_header.file_size = file_size;
    file_header.reserved1 = 0;
    file_header.reserved2 = 0;
    file_header.data_offset = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

    BmpInfoHeader info_header;
    info_header.header_size = BITMAPINFOHEADER_SIZE;
    info_header.width = static_cast<int32_t>(width);
    info_header.height = static_cast<int32_t>(height);
    info_header.planes = 1;
    info_header.bits_per_pixel = BITS_PER_PIXEL_24;
    info_header.compression = BI_RGB;
    info_header.image_size = pixel_data_size;
    info_header.x_pixels_per_meter = 0;
    info_header.y_pixels_per_meter = 0;
    info_header.colors_used = 0;
    info_header.colors_important = 0;

    file.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    std::vector<uint8_t> row_buffer(row_size, 0);
    for (size_t row = 0; row < height; ++row) {
        size_t img_y = height - 1 - row;  // BMP is stored bottom-up
        for (size_t x = 0; x < width; ++x) {
            const Color& c = image.At(x, img_y);
            row_buffer[x * 3] = static_cast<uint8_t>(std::clamp(c.b, 0.0, 1.0) * 255.0);
            row_buffer[x * 3 + 1] = static_cast<uint8_t>(std::clamp(c.g, 0.0, 1.0) * 255.0);
            row_buffer[x * 3 + 2] = static_cast<uint8_t>(std::clamp(c.r, 0.0, 1.0) * 255.0);
        }
        file.write(reinterpret_cast<const char*>(row_buffer.data()), static_cast<std::streamsize>(row_size));
    }

    if (!file) {
        throw std::runtime_error("Failed to write BMP file: " + path);
    }
}
