#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <sstream>
#include <vector>
#include <cmath>
#include <limits>
#include <android/log.h>

using namespace cv;
using namespace std;

vector<pair<string, vector<double>>> momentosDataset;
bool cargado = false;

void cargarMomentos(AAssetManager *mgr) {
    AAsset *file = AAssetManager_open(mgr, "momentos.csv", AASSET_MODE_BUFFER);
    if (!file) return;

    off_t length = AAsset_getLength(file);
    char *buffer = new char[length + 1];
    AAsset_read(file, buffer, length);
    buffer[length] = '\0';

    stringstream ss(buffer);
    string linea;
    getline(ss, linea);

    while (getline(ss, linea)) {
        stringstream ls(linea);
        string etiqueta, valor;
        getline(ls, etiqueta, ',');
        vector<double> descriptores;
        while (getline(ls, valor, ',')) {
            descriptores.push_back(stod(valor));
        }
        momentosDataset.push_back({etiqueta, descriptores});
    }

    delete[] buffer;
    AAsset_close(file);
    cargado = true;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_opencvvvvvvvv_MainActivity_classifyShape(
        JNIEnv *env,
        jobject thiz,
        jbyteArray imageData,
        jint width,
        jint height,
        jobject assetManager) {

    if (!cargado) {
        AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
        cargarMomentos(mgr);
    }

    jbyte *bytes = env->GetByteArrayElements(imageData, nullptr);
    Mat gray(height, width, CV_8UC1, reinterpret_cast<unsigned char *>(bytes));

    Mat binaria;
    threshold(gray, binaria, 0, 255, THRESH_BINARY | THRESH_OTSU);
    if (binaria.at<uchar>(0, 0) > 128) {
        bitwise_not(binaria, binaria);
    }
    morphologyEx(binaria, binaria, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    vector<vector<Point>> contornos;
    findContours(binaria, contornos, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contornos.empty()) {
        env->ReleaseByteArrayElements(imageData, bytes, JNI_ABORT);
        return env->NewStringUTF("No se encontró figura");
    }

    vector<Point> contorno = contornos[0];
    for (const auto& c : contornos) {
        if (contourArea(c) > contourArea(contorno)) {
            contorno = c;
        }
    }

    double area = contourArea(contorno);
    if (area < 100.0 || contorno.size() < 5) {
        env->ReleaseByteArrayElements(imageData, bytes, JNI_ABORT);
        return env->NewStringUTF("Figura demasiado pequeña o no válida");
    }

    __android_log_print(ANDROID_LOG_INFO, "DEBUG", "Área contorno: %f", contourArea(contorno));

    Moments m = moments(contorno);
    double hu[7];
    HuMoments(m, hu);
    vector<double> huTest;
    for (int i = 0; i < 7; ++i) {
        huTest.push_back(-1 * copysign(1.0, hu[i]) * log10(abs(hu[i]) + 1e-30));
    }

    Point2f centro((float)m.m10 / m.m00, (float)m.m01 / m.m00);
    vector<float> firma;
    for (const Point& pt : contorno) {
        firma.push_back(norm(Point2f(pt) - centro));
    }

    Mat input = Mat(firma).reshape(1, 1);
    input.convertTo(input, CV_32F);
    Mat fft;
    dft(input, fft, DFT_COMPLEX_OUTPUT);

    vector<double> fftMag;
    for (int i = 0; i < 10 && i < fft.cols; ++i) {
        float re = fft.at<Vec2f>(0, i)[0];
        float im = fft.at<Vec2f>(0, i)[1];
        fftMag.push_back(sqrt(re * re + im * im));
    }

    double suma = 0.0;
    for (double v : fftMag) suma += v * v;
    double norma = sqrt(suma);
    if (norma > 0) {
        for (double &v : fftMag) v /= norma;
    }

    double mejorDist = numeric_limits<double>::max();
    string mejorClase = "Desconocido";

    for (const auto& [etiqueta, vec] : momentosDataset) {
        if (vec.size() < 17) continue;

        double dist = 0.0;
        for (int i = 0; i < 7; ++i)
            dist += pow(vec[i] - huTest[i], 2);
        for (int i = 0; i < 10 && i < fftMag.size(); ++i)
            dist += pow(vec[7 + i] - fftMag[i], 2);

        dist = sqrt(dist);
        __android_log_print(ANDROID_LOG_INFO, "DEBUG", "Clase: %s | Distancia: %.6f", etiqueta.c_str(), dist);

        if (dist < mejorDist) {
            mejorDist = dist;
            mejorClase = etiqueta;
        }
    }

    __android_log_print(ANDROID_LOG_INFO, "DEBUG", "Clase elegida: %s | Distancia mínima: %.6f", mejorClase.c_str(), mejorDist);

    env->ReleaseByteArrayElements(imageData, bytes, JNI_ABORT);
    return env->NewStringUTF(mejorClase.c_str());
}
