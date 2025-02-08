# Definimos la biblioteca 'common'
add_library(common
        progargs.cpp
        binaryio.cpp
)

# Vinculamos la biblioteca con GSL (si es necesario)
target_link_libraries(common PRIVATE Microsoft.GSL::GSL)
