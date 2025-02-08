#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <array>
#include <stdexcept>
#include <cstdio>
#include <sstream>  // Para ostringstream

namespace {
    constexpr auto IMTOOL_EXECUTABLE = R"(..\imtool-soa\imtool-soa.exe)";
    constexpr auto INPUT_FILE = R"(..\..\..\archivos_entrada\sabatini.ppm)";
    constexpr auto OUTPUT_FILE = R"(output.ppm)";
    constexpr auto OUTPUT_COMPRESSED_FILE = R"(output_compressed.cppm)";
    constexpr auto MAX_LEVEL = 128;
    constexpr size_t BUFFER_SIZE = 128;  // Evita el "magic number" 128

    // Función auxiliar para ejecutar el comando y capturar la salida (incluye stderr)
    std::string execCommand(const std::string& command) {
        std::array<char, BUFFER_SIZE> buffer{};
        std::ostringstream result;  // Cambiamos a ostringstream

        FILE* pipe = popen((command + " 2>&1").c_str(), "r");
        if (pipe == nullptr) {
            throw std::runtime_error("_popen() failed!");
        }

        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result << buffer.data();
        }

        _pclose(pipe);
        return result.str();  // Convertimos el resultado de ostringstream a std::string
    }

    // Función auxiliar para verificar si un archivo existe
    bool fileExists(const std::string& filename) {
        struct stat buffer{};
        return (stat(filename.c_str(), &buffer) == 0);
    }
}

// Prueba funcional para la operación 'info'
TEST(FtestSoa, InfoOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " info";
    std::string const output = execCommand(command);
    EXPECT_NE(output.find("Width:"), std::string::npos);
    EXPECT_NE(output.find("Height:"), std::string::npos);
    EXPECT_NE(output.find("Max Color Value:"), std::string::npos);
}

// Prueba funcional para la operación 'maxlevel'
TEST(FtestSoa, MaxLevelOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " maxlevel " + std::to_string(MAX_LEVEL);
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists(OUTPUT_FILE));

    std::ifstream outputFile(OUTPUT_FILE, std::ios::binary);
    EXPECT_TRUE(outputFile.good());
    outputFile.close();
}

// Prueba funcional para la operación 'resize'
TEST(FtestSoa, ResizeOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " resize 200 150";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists(OUTPUT_FILE));
}

// Prueba funcional para la operación 'cutfreq'
TEST(FtestSoa, CutFreqOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " cutfreq 10";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists(OUTPUT_FILE));
}

// Prueba funcional para la operación 'compress'
TEST(FtestSoa, CompressOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_COMPRESSED_FILE + " compress";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists(OUTPUT_COMPRESSED_FILE));
}

// Prueba de manejo de errores: operación no válida
TEST(FtestSoa, InvalidOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " invalidop";
    std::string const output = execCommand(command);
    EXPECT_NE(output.find("Error: Operación no válida"), std::string::npos);
}

// Prueba de manejo de errores: archivo de entrada no válido
TEST(FtestSoa, InvalidInputFile) {
    std::string const command = std::string(IMTOOL_EXECUTABLE) + " nonexistent.ppm " + std::string(OUTPUT_FILE) + " info";
    std::string const output = execCommand(command);
    EXPECT_NE(output.find("Error al abrir el archivo"), std::string::npos);
}

// Prueba de manejo de errores: parámetros insuficientes
TEST(FtestSoa, InsufficientParameters) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_FILE + " resize 200";
    std::string const output = execCommand(command);
    EXPECT_NE(output.find("Error: La operación resize requiere dos argumentos adicionales"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
