#ifndef PRACTICA1_IMAGEAOS_HPP
#define PRACTICA1_IMAGEAOS_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>

struct KDTreeNode;
constexpr int DEFAULT_MAX_COLOR_VALUE = 255;  // Constante global para el valor máximo del color


struct Pixel {
    uint16_t red, green, blue;  // Soporte para valores de 0 a 65535 (2 bytes por canal de color)
};

// Estructura para representar las dimensiones de la imagen
struct ImageDimensions {
    int width;
    int height;
};

struct Ratios {
    float xRatio;
    float yRatio;
};

struct Color {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct KDTreeNode {
    int colorValue;
    uint16_t red, green, blue;
    std::unique_ptr<KDTreeNode> left;
    std::unique_ptr<KDTreeNode> right;
    // NOLINTBEGIN
    KDTreeNode(const int color, const uint16_t red, const uint16_t green, const uint16_t blue)
        : colorValue(color), red(red), green(green), blue(blue), left(nullptr), right(nullptr) {}
    // NOLINTEND
};

class Image {
private:
    int width, height, maxColorValue;
    std::vector<Pixel> pixels;

public:
    // Constructor con inicialización
    Image(ImageDimensions dimensions = {.width=0, .height=0}, int maxVal = DEFAULT_MAX_COLOR_VALUE)
            : width(dimensions.width), height(dimensions.height), maxColorValue(maxVal) {}

    // Getters
    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] int getMaxColorValue() const { return maxColorValue; }

    // Cargar imagen PPM
    void loadPPM(const std::string &filename);

    // Lee el encabezado del archivo PPM para obtener el ancho, alto y valor máximo de color.
    void readHeader(std::ifstream &file);

    // Determina el formato de bits de los píxeles y los carga en la estructura de la imagen.
    void loadPixels(std::ifstream &file);

    // Carga los píxeles de la imagen en formato de 8 bits por canal.
    void loadPixels8Bit(std::ifstream &file);

    // Carga los píxeles de la imagen en formato de 16 bits por canal.
    void loadPixels16Bit(std::ifstream &file);

    // Guardar imagen PPM
    void savePPM(const std::string &filename) const;

    // Escalar la intensidad de los colores
    void scaleIntensity(float factor);

    // Redimensionar usando interpolación bilineal
    void resize(int newWidth, int newHeight);

    // Declaración de la función `calculateRatios`
    [[nodiscard]] std::pair<float, float> calculateRatios(ImageDimensions dimensions) const;

    // Declaración de la función `updateImage`
    void updateImage(const ImageDimensions& dimensions, const std::vector<Pixel>& newPixels);

    // Calcula el color interpolado de un píxel en una posición redimensionada
    [[nodiscard]] Pixel getInterpolatedPixel(int newX, int newY, Ratios ratios) const;

    // Eliminar colores poco frecuentes

    static std::unordered_map<int, int> replaceRareColorsOptimized(std::unordered_map<int, int> &rareColors,
                                                            const std::unordered_map<int, int> &remainingColors);




    // Compresión simple eliminando colores cercanos
    void compress(const std::string &filename) const;

    // Calcula la frecuencia de cada color en la imagen

    // Genera la tabla de colores y su mapeo en el archivo comprimido
    [[nodiscard]] std::pair<std::unordered_map<int, int>, std::vector<int>> generateColorTable() const;

    // Escribe el encabezado y la tabla de colores en el archivo comprimido
    void writeHeaderAndColorTable(std::ofstream &file, const std::vector<int> &colorList) const;

    // Escribe los índices de los píxeles en el archivo comprimido
    void writePixelIndices(std::ofstream &file, const std::unordered_map<int, int> &colorTable, int indexSize) const;

    [[nodiscard]] std::vector<std::pair<int, int>> calculateColorFrequencies() const;

    static std::unique_ptr<KDTreeNode> buildKDTree(std::vector<KDTreeNode *> &nodes, int depth = 0);

    static KDTreeNode* findNearestColor(KDTreeNode* root, const Color& targetColor, int depth = 0);


    void removeRareColors(int threshold);
    static std::unordered_map<int, int> replaceRareColorsOptimized(std::unordered_map<int, int> &rareColors,
                                                            KDTreeNode* colorTreeRoot);

    static int calculateColorDistance(const Color &color1, const Color &color2);

    void updatePixels(const std::unordered_map<int, int> &rareColors);


};

#endif //PRACTICA1_IMAGEAOS_HPP
