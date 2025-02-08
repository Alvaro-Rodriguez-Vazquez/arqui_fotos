# Definimos el ejecutable 'imtool-aos'
add_executable(imtool-aos main.cpp)

# Añadir la biblioteca imgAOS de imageaos.cpp
add_library(imgAOS ../imgaos/imageaos.cpp)  # Ruta relativa al archivo imageaos.cpp

# Vincular con las bibliotecas necesarias
target_link_libraries(imtool-aos PRIVATE common imgAOS GTest::gtest_main)
