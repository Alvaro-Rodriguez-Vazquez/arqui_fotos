#include "imageaos.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <unordered_map>
#include <string>
#include <cstring>
#include <ranges>

namespace {
    constexpr int MAX_COLOR_8_BIT = 255;
    constexpr int BYTE_SHIFT = 8;
    constexpr int BYTE_MASK = 0xFF;
    constexpr int RED_SHIFT = 16;
    constexpr int GREEN_SHIFT = 8;
    constexpr int BYTE_MASK_SHIFT_16 = 16;
    constexpr int BYTE_MASK_SHIFT_24 = 24;
    constexpr int MAX_COLOR_8_BIT_RANGE = 256;
    constexpr int MAX_COLOR_16_BIT_RANGE = 65536;

}

// Cargar una imagen PPM
void Image::loadPPM(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Error al abrir el archivo");
  }

  readHeader(file);
  loadPixels(file);

  file.close();
}

// Lee el encabezado del archivo PPM para obtener el ancho, alto y valor máximo de color.
void Image::readHeader(std::ifstream &file) {
    std::string magicNumber;
    file >> magicNumber;
    if (magicNumber != "P6") {
        throw std::runtime_error("Formato no soportado");
    }

    file >> width >> height >> maxColorValue;
    file.ignore(1); // Ignorar el espacio en blanco o salto de línea después del maxColorValue
}

// Determina el formato de bits de los píxeles y los carga en la estructura de la imagen.
void Image::loadPixels(std::ifstream &file) {
    pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

    if (maxColorValue <= MAX_COLOR_8_BIT) {
        loadPixels8Bit(file);
    } else {
        loadPixels16Bit(file);
    }
}

// Carga los píxeles de la imagen en formato de 8 bits por canal.
void Image::loadPixels8Bit(std::ifstream &file) {
    std::vector<char> tempBuffer(static_cast<size_t>(width) * static_cast<size_t>(height) * 3);
    file.read(tempBuffer.data(), static_cast<std::streamsize>(tempBuffer.size()));

    std::vector<uint8_t> buffer(tempBuffer.begin(), tempBuffer.end());
    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i].red = buffer[i * 3];
        pixels[i].green = buffer[(i * 3) + 1];
        pixels[i].blue = buffer[(i * 3) + 2];
    }
}

// Carga los píxeles de la imagen en formato de 16 bits por canal.
void Image::loadPixels16Bit(std::ifstream &file) {
    std::vector<char> tempBuffer(static_cast<size_t>(width) * static_cast<size_t>(height) * 3 * sizeof(uint16_t));
    file.read(tempBuffer.data(), static_cast<std::streamsize>(tempBuffer.size()));

    std::vector<uint16_t> buffer(tempBuffer.size() / 2);
    std::memcpy(buffer.data(), tempBuffer.data(), tempBuffer.size());

    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i].red = buffer[i * 3];
        pixels[i].green = buffer[(i * 3) + 1];
        pixels[i].blue = buffer[(i * 3) + 2];
    }
}

// Guardar la imagen PPM
void Image::savePPM(const std::string &filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error al guardar el archivo");
    }

    file << "P6\n" << width << " " << height << "\n" << maxColorValue << "\n";

    if (maxColorValue <= MAX_COLOR_8_BIT) {
        for (const Pixel &pixel : pixels) {
            file.put(static_cast<char>(pixel.red));
            file.put(static_cast<char>(pixel.green));
            file.put(static_cast<char>(pixel.blue));
        }
    } else {
        for (const Pixel &pixel : pixels) {
            file.put(static_cast<char>(pixel.red >> BYTE_SHIFT));
            file.put(static_cast<char>(pixel.red & BYTE_MASK));
            file.put(static_cast<char>(pixel.green >> BYTE_SHIFT));
            file.put(static_cast<char>(pixel.green & BYTE_MASK));
            file.put(static_cast<char>(pixel.blue >> BYTE_SHIFT));
            file.put(static_cast<char>(pixel.blue & BYTE_MASK));
        }
    }

    file.close();
}

void Image::compress(const std::string &filename) const {
    // Genera la tabla de colores
    auto [colorTable, colorList] = generateColorTable();

    // Abre el archivo de salida
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error al guardar el archivo comprimido");
    }

    // Escribir el encabezado y la tabla de colores
    writeHeaderAndColorTable(file, colorList);

    // Determinar el tamaño de los índices
    int indexSize = 0;
    if (colorList.size() <= MAX_COLOR_8_BIT_RANGE) {
        indexSize = 1;
    } else if (colorList.size() <= MAX_COLOR_16_BIT_RANGE) {
        indexSize = 2;
    } else {
        indexSize = 4;
    }

    // Escribir los índices de los píxeles
    writePixelIndices(file, colorTable, indexSize);

    file.close();
}


// Generar la tabla de colores
std::pair<std::unordered_map<int, int>, std::vector<int>> Image::generateColorTable() const {
    std::unordered_map<int, int> colorTable;
    std::vector<int> colorList;
    int index = 0;

    for (const Pixel &pixel : pixels) {
        int const colorValue = (pixel.red << RED_SHIFT) | (pixel.green << GREEN_SHIFT) | pixel.blue;
        if (!colorTable.contains(colorValue)) {
            colorTable[colorValue] = index++;
            colorList.push_back(colorValue);
        }
    }
    return {colorTable, colorList};
}

// Escribir el encabezado y la tabla de colores
void Image::writeHeaderAndColorTable(std::ofstream &file, const std::vector<int> &colorList) const {
    file << "C6 " << width << " " << height << " " << maxColorValue << " " << colorList.size() << "\n";

    for (int const color : colorList) {
        uint16_t const red = (color >> RED_SHIFT) & BYTE_MASK;
        uint16_t const green = (color >> GREEN_SHIFT) & BYTE_MASK;
        uint16_t const blue = color & BYTE_MASK;

        if (maxColorValue <= DEFAULT_MAX_COLOR_VALUE) {
            file.put(static_cast<char>(red));
            file.put(static_cast<char>(green));
            file.put(static_cast<char>(blue));
        } else {
            file.put(static_cast<char>(red >> BYTE_SHIFT));
            file.put(static_cast<char>(red & BYTE_MASK));
            file.put(static_cast<char>(green >> BYTE_SHIFT));
            file.put(static_cast<char>(green & BYTE_MASK));
            file.put(static_cast<char>(blue >> BYTE_SHIFT));
            file.put(static_cast<char>(blue & BYTE_MASK));
        }
    }
}

// Escribir los índices de los píxeles
void Image::writePixelIndices(std::ofstream &file, const std::unordered_map<int, int> &colorTable, int indexSize) const {
    for (const Pixel &pixel : pixels) {
        int const colorValue = (pixel.red << RED_SHIFT) | (pixel.green << GREEN_SHIFT) | pixel.blue;
        int const colorIndex = colorTable.at(colorValue);

        if (indexSize == 1) {
            file.put(static_cast<char>(colorIndex));
        } else if (indexSize == 2) {
            file.put(static_cast<char>(colorIndex & BYTE_MASK));
            file.put(static_cast<char>(colorIndex >> BYTE_SHIFT));
        } else if (indexSize == 4) {
            file.put(static_cast<char>(colorIndex & BYTE_MASK));
            file.put(static_cast<char>((colorIndex >> BYTE_SHIFT) & BYTE_MASK));
            file.put(static_cast<char>((colorIndex >> BYTE_MASK_SHIFT_16) & BYTE_MASK));
            file.put(static_cast<char>((colorIndex >> BYTE_MASK_SHIFT_24) & BYTE_MASK));
        }
    }
}


// Escalar la intensidad de los colores para la versión AOS
void Image::scaleIntensity(float nuevoMaxLevel) {

    // Calcular el factor de escala basado en el nuevo valor máximo
    float const factor = nuevoMaxLevel / static_cast<float>(maxColorValue);

    // Escalar los valores de los píxeles
    for (Pixel &pixel : pixels) {
        pixel.red = static_cast<uint16_t>(std::round(static_cast<float>(pixel.red) * factor));
        pixel.green = static_cast<uint16_t>(std::round(static_cast<float>(pixel.green) * factor));
        pixel.blue = static_cast<uint16_t>(std::round(static_cast<float>(pixel.blue) * factor));
    }

    maxColorValue = static_cast<int>(nuevoMaxLevel);
}

void Image::resize(const int nuevo_ancho, const int nuevo_alto) {
    const ImageDimensions newDimensions = {.width = nuevo_ancho, .height = nuevo_alto};
    auto [xRatio, yRatio] = calculateRatios(newDimensions);

    const Ratios ratios = {.xRatio = xRatio, .yRatio = yRatio};

    std::vector<Pixel> newPixels(static_cast<size_t>(nuevo_ancho) * static_cast<size_t>(nuevo_alto));

    for (int newY = 0; newY < nuevo_alto; ++newY) {
        for (int newX = 0; newX < nuevo_ancho; ++newX) {
            newPixels[(static_cast<size_t>(newY) * static_cast<size_t>(nuevo_ancho)) +
                      static_cast<size_t>(newX)] = getInterpolatedPixel(newX, newY, ratios);
        }
    }

    updateImage(newDimensions, newPixels);
}

std::pair<float, float> Image::calculateRatios(ImageDimensions dimensions) const {
    const float xRatio = static_cast<float>(width - 1) / static_cast<float>(dimensions.width);
    const float yRatio = static_cast<float>(height - 1) / static_cast<float>(dimensions.height);
    return {xRatio, yRatio};
}

Pixel Image::getInterpolatedPixel(const int newX, const int newY, const Ratios ratios) const {
    const float originalX = static_cast<float>(newX) * ratios.xRatio;
    const float originalY = static_cast<float>(newY) * ratios.yRatio;
    const int baseX = static_cast<int>(originalX);
    const int baseY = static_cast<int>(originalY);

    const float deltaX = originalX - static_cast<float>(baseX);
    const float deltaY = originalY - static_cast<float>(baseY);

    const Pixel topLeft = pixels[(static_cast<size_t>(baseY) * static_cast<size_t>(width)) +
                                 static_cast<size_t>(baseX)];
    const Pixel topRight = pixels[(static_cast<size_t>(baseY) * static_cast<size_t>(width)) +
                                  static_cast<size_t>(baseX + 1)];
    const Pixel bottomLeft = pixels[(static_cast<size_t>(baseY + 1) * static_cast<size_t>(width)) +
                                    static_cast<size_t>(baseX)];
    const Pixel bottomRight = pixels[(static_cast<size_t>(baseY + 1) * static_cast<size_t>(width)) +
                                     static_cast<size_t>(baseX + 1)];

    Pixel interpolatedPixel = {.red = 0, .green = 0, .blue = 0};
    interpolatedPixel.red = static_cast<uint16_t>(((1 - deltaX) * (1 - deltaY) * static_cast<float>(topLeft.red)) +
                                                (deltaX * (1 - deltaY) * static_cast<float>(topRight.red)) +
                                                ((1 - deltaX) * deltaY * static_cast<float>(bottomLeft.red)) +
                                                (deltaX * deltaY * static_cast<float>(bottomRight.red)));
    interpolatedPixel.green = static_cast<uint16_t>(((1 - deltaX) * (1 - deltaY) * static_cast<float>(topLeft.green)) +
                                                (deltaX * (1 - deltaY) * static_cast<float>(topRight.green)) +
                                                ((1 - deltaX) * deltaY * static_cast<float>(bottomLeft.green)) +
                                                (deltaX * deltaY * static_cast<float>(bottomRight.green)));
    interpolatedPixel.blue = static_cast<uint16_t>(((1 - deltaX) * (1 - deltaY) * static_cast<float>(topLeft.blue)) +
                                                (deltaX * (1 - deltaY) * static_cast<float>(topRight.blue)) +
                                                ((1 - deltaX) * deltaY * static_cast<float>(bottomLeft.blue)) +
                                                (deltaX * deltaY * static_cast<float>(bottomRight.blue)));
    return interpolatedPixel;
}

void Image::updateImage(const ImageDimensions& dimensions, const std::vector<Pixel>& newPixels) {
    width = dimensions.width;
    height = dimensions.height;
    pixels = newPixels;
}

std::vector<std::pair<int, int>> Image::calculateColorFrequencies() const {
    std::unordered_map<int, int> histogram;
    for (const auto& pixel : pixels) {
        int const colorValue = (pixel.red << BYTE_MASK_SHIFT_16) | (pixel.green << BYTE_SHIFT) | pixel.blue;
        histogram[colorValue]++;
    }

    std::vector<std::pair<int, int>> sortedHistogram(histogram.begin(), histogram.end());
    std::ranges::sort(sortedHistogram,
                      [](const auto &colorFreq1, const auto &colorFreq2) {
                          return colorFreq1.second < colorFreq2.second;
                      });

    return sortedHistogram;
}

// Construcción del KD-Tree
// NOLINTBEGIN(misc-no-recursion)
std::unique_ptr<KDTreeNode> Image::buildKDTree(std::vector<KDTreeNode*>& nodes, const int depth) {
    if (nodes.empty()) {
        return nullptr;
    }

    int const axis = depth % 3;
    auto comparator = [axis](const KDTreeNode* nodeA, const KDTreeNode* nodeB) {
        if (axis == 0) {
            return nodeA->red < nodeB->red;
        } if (axis == 1) {
            return nodeA->green < nodeB->green;
        }
        return nodeA->blue < nodeB->blue;
    };

    std::ranges::sort(nodes, comparator);
    auto medianIt = std::next(nodes.begin(), static_cast<std::ptrdiff_t>(nodes.size() / 2));
    auto root = std::make_unique<KDTreeNode>((*medianIt)->colorValue, (*medianIt)->red, (*medianIt)->green, (*medianIt)->blue);

    std::vector<KDTreeNode*> leftNodes(nodes.begin(), medianIt);
    std::vector<KDTreeNode*> rightNodes(std::next(medianIt), nodes.end());

    root->left = buildKDTree(leftNodes, depth + 1);
    root->right = buildKDTree(rightNodes, depth + 1);

    return root;
}

// Búsqueda recursiva del color más cercano en el KD-Tree
KDTreeNode* Image::findNearestColor(KDTreeNode* root, const Color& targetColor, const int depth) {
    if (root == nullptr) {
        return nullptr;
    }

    KDTreeNode* nextBranch = nullptr;
    KDTreeNode* oppositeBranch = nullptr;
    int const axis = depth % 3;
    int diff = 0;

    if (axis == 0) {
        diff = targetColor.red - root->red;
    } else if (axis == 1) {
        diff = targetColor.green - root->green;
    } else {
        diff = targetColor.blue - root->blue;
    }

    nextBranch = (diff < 0) ? root->left.get() : root->right.get();
    oppositeBranch = (diff < 0) ? root->right.get() : root->left.get();

    Color const rootColor = { .red=root->red, .green=root->green, .blue=root->blue };
    KDTreeNode* best = findNearestColor(nextBranch, targetColor, depth + 1);
    if ((best == nullptr) || calculateColorDistance(targetColor, rootColor) <
                 calculateColorDistance(targetColor, Color{ .red=best->red, .green=best->green, .blue=best->blue })) {
        best = root;
                 }
    if (diff * diff < calculateColorDistance(targetColor, Color{ .red=best->red, .green=best->green, .blue=best->blue })) {
        KDTreeNode* tempBest = findNearestColor(oppositeBranch, targetColor, depth + 1);
        if ((tempBest != nullptr) && calculateColorDistance(targetColor, Color{ .red=tempBest->red, .green=tempBest->green, .blue=tempBest->blue }) <
                        calculateColorDistance(targetColor, Color{ .red=best->red, .green=best->green, .blue=best->blue })) {
            best = tempBest;
                        }
    }
    return best;
}
// NOLINTEND(misc-no-recursion)

// Eliminar colores poco frecuentes
void Image::removeRareColors(const int threshold) {
    auto sortedColors = calculateColorFrequencies();

    std::unordered_map<int, int> rareColors;
    std::unordered_map<int, int> remainingColors;
    std::vector<std::unique_ptr<KDTreeNode>> nodes; // Cambiar a std::unique_ptr para gestión automática

    for (size_t i = 0; i < sortedColors.size(); ++i) {
        if (i < static_cast<size_t>(threshold)) {
            rareColors[sortedColors[i].first] = sortedColors[i].first;
        } else {
            int const colorValue = sortedColors[i].first;
            remainingColors[colorValue] = colorValue;

            uint16_t const red = (colorValue >> BYTE_MASK_SHIFT_16) & BYTE_MASK;
            uint16_t const green = (colorValue >> 8) & BYTE_MASK;
            uint16_t const blue = colorValue & BYTE_MASK;

            // Usar std::make_unique para evitar fugas de memoria
            nodes.push_back(std::make_unique<KDTreeNode>(colorValue, red, green, blue));
        }
    }

    // Crear un vector temporal con punteros sin procesar
    std::vector<KDTreeNode*> rawNodes;
    rawNodes.reserve(nodes.size()); // Reservar espacio para evitar realocaciones
    for (const auto& node : nodes) {
        rawNodes.push_back(node.get()); // Extraer el puntero sin procesar
    }

    // Llamar a buildKDTree con el vector de punteros sin procesar
    const auto colorTreeRoot = buildKDTree(rawNodes); // buildKDTree sigue esperando std::vector<KDTreeNode*>
    rareColors = replaceRareColorsOptimized(rareColors, colorTreeRoot.get()); // Usar .get() para obtener el puntero sin procesar

    updatePixels(rareColors);
}




// Reemplaza los colores poco frecuentes con los más cercanos en el KD-Tree
std::unordered_map<int, int> Image::replaceRareColorsOptimized(std::unordered_map<int, int>& rareColors, KDTreeNode* colorTreeRoot) {
    for (auto& [colorValue, color] : rareColors) {
        uint16_t const redChannel = (colorValue >> BYTE_MASK_SHIFT_16) & BYTE_MASK;
        uint16_t const greenChannel = (colorValue >> BYTE_SHIFT) & BYTE_MASK;
        uint16_t const blueChannel = colorValue & BYTE_MASK;

        // Crear instancia de Color para pasar a findNearestColor
        Color const targetColor = { .red=redChannel, .green=greenChannel, .blue=blueChannel };

        if (const KDTreeNode* nearestColorNode = findNearestColor(colorTreeRoot, targetColor)) {
            color = nearestColorNode->colorValue;
        }
    }
    return rareColors;
}

// Cálculo de distancia de color
inline int Image::calculateColorDistance(const Color& color1, const Color& color2) {
    return ((color1.red - color2.red) * (color1.red - color2.red)) +
           ((color1.green - color2.green) * (color1.green - color2.green)) +
           ((color1.blue - color2.blue) * (color1.blue - color2.blue));
}

// Actualiza los píxeles de la imagen según el mapa de colores raros
void Image::updatePixels(const std::unordered_map<int, int> &rareColors) {
    for (auto& pixel : pixels) {
        int const colorValue = (pixel.red << BYTE_MASK_SHIFT_16) | (pixel.green << 8) | pixel.blue;
        if (rareColors.contains(colorValue)) {
            int const newColor = rareColors.at(colorValue);
            pixel.red = (newColor >> BYTE_MASK_SHIFT_16) & BYTE_MASK;
            pixel.green = (newColor >> BYTE_SHIFT) & BYTE_MASK;
            pixel.blue = newColor & BYTE_MASK;
        }
    }
}

