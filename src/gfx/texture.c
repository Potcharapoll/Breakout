#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Texture texture_load(char *path) {
    struct Texture texture;

    glGenTextures(1, &texture.handle);
    glBindTexture(GL_TEXTURE_2D, texture.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, bpp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *pixels = stbi_load(path, &width, &height, &bpp, 0);
    assert(pixels != NULL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    texture.size[0] = width;
    texture.size[1] = height;
    return texture;
}

void texture_destroy(struct Texture self) {
    glDeleteTextures(1, &self.handle);
}

void texture_bind(struct Texture self) {
    glBindTexture(GL_TEXTURE_2D, self.handle);
}

void texture_unbind(void) {
    glBindTexture(GL_TEXTURE_2D, 0);
}
