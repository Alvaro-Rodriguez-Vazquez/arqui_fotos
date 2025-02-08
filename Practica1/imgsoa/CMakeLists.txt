# Definir la biblioteca 'imgsoa'
add_library(imgsoa_lib
        imagesoa.cpp
        # Otros archivos fuente si los tienes
)

# Vinculamos la biblioteca con las dependencias necesarias
target_link_libraries(imgsoa_lib PRIVATE common Microsoft.GSL::GSL)
