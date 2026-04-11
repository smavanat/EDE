#ifndef __INPUT_H__
#define __INPUT_H__
#include "../externals/GLFW/glfw3.h"

//Enum to hold the state of the keys
typedef enum {
    KEY_JUST_PRESSED, //Was pressed this frame
    KEY_PRESSED, //Held on from the previous frame
    KEY_RELEASED, //Not pressed
    KEY_JUST_RELEASED //Released the previous frame
} keystate;

//Holds all of the keys we currently want to work with
typedef enum {
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    NUM_KEYS
} keys;

//Actual input handling interface
typedef struct {
    keystate key_status[NUM_KEYS];
    double mouseX;
    double mouseY;
} input_handler;

extern input_handler *handler; //Global for use in the callback functions

/*
 * Initialises an input handler struct, setting the initial cursor position to 0 and all of the keys to be 'released'
 * @return a pointer to the newly created input_handler
 */
input_handler *input_handler_init(void);
/**
 * Callback function to be used by glfw when a mouse button's state changes
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_JUST_PRESSED
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_PRESSED or KEY_JUST_PRESSED,
 * sets the state to KEY_PRESSED
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_RELEASED
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_PRESSED or KEY_PRESSED,
 * sets the state to KEY_JUST_RELEASED
 */
void glfw_mouse_callback(GLFWwindow *window, int key, int action, int mods);
/**
 * Callback function to be used by glfw when a key's state changes
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_JUST_PRESSED
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_PRESSED or KEY_JUST_PRESSED,
 * sets the state to KEY_PRESSED
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_RELEASED
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_PRESSED or KEY_PRESSED,
 * sets the state to KEY_JUST_RELEASED
 * If the new state is GLFW_REPEAT set the state to KEY_PRESSED
 */
void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
/**
 * Callback function to be used by glfw when the cursor position changes
 */
void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
/**
 * Simple checker that returns if the cursor is in the window's bounds or not
 */
int cursor_in_bounds(input_handler *ih);

#endif
