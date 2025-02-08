#include "progargs.hpp"
#include "binaryio.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <array>
#include <numbers>
#include <cstdio>

namespace {
    // Constantes para evitar magic numbers en el tamaño de los arrays
    constexpr int MAXLEVEL_ARGUMENTS_SIZE = 5;
    constexpr int RESIZE_ARGUMENTS_SIZE = 6;
    constexpr int COMPRESS_ARGUMENTS_SIZE = 4;
    constexpr int INFO_ARGUMENTS_SIZE = 4;
    constexpr int CUTFREQ_ARGUMENTS_SIZE = 5;
    constexpr int VALID_ARGS_SIZE = 5;
}

// Pruebas para ProgArgs

TEST(ProgArgsTest, ValidArguments) {
    std::array<const char*, VALID_ARGS_SIZE> args = {"imtool", "photo.ppm", "out.ppm", "maxlevel", "255"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getInputFile(), "photo.ppm");
    EXPECT_EQ(programArgs.getOutputFile(), "out.ppm");
    EXPECT_EQ(programArgs.getOperation(), "maxlevel");
    EXPECT_EQ(programArgs.getAdditionalParams().front(), "255");
}

// Test para argumentos válidos de "maxlevel" dentro del rango de 0 a 255
TEST(ProgArgsTest, ValidMaxLevelArguments) {
    std::array<const char*, MAXLEVEL_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "maxlevel", "128"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getInputFile(), "input.ppm");
    EXPECT_EQ(programArgs.getOutputFile(), "output.ppm");
    EXPECT_EQ(programArgs.getOperation(), "maxlevel");
    EXPECT_EQ(programArgs.getAdditionalParams().front(), "128");
}

// Test para "maxlevel" con valor límite inferior (0)
TEST(ProgArgsTest, MaxLevelZero) {
    std::array<const char*, MAXLEVEL_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "maxlevel", "0"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "maxlevel");
    EXPECT_EQ(programArgs.getAdditionalParams().front(), "0");
}

// Test para "maxlevel" con valor límite superior (255)
TEST(ProgArgsTest, MaxLevelMax) {
    std::array<const char*, MAXLEVEL_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "maxlevel", "255"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "maxlevel");
    EXPECT_EQ(programArgs.getAdditionalParams().front(), "255");
}

// Test para operación "resize" con dos parámetros adicionales
TEST(ProgArgsTest, ValidResizeArguments) {
    std::array<const char*, RESIZE_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "resize", "200", "150"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "resize");
    EXPECT_EQ(programArgs.getAdditionalParams().at(0), "200");
    EXPECT_EQ(programArgs.getAdditionalParams().at(1), "150");
}

// Test para operación "info" sin parámetros adicionales
TEST(ProgArgsTest, ValidInfoArguments) {
    std::array<const char*, INFO_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "info"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "info");
    EXPECT_TRUE(programArgs.getAdditionalParams().empty());
}

// Test para operación "cutfreq" con un parámetro adicional
TEST(ProgArgsTest, ValidCutFreqArguments) {
    std::array<const char*, CUTFREQ_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "cutfreq", "10"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "cutfreq");
    EXPECT_EQ(programArgs.getAdditionalParams().front(), "10");
}

// Test para operación "compress" sin parámetros adicionales
TEST(ProgArgsTest, ValidCompressArguments) {
    std::array<const char*, COMPRESS_ARGUMENTS_SIZE> args = {"imtool", "input.ppm", "output.ppm", "compress"};
    constexpr int argc = args.size();

    const ProgArgs programArgs(std::vector<std::string>(args.begin(), args.end()));
    EXPECT_TRUE(programArgs.parse(argc, args.data()));
    EXPECT_EQ(programArgs.getOperation(), "compress");
    EXPECT_TRUE(programArgs.getAdditionalParams().empty());
}

// Pruebas para BinaryIO

TEST(BinaryIOTest, WriteAndReadInt) {
    const std::string filename = "test.bin";
    constexpr int TEST_INT = 123456;
    {
        std::ofstream ofs(filename, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        BinaryIO::writeValue(ofs, TEST_INT);
    }

    {
        std::ifstream ifs(filename, std::ios::binary);
        ASSERT_TRUE(ifs.is_open());
        const int value = BinaryIO::readValue<int>(ifs);
        EXPECT_EQ(value, TEST_INT);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::perror("Error al eliminar el archivo temporal");
    }
}

TEST(BinaryIOTest, WriteAndReadFloat) {
    const std::string filename = "test.bin";
    constexpr float TEST_FLOAT = std::numbers::pi_v<float>;
    {
        std::ofstream ofs(filename, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        BinaryIO::writeValue(ofs, TEST_FLOAT);
    }

    {
        std::ifstream ifs(filename, std::ios::binary);
        ASSERT_TRUE(ifs.is_open());
        const auto value = BinaryIO::readValueFloat(ifs);
        EXPECT_FLOAT_EQ(value, TEST_FLOAT);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::perror("Error al eliminar el archivo temporal");
    }
}

TEST(BinaryIOTest, FileOpenError) {
    const std::ifstream ifs("nonexistent_file.bin", std::ios::binary);
    ASSERT_FALSE(ifs.is_open());
}

TEST(BinaryIOTest, WriteAndReadDouble) {
    const std::string filename = "test_double.bin";
    constexpr double TEST_DOUBLE = std::numbers::pi;
    {
        std::ofstream ofs(filename, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        BinaryIO::writeValue(ofs, TEST_DOUBLE);
    }

    {
        std::ifstream ifs(filename, std::ios::binary);
        ASSERT_TRUE(ifs.is_open());
        const auto value = BinaryIO::readValueDouble(ifs);
        EXPECT_DOUBLE_EQ(value, TEST_DOUBLE);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::perror("Error al eliminar el archivo temporal");
    }
}

TEST(BinaryIOTest, ReadFromEmptyFile) {
    const std::string filename = "empty_test.bin";
    {
        std::ofstream const ofs(filename, std::ios::binary);
    }

    {
        std::ifstream ifs(filename, std::ios::binary);
        ASSERT_TRUE(ifs.is_open());
        EXPECT_THROW({
            BinaryIO::readValue<int>(ifs);
        }, std::runtime_error);
    }

    if (std::remove(filename.c_str()) != 0) {
        std::perror("Error al eliminar el archivo temporal");
    }
}

// Ejecutar todas las pruebas
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
