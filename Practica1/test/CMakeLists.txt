# CMakeLists.txt en el directorio 'test'

# Incluir el directorio 'common' para que las pruebas puedan acceder a sus headers
include_directories("../common")

# Unit test for 'common'
add_executable(utest-common utest-common.cpp)
target_link_libraries(utest-common PRIVATE common GTest::gtest_main)
add_test(NAME utest-common COMMAND utest-common)

# Unit tests for 'imgaos'
add_executable(utest-imgaos utest-imgaos.cpp
        ../imgaos/imageaos.cpp
        ../imgaos/imageaos.hpp)
target_link_libraries(utest-imgaos PRIVATE imgaos common GTest::gtest_main)
add_test(NAME utest-imgaos COMMAND utest-imgaos)

# Unit tests for 'imgsoa'
add_executable(utest-imgsoa utest-imgsoa.cpp
        ../imgsoa/imagesoa.cpp
        ../imgsoa/imagesoa.hpp)
target_link_libraries(utest-imgsoa PRIVATE imgsoa_lib common GTest::gtest_main)
add_test(NAME utest-imgsoa COMMAND utest-imgsoa)

# Functional test for imtool-aos
add_executable(ftest-aos ftest-aos.cpp)
# Quitar la línea que vincula imtool-aos como ejecutable y usar las bibliotecas necesarias
target_link_libraries(ftest-aos PRIVATE common imgaos GTest::gtest_main)
add_test(NAME ftest-aos COMMAND ftest-aos)

# Functional test for imtool-soa
add_executable(ftest-soa ftest-soa.cpp)
# Quitar la línea que vincula imtool-soa como ejecutable y usar las bibliotecas necesarias
target_link_libraries(ftest-soa PRIVATE imgsoa_lib common GTest::gtest_main)
add_test(NAME ftest-soa COMMAND ftest-soa)
