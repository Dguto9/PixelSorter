# PixelSorter
Stylistically sorts the pixels of an image in a way that retains some of the image's original form.

# How does it work?
The program runs the Canny Edge Detector upon the image to return a set of single pixel edge lines. The pixels of the image are then sorted based on input arguments, restarting at each edge line in order to maintain the original form of the image.

# Usage
## Build
This project relies on two other libraries as dependencies. When cloning the repository, be sure to ```--recurse-submodules``` or run ```git submodule update --remote```.
Once the repository is cloned:
```
cd PixelSorter
mkdir bin
cd bin
cmake ..
make
```
## Run
Command: PixelSorter [inputDir] [sortAxis] [sortBy] [sortDirection] [blurSize] [blurStdDev] [thresholdLow] [thresholdHigh]

Arguments:
- inputDir     : the .bmp file to sort
- sortAxis     : sort on the x ('x') or y ('y') axis? Takes: 'x', 'y' (default: x)
- sortBy       : sort based on the red ('r'), green ('g'), blue ('b') or greyscale ('v') pixel values? Takes: 'r', 'g', 'b', 'v' (default: v)
- sortDirection: sort in ascending (1) or descending (-1) order? Takes: -1, 1 (default: 1)
- blurSize     : size of the gaussian blur kernel. Takes any integer value. (default: 7)
- blurStdDev   : standard deviation of the gaussian blur. Takes any float value. (default: 1)
- thresholdLow : low threshold for hysteresis. Takes any float value. (default: 0.07)
- thresholdHigh: high threshold for hysteresis. Takes any float value. (default: 0.17)

Flags:
- -o: Set the output directory
- -h: print this message
- --save-magnitudes: save the magnitude output of the sobel operator
- --save-angles    : save the angle output of the sobel operator
- --save-nms       : save the output of the non-maximum suppression
- --save-hysteresis: save the output of hysteresis thresholding
