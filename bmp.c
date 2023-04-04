#include "struct.h"
#include <stdlib.h>
#include <stdio.h>

BMPFile* readBMP(const char* path) {
    BMPFile* bmp = malloc(sizeof(BMPFile));
    FILE *inputFile = fopen(path, "rb");
    if (inputFile == NULL) {
        printf("Error: Failed to open input file.");
        return NULL;
    }
    fread(&bmp->header, sizeof(BMPHeader), 1, inputFile);
    int width = bmp->header.width;
    int height = bmp->header.height;
    int bpp = bmp->header.bitsPerPixel;
    printf("%d\n", bpp);
    if(bpp != 1 && bpp != 2 && bpp != 8 && bpp != 24) {
        printf("Error: This BMP not supported.");
        return NULL;
    }
    if(bmp->header.compression) {
        printf("Error: Compression not supported.");
        return NULL;
    }
    bmp->pixels = malloc(sizeof(Pixel *) * height);
    for (int i = 0; i < height; i++) {
        bmp->pixels[i] = malloc(sizeof(Pixel) * width);
    }
    if (bpp == 1 || bpp == 2 || bpp == 8) {
        int padding = (4 - ((width * bpp / 8) % 4)) % 4;
        fread(bmp->palette, sizeof(unsigned char), 1024, inputFile);
        for (int i = height - 1; i >= 0; i--) {
            for (int j = 0; j < width; j++) {
                unsigned char pixel;
                fread(&pixel, sizeof(unsigned char), 1, inputFile);
                bmp->pixels[i][j].red = pixel;
                bmp->pixels[i][j].green = pixel;
                bmp->pixels[i][j].blue = pixel;
            }
            fseek(inputFile, padding, SEEK_CUR);
        }
    }
    else if(bpp == 24) {
        uint32_t offset = bmp->header.offset;
        fseek(inputFile, offset, SEEK_SET);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                fread(&bmp->pixels[i][j], sizeof(Pixel), 1, inputFile);
            }
        }
    }
    fclose(inputFile);
    return bmp;
}

void writeBMP(const char* path, BMPFile* bmp) {
    FILE* outputFile = fopen(path, "wb");
    if (outputFile == NULL) {
        printf("Error: Failed to open output file.");
        return;
    }
    int width = bmp->header.width;
    int height = bmp->header.height;
    int bpp = bmp->header.bitsPerPixel;
    // Write header information to output file
    fwrite(&bmp->header, sizeof(BMPHeader), 1, outputFile);

    // Write palette if present
    if (bpp == 1 || bpp == 2 || bpp == 8) {
        int padding = (4 - ((width * bpp / 8) % 4)) % 4;
        fwrite(bmp->palette, sizeof(unsigned char), 1024, outputFile);
        // Write pixel data to output file
        for (int i = height - 1; i >= 0; i--) {
            for (int j = 0; j < width; j++) {
                unsigned  char pixel = bmp->pixels[i][j].red;
                fwrite(&pixel, sizeof(unsigned char), 1, outputFile);
            }
            for (int j = 0; j < padding; j++) {
                fputc(0x00, outputFile);
            }
        }
    }
    else if(bpp == 24) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                fwrite(&bmp->pixels[i][j], sizeof(Pixel), 1, outputFile);
            }
        }
    }



    // Close output file
    fclose(outputFile);
}

void invert_colors(BMPFile *image)
{
    int width = image->header.width;
    int height = image->header.height;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel = image->pixels[i][j];
            pixel.red = 255 - pixel.red;
            pixel.green = 255 - pixel.green;
            pixel.blue = 255 - pixel.blue;
            image->pixels[i][j] = pixel;
        }
    }
}

void black_white_colors(BMPFile *image)
{
    int width = image->header.width;
    int height = image->header.height;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel = image->pixels[i][j];
            uint8_t gray = (pixel.red + pixel.green + pixel.blue) / 3;
            pixel.red = gray;
            pixel.green = gray;
            pixel.blue = gray;
            image->pixels[i][j] = pixel;
        }
    }
}

// comparison function for qsort
int compare_uint8_t(const void *a, const void *b)
{
    const uint8_t *aa = (const uint8_t *)a;
    const uint8_t *bb = (const uint8_t *)b;
    return (*aa > *bb) - (*aa < *bb);
}

void median_filter(BMPFile *image, int window_size)
{
    // make sure window size is odd
    if (window_size % 2 == 0)
    {
        window_size++;
    }

    // calculate the number of padding pixels needed
    int pad = window_size / 2;
    int width = image->header.width;
    int height = image->header.height;
    // allocate memory for the padded image
    Pixel **padded_pixels = malloc(sizeof(Pixel *) * (height + 2 * pad));
    for (int i = 0; i < height + 2 * pad; i++)
    {
        padded_pixels[i] = malloc(sizeof(Pixel) * (width + 2 * pad));
    }

    // copy the image pixels to the padded image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            padded_pixels[i + pad][j + pad] = image->pixels[i][j];
        }
    }

    // pad the edges with border pixels
    for (int i = 0; i < pad; i++)
    {
        for (int j = 0; j < width + 2 * pad; j++)
        {
            padded_pixels[i][j] = padded_pixels[pad][j];
            padded_pixels[height + 2 * pad - i - 1][j] = padded_pixels[height + pad - 1][j];
        }
    }
    for (int i = 0; i < height + 2 * pad; i++)
    {
        for (int j = 0; j < pad; j++)
        {
            padded_pixels[i][j] = padded_pixels[i][pad];
            padded_pixels[i][width + 2 * pad - j - 1] = padded_pixels[i][width + pad - 1];
        }
    }

    // apply median filter to each pixel
    for (int i = pad; i < height + pad; i++)
    {
        for (int j = pad; j < width + pad; j++)
        {
            // get values in the window
            uint8_t r[window_size * window_size], g[window_size * window_size], b[window_size * window_size];
            int k = 0;
            for (int ii = -pad; ii <= pad; ii++)
            {
                for (int jj = -pad; jj <= pad; jj++)
                {
                    r[k] = padded_pixels[i + ii][j + jj].red;
                    g[k] = padded_pixels[i + ii][j + jj].green;
                    b[k] = padded_pixels[i + ii][j + jj].blue;
                    k++;
                }
            }

            // sort the values
            qsort(r, window_size * window_size, sizeof(uint8_t), compare_uint8_t);
            qsort(g, window_size * window_size, sizeof(uint8_t), compare_uint8_t);
            qsort(b, window_size * window_size, sizeof(uint8_t), compare_uint8_t);

            // set the pixel value to the median value
            image->pixels[i - pad][j - pad].red = r[window_size * window_size / 2];
            image->pixels[i - pad][j - pad].green = g[window_size * window_size / 2];
            image->pixels[i - pad][j - pad].blue = b[window_size * window_size / 2];
        }
    }

    // free memory
    for (int i = 0; i < height + 2 * pad; i++)
    {
        free(padded_pixels[i]);
    }
    free(padded_pixels);
}

double pow(double base, double exponent)
{
    if (exponent == 0.0)
    {
        return 1.0;
    }
    else if (exponent == 1.0)
    {
        return base;
    }
    else if (exponent < 0.0)
    {
        return 1.0 / pow(base, -exponent);
    }
    else
    {
        double result = base;
        for (int i = 1; i < exponent; i++)
        {
            result *= base;
        }
        return result;
    }
}

void gamma_correction(BMPFile *image, double gamma)
{
    int width = image->header.width;
    int height = image->header.height;
    double inv_gamma = 1.0 / gamma;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            uint8_t r = pow(image->pixels[i][j].red / 255.0, inv_gamma) * 255.0;
            uint8_t g = pow(image->pixels[i][j].green / 255.0, inv_gamma) * 255.0;
            uint8_t b = pow(image->pixels[i][j].blue / 255.0, inv_gamma) * 255.0;
            image->pixels[i][j].red = r;
            image->pixels[i][j].green = g;
            image->pixels[i][j].blue = b;
        }
    }
}