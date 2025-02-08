#include "progargs.hpp"
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>

ProgArgs::ProgArgs(const std::vector<std::string>& args) {
    inputFile = (args.size() > 1) ? args[1] : "";
    outputFile = (args.size() > 2) ? args[2] : "";
    operation = (args.size() > 3) ? args[3] : "";

    validateArgs(args);

    // Los argumentos adicionales dependen de la operación seleccionada
    for (std::size_t i = 4; i < args.size(); ++i) {
        additionalParams.emplace_back(args[i]);
    }
}


std::string ProgArgs::getInputFile() const {
    return inputFile;
}

std::string ProgArgs::getOutputFile() const {
    return outputFile;
}

std::string ProgArgs::getOperation() const {
    return operation;
}

std::vector<std::string> ProgArgs::getAdditionalParams() const {
    return additionalParams;
}


bool ProgArgs::parse(const std::vector<std::string>& args) {
    try {
        validateArgs(args);
        return true;  // La validación fue exitosa
    } catch ([[maybe_unused]] const std::invalid_argument& e) {
        return false;  // Hubo un error en la validación
    }
}


void ProgArgs::validateArgs(const std::vector<std::string>& args) {
    constexpr int MIN_ARG_COUNT = 4;
    constexpr int MAXLEVEL_ARG_COUNT = 5;

    if (args.size() < MIN_ARG_COUNT) {
        throw std::invalid_argument("Error: Número inválido de argumentos. Se requieren al menos 3.");
    }

    const std::string& operation = args[3];
    if (operation != "info" && operation != "maxlevel" && operation != "resize" &&
        operation != "cutfreq" && operation != "compress") {
        throw std::invalid_argument("Error: Operación no válida: " + operation);
    }

    if (operation == "maxlevel") {
        if (args.size() != MAXLEVEL_ARG_COUNT) {
            throw std::invalid_argument("Error: La operación maxlevel requiere un argumento adicional.");
        }

        // Validación adicional para el valor de maxlevel
        int const maxlevel_value = std::stoi(args[4]);
        if (constexpr int MAX_LEVEL_VALUE = 65535; maxlevel_value < 0 || maxlevel_value > MAX_LEVEL_VALUE) {
            throw std::invalid_argument("Error: Valor fuera de rango para maxlevel. Debe estar entre 0 y 255.");
        }
    }

    if (constexpr int RESIZE_ARG_COUNT = 6; operation == "resize" && args.size() != RESIZE_ARG_COUNT) {
        throw std::invalid_argument("Error: La operación resize requiere dos argumentos adicionales (nuevo ancho y alto).");
    }

    if (operation == "cutfreq" && args.size() != MAXLEVEL_ARG_COUNT) {
        throw std::invalid_argument("Error: La operación cutfreq requiere un argumento adicional.");
    }

    if (operation == "compress" && args.size() != MIN_ARG_COUNT) {
        throw std::invalid_argument("Error: La operación compress no acepta argumentos adicionales.");
    }
}

// Función principal modificada
int mainWithArgs(const std::vector<std::string>& args) {
    // Crear una instancia de ProgArgs y analizar los argumentos
    ProgArgs progArgs(args);
    if (!progArgs.parse(args)) {
        std::cerr << "Error en los argumentos." << '\n';
        return 1;
    }

    // Resto del código...
    return 0;
}

int main(int argc, char* argv[]) {
    // Convierte los argumentos a un vector de strings sin aritmética de punteros
    std::vector<std::string> args(argc);
    for (int i = 0; i < argc; ++i) {
        args[i] = argv[i];
    }

    // Llama a la función principal con los argumentos como std::vector<std::string>
    return mainWithArgs(args);
}
