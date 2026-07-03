#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "filter_pipeline.h"
#include "filters.h"

struct ProgramArgs {
    std::string input_path;
    std::string output_path;
    FilterPipeline pipeline;
};

class ArgumentParser {
public:
    static ProgramArgs Parse(int argc, char** argv) {
        if (argc < 3) {
            throw std::invalid_argument("usage");
        }

        ProgramArgs args;
        args.input_path = argv[1];
        args.output_path = argv[2];

        int i = 3;
        while (i < argc) {
            std::string token = argv[i];
            if (token.empty() || token[0] != '-') {
                throw std::invalid_argument("Expected filter name starting with '-', got: " + token);
            }
            std::string name = token.substr(1);

            if (name == "crop") {
                if (i + 2 >= argc) {
                    throw std::invalid_argument("-crop requires two arguments: width height");
                }
                size_t width = ParsePositiveSize(argv[i + 1], "crop width");
                size_t height = ParsePositiveSize(argv[i + 2], "crop height");
                args.pipeline.AddFilter(std::make_unique<CropFilter>(width, height));
                i += 3;
            } else if (name == "gs") {
                args.pipeline.AddFilter(std::make_unique<GrayscaleFilter>());
                ++i;
            } else if (name == "neg") {
                args.pipeline.AddFilter(std::make_unique<NegativeFilter>());
                ++i;
            } else if (name == "sharp") {
                args.pipeline.AddFilter(std::make_unique<SharpeningFilter>());
                ++i;
            } else if (name == "edge") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("-edge requires one argument: threshold");
                }
                double threshold = ParseDouble(argv[i + 1], "edge threshold");
                args.pipeline.AddFilter(std::make_unique<EdgeDetectionFilter>(threshold));
                i += 2;
            } else if (name == "blur") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("-blur requires one argument: sigma");
                }
                double sigma = ParseDouble(argv[i + 1], "blur sigma");
                if (sigma <= 0.0) {
                    throw std::invalid_argument("blur sigma must be positive");
                }
                args.pipeline.AddFilter(std::make_unique<GaussianBlurFilter>(sigma));
                i += 2;
            } else if (name == "crystallize") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("-crystallize requires one argument: cell_size");
                }
                size_t cell_size = ParsePositiveSize(argv[i + 1], "crystallize cell_size");
                args.pipeline.AddFilter(std::make_unique<CrystallizeFilter>(cell_size));
                i += 2;
            } else {
                throw std::invalid_argument("Unknown filter: " + name);
            }
        }

        return args;
    }

private:
    static size_t ParsePositiveSize(const std::string& s, const std::string& name) {
        try {
            long long val = std::stoll(s);
            if (val <= 0) {
                throw std::invalid_argument(name + " must be a positive integer, got: " + s);
            }
            return static_cast<size_t>(val);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(name + " must be a positive integer, got: " + s);
        } catch (const std::out_of_range&) {
            throw std::invalid_argument(name + " is out of range: " + s);
        }
    }

    static double ParseDouble(const std::string& s, const std::string& name) {
        try {
            size_t pos = 0;
            double val = std::stod(s, &pos);
            if (pos != s.size()) {
                throw std::invalid_argument(name + " must be a number, got: " + s);
            }
            return val;
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(name + " must be a number, got: " + s);
        } catch (const std::out_of_range&) {
            throw std::invalid_argument(name + " is out of range: " + s);
        }
    }
};
