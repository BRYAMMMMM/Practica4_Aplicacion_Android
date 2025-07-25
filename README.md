# Practica4_Aplicacion_Android

Aplicación Android desarrollada en **Android Studio** que permite al usuario dibujar una figura en pantalla y clasificarla mediante el cálculo de **Momentos de Hu** y **Firma de la Figura**, usando código nativo en **C++ con OpenCV**.

## Características principales

- **Interfaz de dibujo:** El usuario puede dibujar a mano alzada una figura en el lienzo (DrawingView).
- **Procesamiento nativo:** Se usa OpenCV en C++ para:
  - Binarización de la imagen.
  - Inversión de colores y rellenado de figura.
  - Cálculo de **Momentos invariantes de Hu**.
  - Obtención de la **Firma de la Figura (Shape Signature)**.
  - Clasificación simple mediante distancia Euclídea con datos almacenados en `momentos.csv`.
- **Interfaz amigable:** Botones para detectar la figura y limpiar el lienzo.


## Requisitos

- **Android Studio Giraffe (o superior)**.
- **NDK y CMake** habilitados.
- **OpenCV para Android** (SDK integrado en `app/src/main/jniLibs`).
- **Dispositivo Android 5.0 (API 21)** o superior.

## Instalación y ejecución

1. Clona este repositorio:
   ```bash
   git clone https://github.com/BRYAMMMMM/Practica4_Aplicacion_Android.git
   cd Practica4_Aplicacion_Android
   
Abre el proyecto con Android Studio.

Asegúrate de que el NDK y CMake estén instalados (desde el SDK Manager).

Sincroniza el proyecto y compílalo.

Conecta tu dispositivo Android (o usa un emulador) y ejecuta la aplicación.

## Autores
Bryam Peralta – Desarrollo Android y C++.

Anthony Benavides - Desarrollo Android y C++.
