#include "struct.h"

BMPFile* readBMP(const char* path);
void writeBMP(const char* path, BMPFile* bmp);
void invert_colors(BMPFile *image);
void black_white_colors(BMPFile *image);
void median_filter(BMPFile *image, int window_size);
void gamma_correction(BMPFile *image, double gamma);