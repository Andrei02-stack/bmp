#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_header.h"

// Constante
#define NO_BITS 8
#define LINE_LENGTH 100
#define DELIMS " \r\n"
#define BASE 10

// am pus datele intr-un struct pentru a nu folosi variabile globale
// (erori checkstyle)
typedef struct {
    bmp_fileheader file;
    bmp_infoheader info;
    char **imageData;
    int padding, noBytes;
} bmp;

// functii auxiliare
int min(int a, int b) {
    if (a > b) {
        return b;
    }
    return a;
}

int  max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

// Task 1
void save(bmp *img, char *path) {
    // deschidere fisier
    FILE *out = fopen(path, "wb");
    // scriere headere
    img->file.imageDataOffset = sizeof(bmp_fileheader) + sizeof(bmp_infoheader);
    fwrite(&img->file, sizeof(bmp_fileheader), 1, out);
    fwrite(&img->info, sizeof(bmp_infoheader), 1, out);
    // scriere matrice
    int i = 0, j = 0;
    // ordine inversa
    while (i < img->info.height) {
        fwrite(img->imageData[i], img->noBytes, img->info.width, out);
        while (j < img->padding) {
            fputc(0, out);
            j++;
        }
        i++;
        j = 0;
    }
}

void load(bmp *img, char *path) {
    // deschidere fisier
    FILE *in = fopen(path, "rb");
    bmp_infoheader *info = &img->info;
    // citire headere
    fread(&img->file, sizeof(bmp_fileheader), 1, in);
    fread(info, sizeof(bmp_infoheader), 1, in);
    // seek la image data
    fseek(in, img->file.imageDataOffset, SEEK_SET);
    // determinare bytes/pixel si padding la sfarsit de linie
    img->noBytes = info->bitPix / NO_BITS;
    if (info->width * img->noBytes % 4 != 0) {
        img->padding = 4 - info->width * img->noBytes % 4;
    } else {
        img->padding = 0;
    }
    // citire imagine
    img->imageData = malloc(info->height * sizeof(int *));
    int i = 0;
    while (i < info->height) {
        img->imageData[i] = malloc(img->noBytes * info->width);
        fread(img->imageData[i], img->noBytes, info->width, in);
        // skip la padding
        if (i != info->height - 1) {
            fseek(in, img->padding, SEEK_CUR);
            }
            i++;
    }
    fclose(in);
}

void unload(bmp *img) {
    // eliberare memorie
    int i = img->info.height - 1;
    while (i >= 0) {
        free(img->imageData[i]);
        i--;
    }
    free(img->imageData);
}

// Task 2
void insert(bmp *img, char *path, int y, int x) {
    // incarcare imaginea noua
    bmp newImg;
    load(&newImg, path);
    // copiere imaginea noua in imaginea curenta
    int i = 0;
    while (i < min(img->info.height - x, newImg.info.height)) {
         memcpy(
                &img->imageData[i + x][img->noBytes * y],  // dest
                newImg.imageData[i],  // src
                img->noBytes * min(img->info.width - y, newImg.info.width));
                i++;
    }
    unload(&newImg);
}

// Task 3
void putPixel(bmp *img, char r, char g, char b, int y, int x, int width) {
    // calculam offsetul
    int offset = (width - 1) / 2;
    int startY = y - offset;
    int endY = y + offset;
    int startX = x - offset;
    int endX = x + offset;

    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            if (i < 0 || i >= img->info.height ||
                j < 0 || j >= img->info.width) {
                continue;
            }
            img->imageData[i][j * img->noBytes] = b;
            img->imageData[i][j * img->noBytes + 1] = g;
            img->imageData[i][j * img->noBytes + 2] = r;
        }
    }
}

void drawLine(bmp *img, int y1, int x1, int y2, int x2,
              char r, char g, char b, int width) {
    int xMax = max(x1, x2);
    int xMin = min(x1, x2);
    int yMax = max(y1, y2);
    int yMin = min(y1, y2);
    int start = 0;
    int end = 0;
    int ox = 0;

    if (xMax - xMin > yMax - yMin) {
        start = xMin;
        end = xMax;
        ox = 1;
    } else if (xMax - xMin < yMax - yMin) {
        start = yMin;
        end = yMax;
        ox = 0;
    } else {
        start = xMin;
        end = xMax;
        ox = 2;
    }

    for (float i = (float)start; i <= (float)end; i++) {
        // Daca mergem pe Ox
        if (ox == 1) {
            float y = (i - (float)x1) / ((float)x2 - (float)x1) * ((float)y2 - (float)y1) + (float)y1;
            putPixel(img, r, g, b, (int)y, (int)i, width);
            // Daca mergem pe Oy
        } else if (ox == 0) {
            float x = (i - (float)y1) / ((float)y2 - (float)y1) * ((float)x2 - (float)x1) + (float)x1;
            putPixel(img, r, g, b, (int)i, (int)x, width);
            // Daca mergem diagonal
        } else {
            putPixel(img, r, g, b, (int)i, (int)i, width);
        }
    }
}

void drawRectangle(bmp *img, int y1, int x1, int w, int h,
                   char r, char g, char b, int width) {
    int y = y1 + h;
    int x = x1;
    int yDest = y;
    int xDest = x + w;
    // Latura de sus
    drawLine(img, y, x, yDest, xDest, r, g, b, width);

    y = yDest;
    yDest = y - h;
    x = xDest;
    xDest = x;
    // Latura din dreapta
    drawLine(img, y, x, yDest, xDest, r, g, b, width);

    y = yDest;
    yDest = y;
    x = xDest;
    xDest = x - w;
    // Latura de jos
    drawLine(img, y, x, yDest, xDest, r, g, b, width);

    y = y1;
    yDest = y + h;
    x = xDest;
    xDest = x;
    // Latura din stanga
    drawLine(img, y, x, yDest, xDest, r, g, b, width);
}

void drawTriangle(bmp *img, int y1, int x1, int y2, int x2, int y3, int x3 ,
              char r, char g, char b, int width) {
    drawLine(img, y1, x1, y2, x2, r, g, b, width);
    drawLine(img, y2, x2, y3, x3, r, g, b, width);
    drawLine(img, y3, x3, y1, x1, r, g, b, width);
}

// Task 4
void fillHelper(bmp *img, int y, int x, char orig_r, char orig_g, char orig_b,
                char r, char g, char b, int **visited) {
    if (y < 0 || y >= img->info.height ||
        x < 0 || x >= img->info.width ||
        visited[y][x] == 1) {
        return;
    }

    if (img->imageData[y][x * img->noBytes + 2] != orig_r ||
        img->imageData[y][x * img->noBytes + 1] != orig_g ||
        img->imageData[y][x * img->noBytes] != orig_b) {
        visited[y][x] = 1;
        return;
    }

    img->imageData[y][x * img->noBytes + 2] = r;
    img->imageData[y][x * img->noBytes + 1] = g;
    img->imageData[y][x * img->noBytes] = b;
    visited[y][x] = 1;

    fillHelper(img, y + 1, x, orig_r, orig_g, orig_b, r, g, b, visited);
    fillHelper(img, y - 1, x, orig_r, orig_g, orig_b, r, g, b, visited);
    fillHelper(img, y, x + 1, orig_r, orig_g, orig_b, r, g, b, visited);
    fillHelper(img, y, x - 1, orig_r, orig_g, orig_b, r, g, b, visited);
}

void fill(bmp *img, int y, int x, char r, char g, char b) {
    char orig_r = img->imageData[y][x * img->noBytes + 2];
    char orig_g = img->imageData[y][x * img->noBytes + 1];
    char orig_b = img->imageData[y][x * img->noBytes];
    int **visited = malloc(sizeof(int*) * img->info.height);

    for (int i = 0; i < img->info.height; i++) {
        visited[i] = malloc(sizeof(int) * img->info.width);

        for (int j = 0; j < img->info.width; j++) {
            visited[i][j] = 0;
        }
    }

    fillHelper(img, y, x, orig_r, orig_g, orig_b, r, g, b, visited);

    for (int i = 0; i < img->info.height; i++) {
        free(visited[i]);
    }
    free(visited);
}

int main() {
    // culorile de scriere
    long draw_r = 0;
    long draw_g = 0;
    long draw_b = 0;
    // marimea unei linii
    long lineWidth = 0;

    bmp image;
    char line[LINE_LENGTH], *cmd = NULL, *arg = NULL;
    // main loop
    while (fgets(line, LINE_LENGTH, stdin)) {
        // impartire pe comenzi si argumente
        cmd = strtok(line, DELIMS);
        arg = strtok(NULL, DELIMS);
        // comenzi
        if (strcmp(cmd, "quit") == 0)
        break;
        if (strcmp(cmd, "save") == 0) {
            save(&image, arg);
        } else if (strcmp(cmd, "edit") == 0) {
            load(&image, arg);
        } else if (strcmp(cmd, "insert") == 0) {
            // obtinere argumente
            char *path = arg;
            char *y = strtok(NULL, DELIMS);
            char *x = strtok(NULL, DELIMS);
            insert(&image, path, atoi(y), atoi(x));
        } else if (strcmp(cmd, "set") == 0) {
            if (strcmp(arg, "draw_color") == 0) {
                draw_r = strtol(strtok(NULL, DELIMS), NULL, BASE);
                draw_g = strtol(strtok(NULL, DELIMS), NULL, BASE);
                draw_b = strtol(strtok(NULL, DELIMS), NULL, BASE);
            } else if (strcmp(arg, "line_width") == 0) {
                lineWidth = strtol(strtok(NULL, DELIMS), NULL, BASE);
            }
        } else if (strcmp(cmd, "draw") == 0) {
            if (strcmp(arg, "line") == 0) {
                long x1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long x2 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y2 = strtol(strtok(NULL, DELIMS), NULL, BASE);

                drawLine(&image, (int)y1, (int)x1, (int)y2, (int)x2, (char)draw_r,
                 (char)draw_g, (char)draw_b, (int)lineWidth);
            } else if (strcmp(arg, "rectangle") == 0) {
                long x1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long w = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long h = strtol(strtok(NULL, DELIMS), NULL, BASE);

                drawRectangle(&image, (int)y1, (int)x1, (int)w, (int)h, (char)draw_r,
                (char)draw_g, (char)draw_b, (int)lineWidth);
            } else if (strcmp(arg, "triangle") == 0) {
                long x1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y1 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long x2 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y2 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long x3 = strtol(strtok(NULL, DELIMS), NULL, BASE);
                long y3 = strtol(strtok(NULL, DELIMS), NULL, BASE);

                drawTriangle(&image, (int)y1, (int)x1, (int)y2, (int)x2, (int)y3,
                 (int)x3, (char)draw_r, (char)draw_g, (char)draw_b, (int)lineWidth);
            }
        } else if (strcmp(cmd, "fill") == 0) {
            long x = strtol(arg, NULL, BASE);
            long y = strtol(strtok(NULL, DELIMS), NULL, BASE);

            fill(&image, (int)y, (int)x, (char)draw_r, (char)draw_g, (char)draw_b);
        }
    }
    unload(&image);
    return 0;
}
