#ifndef PRACTICA1_IMAGESOA_HPP
#define PRACTICA1_IMAGESOA_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <tuple>
#include <memory>

struct Pixel {
    uint16_t r, g, b;
};

struct KDTreeNode {
    int colorValue;
    uint16_t red, green, blue; // Colores como miembros individuales
    std::unique_ptr<KDTreeNode> left;
    std::unique_ptr<KDTreeNode> right;
    // Inicia la sección donde se desactiva el chequeo
    // NOLINTBEGIN
    KDTreeNode(int colorValue, uint16_t red, uint16_t green, uint16_t blue)
        : colorValue(colorValue), red(red), green(green), blue(blue), left(nullptr), right(nullptr) {}
    // Termina la sección donde se desactiva el chequeo
    // NOLINTEND
};



struct InterpolationParams {
    int posX;
    int posY;
    float xRatio;
    float yRatio;
};

struct Color {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

class Image {
public:
    int width, height, maxColorValue;
    std::vector<uint16_t> red;   // Almacena el canal rojo
    std::vector<uint16_t> green; // Almacena el canal verde
    std::vector<uint16_t> blue;  // Almacena el canal azul

    void loadPPM(const std::string &filename);
    void savePPM(const std::string &filename) const;
    void scaleIntensity(float factor);
    void resize(int nuevo_ancho, int nuevo_alto);
    void borrarMenosFrecuentes(int threshold);

    static void escribirColorTabla(std::ofstream &file,
                         const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &coloresUnicos);

    void writePixelIndices(std::ofstream &file, const std::unordered_map<int, int> &colorTabla,
                           const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &uniqueColors) const;

    void compress(const std::string &filename) const;

private:
    static constexpr int BUCKET_SIZE = 8;

    void loadPPM_8bit(std::ifstream &file);
    void loadPPM_16bit(std::ifstream &file);

    [[nodiscard]] uint16_t getPixelRed(int posX, int posY) const;
    [[nodiscard]] uint16_t getPixelGreen(int posX, int posY) const;
    [[nodiscard]] uint16_t getPixelBlue(int posX, int posY) const;

    [[nodiscard]] uint16_t getInterpolatedChannel(const InterpolationParams &params, uint16_t (Image::*getPixel)(int, int) const) const;
    [[nodiscard]] std::vector<std::pair<int, int>> frecuenciaColores() const;

    static std::unique_ptr<KDTreeNode> construccionKDTree(std::vector<std::unique_ptr<KDTreeNode>> &nodes, int depth = 0);

    // Cambiar el tipo de parámetro a Color
    static KDTreeNode* colorCercano(KDTreeNode* raiz, const Color& colorObjetivo, int profundidad = 0);

    static void borrarKDTree(std::unique_ptr<KDTreeNode> &raiz);

    static std::unordered_map<int, int> remplazoColores(std::unordered_map<int, int>& coloresPocoFrecuentes, KDTreeNode* colorRaiz);

    // Usar solo la versión de calculateColorDistance que acepta Color
    static int calcularEuclidea(const Color &color1, const Color &color2);

    void actualizarPixeles(const std::unordered_map<int, int> &coloresPocoFrecuentes);

    void cabeceraPPM(std::ofstream &file) const;

    void calcularColorTabla(std::unordered_map<int, int> &colorTable,
                          std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &uniqueColors) const;
};

#endif // PRACTICA1_IMAGESOA_HPP
