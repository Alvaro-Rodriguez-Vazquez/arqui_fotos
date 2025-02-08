#include "./imgsoa/imagesoa.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <string>

// Prueba de carga de imagen en formato PPM
TEST(ImageSoaTest, LoadPPM) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";

    // Verifica que el archivo se cargue correctamente
    ASSERT_NO_THROW(image.loadPPM(inputFile));
    EXPECT_GT(image.width, 0);
    EXPECT_GT(image.height, 0);
    EXPECT_GE(image.maxColorValue, 0);
}

// Prueba de guardado de imagen en formato PPM
TEST(ImageSoaTest, SavePPM) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    const std::string outputFile = "sabatini_copy.ppm";

    // Cargar y luego guardar la imagen
    ASSERT_NO_THROW(image.loadPPM(inputFile));
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
    EXPECT_EQ(width, image.width);
    EXPECT_EQ(height, image.height);
    EXPECT_EQ(maxColorValue, image.maxColorValue);

    ifs.close();
    if (std::remove(outputFile.c_str()) != 0) {
        FAIL() << "Error al eliminar el archivo de salida";
    }
}

// Prueba de escala de intensidad
TEST(ImageSoaTest, ScaleIntensity) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    ASSERT_NO_THROW(image.loadPPM(inputFile));

    int const initialMaxColorValue = image.maxColorValue;
    float const newMaxLevel = static_cast<float>(initialMaxColorValue) / 2.0F;

    // Escala la intensidad y verifica el valor máximo de color actualizado
    image.scaleIntensity(newMaxLevel);
    EXPECT_EQ(image.maxColorValue, static_cast<int>(newMaxLevel));
}

// Prueba de redimensionamiento de la imagen
TEST(ImageSoaTest, ResizeImage) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    ASSERT_NO_THROW(image.loadPPM(inputFile));

    int const originalWidth = image.width;
    int const originalHeight = image.height;

    // Redimensiona la imagen a la mitad de su tamaño original
    image.resize(originalWidth / 2, originalHeight / 2);
    EXPECT_EQ(image.width, originalWidth / 2);
    EXPECT_EQ(image.height, originalHeight / 2);
}

// Prueba de eliminación de colores poco frecuentes
TEST(ImageSoaTest, RemoveRareColors) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    ASSERT_NO_THROW(image.loadPPM(inputFile));

    // Llama a removeRareColors y verifica que se complete sin excepciones
    ASSERT_NO_THROW(image.borrarMenosFrecuentes(5));
}

// Prueba de compresión
TEST(ImageSoaTest, CompressImage) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    const std::string compressedFile = "sabatini_compressed.ppm";

    ASSERT_NO_THROW(image.loadPPM(inputFile));
    ASSERT_NO_THROW(image.compress(compressedFile));

    std::ifstream ifs(compressedFile, std::ios::binary);
    EXPECT_TRUE(ifs.is_open());
    ifs.close();
    if (std::remove(compressedFile.c_str()) != 0) {
        FAIL() << "Error al eliminar el archivo de compresión";
    }
}

// Prueba de cálculo de histograma de colores usando el nuevo método de prueba
TEST(ImageSoaTest, CalculateHistogram) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    ASSERT_NO_THROW(image.loadPPM(inputFile));

    //auto histogram = image.testCalculateColorFrequencies();
    //EXPECT_FALSE(histogram.empty());
}

// Prueba de generación de tabla de colores
TEST(ImageSoaTest, GenerateColorTable) {
    Image image;
    const std::string inputFile = "../../../archivos_entrada/sabatini.ppm";
    ASSERT_NO_THROW(image.loadPPM(inputFile));

    //auto [colorTable, uniqueColors] = image.testCreateColorTable();
    //EXPECT_FALSE(colorTable.empty());
    //EXPECT_FALSE(uniqueColors.empty());
}

// Función principal para ejecutar todas las pruebas
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
