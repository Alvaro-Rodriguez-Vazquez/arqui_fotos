#include "imagesoa.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <execution>
#include <tuple>
#include <memory>

namespace {
    constexpr int MAX_COLOR_8_BIT = 255;
    constexpr int DESPLAZAMIENTO_8_BITS = 8;
    constexpr int DESPLAZAMIENTO_16_BITS = 16;
    constexpr int MASCARA_BYTE = 0xFF;
    constexpr int CANTIDAD_CANALES_8_BITS = 3;
    constexpr int CANTIDAD_CANALES_16_BITS = 6;
    constexpr int MAXIMO_COLOR_16_BITS = 65535;
    constexpr int LIMITE_COLOR_TABLA = 256;
    constexpr int BYTE_INFERIOR_AZUL = 5;
}

// Función auxiliar para cargar datos de 8 bits
void Image::loadPPM_8bit(std::ifstream &file) {
    std::vector<char> buffer(static_cast<size_t>(width) * static_cast<size_t>(height) * CANTIDAD_CANALES_8_BITS);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    red.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    green.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    blue.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

    for (size_t i = 0; i < red.size(); ++i) {
        red[i] = static_cast<uint8_t>(buffer[i * CANTIDAD_CANALES_8_BITS]);
        green[i] = static_cast<uint8_t>(buffer[(i * CANTIDAD_CANALES_8_BITS) + 1]);
        blue[i] = static_cast<uint8_t>(buffer[(i * CANTIDAD_CANALES_8_BITS) + 2]);
    }
}

// Función auxiliar para cargar datos de 16 bits
void Image::loadPPM_16bit(std::ifstream &file) {
    std::vector<char> buffer(static_cast<size_t>(width) * static_cast<size_t>(height) * CANTIDAD_CANALES_16_BITS);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    red.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    green.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    blue.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

    for (size_t i = 0; i < red.size(); ++i) {
        red[i] = static_cast<uint16_t>((static_cast<unsigned char>(buffer[i * CANTIDAD_CANALES_16_BITS]) << DESPLAZAMIENTO_8_BITS) |
                                        static_cast<unsigned char>(buffer[(i * CANTIDAD_CANALES_16_BITS) + 1]));
        green[i] = static_cast<uint16_t>((static_cast<unsigned char>(buffer[(i * CANTIDAD_CANALES_16_BITS) + 2]) << DESPLAZAMIENTO_8_BITS) |
                                          static_cast<unsigned char>(buffer[(i * CANTIDAD_CANALES_16_BITS) + 3]));
        blue[i] = static_cast<uint16_t>((static_cast<unsigned char>(buffer[(i * CANTIDAD_CANALES_16_BITS) + 4]) << DESPLAZAMIENTO_8_BITS) |
                                         static_cast<unsigned char>(buffer[(i * CANTIDAD_CANALES_16_BITS) + BYTE_INFERIOR_AZUL]));
    }
}

void Image::loadPPM(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error al abrir el archivo");
    }

    std::string esperar_P6;
    file >> esperar_P6;
    if (esperar_P6 != "P6") {
        throw std::runtime_error("Formato no adecuado");
    }

    file >> width >> height >> maxColorValue;
    file.ignore(1);

    if (maxColorValue <= MAX_COLOR_8_BIT) {
        loadPPM_8bit(file);
    } else if (maxColorValue <= MAXIMO_COLOR_16_BITS) {
        loadPPM_16bit(file);
    } else {
        throw std::runtime_error("Valor de maxColorValue fuera de rango");
    }

    file.close();
}

// Guardar la imagen PPM
void Image::savePPM(const std::string &filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error al guardar el archivo");
    }

    file << "P6\n" << width << " " << height << "\n" << maxColorValue << "\n";

    if (maxColorValue <= MAX_COLOR_8_BIT) {
        for (size_t i = 0; i < red.size(); ++i) {
            file.put(static_cast<char>(red[i]));
            file.put(static_cast<char>(green[i]));
            file.put(static_cast<char>(blue[i]));
        }
    } else {
        for (size_t i = 0; i < red.size(); ++i) {
            file.put(static_cast<char>(red[i] >> DESPLAZAMIENTO_8_BITS));
            file.put(static_cast<char>(red[i] & MASCARA_BYTE));
            file.put(static_cast<char>(green[i] >> DESPLAZAMIENTO_8_BITS));
            file.put(static_cast<char>(green[i] & MASCARA_BYTE));
            file.put(static_cast<char>(blue[i] >> DESPLAZAMIENTO_8_BITS));
            file.put(static_cast<char>(blue[i] & MASCARA_BYTE));
        }
    }

    file.close();
}

void Image::scaleIntensity(float nuevoMaxLevel) {
    float const factor = nuevoMaxLevel / static_cast<float>(maxColorValue);

    std::for_each(std::execution::par, red.begin(), red.end(), [factor, nuevoMaxLevel](uint16_t &canal) {
        canal = static_cast<uint16_t>(std::clamp(static_cast<int>(static_cast<float>(canal) * factor), 0, static_cast<int>(nuevoMaxLevel)));
    });
    std::for_each(std::execution::par, green.begin(), green.end(), [factor, nuevoMaxLevel](uint16_t &canal) {
        canal = static_cast<uint16_t>(std::clamp(static_cast<int>(static_cast<float>(canal) * factor), 0, static_cast<int>(nuevoMaxLevel)));
    });
    std::for_each(std::execution::par, blue.begin(), blue.end(), [factor, nuevoMaxLevel](uint16_t &canal) {
        canal = static_cast<uint16_t>(std::clamp(static_cast<int>(static_cast<float>(canal) * factor), 0, static_cast<int>(nuevoMaxLevel)));
    });

    maxColorValue = static_cast<int>(nuevoMaxLevel);
}

// Función para obtener un valor rojo
[[nodiscard]] uint16_t Image::getPixelRed(int posX, int posY) const {
    return red[(static_cast<size_t>(posY) * static_cast<size_t>(width)) + static_cast<size_t>(posX)];
}

// Función para obtener un valor verde
[[nodiscard]] uint16_t Image::getPixelGreen(int posX, int posY) const {
    return green[(static_cast<size_t>(posY) * static_cast<size_t>(width)) + static_cast<size_t>(posX)];
}

// Función para obtener un valor azul
[[nodiscard]] uint16_t Image::getPixelBlue(int posX, int posY) const {
    return blue[(static_cast<size_t>(posY) * static_cast<size_t>(width)) + static_cast<size_t>(posX)];
}

namespace {
    template<typename T>
    inline T linearInterpolate(T value0, T value1, float tValue) {
        return static_cast<T>(value0 + (tValue * (value1 - value0)));
    }
}

// Función para redimensionar la imagen
void Image::resize(int nuevo_ancho, int nuevo_alto) {
    std::vector<uint16_t> nuevoRed(static_cast<size_t>(nuevo_ancho) * static_cast<size_t>(nuevo_alto));
    std::vector<uint16_t> nuevoGreen(static_cast<size_t>(nuevo_ancho) * static_cast<size_t>(nuevo_alto));
    std::vector<uint16_t> nuevoBlue(static_cast<size_t>(nuevo_ancho) * static_cast<size_t>(nuevo_alto));

    float const xRatio = static_cast<float>(width) / static_cast<float>(nuevo_ancho);
    float const yRatio = static_cast<float>(height) / static_cast<float>(nuevo_alto);

    for (int posY = 0; posY < nuevo_alto; ++posY) {
        for (int posX = 0; posX < nuevo_ancho; ++posX) {
            size_t const nuevo_indice = (static_cast<size_t>(posY) * static_cast<size_t>(nuevo_ancho)) + static_cast<size_t>(posX);

            InterpolationParams const params{.posX=posX, .posY=posY, .xRatio=xRatio, .yRatio=yRatio};

            nuevoRed[nuevo_indice] = getInterpolatedChannel(params, &Image::getPixelRed);
            nuevoGreen[nuevo_indice] = getInterpolatedChannel(params, &Image::getPixelGreen);
            nuevoBlue[nuevo_indice] = getInterpolatedChannel(params, &Image::getPixelBlue);
        }
    }

    width = nuevo_ancho;
    height = nuevo_alto;
    red = std::move(nuevoRed);
    green = std::move(nuevoGreen);
    blue = std::move(nuevoBlue);
}


uint16_t Image::getInterpolatedChannel(const InterpolationParams& params, uint16_t (Image::*getPixel)(int, int) const) const {
    float const originalX = static_cast<float>(params.posX) * params.xRatio;
    float const originalY = static_cast<float>(params.posY) * params.yRatio;

    int baseX = static_cast<int>(originalX);
    int baseY = static_cast<int>(originalY);

    float deltaX = originalX - static_cast<float>(baseX);
    float deltaY = originalY - static_cast<float>(baseY);

    // Asegurarse de que los índices estén dentro de los límites de la imagen
    if (baseX >= width - 1) {
        baseX = width - 2;
        deltaX = 1.0F;
    }
    if (baseY >= height - 1) {
        baseY = height - 2;
        deltaY = 1.0F;
    }

    uint16_t const arribaIzquierda = (this->*getPixel)(baseX, baseY);
    uint16_t const arribaDerecha = (this->*getPixel)(baseX + 1, baseY);
    uint16_t const abajoIzquierda = (this->*getPixel)(baseX, baseY + 1);
    uint16_t const abajoDerecha = (this->*getPixel)(baseX + 1, baseY + 1);

    return linearInterpolate(
        linearInterpolate(arribaIzquierda, arribaDerecha, deltaX),
        linearInterpolate(abajoIzquierda, abajoDerecha, deltaX),
        deltaY
    );
}


std::vector<std::pair<int, int>> Image::frecuenciaColores() const {
    std::unordered_map<int, int> histograma;
    for (size_t i = 0; i < red.size(); ++i) {
        int const valorColor = (red[i] << 16) | (green[i] << 8) | blue[i];
        histograma[valorColor]++;
    }

    std::vector<std::pair<int, int>> histogramaOrdenado(histograma.begin(), histograma.end());
    std::ranges::sort(histogramaOrdenado,
                      [](const auto &frecuenciaColor1, const auto &frecuenciaColor2) {
                          return frecuenciaColor1.second < frecuenciaColor2.second;
                      });

    return histogramaOrdenado;
}

// Generación del árbol KDTree
// NOLINTBEGIN(misc-no-recursion)
std::unique_ptr<KDTreeNode> Image::construccionKDTree(std::vector<std::unique_ptr<KDTreeNode>>& nodes, int depth) {
    if (nodes.empty()) {
        return nullptr;
    }
    int const canalColor = depth % 3;
    auto comparator = [canalColor](const std::unique_ptr<KDTreeNode>& nodeA, const std::unique_ptr<KDTreeNode>& nodeB) {
        if (canalColor == 0) {
            return nodeA->red < nodeB->red;
        } if (canalColor == 1) {
            return nodeA->green < nodeB->green;
        }             return nodeA->blue < nodeB->blue;

    };

    std::ranges::sort(nodes, comparator);

    // Encuentra el elemento medio
    size_t const elementoMedio = nodes.size() / 2;

    std::unique_ptr<KDTreeNode> raiz = std::move(nodes[elementoMedio]);

    std::vector<std::unique_ptr<KDTreeNode>> nodosIzquierda;
    std::vector<std::unique_ptr<KDTreeNode>> nodosDerecha;

    // Transferencia de elementos
    nodosIzquierda.reserve(elementoMedio);
    for (size_t i = 0; i < elementoMedio; ++i) {
        nodosIzquierda.push_back(std::move(nodes[i]));
    }

    nodosDerecha.reserve(nodes.size() - elementoMedio - 1);
    for (size_t i = elementoMedio + 1; i < nodes.size(); ++i) {
        nodosDerecha.push_back(std::move(nodes[i]));
    }

    // Recursividad
    raiz->left = construccionKDTree(nodosIzquierda, depth + 1);
    raiz->right = construccionKDTree(nodosDerecha, depth + 1);
    return raiz;
}


// Buscar el color más cercano para cambiarlo después
KDTreeNode* Image::colorCercano(KDTreeNode* raiz, const Color& colorObjetivo, int profundidad) {
    if (raiz == nullptr) {
        return nullptr;
    }
    KDTreeNode* ramaSiguiente = nullptr;
    KDTreeNode* ramaContraria = nullptr;
    int const colorCanal = profundidad % 3;
    int diff = 0;
    if (colorCanal == 0) {
        diff = colorObjetivo.red - raiz->red;
    } else if (colorCanal == 1) {
        diff = colorObjetivo.green - raiz->green;
    } else {
        diff = colorObjetivo.blue - raiz->blue;
    }
    if (diff < 0) {
        ramaSiguiente = raiz->left.get();
        ramaContraria = raiz->right.get();
    } else {
        ramaSiguiente = raiz->right.get();
        ramaContraria = raiz->left.get();
    }
    // Convertir el color de la raíz en una estructura Color
    Color const colorRaiz = {.red = raiz->red, .green = raiz->green, .blue = raiz->blue};
    // Llamada recursiva para encontrar el mejor color
    KDTreeNode* mejorColor = colorCercano(ramaSiguiente, colorObjetivo, profundidad + 1);
    if ((mejorColor == nullptr) || calcularEuclidea(colorObjetivo, colorRaiz) <
                             calcularEuclidea(colorObjetivo, {.red = mejorColor->red, .green = mejorColor->green, .blue = mejorColor->blue})) {
        mejorColor = raiz;
                             }

    // Comprobar si es necesario buscar en la rama opuesta
    if (diff * diff < calcularEuclidea(colorObjetivo, {.red = mejorColor->red, .green = mejorColor->green, .blue = mejorColor->blue})) {
        KDTreeNode* mejorColorTemporal = colorCercano(ramaContraria, colorObjetivo, profundidad + 1);
        if ((mejorColorTemporal != nullptr) && calcularEuclidea(colorObjetivo, {.red = mejorColorTemporal->red, .green = mejorColorTemporal->green, .blue = mejorColorTemporal->blue}) <
                                     calcularEuclidea(colorObjetivo, {.red = mejorColor->red, .green = mejorColor->green, .blue = mejorColor->blue})) {
            mejorColor = mejorColorTemporal;
                                     }
    }
    return mejorColor;
}
// NOLINTEND(misc-no-recursion)


// Eliminar el árbol
void Image::borrarKDTree(std::unique_ptr<KDTreeNode>& raiz) {
    raiz.reset();
}

//Borrar colores raros
void Image::borrarMenosFrecuentes(int threshold) {
    const auto coloresOrdenados = frecuenciaColores();

    std::unordered_map<int, int> coloresPocoFrecuentes;
    std::unordered_map<int, int> restoColores;
    std::vector<std::unique_ptr<KDTreeNode>> nodos;

    for (size_t i = 0; i < coloresOrdenados.size(); ++i) {
        if (i < static_cast<size_t>(threshold)) {
            coloresPocoFrecuentes[coloresOrdenados[i].first] = coloresOrdenados[i].first;
        } else {
            int const valorColor = coloresOrdenados[i].first;
            restoColores[valorColor] = valorColor;

            uint16_t const red = (valorColor >> 16) & 0xFF;
            uint16_t const green = (valorColor >> 8) & 0xFF;
            uint16_t const blue = valorColor & 0xFF;

            nodos.push_back(std::make_unique<KDTreeNode>(valorColor, red, green, blue));
        }
    }

    auto colorRaiz = construccionKDTree(nodos);
    coloresPocoFrecuentes = remplazoColores(coloresPocoFrecuentes, colorRaiz.get());
    borrarKDTree(colorRaiz);

    actualizarPixeles(coloresPocoFrecuentes);
}

std::unordered_map<int, int> Image::remplazoColores(std::unordered_map<int, int>& coloresPocoFrecuentes, KDTreeNode* colorRaiz) {
    for (auto& [valorColor, color] : coloresPocoFrecuentes) {
        uint16_t const canalRed = (valorColor >> 16) & 0xFF;
        uint16_t const canalGreen = (valorColor >> 8) & 0xFF;
        uint16_t const canalBlue = valorColor & 0xFF;

        // Crear una estrucutra Color
        Color const targetColor = {.red = canalRed, .green = canalGreen, .blue = canalBlue};

        if (KDTreeNode* nodoColorCercano = colorCercano(colorRaiz, targetColor)) {
            color = nodoColorCercano->colorValue;
        }
    }
    return coloresPocoFrecuentes;
}

// Calcular distancia euclidea sin raíz cuadrada
inline int Image::calcularEuclidea(const Color& color1, const Color& color2) {
    return ((color1.red - color2.red) * (color1.red - color2.red)) +
           ((color1.green - color2.green) * (color1.green - color2.green)) +
           ((color1.blue - color2.blue) * (color1.blue - color2.blue));
}

// Actualización de pixeles antiguos por los nuevos
void Image::actualizarPixeles(const std::unordered_map<int, int> &coloresPocoFrecuentes) {
    for (size_t i = 0; i < red.size(); ++i) {
        int const valorColor = (red[i] << 16) | (green[i] << 8) | blue[i];
        if (coloresPocoFrecuentes.contains(valorColor)) {
            int const nuevoColor = coloresPocoFrecuentes.at(valorColor);
            red[i] = (nuevoColor >> DESPLAZAMIENTO_16_BITS) & MASCARA_BYTE;
            green[i] = (nuevoColor >> DESPLAZAMIENTO_8_BITS) & MASCARA_BYTE;
            blue[i] = nuevoColor & MASCARA_BYTE;
        }
    }
}

// Función auxiliar para escribir el encabezado CPPM
void Image::cabeceraPPM(std::ofstream &file) const {
    file << "C6 " << width << " " << height << " " << maxColorValue << " ";
}

// Crear la tabla de colores y el mapa de índices
void Image::calcularColorTabla(std::unordered_map<int, int> &colorTable, std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &uniqueColors) const {
    int indice = 0;
    for (size_t i = 0; i < red.size(); ++i) {
        int const valorColor = (red[i] << 16) | (green[i] << 8) | blue[i];
        if (!colorTable.contains(valorColor)) {
            colorTable[valorColor] = indice++;
            uniqueColors.emplace_back(red[i], green[i], blue[i]);
        }
    }
}

// Escribir la tabla de colores en el archivo de salida
void Image::escribirColorTabla(std::ofstream &file, const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &coloresUnicos) {
    file << coloresUnicos.size() << "\n";
    for (const auto &[r, g, b] : coloresUnicos) {
        file.put(static_cast<char>(r));
        file.put(static_cast<char>(g));
        file.put(static_cast<char>(b));
    }
}

// Escribir los índices de los píxeles en el archivo de salida
void Image::writePixelIndices(std::ofstream &file, const std::unordered_map<int, int> &colorTabla, const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> &uniqueColors) const {
    for (size_t i = 0; i < red.size(); ++i) {
        int const valorColor = (red[i] << 16) | (green[i] << 8) | blue[i];
        int const indiceColor = colorTabla.at(valorColor);

        if (uniqueColors.size() <= LIMITE_COLOR_TABLA) {
            file.put(static_cast<char>(indiceColor));
        } else {
            file.put(static_cast<char>(indiceColor & MASCARA_BYTE));
            file.put(static_cast<char>((indiceColor >> DESPLAZAMIENTO_8_BITS) & MASCARA_BYTE));
        }
    }
}

// Compresión CPPM
void Image::compress(const std::string &filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error al guardar el archivo");
    }

    cabeceraPPM(file);

    std::unordered_map<int, int> colorTable;
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> uniqueColors;
    calcularColorTabla(colorTable, uniqueColors);

    escribirColorTabla(file, uniqueColors);
    writePixelIndices(file, colorTable, uniqueColors);

    file.close();
}
