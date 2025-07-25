#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

Mat skeletonize(const Mat& img) {
    Mat skeleton = Mat::zeros(img.size(), CV_8UC1);
    Mat temp, eroded;
    Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));

    Mat binary = img.clone();
    bool done;
    do {
        erode(binary, eroded, element);
        dilate(eroded, temp, element);
        subtract(binary, temp, temp);
        bitwise_or(skeleton, temp, skeleton);
        eroded.copyTo(binary);
        done = (countNonZero(binary) == 0);
    } while (!done);

    return skeleton;
}

void procesarImagen(const string& ruta, const string& etiqueta, ofstream& salidaCSV, bool usarSkeleton = false) {
    Mat img = imread(ruta, IMREAD_GRAYSCALE);
    if (img.empty()) return;

    Mat binaria;
    threshold(img, binaria, 0, 255, THRESH_BINARY | THRESH_OTSU);

    if (binaria.at<uchar>(0, 0) > 128) {
        bitwise_not(binaria, binaria);
    }

    morphologyEx(binaria, binaria, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    vector<vector<Point>> contornos;
    findContours(binaria, contornos, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (contornos.empty()) return;

    auto maxIt = max_element(contornos.begin(), contornos.end(),
                             [](const vector<Point>& a, const vector<Point>& b) {
                                 return contourArea(a) < contourArea(b);
                             });
    vector<Point> contorno = *maxIt;

    Mat figuraRellena = Mat::zeros(binaria.size(), CV_8UC1);
    drawContours(figuraRellena, vector<vector<Point>>{contorno}, -1, Scalar(255), FILLED);

    Mat procesada = usarSkeleton ? skeletonize(figuraRellena) : figuraRellena;

    Moments m = moments(contorno);
    double hu[7];
    HuMoments(m, hu);

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
    if (norma > 0.0) {
        for (double& v : fftMag) v /= norma;
    }

    salidaCSV << etiqueta;
    for (int i = 0; i < 7; ++i) {
        salidaCSV << "," << -1 * copysign(1.0, hu[i]) * log10(abs(hu[i]) + 1e-30);
    }
    for (double v : fftMag) {
        salidaCSV << "," << v;
    }
    salidaCSV << "\n";
}

int main() {
    string carpetaBase = ".";
    ofstream salida("momentos.csv");
    salida << "etiqueta,hu1,hu2,hu3,hu4,hu5,hu6,hu7,fft1,fft2,fft3,fft4,fft5,fft6,fft7,fft8,fft9,fft10\n";

    bool usarSkeleton = false;

    for (const auto& clase : {"circle", "square", "triangle"}) {
        string rutaClase = carpetaBase + "/" + clase;
        for (const auto& entrada : fs::directory_iterator(rutaClase)) {
            if (entrada.is_regular_file()) {
                procesarImagen(entrada.path().string(), clase, salida, usarSkeleton);
            }
        }
    }

    salida.close();
    cout << " Listo. Momentos guardados en momentos.csv\n";
    return 0;
}
