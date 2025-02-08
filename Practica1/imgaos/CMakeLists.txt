# Definir la biblioteca 'imgaos'
add_library(imgaos
        imageaos.cpp  # Cambia este archivo por el que corresponde a tu implementación
        # Agrega otros archivos fuente de la biblioteca aquí
)

# Vinculamos la biblioteca con 'common' (si es necesario) y GSL
target_link_libraries(imgaos PRIVATE common Microsoft.GSL::GSL)
