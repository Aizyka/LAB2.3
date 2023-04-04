#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defines.h"

int main() {
    char input[100];
    char output[100];
    int filter;
    printf("Input file: ");
    scanf("%99s", input);
    printf("Output file: ");
    scanf("%99s", output);
    printf("\nChoose Filter:\n"
           "1 - Negative\n"
           "2 - Grayscale\n"
           "3 - Gamma correction\n"
           "4 - Median\n"
           "Your choice: ");
    while(!scanf("%d", &filter) || filter < 1 || filter > 4) {
        printf("Invalid filter! Valid filter [1-4]\n");
        rewind(stdin);
        printf("Your choice: ");
    }
    BMPFile* bmp = readBMP(input);
    if(bmp == NULL)
    {
        printf("\nBMP Load Error!");
        return EXIT_FAILURE;
    }
    double gamma;
    int median;
    switch(filter) {
        case 1:
            invert_colors(bmp);
            break;
        case 2:
            black_white_colors(bmp);
            break;
        case 3:
            printf("Enter gamma value: ");
            while(!scanf("%lf", &gamma) || gamma < 0.0 || gamma > 1.0) {
                printf("Invalid gamma! Valid gamma [0.0-1.0]\n");
                rewind(stdin);
                printf("Enter gamma value: ");
            }
            gamma_correction(bmp,gamma);
            break;
        case 4:
            printf("Enter median value: ");
            while(!scanf("%d", &median) || median <= 0) {
                printf("Invalid median! Valid median >0\n");
                rewind(stdin);
                printf("Enter median value: ");
            }
            median_filter(bmp, median);
            break;
        default:
            break;
    }
    writeBMP(output, bmp);
    printf("Done.");

    for(int i = 0; i < bmp->header.height; i++) {
        free(bmp->pixels[i]);
    }
    free(bmp->pixels);
    free(bmp);
    return EXIT_SUCCESS;
}