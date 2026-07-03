#pragma once

#include <string>

#include "image.h"

class BmpReader {
public:
    static Image Load(const std::string& path);
};

class BmpWriter {
public:
    static void Save(const std::string& path, const Image& image);
};
