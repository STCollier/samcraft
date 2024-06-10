#include "stb/stb_image.h"

#include "image.h"

struct Image image_new(const char* path) {
    struct Image image;

    image.texture = stbi_load(path, &image.width, &image.height, &image.channels, 0);
    image.size = image.width * image.height * 4;

    if (!image.texture) {
        ERROR_MSG("Failed to load image", path);
        exit(EXIT_FAILURE);
    }

    return image;
}

// free() afterwards
unsigned char* image_getPixel(struct Image image, int x, int y) {
    unsigned char* pixelData = malloc(image.channels);

    for (int i = 0; i < image.channels; i++) {
        pixelData[i] = image.texture[image.channels * ((image.width * y) + x) + i];
    }

    return pixelData;
}

// free() afterwards
unsigned char* image_getAveragePixel(struct Image image, unsigned char* pixels) {
    unsigned char* pixelData = malloc(image.channels);

    for (int i = 0; i < image.channels; i++) {
        for (int j = 0; j < 4; j++) {
            pixelData[i] += pixels[j + i * image.channels];
        }
    }

    for (int i = 0; i < 4; i++) {
        pixelData[i] /= 4;
    }

    return pixelData;
}

// free() image.texture afterwards
struct Image image_combine(struct Image a, struct Image b) {
    if (a.channels != b.channels) {
        ERROR("Varying number of channels when attempting to combine images");
        exit(EXIT_FAILURE);
    }

    const size_t chn = a.channels;

    size_t cwidth = a.width + b.width + 1; // Combined width + 1 pixel padding
    size_t cheight = a.height > b.height ? a.height : b.height; // Max height
    size_t csize = (cwidth + 1) * cheight * chn; // Combined # of pixels

    unsigned char* pixelData = calloc(csize, 1);
    
    for (int i = 0; i < a.height; i++) {
        memcpy(pixelData + i * chn * cwidth, a.texture + i * chn * a.width, a.width * chn);
    }

    for (int i = 0; i < b.height; i++) {
        memcpy(pixelData + i * chn * cwidth + (a.width + 1) * chn, b.texture + i * chn * b.width, b.width * chn);
    }

    struct Image result = {
        .channels = chn,
        .width = cwidth,
        .height = cheight,
        .size = csize,
        .texture = malloc(csize)
    };

    memcpy(result.texture, pixelData, csize);

    free(pixelData);

    return result;
}

struct Image spritesheet_new(size_t length, const char* srcs[]) {
    struct Image combined[length - 1];

    for (size_t i = 1; i < length; i++) {
        if (i == 1) {
            combined[0] = image_combine(image_new(srcs[i - 1]), image_new(srcs[i]));
        } else {
            combined[i - 1] = image_combine(combined[i - 2], image_new(srcs[i]));
        }
    }

    struct Image final = combined[length - 2];

    struct Image spritesheet = {
        .channels = 4,
        .width = final.width,
        .height = final.height,
        .texture = malloc(final.size)
    };

    memcpy(spritesheet.texture, final.texture, final.size);

    for (size_t i = 0; i < length - 1; i++) {
        free(combined[i].texture);
    }

    return spritesheet;
}

struct Image data_spritesheet_new(size_t length, struct Image data[]) {
    struct Image combined[length - 1];

    for (size_t i = 1; i < length; i++) {
        if (i == 1) {
            combined[0] = image_combine(data[i - 1], data[i]);
        } else {
            combined[i - 1] = image_combine(combined[i - 2], data[i]);
        }
    }

    struct Image final = combined[length - 2];

    struct Image spritesheet = {
        .channels = final.channels,
        .width = final.width,
        .height = final.height,
        .texture = malloc(final.size)
    };

    memcpy(spritesheet.texture, final.texture, final.size);

    for (size_t i = 0; i < length - 1; i++) {
        free(combined[i].texture);
    }

    return spritesheet;
}

void image_printPixel(struct Image image, unsigned char* pixel) {
    switch (image.channels) {
        case 1:
            printf("\x1B[48;2;%i;0;0m  ", pixel[0]);
        case 2:
            printf("\x1B[48;2;%i;%i;0m  ", pixel[0], pixel[1]);
        default:
            printf("\x1B[48;2;%i;%i;%im  ", pixel[0], pixel[1], pixel[2]);
    }
}