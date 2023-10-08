#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;

/*  
    Grading:
    80% if your program works at least for images with the same width and height.
    20% if your program works with arbitrary resolutions (bilinear interpolation).
    Due: Friday, April 21 11:59pm
*/

/*STRUCTS FROM PROJECT 1 PDF*/
typedef struct tagBITMAPFILEHEADER {
    WORD bfType; /*specifies the file type*/
    DWORD bfSize; /*specifies the size in bytes of the bitmap file*/
    WORD bfReserved1; /*reserved; must be 0*/
    WORD bfReserved2; /*reserved; must be 0*/
    DWORD bfOffBits; /*species the offset in bytes from the bitmapfileheader to the bitmap bits*/
}tagBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize; /*specifies the number of bytes required by the struct*/
    LONG biWidth; /*specifies width in pixels*/
    LONG biHeight; /*species height in pixels*/
    WORD biPlanes; /*specifies the number of color planes, must be 1*/
    WORD biBitCount; /*specifies the number of bit per pixel*/
    DWORD biCompression;/*specifies the type of compression*/
    DWORD biSizeImage; /*size of image in bytes*/
    LONG biXPelsPerMeter; /*number of pixels per meter in x axis*/
    LONG biYPelsPerMeter; /*number of pixels per meter in y axis*/
    DWORD biClrUsed; /*number of colors used by th ebitmap*/
    DWORD biClrImportant; /*number of colors that are important*/
}tagBITMAPINFOHEADER;

void checkRatio(char *str, float *ratio, FILE *in1, FILE *in2, FILE *out); /*sets ratio if valid*/

BYTE get_red(BYTE *imagedata, float x, float y, int imagewidth, int imageheight);
BYTE get_green(BYTE *imagedata, float x, float y, int imagewidth, int imageheight);
BYTE get_blue(BYTE *imagedata, float x, float y, int imagewidth, int imageheight);

int main(int argc, char *argv[]){
    int i, j, first;

    float ratio;
    BYTE padding;
/*formatting final*/
    tagBITMAPFILEHEADER *large_file;
    tagBITMAPINFOHEADER *large_info, *small_info;
    BYTE *large_pixel_data, *small_pixel_data;
/*Structs for file 1*/
    tagBITMAPFILEHEADER f1_file;
    tagBITMAPINFOHEADER f1_info;
    BYTE *pixel_data_1, *temp1;
/*Structs for file 2*/
    tagBITMAPFILEHEADER f2_file;
    tagBITMAPINFOHEADER f2_info;
    BYTE *pixel_data_2, *temp2;

/*intermediate values for color*/
    int x0, y0, x1, y1, x2, y2;
    float dx, dy, scale_x, scale_y;

    BYTE red_left_upper, red_right_upper, red_left_lower, red_right_lower;
    float red_left, red_right, red_res, large_red;

    BYTE green_left_upper, green_right_upper, green_left_lower, green_right_lower;
    float green_left, green_right, green_res, large_green;

    BYTE blue_left_upper, blue_right_upper, blue_left_lower, blue_right_lower;
    float blue_left, blue_right, blue_res, large_blue;

    BYTE red, green, blue;

    BYTE p = 0;

    char *extension;

    FILE *bmp1;
    FILE *bmp2;
    FILE *out_file;

    if (argc != 5){
        perror("Insufficient comand line arguments. Exiting.\n");
        exit(EXIT_FAILURE);
    }

/*Read image 1 (file headers and pixel data)*/
    bmp1 = fopen(argv[1], "rb");
/*Read image 2 (file headers and pixel data)*/
    bmp2 = fopen(argv[2], "rb");
/*Out file*/
    out_file = fopen(argv[4], "wb"); 
    /*FILE *temp_file = fopen(argv[5], "wb"); FOR COPYING OVER SECOND IMG*/

/*check argc value -> error if not enough arguments passed through command line*/
    /*CHECK IF BMP EXT: first letters of header should be BM*/
    if (bmp1 == NULL){
        if (bmp2 != NULL){
            fclose(bmp2);
        }

        if (out_file != NULL){
            fclose(out_file);
        }
        bmp1 = NULL;
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        perror("File 1 not valid. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    if(bmp2 == NULL){
        if (bmp1 != NULL){
            fclose(bmp2);
        }

        if (out_file != NULL){
            fclose(out_file);
        }
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        perror("File 2 not valid. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    if (out_file == NULL){ /*check extension*/
        perror("Error reading outfile. Exiting.\n");
        if (bmp1 != NULL){
            fclose(bmp2);
        }

        if (bmp2 != NULL){
            fclose(out_file);
        }
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }
    
    extension = strrchr(argv[4], '.');
    if (!extension){
        perror("Outfile does not have .bmp extension. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }
    else if (strcmp(extension+1, "bmp") != 0){
        perror("Outfile does not have .bmp extension. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

/*check ratio validity, and assign value to ratio variable*/
    checkRatio(argv[3], &ratio, bmp1, bmp2, out_file);

/*Read in first file*/    
    if (fread(&(f1_file.bfType), sizeof(WORD), 1, bmp1) != 1){ 
        perror("Error reading file.bfType. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }
    /*check if bit map file*/
    if (f1_file.bfType != 0x4D42){
        perror("File 1 not a bit map file. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_file.bfSize), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.bfSize. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }
    
    if (fread(&(f1_file.bfReserved1), sizeof(WORD), 1, bmp1) != 1){ 
        perror("Error reading file.bfReserved1. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_file.bfReserved2), sizeof(WORD), 1, bmp1) != 1){ 
        perror("Error reading file.bfReserved2. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_file.bfOffBits), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.bfOffBites. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }
    

    /*Info Header*/
    if (fread(&(f1_info.biSize), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.biSize. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biWidth), sizeof(LONG), 1, bmp1) != 1){ 
        perror("Error reading file.biWidth. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biHeight), sizeof(LONG), 1, bmp1) != 1){ 
        perror("Error reading file.biHeigth. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biPlanes), sizeof(WORD), 1, bmp1) != 1){ 
        perror("Error reading file.biPlanes. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biBitCount), sizeof(WORD), 1, bmp1) != 1){ 
        perror("Error reading file.biBitCount. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biCompression), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.biCompression. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biSizeImage), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.biSizeImage. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biXPelsPerMeter), sizeof(LONG), 1, bmp1) != 1){ 
        perror("Error reading file.biXPelsPerMeter. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biYPelsPerMeter), sizeof(LONG), 1, bmp1) != 1){ 
        perror("Error reading file.biYPelsPerMeter. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biClrUsed), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.biClrUsed. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f1_info.biClrImportant), sizeof(DWORD), 1, bmp1) != 1){ 
        perror("Error reading file.biClrImportant. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    /*process pixel data*/
    padding = (4 - ((f1_info.biWidth*3) % 4)) % 4;

    pixel_data_1 = (BYTE*)malloc(3*f1_info.biWidth*f1_info.biWidth);/*allocate space*/
    temp1 = pixel_data_1; /*to reset before free()*/

    if (pixel_data_1 == NULL){
        perror("Memory allocation error with pixel data. Exiting.\n");
        fclose(bmp1);
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        bmp1 = NULL;
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < f1_info.biHeight; i++){
        for(j = 0; j < f1_info.biWidth; j++){
            /*WRITE CONDITIONAL CHECKS FOR FREAD*/
            if (fread(pixel_data_1, sizeof(BYTE), 1, bmp1) != 1){
                pixel_data_1 = temp1;
                free(pixel_data_1);
                temp1 = NULL;
                perror("Error reading pixel data in file 1. Exiting.\n\n");
                fclose(bmp1);
                fclose(bmp2);
                fclose(out_file);
                bmp2 = NULL;
                out_file = NULL;
                bmp1 = NULL;
                exit(EXIT_FAILURE);
            }

            if (fread(pixel_data_1 + 1, sizeof(BYTE), 1, bmp1) != 1){
                pixel_data_1 = temp1;
                free(pixel_data_1);
                temp1 = NULL;
                fclose(bmp1);
                fclose(bmp2);
                fclose(out_file);
                bmp2 = NULL;
                out_file = NULL;
                bmp1 = NULL;
                perror("Error reading pixel data in file 1. Exiting.\n\n");
                exit(EXIT_FAILURE);
            }

            if (fread(pixel_data_1 + 2, sizeof(BYTE), 1, bmp1) != 1){
                pixel_data_1 = temp1;
                free(pixel_data_1);
                temp1 = NULL;
                fclose(bmp1);
                fclose(bmp2);
                fclose(out_file);
                bmp2 = NULL;
                out_file = NULL;
                bmp1 = NULL;
                perror("Error reading pixel data in file 1. Exiting.\n\n");
                exit(EXIT_FAILURE);
            }

            pixel_data_1 += 3;
        }
        /*skip padding*/
        if(fseek(bmp1, padding, SEEK_CUR) != 0){
            pixel_data_1 = temp1;
            free(pixel_data_1);
            temp1 = NULL;
            fclose(bmp1);
            fclose(bmp2);
            fclose(out_file);
            bmp2 = NULL;
            out_file = NULL;
            bmp1 = NULL;
            perror("Was not able to skip padding. Exiting\n");
            exit(EXIT_FAILURE);
        }
    }
    pixel_data_1 = temp1; /*reset*/
    
    fclose(bmp1);
    bmp1 = NULL;

/*Read in second file*/        
    if (fread(&(f2_file.bfType), sizeof(WORD), 1, bmp2) != 1){ 
        perror("Error reading file.bfType. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    /*check if bit map file*/
    if (f2_file.bfType != 0x4D42){
        perror("File 1 not a bit map file. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_file.bfSize), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.bfSize. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    
    if (fread(&(f2_file.bfReserved1), sizeof(WORD), 1, bmp2) != 1){ 
        perror("Error reading file.bfReserved1. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_file.bfReserved2), sizeof(WORD), 1, bmp2) != 1){ 
        perror("Error reading file.bfReserved2. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_file.bfOffBits), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.bfOffBites. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    

    /*Info Header*/
    if (fread(&(f2_info.biSize), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.biSize. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biWidth), sizeof(LONG), 1, bmp2) != 1){ 
        perror("Error reading file.biWidth. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biHeight), sizeof(LONG), 1, bmp2) != 1){ 
        perror("Error reading file.biHeigth. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biPlanes), sizeof(WORD), 1, bmp2) != 1){ 
        perror("Error reading file.biPlanes. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biBitCount), sizeof(WORD), 1, bmp2) != 1){ 
        perror("Error reading file.biBitCount. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biCompression), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.biCompression. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biSizeImage), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.biSizeImage. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biXPelsPerMeter), sizeof(LONG), 1, bmp2) != 1){ 
        perror("Error reading file.biXPelsPerMeter. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biYPelsPerMeter), sizeof(LONG), 1, bmp2) != 1){ 
        perror("Error reading file.biYPelsPerMeter. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biClrUsed), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.biClrUsed. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        fclose(out_file);
        bmp2 = NULL;
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fread(&(f2_info.biClrImportant), sizeof(DWORD), 1, bmp2) != 1){ 
        perror("Error reading file.biClrImportant. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        bmp2 = NULL;
        fclose(out_file);
        exit(EXIT_FAILURE);
    }
    
    /*process pixel data*/
    padding = (4 - ((f2_info.biWidth*3) % 4)) % 4;

    pixel_data_2 = (BYTE*)malloc(3*f2_info.biWidth*f2_info.biWidth);/*allocate space*/
    temp2 = pixel_data_2; /*to reset before free()*/

    if (pixel_data_2 == NULL){
        perror("Memory allocation error with pixel data. Exiting.\n");
        free(pixel_data_1);
        temp1 = NULL;
        fclose(bmp2);
        bmp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < f2_info.biHeight; i++){
        for(j = 0; j < f2_info.biWidth; j++){
            /*WRITE CONDITIONAL CHECKS FOR FREAD*/

            if (fread(pixel_data_2, sizeof(BYTE), 1, bmp2) != 1){
                free(pixel_data_1);
                temp1 = NULL;
                pixel_data_2 = temp2;
                free(pixel_data_2);
                temp2 = NULL;
                fclose(bmp2);
                bmp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                perror("Error reading pixel data in file 2. Exiting.\n\n");
                exit(EXIT_FAILURE);
            }

            if (fread(pixel_data_2 + 1, sizeof(BYTE), 1, bmp2) != 1){
                free(pixel_data_1);
                pixel_data_2 = temp2;
                free(pixel_data_2);
                temp1 = NULL;
                temp2 = NULL;
                fclose(bmp2);
                bmp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                perror("Error reading pixel data in file 1. Exiting.\n\n");
                exit(EXIT_FAILURE);
            }
            
            if (fread(pixel_data_2 + 2, sizeof(BYTE), 1, bmp2) != 1){
                free(pixel_data_1);
                pixel_data_2 = temp2;
                free(pixel_data_2);
                temp1 = NULL;
                temp2 = NULL;
                fclose(bmp2);
                bmp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                perror("Error reading pixel data in file 1. Exiting.\n\n");
                exit(EXIT_FAILURE);
            }

            pixel_data_2 += 3;
        }
        /*skip padding*/
        if(fseek(bmp2, padding, SEEK_CUR) != 0){
            free(pixel_data_1);
            pixel_data_2 = temp2;
            free(pixel_data_2);
            temp1 = NULL;
            temp2 = NULL;
            fclose(bmp2);
            bmp2 = NULL;
            fclose(out_file);
            out_file = NULL;
            perror("Was not able to skip padding. Exiting\n");
            exit(EXIT_FAILURE);
        }
    }

    pixel_data_2 = temp2; /*reset*/

/*    printf("p1: %p\n", (void*)&pixel_data_1);
    printf("p2: %p\n", (void*)&pixel_data_2);*/
    
    fclose(bmp2);
    bmp2 = NULL;

/*see which file is larger (width)*/
    if (f1_info.biWidth > f2_info.biWidth){
        large_file = &f1_file;
        large_info = &f1_info;
        large_pixel_data = pixel_data_1;
        small_info = &f2_info;
        small_pixel_data = pixel_data_2;

        first = 1; /*1 if larger file was first input*/
    }
    else{
        large_file = &f2_file;
        large_info = &f2_info;
        large_pixel_data = pixel_data_2;

        small_info = &f1_info;
        small_pixel_data = pixel_data_1;

        first = 0; /*0 if smaller file was first input*/
    }

    if (fwrite(&(large_file->bfType), sizeof(WORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }

    if (fwrite(&(large_file->bfSize), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_file->bfReserved1), sizeof(WORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_file->bfReserved2), sizeof(WORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_file->bfOffBits), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biSize), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biWidth), sizeof(LONG), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biHeight), sizeof(LONG), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biPlanes), sizeof(WORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biBitCount), sizeof(WORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biCompression), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biSizeImage), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biXPelsPerMeter), sizeof(LONG), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biYPelsPerMeter), sizeof(LONG), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biClrUsed), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    if (fwrite(&(large_info->biClrImportant), sizeof(DWORD), 1, out_file) != 1){
        perror("Error writing bfType to outfile. Exiting.\n");
        free(pixel_data_1);
        free(pixel_data_2);
        temp1 = NULL;
        temp2 = NULL;
        fclose(out_file);
        out_file = NULL;
        exit(EXIT_FAILURE);
    }
    
    padding = (4 - ((large_info->biWidth*3) % 4)) % 4;

    /*scale x and y*/
    scale_x = (float)(small_info->biWidth) / (float)(large_info->biWidth);
    scale_y = (float)(small_info->biHeight) / (float)(large_info->biHeight);

    for (i = 0; i < large_info->biHeight; i++){
        for (j = 0; j < large_info->biWidth; j++){
     
            x0 = j * scale_x;
            y0 = i * scale_y;
            
            x1 = (int)floor(x0);
            y1 = (int)floor(y0);
        
            x2 = (int)ceil(x0);
            y2 = (int)ceil(y0);

            dx = x0 - x1;
            dy = y0 - y1;
        
            red_left_upper = get_red(small_pixel_data, x1, y2, small_info->biWidth, small_info->biHeight);
            red_right_upper = get_red(small_pixel_data, x2, y2, small_info->biWidth, small_info->biHeight);
            red_left_lower = get_red(small_pixel_data, x1, y1, small_info->biWidth, small_info->biHeight);
            red_right_lower = get_red(small_pixel_data, x2, y1, small_info->biWidth, small_info->biHeight);

            red_left = red_left_upper * (1 - dy) + red_left_lower * dy;
            red_right = red_right_upper * (1 - dy) + red_right_lower * dy;
            red_res = red_left * (1 - dx) + red_right * dx;

            green_left_upper = get_green(small_pixel_data, x1, y2, small_info->biWidth, small_info->biHeight);
            green_right_upper = get_green(small_pixel_data, x2, y2, small_info->biWidth, small_info->biHeight);
            green_left_lower = get_green(small_pixel_data, x1, y1, small_info->biWidth, small_info->biHeight);
            green_right_lower = get_green(small_pixel_data, x2, y1, small_info->biWidth, small_info->biHeight);

            green_left = green_left_upper * (1 - dy) + green_left_lower * dy;
            green_right = green_right_upper * (1 - dy) + green_right_lower * dy;
            green_res = green_left * (1 - dx) + green_right * dx;

            blue_left_upper = get_blue(small_pixel_data, x1, y2, small_info->biWidth, small_info->biHeight);
            blue_right_upper = get_blue(small_pixel_data, x2, y2, small_info->biWidth, small_info->biHeight);
            blue_left_lower = get_blue(small_pixel_data, x1, y1, small_info->biWidth, small_info->biHeight);
            blue_right_lower = get_blue(small_pixel_data, x2, y1, small_info->biWidth, small_info->biHeight);

            blue_left = blue_left_upper * (1 - dy) + blue_left_lower * dy;
            blue_right = blue_right_upper * (1 - dy) + blue_right_lower * dy;
            blue_res = blue_left * (1 - dx) + blue_right * dx;

        
            large_red = *large_pixel_data;
            large_green = *(large_pixel_data + 1);
            large_blue = *(large_pixel_data + 2);
        

            if (first == 1){              
                red = (BYTE)((large_red * ratio) + (red_res * (1 - ratio)));
                green = (BYTE)((large_green * ratio) + (green_res * (1 - ratio)));
                blue = (BYTE)((large_blue * ratio) + (blue_res * (1 - ratio)));
            }
            else {
                red = (BYTE)((red_res * ratio) + (large_red * (1 - ratio)));
                green = (BYTE)((green_res * ratio) + (large_green * (1 - ratio)));
                blue = (BYTE)((blue_res * ratio) + (large_blue * (1 - ratio)));
            }

            large_pixel_data+=3;
            
            if (fwrite(&red, sizeof(BYTE), 1, out_file) != 1){
                perror("Error writing pixel data to out file. Exiting.\n");
                pixel_data_1 = temp1;
                pixel_data_2 = temp2;
                free(pixel_data_1);
                free(pixel_data_2);
                temp1 = NULL;
                temp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                exit(EXIT_FAILURE);
            }

            if (fwrite(&green, sizeof(BYTE), 1, out_file) != 1){
                perror("Error writing pixel data to out file. Exiting.\n");
                pixel_data_1 = temp1;
                pixel_data_2 = temp2;
                free(pixel_data_1);
                free(pixel_data_2);
                temp1 = NULL;
                temp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                exit(EXIT_FAILURE);
            }

            if (fwrite(&blue, sizeof(BYTE), 1, out_file) != 1){
                perror("Error writing pixel data to out file. Exiting.\n");
                pixel_data_1 = temp1;
                pixel_data_2 = temp2;
                free(pixel_data_1);
                free(pixel_data_2);
                temp1 = NULL;
                temp2 = NULL;
                fclose(out_file);
                out_file = NULL;
                exit(EXIT_FAILURE);
            }
        }

        if (fwrite(&p, sizeof(BYTE), padding, out_file) != padding){
            perror("Error writing padding to out file. Exiting.\n");
            pixel_data_1 = temp1;
            pixel_data_2 = temp2;
            free(pixel_data_1);
            free(pixel_data_2);
            temp1 = NULL;
            temp2 = NULL;
            fclose(out_file);
            out_file = NULL;
            exit(EXIT_FAILURE);
        }
    }

    fclose(out_file);
    out_file = NULL;

    pixel_data_1 = temp1;
    pixel_data_2 = temp2;

    free(pixel_data_1);
    temp1 = NULL;
    pixel_data_1 = NULL;
    free(pixel_data_2);
    temp2 = NULL;
    pixel_data_2 = NULL;

    large_pixel_data = NULL;
    small_pixel_data = NULL;

    return 0;
}

void checkRatio(char *str, float *ratio, FILE *in1, FILE *in2, FILE *out){
    int i; 
    /*not valid to begin with*/
    if (str[0] != '1' && str[0] != '0'){
        perror("Invalid ratio input. Ratio must be within 0 and 1 (inclusive). Exiting.\n");
        fclose(in1);
        fclose(in2);
        fclose(out);
        exit(EXIT_FAILURE);
    }
    /*if input 1 with invalid after or greater than 1: 10., 1!sndjc*/
    else if (str[0] == '1' && (str[1] != '.' && str[1] != '\0')){
        perror("Invalid ratio input. Ratio must be within 0 and 1 (inclusive). Exiting.\n");
        fclose(in1);
        fclose(in2);
        fclose(out);
        exit(EXIT_FAILURE);
    }
    /*input begin with 0 but wrong after*/
    else if (str[0] == '0' && (str[1] != '.' && str[1] != '\0')){
        perror("Invalid ratio input. Ratio must be within 0 and 1 (inclusive). Exiting.\n");
        fclose(in1);
        fclose(in2);
        fclose(out);
        exit(EXIT_FAILURE);
    }

    for (i = 2; i < strlen(str); i++){
        if (!isdigit(str[i])){
            perror("Invalid ratio input. Ratio must be within 0 and 1 (inclusive). Exiting.\n");
            fclose(in1);
            fclose(in2);
            fclose(out);
            exit(EXIT_FAILURE);
        }
    }

    *ratio = atof(str);
    if (*ratio > 1 || *ratio < 0){
        perror("Ratio input too large. Ratio must be within 0 and 1 (inclusive). Exiting.\n");
        fclose(in1);
        fclose(in2);
        fclose(out);
        exit(EXIT_FAILURE);
    }
    return;
}

BYTE get_red(BYTE *imagedata, float x, float y, int imagewidth, int imageheight){
    int offset;
    BYTE *temp = imagedata;

    offset = x + imagewidth * y;

    return *(temp + (3*offset));
}

BYTE get_green(BYTE *imagedata, float x, float y, int imagewidth, int imageheight){
    int offset;
    BYTE *temp = imagedata + 1;

    offset = x + imagewidth * y;

    return *(temp + (3*offset));
}

BYTE get_blue(BYTE *imagedata, float x, float y, int imagewidth, int imageheight){
    int offset;
    BYTE *temp = imagedata + 2;

    offset = x + imagewidth * y;

    return *(temp + (3*offset));
}