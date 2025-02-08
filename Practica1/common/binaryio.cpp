#include "binaryio.hpp"

#include <cmath>
#include <fstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstring> // Para usar std::memcpy

std::vector<uint8_t> BinaryIO::readBinaryFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error: No se puede abrir el archivo para lectura: " + filePath);
    }

    file.seekg(0, std::ios::end);
    const auto fileSize = static_cast<std::vector<uint8_t>::size_type>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);
    for (std::vector<uint8_t>::size_type i = 0; i < fileSize; ++i) {
        int const byte = file.get();
        if (byte == EOF) {
            throw std::runtime_error("Error: No se pudo leer el archivo completo: " + filePath);
        }
        buffer[i] = static_cast<uint8_t>(byte);
    }

    return buffer;
}

void BinaryIO::writeBinaryFile(const std::string& filePath, const std::vector<uint8_t>& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error: No se puede abrir el archivo para escritura: " + filePath);
    }

    for (auto byte : data) {
        file.put(static_cast<char>(byte));
        if (file.fail()) {
            throw std::runtime_error("Error: No se pudo escribir el archivo completo: " + filePath);
        }
    }
}

float BinaryIO::readValueFloat(std::istream& input) {
    float value = NAN;
    std::vector<char> buffer(sizeof(float));
    if (!input.read(buffer.data(), sizeof(float))) {
        throw std::runtime_error("Error: No se pudo leer el valor float.");
    }
    std::memcpy(&value, buffer.data(), sizeof(float)); // Copia los bytes manualmente
    return value;
}

double BinaryIO::readValueDouble(std::istream& input) {
    double value = NAN;
    std::vector<char> buffer(sizeof(double));
    if (!input.read(buffer.data(), sizeof(double))) {
        throw std::runtime_error("Error: No se pudo leer el valor double.");
    }
    std::memcpy(&value, buffer.data(), sizeof(double)); // Copia los bytes manualmente
    return value;
}

void BinaryIO::writeValue(std::ostream& output, const float& value) {
    std::vector<char> buffer(sizeof(float));
    std::memcpy(buffer.data(), &value, sizeof(float)); // Copia los bytes manualmente
    if (!output.write(buffer.data(), sizeof(float))) {
        throw std::runtime_error("Error: No se pudo escribir el valor float.");
    }
}

void BinaryIO::writeValue(std::ostream& output, const double& value) {
    std::vector<char> buffer(sizeof(double));
    std::memcpy(buffer.data(), &value, sizeof(double)); // Copia los bytes manualmente
    if (!output.write(buffer.data(), sizeof(double))) {
        throw std::runtime_error("Error: No se pudo escribir el valor double.");
    }
}
