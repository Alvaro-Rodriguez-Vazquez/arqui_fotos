#include "./imgaos/imageaos.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <string>

// Namespace anónimo para limitar el alcance de getInputFile a este archivo D
namespace {
    const std::string& getInputFile() {
        static const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
        return inputFile;
    }
}

// Prueba de carga de imagen en formato PPM
TEST(ImageAosTest, LoadPPM) {
    Image image;

    // Verifica que el archivo se cargue correctamente
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));
    EXPECT_GT(image.getWidth(), 0);
    EXPECT_GT(image.getHeight(), 0);
    EXPECT_GE(image.getMaxColorValue(), 0);
}

// Prueba de guardado de imagen en formato PPM
TEST(ImageAosTest, SavePPM) {
    Image image;
    const std::string outputFile = "photo_copy.ppm";

    // Cargar y luego guardar la imagen
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));
    ASSERT_NO_THROW(image.savePPM(outputFile));

    // Verifica que el archivo de salida se haya creado
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());

    // Verifica que contenga el encabezado PPM
    std::string magicNumber;
    int width = 0;
    int height = 0;
    int maxColorValue = 0;
    ifs >> magicNumber >> width >> height >> maxColorValue;
    EXPECT_EQ(magicNumber, "P6");
    EXPECT_EQ(width, image.getWidth());
    EXPECT_EQ(height, image.getHeight());
    EXPECT_EQ(maxColorValue, image.getMaxColorValue());

    ifs.close();
    if (std::remove(outputFile.c_str()) != 0) {
        FAIL() << "Error al eliminar el archivo de salida";
    }
}

// Prueba de escala de intensidad
TEST(ImageAosTest, ScaleIntensity) {
    Image image;
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));

    const int initialMaxColorValue = image.getMaxColorValue();
    const float newMaxLevel = static_cast<float>(initialMaxColorValue) / 2.0F;

    // Escala la intensidad y verifica el valor máximo de color actualizado
    image.scaleIntensity(newMaxLevel);
    EXPECT_EQ(image.getMaxColorValue(), static_cast<int>(newMaxLevel));
}

// Prueba de redimensionamiento de la imagen
TEST(ImageAosTest, ResizeImage) {
    Image image;
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));

    const int originalWidth = image.getWidth();
    const int originalHeight = image.getHeight();

    // Redimensiona la imagen a la mitad de su tamaño original
    image.resize(originalWidth / 2, originalHeight / 2);
    EXPECT_EQ(image.getWidth(), originalWidth / 2);
    EXPECT_EQ(image.getHeight(), originalHeight / 2);
}

// Prueba de eliminación de colores poco frecuentes
TEST(ImageAosTest, RemoveRareColors) {
    Image image;
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));

    // Llama a removeRareColors y verifica que se complete sin excepciones
    ASSERT_NO_THROW(image.removeRareColors(5));
}

// Prueba de compresión
TEST(ImageAosTest, CompressImage) {
    Image image;
    const std::string compressedFile = "photo_compressed.ppm";

    ASSERT_NO_THROW(image.loadPPM(getInputFile()));
    ASSERT_NO_THROW(image.compress(compressedFile));

    std::ifstream ifs(compressedFile, std::ios::binary);
    EXPECT_TRUE(ifs.is_open());
    ifs.close();
    if (std::remove(compressedFile.c_str()) != 0) {
        FAIL() << "Error al eliminar el archivo de compresión";
    }
}

// Prueba de cálculo de histograma de colores
TEST(ImageAosTest, CalculateHistogram) {
    Image image;
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));

    //auto histogram = image.calculateHistogram();
    //EXPECT_FALSE(histogram.empty());
}

// Prueba de generación de tabla de colores
TEST(ImageAosTest, GenerateColorTable) {
    Image image;
    ASSERT_NO_THROW(image.loadPPM(getInputFile()));

    auto [colorTable, colorList] = image.generateColorTable();
    EXPECT_FALSE(colorTable.empty());
    EXPECT_FALSE(colorList.empty());
}

// Función principal para ejecutar todas las pruebas
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
