#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include "imgaos/imageaos.hpp"
#include "common/progargs.hpp"

namespace {
    constexpr int MAX_COLOR_VALUE = 65535;

    void printUsage() {
        std::cerr << "Usage: imtool input.ppm output.ppm [info | maxlevel <level> | resize <width> <height> | cutfreq <n> | compress]\n";
    }

    void handleInfo(Image& image, const std::string& inputFile) {
        image.loadPPM(inputFile);
        std::cout << "Width: " << image.getWidth()
                  << ", Height: " << image.getHeight()
                  << ", Max Color Value: " << image.getMaxColorValue() << '\n';
    }

    struct MaxLevelArgs {
        Image* image;
        std::string inputFile;
        std::string outputFile;
        std::string level;
    };

    void handleMaxLevel(const MaxLevelArgs& args) {
        const int newMaxLevel = std::stoi(args.level);
        if (newMaxLevel < 0 || newMaxLevel > MAX_COLOR_VALUE) {
            std::cerr << "Error: Invalid maxlevel: " << newMaxLevel << '\n';
            return;
        }
        args.image->loadPPM(args.inputFile);
        args.image->scaleIntensity(static_cast<float>(newMaxLevel));
        args.image->savePPM(args.outputFile);
    }

    struct ResizeArgs {
        Image* image;
        std::string inputFile;
        std::string outputFile;
        std::string width;
        std::string height;
    };

    void handleResize(const ResizeArgs& args) {
        const int newWidth = std::stoi(args.width);
        const int newHeight = std::stoi(args.height);
        if (newWidth <= 0 || newHeight <= 0) {
            std::cerr << "Error: Invalid dimensions for resize\n";
            return;
        }
        args.image->loadPPM(args.inputFile);
        args.image->resize(newWidth, newHeight);
        args.image->savePPM(args.outputFile);
    }

    struct CutFreqArgs {
        Image* image;
        std::string inputFile;
        std::string outputFile;
        std::string colorCountStr;
    };

    void handleCutFreq(const CutFreqArgs& args) {
        const int colorCount = std::stoi(args.colorCountStr);
        if (colorCount <= 0) {
            std::cerr << "Error: Invalid number of colors to cut: " << colorCount << '\n';
            return;
        }
        args.image->loadPPM(args.inputFile);
        args.image->removeRareColors(colorCount);
        args.image->savePPM(args.outputFile);
    }

    struct CompressArgs {
        Image* image;
        std::string inputFile;
        std::string outputFile;
    };

    void handleCompress(const CompressArgs& args) {
        args.image->loadPPM(args.inputFile);
        args.image->compress(args.outputFile);
    }

    int processOperation(const ProgArgs& progArgs, Image& image) {
        const std::string& operation = progArgs.getOperation();
        const std::string& inputFile = progArgs.getInputFile();
        const std::string& outputFile = progArgs.getOutputFile();
        const auto& additionalParams = progArgs.getAdditionalParams();

        if (operation == "info") {
            handleInfo(image, inputFile);
        } else if (operation == "maxlevel") {
            handleMaxLevel(MaxLevelArgs{.image = &image, .inputFile = inputFile, .outputFile = outputFile, .level = additionalParams.at(0)});
        } else if (operation == "resize" && additionalParams.size() >= 2) {
            handleResize(ResizeArgs{.image = &image, .inputFile = inputFile, .outputFile = outputFile, .width = additionalParams.at(0), .height = additionalParams.at(1)});
        } else if (operation == "cutfreq") {
            handleCutFreq(CutFreqArgs{.image = &image, .inputFile = inputFile, .outputFile = outputFile, .colorCountStr = additionalParams.at(0)});
        } else if (operation == "compress") {
            handleCompress(CompressArgs{.image = &image, .inputFile = inputFile, .outputFile = outputFile});
        } else {
            std::cerr << "Error: Invalid option: " << operation << '\n';
            printUsage();
            return -1;
        }
        return 0;
    }
}

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv, argv + argc);

    try {
        const ProgArgs progArgs(args);
        Image image;
        return processOperation(progArgs, image);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << '\n';
        printUsage();
        return -1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return -1;
    }
}
