#include "Matrix.h"
#include "Image.h"
#include "ImageProcessing/Utils.h"
#include <iostream>
#include <math.h>

void recursiveThreshold(Matrix* edges, int column, int row, int reachX, int reachY);

int main() {
	Image image("C:/Users/dillo/OneDrive/Documents/Projects/MatrixModule/PixelSorter/Images/ethan.bmp");
	Matrix imageGrey = (image.pixels[0]+image.pixels[1]+image.pixels[2]) * (1.0f/3.0f);
	Matrix k_sobelx(3, 3);
	Matrix k_sobely(3, 3);
	Matrix k_gaussian(7, 7);
	float total_gaussian = 0;
	for (int i = 0; i < k_gaussian.rows; i++) {
		for (int j = 0; j < k_gaussian.columns; j++) {
			k_gaussian.data[j + (i*k_gaussian.columns)] = imgProcUtils::gaussian2D(j - ((k_gaussian.columns - 1) / 2.0f), i - ((k_gaussian.rows - 1) / 2.0f), 1);
			total_gaussian += k_gaussian.data[j + (i * k_gaussian.columns)];
		}
	}
	k_gaussian = k_gaussian * (1.0f / total_gaussian);
	k_gaussian.print();
	float sobelxvals[] = { 1, 0, -1, 2, 0, -2, 1, 0, -1 };
	float sobelyvals[] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
	memcpy(k_sobelx.data, sobelxvals, sizeof(sobelxvals));
	memcpy(k_sobely.data, sobelyvals, sizeof(sobelyvals));
	imageGrey = imageGrey.convolve(k_gaussian);
	Matrix sobelX = imageGrey.convolve(k_sobelx);
	Matrix sobelY = imageGrey.convolve(k_sobely);
	Matrix magnitudes(image.height, image.width);
	Matrix angles(image.height, image.width);
	Matrix anglesRound(image.height, image.width);
	Matrix nms(image.height, image.width);
	for (int i = 0; i < image.width * image.height; i++) {
		magnitudes.data[i] = sqrt(pow(sobelX.data[i], 2) + pow(sobelY.data[i], 2));
		angles.data[i] = atan2(sobelY.data[i], sobelX.data[i]);
		angles.data[i] += (angles.data[i] < 0) ? imgProcUtils::pi : 0;
	}
	nms = magnitudes;
	
	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++) {
			if (i == 0 || i == (image.height - 1) || j == 0 || j == (image.width - 1)) {
				nms.data[j + (i * image.width)] = 0.0;
				continue;
			}
			if (angles.data[j + (i * image.width)] < (imgProcUtils::pi / 8.0f) || angles.data[j + (i * image.width)] >= (7.0f * (imgProcUtils::pi / 8.0f))) {
				if (magnitudes.data[j + (i * image.width)] < magnitudes.data[(j + 1) + ((i)*image.width)] || magnitudes.data[j + (i * image.width)] < magnitudes.data[(j - 1) + ((i)*image.width)]) {
					nms.data[j + (i * image.width)] = 0.0;
				}
				anglesRound.data[j + (i * image.width)] = 0.25;
			}
			else if (angles.data[j + (i * image.width)] < (3.0f * (imgProcUtils::pi / 8.0f))) {
				if (magnitudes.data[j + (i * image.width)] < magnitudes.data[(j + 1) + ((i + 1)*image.width)] || magnitudes.data[j + (i * image.width)] < magnitudes.data[(j - 1) + ((i - 1)*image.width)]) {
					nms.data[j + (i * image.width)] = 0.0;
				}
				anglesRound.data[j + (i * image.width)] = 0.5;
			}
			else if (angles.data[j + (i * image.width)] < (5.0f * (imgProcUtils::pi / 8.0f))) {
				if (magnitudes.data[j + (i * image.width)] < magnitudes.data[(j) + ((i + 1) * image.width)] || magnitudes.data[j + (i * image.width)] < magnitudes.data[(j) + ((i - 1) * image.width)]) {
					nms.data[j + (i * image.width)] = 0.0;
				}
				anglesRound.data[j + (i * image.width)] = 0.75;
			}
			else if (angles.data[j + (i * image.width)] < (7.0f * (imgProcUtils::pi / 8.0f))) {
				if (magnitudes.data[j + (i * image.width)] < magnitudes.data[(j + 1) + ((i - 1) * image.width)] || magnitudes.data[j + (i * image.width)] < magnitudes.data[(j - 1) + ((i + 1) * image.width)]) {
					nms.data[j + (i * image.width)] = 0.0;
				}
				anglesRound.data[j + (i * image.width)] = 1;
			}
			else {
				std::cout << angles.data[j + (i * image.width)] / imgProcUtils::pi << std::endl;
			}
		}
	}
	float lowThreshold = 0.07;
	float highThreshold = 0.17;
	for (int i = 0; i < image.height * image.width; i++) {
		nms.data[i] = nms.data[i] < lowThreshold ? 0 : (nms.data[i] > highThreshold ? 1 : 0.5);
	}

	Matrix hysteresis = nms;
	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++) {
			recursiveThreshold(&hysteresis, j, i, 1, 1);
		}
	}

	for (int i = 0; i < image.height * image.width; i++) {
		magnitudes.data[i] = imgProcUtils::clamp(magnitudes.data[i], 0, 1);
		hysteresis.data[i] = hysteresis.data[i] < 1 ? 0 : 1;
	}

	Image hyst = hysteresis;
	Image nonmax = nms;
	Image magn = magnitudes;
	Image anglR = anglesRound;
	hyst.saveToBMP("C:/Users/dillo/OneDrive/Documents/Projects/MatrixModule/PixelSorter/Images/Output/ethanHysteresis.bmp");
	nonmax.saveToBMP("C:/Users/dillo/OneDrive/Documents/Projects/MatrixModule/PixelSorter/Images/Output/ethanNMS.bmp");
	magn.saveToBMP("C:/Users/dillo/OneDrive/Documents/Projects/MatrixModule/PixelSorter/Images/Output/ethanMagnitudes.bmp");
	anglR.saveToBMP("C:/Users/dillo/OneDrive/Documents/Projects/MatrixModule/PixelSorter/Images/Output/ethanAngles.bmp");
	return 0;
}

void recursiveThreshold(Matrix* edges, int column, int row, int reachX, int reachY) {
	if (edges->data[column + (row * edges->columns)] == 1) {
		for (int i = -reachY; i <= reachY; i++) {
			for (int j = -reachX; j <= reachX; j++) {
				if (column + j >= 0 && column + j < edges->columns && row + i >= 0 && row + i < edges->rows) {
					if (edges->data[(column + j) + ((row + i) * edges->columns)] == 0.5) {
						edges->data[(column + j) + ((row + i) * edges->columns)] = 1;
						recursiveThreshold(edges, column + j, row + i, reachX, reachY);
					}
				}
			}
		}
	}
	return;
}
