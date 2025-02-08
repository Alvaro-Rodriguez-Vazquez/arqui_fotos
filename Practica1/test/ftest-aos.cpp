#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <filesystem>
#include <array>  // Necesario para std::array

// Espacio de nombres anónimo para restringir el alcance y eliminar *warnings*
namespace {
    constexpr auto IMTOOL_EXECUTABLE = R"(..\imtool-aos\imtool-aos.exe)";
    constexpr auto INPUT_FILE = R"(..\..\..\archivos_entrada\sabatini.ppm)";
    constexpr auto OUTPUT_CHECK_FILE = R"(output_check.ppm)";
    constexpr size_t BUFFER_SIZE = 128;
    constexpr int SLEEP_DURATION_MS = 100;

    // Función auxiliar para ejecutar el comando y capturar la salida (incluye stderr)
    std::string execCommand(const std::string& command) {
        std::array<char, BUFFER_SIZE> buffer{};
        std::string result;
        FILE* pipe = popen((command + " 2>&1").c_str(), "r");
        if (pipe == nullptr) {
            throw std::runtime_error("_popen() failed!");
        }

        // Usamos un bucle más seguro y explícito
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result.append(buffer.data());
        }
        _pclose(pipe);
        return result;
    }

    // Función auxiliar para verificar si un archivo existe
    bool fileExists(const std::string& filename) {
        struct stat buffer{};
        return (stat(filename.c_str(), &buffer) == 0);
    }
}

// Prueba funcional para la operación 'info'
TEST(FtestAos, InfoOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output.ppm info";
    std::cout << "Command executed: " << command << '\n';
    std::string const output = execCommand(command);

    EXPECT_NE(output.find("Width:"), std::string::npos);
    EXPECT_NE(output.find("Height:"), std::string::npos);
    EXPECT_NE(output.find("Max Color Value:"), std::string::npos);
}

// Prueba funcional de integridad del archivo de salida
TEST(FtestAos, OutputFileIntegrity) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " " + OUTPUT_CHECK_FILE + " maxlevel 233";
    std::cout << "Command executed: " << command << '\n';

    std::cout << "Current working directory: " << std::filesystem::current_path() << '\n';

    try {
        std::string const output = execCommand(command);
        std::cout << "Command output: " << output << '\n';
    } catch (const std::exception& e) {
        FAIL() << "Error: La ejecución del comando falló. " << e.what();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));

    ASSERT_TRUE(fileExists(OUTPUT_CHECK_FILE)) << "Error: El archivo output_check.ppm no se generó.";

    std::ifstream outputFile(OUTPUT_CHECK_FILE, std::ios::binary);
    ASSERT_TRUE(outputFile.good()) << "Error: El archivo output_check.ppm no se pudo abrir.";

    std::string magicNumber;
    outputFile >> magicNumber;
    EXPECT_EQ(magicNumber, "P6") << "Error: Formato incorrecto, se esperaba 'P6' en el encabezado del archivo.";
    outputFile.close();
}

// Pruebas funcionales para otras operaciones

TEST(FtestAos, MaxLevelOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output_max.ppm maxlevel 128";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists("output_max.ppm"));
}

TEST(FtestAos, ResizeOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output_resized.ppm resize 200 150";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists("output_resized.ppm"));
}

TEST(FtestAos, CutFreqOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output_cutfreq.ppm cutfreq 10";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists("output_cutfreq.ppm"));
}

TEST(FtestAos, CompressOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output_compressed.ppm compress";
    execCommand(command); // Ejecuta el comando sin almacenar la salida
    EXPECT_TRUE(fileExists("output_compressed.ppm"));
}

TEST(FtestAos, InvalidOperation) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output_invalid.ppm invalidop";
    std::cout << "Command executed: " << command << '\n';
    std::string const output = execCommand(command);
    std::cout << "Output received: " << output << '\n';
    EXPECT_NE(output.find("Error: Operación no válida"), std::string::npos);
}

TEST(FtestAos, InvalidInputFile) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" nonexistent.ppm ") + " output.ppm info";
    std::cout << "Command executed: " << command << '\n';
    std::string const output = execCommand(command);
    std::cout << "Output received: " << output << '\n';
    EXPECT_NE(output.find("Error al abrir el archivo"), std::string::npos);
}

TEST(FtestAos, InsufficientParameters) {
    std::string const command = IMTOOL_EXECUTABLE + std::string(" ") + INPUT_FILE + " output.ppm resize 200";
    std::cout << "Command executed: " << command << '\n';
    std::string const output = execCommand(command);
    std::cout << "Output received: " << output << '\n';
    EXPECT_NE(output.find("Error: La operación resize requiere dos argumentos adicionales"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}