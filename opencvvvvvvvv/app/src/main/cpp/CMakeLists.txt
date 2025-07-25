cmake_minimum_required(VERSION 3.22.1)

project("opencvvvvvvvv")

# Usa OpenCV en modo estático
set(OpenCV_STATIC ON)

# Ruta al OpenCV Android SDK (ajusta si lo cambias de ubicación)
set(OpenCV_DIR /home/bryam/Imágenes/Septimo_ciclo/OpenCV-android-sdk/sdk/native/jni)

# Buscar OpenCV
find_package(OpenCV REQUIRED)

# Crear la librería compartida para la app
add_library(opencvvvvvvvv SHARED native-lib.cpp)

# Encontrar bibliotecas necesarias de Android
find_library(log-lib log)
find_library(jnigraphics-lib jnigraphics)

# Enlazar las bibliotecas
target_link_libraries(opencvvvvvvvv
        ${OpenCV_LIBS}
        ${log-lib}
        ${jnigraphics-lib}
)
