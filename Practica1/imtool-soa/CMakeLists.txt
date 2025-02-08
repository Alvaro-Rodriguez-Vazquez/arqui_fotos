# Definimos el ejecutable 'imtool-soa'
add_executable(imtool-soa main.cpp)

# Vinculamos con las bibliotecas necesarias
target_link_libraries(imtool-soa PRIVATE common GTest::gtest_main imgsoa_lib)
