#include "Matrix.h"
#include "Image.h"
#include "ImageProcessingLibrary/Utils.h"
#include <iostream>
#include <math.h>
#include <string>

std::string inputDir;
std::string outputDir;
char sortAxis = 'x';
char sortBy = 'v';
int sortDir = 1;
int blurSize = 7;
float blurStdDev = 1;
float thresholdLow = 0.07;
float thresholdHigh = 0.17;
bool saveMagnitudes = 0;
bool saveAngles = 0;
bool saveNMS = 0;
bool saveHysteresis = 0;

void recursiveThreshold(Matrix* edges, int column, int row, int reachX, int reachY);
void sortImageX(Image* result, Matrix* sortBy, Matrix* edges);
void sortImageY(Image* result, Matrix* sortBy, Matrix* edges);
void help();

int main(int argc, char* argv[]) {
	if (argc == 1) {
		help();
		return 0;
	}
	int argNum = 0;
	for (int i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "-o", 2)) {
			if (argv[i][2]) {
				outputDir = argv[i] + 2;
			}
			else {
				outputDir = argv[i + 1];
				i++;
			}
		}
		else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			help();
			return 0;
		}
		else if (!strcmp(argv[i], "--save-magnitudes")) {
			saveMagnitudes = 1;
		}
		else if (!strcmp(argv[i], "--save-angles")) {
			saveAngles = 1;
		}
		else if (!strcmp(argv[i], "--save-nms")) {
			saveNMS = 1;
		}
		else if (!strcmp(argv[i], "--save-hysteresis")) {
			saveHysteresis = 1;
		}
		else {
			switch (argNum) {
			case 0:
				inputDir = argv[i];
				break;
			case 1:
				sortAxis = argv[i][0];
				break;
			case 2:
				sortBy = argv[i][0];
				break;
			case 3:
				sortDir = atoi(argv[i]);
				break;
			case 4:
				blurSize = atoi(argv[i]);
				break;
			case 5:
				blurStdDev = atof(argv[i]);
				break;
			case 6:
				thresholdLow = atof(argv[i]);
				break;
			case 7:
				thresholdHigh = atof(argv[i]);
				break;
			default:
				break;
			}
			argNum++;
		}
	}
	if (outputDir.empty()) {
		outputDir = inputDir.insert(inputDir.length() - 4, "_Sorted");
	}
	Image image(inputDir.c_str());
	Matrix imageGrey = (image.pixels[0]+image.pixels[1]+image.pixels[2]) * (1.0f/3.0f);
	Matrix k_sobelx(3, 3);
	Matrix k_sobely(3, 3);
	Matrix k_gaussian(blurSize, blurSize);
	float total_gaussian = 0;
	for (int i = 0; i < k_gaussian.rows; i++) {
		for (int j = 0; j < k_gaussian.columns; j++) {
			k_gaussian.data[j + (i*k_gaussian.columns)] = imgProcUtils::gaussian2D(j - ((k_gaussian.columns - 1) / 2.0f), i - ((k_gaussian.rows - 1) / 2.0f), blurStdDev);
			total_gaussian += k_gaussian.data[j + (i * k_gaussian.columns)];
		}
	}
	k_gaussian = k_gaussian * (1.0f / total_gaussian);
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

	for (int i = 0; i < image.height * image.width; i++) {
		nms.data[i] = nms.data[i] < thresholdLow ? 0 : (nms.data[i] > thresholdHigh ? 1 : 0.5);
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

	Matrix sorted(image.height, image.width);
	if (sortBy == 'r') {
		sorted = image.pixels[2];
	}
	else if (sortBy == 'g') {
		sorted = image.pixels[1];
	}
	else if (sortBy == 'b') {
		sorted = image.pixels[0];
	}
	else {
		sorted = imageGrey;
	}
	Image sort = image;

	if (sortAxis == 'y') {
		sortImageY(&sort, &sorted, &hysteresis);
	}
	else {
		sortImageX(&sort, &sorted, &hysteresis);
	}

	sort.saveToBMP(outputDir.c_str());
	if (saveHysteresis) {
		Image hyst = hysteresis;
		hyst.saveToBMP(outputDir.insert(outputDir.length() - 4, "_Hysteresis").c_str());
	}
	if (saveNMS) {
		Image nonmax = nms;
		nonmax.saveToBMP(outputDir.insert(outputDir.length() - 4, "_NMS").c_str());
	}
	if (saveMagnitudes) {
		Image magn = magnitudes;
		magn.saveToBMP(outputDir.insert(outputDir.length() - 4, "_Magnitudes").c_str());
	}
	if (saveAngles) {
		Image anglR = anglesRound;
		anglR.saveToBMP(outputDir.insert(outputDir.length() - 4, "_Angles").c_str());
	}
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

void sortImageX(Image* result, Matrix* sortBy, Matrix* edges) {
	int segmentStart = 0;
	for (int i = 0; i < result->height; i++) {
		for (int j = 0; j < result->width; j++) {
			if (edges->data[j + (i * result->width)] == 1 || j == result->width - 1) {
				for (int k = 1; k <= j - segmentStart; k++) {
					int swap = false;
					for (int l = segmentStart; l <= (j - k); l++) {
						if (sortDir * sortBy->data[l + (i * result->width)] > sortDir * sortBy->data[(l + 1) + (i * result->width)]) {
							std::swap(sortBy->data[l + (i * result->width)], sortBy->data[(l + 1) + (i * result->width)]);
							for (int m = 0; m < 3; m++) {
								std::swap(result->pixels[m].data[l + (i * result->width)], result->pixels[m].data[(l + 1) + (i * result->width)]);
							}
							swap = true;
						}
					}
					if (!swap) {
						break;
					}
				}
				segmentStart = j;
			}
		}
		segmentStart = 0;
	}
	return;
}

void sortImageY(Image* result, Matrix* sortBy, Matrix* edges) {
	int segmentStart = 0;
	for (int i = 0; i < result->width; i++) {
		for (int j = 0; j < result->height; j++) {
			if (edges->data[i + (j * result->width)] == 1 || j == result->height - 1) {
				for (int k = 1; k <= j - segmentStart; k++) {
					int swap = false;
					for (int l = segmentStart; l <= (j - k); l++) {
						if (sortDir * sortBy->data[i + (l * result->width)] > sortDir * sortBy->data[i + ((l + 1) * result->width)]) {
							std::swap(sortBy->data[i + (l * result->width)], sortBy->data[i + ((l + 1) * result->width)]);
							for (int m = 0; m < 3; m++) {
								std::swap(result->pixels[m].data[i + (l * result->width)], result->pixels[m].data[i + ((l + 1) * result->width)]);
							}
							swap = true;
						}
					}
					if (!swap) {
						break;
					}
				}
				segmentStart = j;
			}
		}
		segmentStart = 0;
	}
	return;
}

void help() {
	std::cout << "PixelSorter - Dillon Gutowski (Dguto9) - https://github.com/dguto9/pixel-sorter\n\n" <<	
		"PixelSorter [inputDir] [sortAxis] [sortBy] [sortDirection] [blurSize] [blurStdDev] [thresholdLow] [thresholdHigh]\n\n" <<
		"Arguments:\n" <<
		"\tinputDir     : the .bmp file to sort\n" <<
		"\tsortAxis     : sort on the x ('x') or y ('y') axis? Takes: 'x', 'y' (default: x)\n" <<
		"\tsortBy       : sort based on the red ('r'), green ('g'), blue ('b') or greyscale ('v') pixel values? Takes: 'r', 'g', 'b', 'v' (default: v)\n" <<
		"\tsortDirection: sort in ascending (1) or descending (-1) order? Takes: -1, 1 (default: 1)\n" <<
		"\tblurSize     : size of the gaussian blur kernel. Takes any integer value. (default: 7)\n" <<
		"\tblurStdDev   : standard deviation of the gaussian blur. Takes any float value. (default: 1)\n" <<
		"\tthresholdLow : low threshold for hysteresis. Takes any float value. (default: 0.07)\n" <<
		"\tthresholdHigh: high threshold for hysteresis. Takes any float value. (default: 0.17)\n\n" <<
		"Flags:\n" <<
		"\t-o: Set the output directory\n" <<
		"\t-h: print this message\n" <<
		"\t--save-magnitudes: save the magnitude output of the sobel operator\n" <<
		"\t--save-angles    : save the angle output of the sobel operator\n" <<
		"\t--save-nms       : save the output of the non-maximum suppression\n" <<
		"\t--save-hysteresis: save the output of hysteresis thresholding\n" << std::endl;
	return;
}
