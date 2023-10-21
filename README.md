# pixel-sorter
Stylistically sorts the pixels of an image in a way that retains some of the image's original form.

# How does it work?
The program runs canny edge detection upon the image to return a set of single pixel edge lines. The image is then sorted (either vertically or horizontally), restarting at each edge line in order to maintain the original form of the image.
