This repo contains a class (stitchImg) which initialises the QR detection process, seeks QR segment corners, and stitches segments together. The stitched image is then read using zbar's QR detection library. 

This class uses OpenCV's homographic transform to map a skewed perspective QR code onto a 2D plane, which can then be read by zbar.

To see an example of how this class is used, see extest.cpp source file.

To make this project, the makefile paths must be modified to the user's machine.
