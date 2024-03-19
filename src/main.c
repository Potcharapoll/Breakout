#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define WIDTH 530
#define HEIGHT 720
#define RECT_COUNT 120 
#define COLOR_CHANGE_AT 20
#define SPEED_UP_RATE 5
#define MAX_SPEED_UP_COUNT 70
#define BALL_SPEED 300
#define PLAYER_SPEED 700

enum RectangleColor {
    BLACK,
    WHITE,
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE
};

enum ShaderType {
    SHADER_BASIC,
    SHADER_TEXT
};

enum GameState {
    READY,
    INGAME,
    PAUSE
};

#define SHADER_LAST SHADER_TEXT+1
#define COLOR_LAST PURPLE+1

vec3 RECT_COLOR[COLOR_LAST] = {
    {0.0f,0.0f,0.0f},
    {1.00,1.00,1.00},
    {0.97,0.44,0.44},
    {0.98,0.57,0.23},
    {0.98,0.80,0.08},
    {0.29,0.87,0.50},
    {0.37,0.64,0.98},
    {0.75,0.51,0.98},
};

struct Shader {
    GLuint handle, vs, fs;
};

struct Vertex {
    vec3 pos;
    vec3 color;
};

struct Rectangle {
    ivec2 size;
    float x, y;

    struct Vertex vertices[4];
};

struct Node {
    int id;
    void *data;
    struct Node *prev;
    struct Node *next;
};

struct LinkedList {
    size_t data_size;
    size_t length;

    void (*free_node_callback)(void**, void*);
    struct Node *head;
};

typedef struct {
    vec2 min, max;
} box2f;


struct Text {
    GLuint texture;
    ivec2 size;
    unsigned int frame_size;
};

struct State {
    GLFWwindow *window;
    GLuint VAO, VBO, IBO;

    struct {
        float delta_time;
        float last_frame, current_frame;
        unsigned int fps;
    } time;

    struct {
        struct {
            bool pressed;
        } left, right, space;
    } input;

    enum GameState state;

    enum ShaderType current_shader;
    struct Shader shaders[SHADER_LAST];
    struct Shader shader;

    struct Rectangle player;
    struct Rectangle ball;
    struct Rectangle text_box;
    struct Rectangle health_bar;
    struct LinkedList block;
    struct Text text;

    mat4 proj;
    unsigned int score;
    unsigned int speed_up_count;
    unsigned short health;
};

bool aabb_collide(box2f a, box2f b) {
    for (int i = 0; i < 2; ++i) {
        if (a.min[i] > b.max[i] || a.max[i] < b.min[i]) {
            return false;
        }
    }
    return true;
}



struct LinkedList linked_list_init(size_t data_size, void (*free_node_callback)(void**,void*)) {
    struct LinkedList new_list = {
        .data_size = data_size,
        .free_node_callback = free_node_callback,
        .head = NULL,
        .length = 0
    };
    return new_list;
}

void linked_list_destroy(struct LinkedList *self) {
    while (self->head != NULL) {
        struct Node *node = self->head;
        self->head = self->head->next;
        free(node->data); 
        free(node);
    }
    self->head = NULL;
}

void linked_list_append(struct LinkedList *self, void *data) {
    struct Node *node = malloc(sizeof(*node));
    node->data = malloc(self->data_size);
    node->id = self->length;
    self->length++;
    memcpy(node->data, data, self->data_size);

    node->next = NULL;
    node->prev = NULL;

    if (self->head == NULL) {
        self->head = node;
        return;
    }

    struct Node *curr = self->head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = node;
    node->prev = curr;
}

void free_node_rect(void **_node, void *_state) {
    struct Node **node = (struct Node**)_node;
    struct Node *free_node = (struct Node*)*_node;
    struct State *state = (struct State*)_state;

    if ((*node) == state->block.head) {

        if ((*node)->next) {
            (*node) = (*node)->next;
        } else {
            (*node) = NULL;
        }
        state->block.head = *node;  
    }
    else if ((*node)->prev && (*node)->next) {
        (*node)->prev->next = (*node)->next;
        (*node)->next->prev = (*node)->prev;
        (*node) = (*node)->next;
    }
    else if ((*node)->prev) {
        (*node)->prev->next = NULL;
        (*node) = NULL;
    }

    free(free_node);
    state->block.length--;
}

void vertices_set(struct Vertex vertices[static 4], ivec2 size, vec2 pos) {
    glm_vec3_copy((vec3){pos[0],         pos[1],         0.0}, vertices[0].pos);
    glm_vec3_copy((vec3){pos[0]+size[0], pos[1],         0.0}, vertices[1].pos);
    glm_vec3_copy((vec3){pos[0]+size[0], pos[1]+size[1], 0.0}, vertices[2].pos);
    glm_vec3_copy((vec3){pos[0],         pos[1]+size[1], 0.0}, vertices[3].pos);
}

void color_set(struct Vertex vertices[static 4], vec3 color) {
    glm_vec3_copy(color, vertices[0].color);
    glm_vec3_copy(color, vertices[1].color);
    glm_vec3_copy(color, vertices[2].color);
    glm_vec3_copy(color, vertices[3].color);
}

struct Rectangle rect_init(ivec2 size, float x, float y, vec3 color) {
    struct Rectangle rect = {
        .size = { size[0], size[1] },
        .x    = x,
        .y    = y 
    };

    vertices_set(rect.vertices, size, (vec2){x,y});
    color_set(rect.vertices, color);
    return rect;
}

GLuint _compile(char *path, GLenum type) {
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

void shader_use(struct State *state, enum ShaderType shader) {
    state->shader = state->shaders[shader];
    glUseProgram(state->shader.handle);

    state->current_shader = shader;
}

void shader_delete(struct Shader self) {
    glDeleteProgram(self.handle);
    glDeleteShader(self.vs);
    glDeleteShader(self.fs);
}

GLuint texture_load(char *path) {
    GLuint handle;

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

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
    return handle;
}

struct Text text_init(ivec2 size, unsigned int frame_size) {
    struct Text text = {
        .texture = texture_load("../res/images/font.png"),
        .size = {size[0], size[1]},
        .frame_size = frame_size
    };
    return text;
}

void text_render(struct State *state, char *text, float x, float y, float scale, vec3 txt_color) {

    float size = state->text.frame_size * scale;
    short rows = 0;
    short columns = 0;

    const short first_alphabet = 4;
    const short first_digit    = 1;
    const short txt_padding    = 12;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, state->text.texture);

    shader_use(state, SHADER_TEXT);
    glUniform3f(glGetUniformLocation(state->shader.handle, "txt_color"), txt_color[0], txt_color[1], txt_color[2]);

    for (char *c = text; *c; c++) {

        if (isspace(*c)) {
            x += size - size/txt_padding;
            c++;
        }

        if (isalpha(*c)) {
            rows = *c - 'A';
            columns = first_alphabet;
        }
        else if (isdigit(*c)) {
            rows = *c - '0' + 2;
            columns = first_digit;

        }

        if (rows > 7) {
            columns -= (rows/state->text.frame_size);
            rows %= state->text.frame_size;
        }

        float vertices[] = {
            x,      y,      0.0f,     0.0f, 0.0f,
            x+size, y,      0.0f,     1.0f, 0.0f,
            x+size, y+size, 0.0f,     1.0f, 1.0f,
            x,      y+size, 0.0f,     0.0f, 1.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glUniform2f(glGetUniformLocation(state->shader.handle, "img_size"), (float)state->text.size[0], (float)state->text.size[1]);
        glUniform2f(glGetUniformLocation(state->shader.handle, "uv"), (float)rows, (float)columns);
        glUniform1f(glGetUniformLocation(state->shader.handle, "frame_size"), (float)state->text.frame_size);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        
        x += size - size/txt_padding;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void spawn_block(struct LinkedList *self) {
    size_t color_index = WHITE;
    float y = HEIGHT - 75.0f;
    float x = 15.0f;
    for (int i = 0; i < RECT_COUNT; i++) {
        if (i%10==0) {
            y -= 15.0f;
            x = 15.0f;
        }
        if (i%COLOR_CHANGE_AT==0) {
            color_index++;
        }
        struct Rectangle rect = rect_init((ivec2){45.0f,10.0f}, x, y, RECT_COLOR[color_index]);
        linked_list_append(self, (void*)&rect);
        x += 50.0f;
    }

}

void state_init(struct State *state) {
    glGenVertexArrays(1, &state->VAO);
    glGenBuffers(1, &state->VBO);
    glGenBuffers(1, &state->IBO);

    glBindVertexArray(state->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(struct Vertex), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, color));
    glEnableVertexAttribArray(1);

    unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    state->shaders[SHADER_BASIC] = shader_load("../res/shaders/basic.vs", "../res/shaders/basic.fs");
    state->shaders[SHADER_TEXT] = shader_load("../res/shaders/text.vs", "../res/shaders/text.fs");

    glm_ortho(0, WIDTH, 0, HEIGHT, 0.0f, 1.0f, state->proj);

    shader_use(state, SHADER_BASIC);
    glUniformMatrix4fv(glGetUniformLocation(state->shader.handle, "proj"), 1, GL_FALSE, &state->proj[0][0]);

    shader_use(state, SHADER_TEXT);
    glUniformMatrix4fv(glGetUniformLocation(state->shader.handle, "proj"), 1, GL_FALSE, &state->proj[0][0]);

    state->state  = READY;
    state->score  = 0;
    state->health = 3;
    state->speed_up_count = 0;

    // TODO: change to use pointer to accual RECT_COLOR instead of copy from it to decrease cost of the operation.
    state->player     = rect_init((ivec2){150,10}, (WIDTH/2.0f-150.0f/2.0f), 10.0f, RECT_COLOR[WHITE]);
    state->ball       = rect_init((ivec2){10,10}, (WIDTH/2.0f-10.0f/2.0f), 25.0f, RECT_COLOR[WHITE]);
    state->text_box   = rect_init((ivec2){WIDTH, 50}, 0, HEIGHT-50.0f, RECT_COLOR[BLACK]);
    state->text       = text_init((ivec2){64,40}, 8);
    state->health_bar = rect_init((ivec2){10,40}, WIDTH - 3.0f*15.0f, HEIGHT-45.0f, RECT_COLOR[WHITE]);
    state->block      = linked_list_init(sizeof(struct Rectangle), free_node_rect);
    spawn_block(&state->block);
}

void reset_ball_pos(struct State *state) {
    state->ball.x = (state->player.x + state->player.x + state->player.size[0])/2.0f - state->ball.size[0]/2.0f;
    state->ball.y = state->player.y + state->player.size[1] + 5.0f;
    vertices_set(state->ball.vertices, state->ball.size, (vec2) { state->ball.x, state->ball.y });
}

int main(void) {

    struct State state;

    assert(glfwInit());
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    state.window = glfwCreateWindow(WIDTH, HEIGHT, "Breakout", NULL, NULL);
    assert(state.window != NULL);
    glfwMakeContextCurrent(state.window);
    glfwSwapInterval(1);

    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    state_init(&state);
 
    state.time.last_frame = glfwGetTime();
    state.time.fps = 0;
    float accumulator = 0.0f;
    int BALL_SPEEDX = BALL_SPEED; 
    int BALL_SPEEDY = BALL_SPEED;

    glEnable(GL_CULL_FACE);

    glViewport(0, 0, WIDTH, HEIGHT);
    while (!glfwWindowShouldClose(state.window)) {

        state.time.current_frame = glfwGetTime();
        state.time.delta_time = state.time.current_frame - state.time.last_frame;
        state.time.last_frame = state.time.current_frame;
        accumulator += state.time.delta_time;
        state.time.fps++;

        if (accumulator >= 1.0f) {
            char buffer[24];
            snprintf(buffer, sizeof(buffer), "Breakout (FPS: %d)", state.time.fps);
            glfwSetWindowTitle(state.window, buffer);
            state.time.fps = 0;
            accumulator = 0.0f;
        }

        memset(&state.input, 0, sizeof(state.input));
        if (glfwGetKey(state.window, GLFW_KEY_A) || glfwGetKey(state.window, GLFW_KEY_LEFT)) {
            state.input.left.pressed = true;
        }

        if (glfwGetKey(state.window, GLFW_KEY_D) || glfwGetKey(state.window, GLFW_KEY_RIGHT)) {
            state.input.right.pressed = true;
        }

        if (glfwGetKey(state.window, GLFW_KEY_SPACE)) {
            state.input.space.pressed = true;
        }

        glClearColor(0.2, 0.3, 0.4, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (state.player.x < 0) {
            state.player.x = 0;
        }
        if (state.player.x + state.player.size[0] > WIDTH) {
            state.player.x = WIDTH - state.player.size[0];
        }

        if (state.state == INGAME) {

            box2f player = {
                {state.player.x, state.player.y},
                {state.player.x + state.player.size[0], state.player.y + state.player.size[1]}
            };
            box2f ball = {
                {state.ball.x, state.ball.y},
                {state.ball.x + state.ball.size[0], state.ball.y + state.ball.size[1]}
            };

            short *rect_remove_index = malloc(RECT_COUNT*sizeof(short));
            size_t idx = 0;
            for (struct Node *node = state.block.head; node != NULL; node = node->next) {

                struct Rectangle *rect = (struct Rectangle*)node->data;
                box2f rect_box = { {rect->x, rect->y}, {rect->x + rect->size[0], rect->y + rect->size[1]} };

                if (aabb_collide(rect_box, ball)) {
                    BALL_SPEEDY *= -1;
                    rect_remove_index[idx++] = node->id;
                }
            }

            for (size_t i = 0; i < idx; i++) {
                for (struct Node *node = state.block.head; node != NULL; node = node->next) {
                    if (node->id == rect_remove_index[i]) {
                        state.block.free_node_callback((void**)&node, (void*)&state);
                        state.score++;

                        if (state.speed_up_count < 70) {
                            BALL_SPEEDY = (BALL_SPEEDY < 0) ? -BALL_SPEED + (-SPEED_UP_RATE*state.score) 
                                : BALL_SPEED + (SPEED_UP_RATE*state.score);

                            BALL_SPEEDX = (BALL_SPEEDX < 0) ? -BALL_SPEED + (-SPEED_UP_RATE*state.score) 
                                : BALL_SPEED + (SPEED_UP_RATE*state.score);
                            state.speed_up_count++;
                        }

                        if (node == NULL) break;
                        i++;
                    }
                }
            }
            free(rect_remove_index);

            if (state.ball.x < 0 || state.ball.x + state.ball.size[0] > WIDTH) {
                BALL_SPEEDX *= -1;
            }

            if (state.ball.y + state.ball.size[1] > HEIGHT-50 || aabb_collide(player, ball)) {
                BALL_SPEEDY *= -1;
            }

            if (state.ball.y < 0) {
                if (state.health > 0) {
                    state.health--;
                    reset_ball_pos(&state);
                    state.state = READY;
                } else {
                    assert(state.health);
                }

            }

            state.ball.x += BALL_SPEEDX * state.time.delta_time;
            state.ball.y += BALL_SPEEDY * state.time.delta_time;
            vertices_set(state.ball.vertices, state.ball.size, (vec2){state.ball.x, state.ball.y});
        }

        // player input
        if (state.input.left.pressed) {
            state.player.x -= PLAYER_SPEED * state.time.delta_time;
            vertices_set(state.player.vertices, state.player.size, (vec2){state.player.x, state.player.y});
        }

        if (state.input.right.pressed) {
            state.player.x += PLAYER_SPEED * state.time.delta_time;
            vertices_set(state.player.vertices, state.player.size, (vec2){state.player.x, state.player.y});
        }

        if (state.input.space.pressed) {
            if (state.state == READY) {
                if (WIDTH - (state.ball.x+state.ball.x+state.ball.size[0])/2.0 > WIDTH/2.0) {
                    BALL_SPEEDX = -BALL_SPEEDX;
                }
                state.state = INGAME;
            }
        }

        // ball follows player position when in READY state
        if (state.state == READY && (state.input.left.pressed || state.input.right.pressed)) {
            reset_ball_pos(&state);
        }

        // respawn blocks when no longer have any blocks left
        if (state.block.head == NULL) {
            spawn_block(&state.block);
        }

        shader_use(&state, SHADER_BASIC);
        glBindVertexArray(state.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, state.VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, color));
        glEnableVertexAttribArray(1);

       // render text_box (black box)
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(state.text_box.vertices), state.text_box.vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render health bar
        for (int health = 0; health < state.health; health++) {
            vertices_set(state.health_bar.vertices, (ivec2){10,40}, (vec2){WIDTH - 3.0f*10.0f - 15.0f*health, HEIGHT - 45.0f});
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(state.health_bar.vertices), state.health_bar.vertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }

        // TODO: change form using Uniform to something more less costly render rectangle
        for (struct Node *node = state.block.head; node != NULL; node = node->next) {
            struct Rectangle *rect = (struct Rectangle*)node->data;
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rect->vertices), rect->vertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }

        // render ball
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(state.ball.vertices), state.ball.vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render player
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(state.player.vertices), state.player.vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render score text
        char score[255];
        snprintf(score, sizeof(score), "SCORE %d", state.score);
        text_render(&state, score, 10.0f, HEIGHT-50.0f, 5.0f, (vec3){1.0f, 1.0f, 1.0f});

        glfwPollEvents();
        glfwSwapBuffers(state.window);
    }
    
    glDeleteVertexArrays(1, &state.VAO);
    glDeleteBuffers(1, &state.VBO);
    glDeleteBuffers(1, &state.IBO);
    shader_delete(state.shaders[SHADER_BASIC]);
    shader_delete(state.shaders[SHADER_TEXT]);
    linked_list_destroy(&state.block);
    glfwDestroyWindow(state.window);
    glfwTerminate();
    return 0;
}
