#include <iostream>
#include <stdexcept>

#include "argument_parser.h"
#include "bmp_io.h"

static void PrintHelp(const char* program_name) {
    std::cout << "Usage: " << program_name
              << " <input.bmp> <output.bmp> [-filter1 [args...]] [-filter2 [args...]] ...\n\n"
              << "Available filters:\n"
              << "  -crop <width> <height>      Crop image to given size (top-left)\n"
              << "  -gs                         Convert to grayscale\n"
              << "  -neg                        Apply negative\n"
              << "  -sharp                      Sharpen image\n"
              << "  -edge <threshold>           Edge detection (0.0 - 1.0)\n"
              << "  -blur <sigma>               Gaussian blur\n"
              << "  -crystallize <cell_size>    Crystallize (Voronoi) effect\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintHelp(argv[0]);
        return 0;
    }

    if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        PrintHelp(argv[0]);
        return 0;
    }

    try {
        ProgramArgs args = ArgumentParser::Parse(argc, argv);
        Image image = BmpReader::Load(args.input_path);
        Image result = args.pipeline.Apply(image);
        BmpWriter::Save(args.output_path, result);
    } catch (const std::invalid_argument& e) {
        std::string msg = e.what();
        if (msg == "usage") {
            PrintHelp(argv[0]);
            return 1;
        }
        std::cerr << "Argument error: " << msg << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
