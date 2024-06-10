#ifndef IMAGE_H
#define IMAGE_H

#include "../util/common.h"

struct Image {
    int width, height, channels;
    unsigned char* texture;
    size_t size;
};

struct Image image_new(const char* path);
unsigned char* image_getPixel(struct Image image, int x, int y);
unsigned char* image_getAveragePixel(struct Image image, unsigned char* pixels);
struct Image image_combine(struct Image a, struct Image b);
struct Image spritesheet_new(size_t length, const char* srcs[]);
struct Image data_spritesheet_new(size_t length, struct Image data[]);
void image_printPixel(struct Image image, unsigned char* pixel);

#endif