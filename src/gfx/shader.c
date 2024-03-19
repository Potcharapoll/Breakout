#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static GLuint _compile(char *path, GLenum type) {
    FILE *f;
    long len;
    char *txt;

    f = fopen(path, "rb");
    assert(f != NULL);

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    assert(len > 0);
    fseek(f, 0, SEEK_SET);

    txt = malloc(len);
    assert(txt != NULL);
    fread(txt, 1, len, f);
    assert(strlen(txt) > 0);
    fclose(f);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&txt, (const GLint*)&len);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char buf[1024];
        glGetShaderInfoLog(shader, 1024, NULL, buf);
        printf("%s\n", buf);
    }
    return shader;
}

struct Shader shader_load(char *vs_path, char *fs_path) {
    struct Shader shader = {
        .vs = _compile(vs_path, GL_VERTEX_SHADER),
        .fs = _compile(fs_path, GL_FRAGMENT_SHADER),
        .handle = glCreateProgram()
    };
    glAttachShader(shader.handle, shader.vs);
    glAttachShader(shader.handle, shader.fs);

    int success;
    glLinkProgram(shader.handle);
    glGetProgramiv(shader.handle, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char buf[1024];
        glGetProgramInfoLog(shader.handle, 1024, NULL, buf);
        printf("%s\n", buf);
    }

    glValidateProgram(shader.handle);
    glGetProgramiv(shader.handle, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE) {
        char buf[1024];
        glGetProgramInfoLog(shader.handle, 1024, NULL, buf);
        printf("%s\n", buf);
    }
    return shader;
}

void shader_use(struct Shader self) {
    glUseProgram(self.handle);
}

void shader_delete(struct Shader self) {
    glDeleteProgram(self.handle);
    glDeleteShader(self.vs);
    glDeleteShader(self.fs);
}

