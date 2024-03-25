#ifndef GAME_H
#define GAME_H
#include "../entity/entity.h"
#include "../util/linked_list.h"
#include "../util/sound.h"
#include "../util/types.h"
#include "renderer.h"
#include "text.h"

enum GameState {
    READY,
    INGAME,
    PAUSE
};

enum Sounds {
    SOUND_BALL,
    SOUND_LAST
};

struct Game {
    struct {
        f32 delta_time;
        f32 last_frame, current_frame;
        u32 fps;
    } time;

    struct {
        struct {
            bool pressed;
        } left, right, space;
    } input;

    enum GameState state;
    struct Renderer renderer;

    struct Entity player;
    
    struct Entity ball;
    s32 ball_speedX, ball_speedY;

    struct Entity text_box;
    struct Entity health_bar;

    struct LinkedList block;
    struct Text text;

    sound_context_t sound_context;
    sound_t *sounds[SOUND_LAST];

    u32 score;
    u16 speed_up_count;
    u16 health;
};      

void game_init(struct Game *self);
void game_destroy(struct Game *self);
void game_run(GLFWwindow *window, struct Game *self);
#endif
