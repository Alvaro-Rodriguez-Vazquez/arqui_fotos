#ifndef PRACTICA1_PROGARGS_HPP
#define PRACTICA1_PROGARGS_HPP

#include <string>
#include <vector>

class ProgArgs {
public:
    // Constructor que recibe los argumentos de entrada como vector de strings
    ProgArgs(const std::vector<std::string>& args);

    // Obtener la ruta del archivo de entrada
    [[nodiscard]] std::string getInputFile() const;

    // Obtener la ruta del archivo de salida
    [[nodiscard]] std::string getOutputFile() const;

    // Obtener la operación solicitada
    [[nodiscard]] std::string getOperation() const;

    // Obtener parámetros adicionales si existen
    [[nodiscard]] std::vector<std::string> getAdditionalParams() const;

    // Función para analizar y validar los argumentos
    static bool parse(const std::vector<std::string>& args);

private:
    // Función para validar los argumentos recibidos
    static void validateArgs(const std::vector<std::string>& args);

    std::string inputFile;
    std::string outputFile;
    std::string operation;
    std::vector<std::string> additionalParams;
};

#endif // PRACTICA1_PROGARGS_HPP
