#include "game.h"

#include "../util/defs.h"
#include "../util/aabb.h"

#include <string.h>

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

static void free_node_entity(void *_self, void **_node) {
    struct LinkedList *list = (struct LinkedList*)_self;
    struct Node **node = (struct Node**)_node;
    struct Node *free_node = (struct Node*)*_node;

    if ((*node) == list->head) {

        if ((*node)->next) {
            (*node) = (*node)->next;
        } else {
            (*node) = NULL;
        }
        list->head = *node;  
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
    list->length--;
}

static void spawn_block(struct LinkedList *self) {
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
        struct Entity block = entity_init((ivec2){45.0f,10.0f}, x, y, RECT_COLOR[color_index]);
        linked_list_append(self, (void*)&block);
        x += 50.0f;
    }
}

static void ball_reset_pos(struct Game *self) {
    self->ball.x = (self->player.x + self->player.x + self->player.size[0])/2.0f - self->ball.size[0]/2.0f;
    self->ball.y = self->player.y + self->player.size[1] + 5.0f;
    entity_vertices_set(&self->ball, self->ball.size, (vec2) { self->ball.x, self->ball.y});
}

void game_init(struct Game *self) {
    self->state  = READY;
    self->score  = 0;
    self->health = 3;
    self->speed_up_count = 0;
    self->ball_speedX = BALL_SPEED;
    self->ball_speedY = BALL_SPEED;

    renderer_init(&self->renderer);

    // TODO: change to use pointer to accual RECT_COLOR instead of copy from it to decrease cost of the operation.
    self->player     = entity_init((ivec2){150,10}, (WIDTH/2.0f-150.0f/2.0f), 10.0f, RECT_COLOR[WHITE]);
    self->ball       = entity_init((ivec2){10,10}, (WIDTH/2.0f-10.0f/2.0f), 25.0f, RECT_COLOR[WHITE]);
    self->text_box   = entity_init((ivec2){WIDTH, 50}, 0, HEIGHT-50.0f, RECT_COLOR[BLACK]);
    self->text       = text_init((ivec2){64,40}, 8);
    self->health_bar = entity_init((ivec2){10,40}, WIDTH - 3.0f*15.0f, HEIGHT-45.0f, RECT_COLOR[WHITE]);
    self->block      = linked_list_init(sizeof(struct Entity), free_node_entity);
    spawn_block(&self->block);
}

void game_destroy(struct Game *self) {
    renderer_destroy(&self->renderer);
    linked_list_destroy(&self->block);
}

static void input(GLFWwindow *window, struct Game *self) {
    memset(&self->input, 0, sizeof(self->input));
    if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT)) {
        self->input.left.pressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT)) {
        self->input.right.pressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        self->input.space.pressed = true;
    }

    // player input
    if (self->input.left.pressed) {
        self->player.x -= PLAYER_SPEED * self->time.delta_time;
        entity_vertices_set(&self->player, self->player.size, (vec2){self->player.x, self->player.y});
    }

    if (self->input.right.pressed) {
        self->player.x += PLAYER_SPEED * self->time.delta_time;
        entity_vertices_set(&self->player, self->player.size, (vec2){self->player.x, self->player.y});
    }

    if (self->input.space.pressed) {
        if (self->state == READY) {
            if (WIDTH - (self->ball.x + self->ball.x + self->ball.size[0])/2.0 > WIDTH/2.0) {
                self->ball_speedX = -self->ball_speedX;
            }
            self->state = INGAME;
        }
    }
}

static void render(struct Game *self) {
        renderer_shader_use(&self->renderer, SHADER_BASIC);

        vao_bind(self->renderer.vao);

        vbo_bind(self->renderer.vbo);
        vao_attr(0, 3, GL_FLOAT, sizeof(struct Vertex), offsetof(struct Vertex, pos));
        vao_attr(1, 3, GL_FLOAT, sizeof(struct Vertex), offsetof(struct Vertex, color));

       // render text_box (black box)
       vbo_subdata(self->renderer.vbo, sizeof(self->text_box.vertices), self->text_box.vertices);
       glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render health bar
        for (int health = 0; health < self->health; health++) {
            entity_vertices_set(&self->health_bar, (ivec2){10,40}, (vec2){WIDTH - 3.0f*10.0f - 15.0f*health, HEIGHT - 45.0f});
            vbo_subdata(self->renderer.vbo, sizeof(self->health_bar.vertices), self->health_bar.vertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }

        // TODO: change form using Uniform to something more less costly render block
        for (struct Node *node = self->block.head; node != NULL; node = node->next) {
            struct Entity *block = (struct Entity*)node->data;
            vbo_subdata(self->renderer.vbo, sizeof(block->vertices), block->vertices);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }

        // render ball
        vbo_subdata(self->renderer.vbo, sizeof(self->ball.vertices), self->ball.vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render player
        vbo_subdata(self->renderer.vbo, sizeof(self->player.vertices), self->player.vertices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        // render score text
        char score[255];
        snprintf(score, sizeof(score), "SCORE %d", self->score);
        text_render(self->text, &self->renderer, score, 10.0f, HEIGHT-50.0f, 5.0f, (vec3){1.0f, 1.0f, 1.0f});
}

static void update(struct Game *self) {

    // ball follows player position when in READY state
    if (self->state == READY && (self->input.left.pressed || self->input.right.pressed)) {
        ball_reset_pos(self);
    }

    if (self->player.x < 0) {
        self->player.x = 0;
    }
    if (self->player.x + self->player.size[0] > WIDTH) {
        self->player.x = WIDTH - self->player.size[0];
    }

    if (self->state == INGAME) {

        box2f player = {
            {self->player.x, self->player.y},
            {self->player.x + self->player.size[0], self->player.y + self->player.size[1]}
        };
        box2f ball = {
            {self->ball.x, self->ball.y},
            {self->ball.x + self->ball.size[0], self->ball.y + self->ball.size[1]}
        };

        u16 *rect_remove_index = malloc(RECT_COUNT*sizeof(u16));
        size_t idx = 0;

        for (struct Node *node = self->block.head; node != NULL; node = node->next) {

            struct Entity *block = (struct Entity*)node->data;
            box2f block_box = { {block->x, block->y}, {block->x + block->size[0], block->y + block->size[1]} };

            if (aabb_collide(block_box, ball)) {
                self->ball_speedY *= -1;
                rect_remove_index[idx++] = node->id;
            }
        }

        for (size_t i = 0; i < idx; i++) {
            for (struct Node *node = self->block.head; node != NULL; node = node->next) {
                if (node->id == rect_remove_index[i]) {
                    self->block.free_node_callback((void*)&self->block, (void**)&node);
                    self->score++;

                    if (self->speed_up_count < 70) {
                        self->ball_speedY = (self->ball_speedY < 0) ? -BALL_SPEED + (-SPEED_UP_RATE*self->score) 
                            : BALL_SPEED + (SPEED_UP_RATE*self->score);

                        self->ball_speedX = (self->ball_speedX < 0) ? -BALL_SPEED + (-SPEED_UP_RATE*self->score) 
                            : BALL_SPEED + (SPEED_UP_RATE*self->score);
                        self->speed_up_count++;
                    }

                    if (node == NULL) break;
                    i++;
                }
            }
        }
        free(rect_remove_index);

        if (self->ball.x < 0 || self->ball.x + self->ball.size[0] > WIDTH) {
            self->ball_speedX *= -1;
        }

        if (self->ball.y + self->ball.size[1] > HEIGHT-50 || aabb_collide(player, ball)) {
            self->ball_speedY *= -1;
        }

        if (self->ball.y < 0) {
            if (self->health > 0) {
                self->health--;
                ball_reset_pos(self);
                self->state = READY;
            } else {
                assert(self->health);
            }

        }

        self->ball.x += self->ball_speedX * self->time.delta_time;
        self->ball.y += self->ball_speedY * self->time.delta_time;
        entity_vertices_set(&self->ball, self->ball.size, (vec2){self->ball.x, self->ball.y});
    }

        // respawn blocks when no longer have any blocks left
        if (self->block.head == NULL) {
            spawn_block(&self->block);
        }
}

void game_run(GLFWwindow *window, struct Game *self) {

    float accumulator = 0.0f;
    self->time.last_frame = glfwGetTime();
    self->time.fps = 0;

    while (!glfwWindowShouldClose(window)) {
        self->time.current_frame = glfwGetTime();
        self->time.delta_time = self->time.current_frame - self->time.last_frame;
        self->time.last_frame = self->time.current_frame;
        accumulator += self->time.delta_time;
        self->time.fps++;

        if (accumulator >= 1.0f) {
            char buffer[24];
            snprintf(buffer, sizeof(buffer), "Breakout (FPS: %d)", self->time.fps);
            glfwSetWindowTitle(window, buffer);
            self->time.fps = 0;
            accumulator = 0.0f;
        }

        glClearColor(0.2, 0.3, 0.4, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        input(window, self);
        update(self);
        render(self);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}
