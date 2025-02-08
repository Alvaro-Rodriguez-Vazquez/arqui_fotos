#ifndef PRACTICA1_BINARYIO_HPP
#define PRACTICA1_BINARYIO_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits> // Para std::enable_if y std::is_integral

// Clase BinaryIO para operaciones de E/S en binario
class BinaryIO {
public:
    // Lee un archivo binario y devuelve el contenido en un vector de bytes
    static std::vector<uint8_t> readBinaryFile(const std::string& filePath);

    // Escribe el contenido de un vector de bytes en un archivo binario
    static void writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data);

    // Lee un valor de tipo T desde un flujo binario (solo para tipos enteros)
    template <typename T>
    static T
    readValue(std::istream& input) requires std::is_integral_v<T> {
        T value = 0;
        std::vector<char> buffer(sizeof(T));
        if (!input.read(buffer.data(), sizeof(T))) {
            throw std::runtime_error("Error: No se pudo leer el valor.");
        }

        // Copiar bytes a `value` usando desplazamiento de bits
        for (size_t i = 0; i < sizeof(T); ++i) {
            constexpr int BYTE_SHIFT = 8;
            value |= static_cast<T>(static_cast<unsigned char>(buffer[i])) << (BYTE_SHIFT * i);
        }

        return value;
    }

    // Escribe un valor de tipo T en un flujo binario (solo para tipos enteros)
    template <typename T>
    static void
    writeValue(std::ostream& output, const T& value) requires std::is_integral_v<T> {
        std::vector<char> buffer(sizeof(T));
        constexpr uint8_t BYTE_MASK = 0xFF;
        for (size_t i = 0; i < sizeof(T); ++i) {
            constexpr int BYTE_SHIFT = 8;
            buffer[i] = static_cast<char>((value >> (BYTE_SHIFT * i)) & BYTE_MASK);
        }

        if (!output.write(buffer.data(), sizeof(T))) {
            throw std::runtime_error("Error: No se pudo escribir el valor.");
        }
    }

    // Lee un valor de punto flotante desde un flujo binario sin reinterpret_cast
    static float readValueFloat(std::istream& input);

    static double readValueDouble(std::istream& input);

    // Escribe un valor de punto flotante en un flujo binario sin reinterpret_cast
    static void writeValue(std::ostream& output, const float& value);

    static void writeValue(std::ostream& output, const double& value);
};

#endif // PRACTICA1_BINARYIO_HPP