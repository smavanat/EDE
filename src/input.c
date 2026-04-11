#include "../include/input.h"
#include <stdio.h>
#include <stdlib.h>

input_handler *input_handler_init(void) {
    input_handler *ret = malloc(sizeof(input_handler));

    ret->mouseX = 0;
    ret->mouseY = 0;

    for(int i = 0; i < NUM_KEYS; i++) {
        ret->key_status[i] = KEY_RELEASED;
    }

    return ret;
}

void glfw_mouse_callback(GLFWwindow *window, int key, int action, int mods) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED) && cursor_in_bounds(handler) && key < 3) {
        int idx = MOUSE_BUTTON_LEFT+key;
        if(action == GLFW_PRESS) {
            if(handler->key_status[idx] == KEY_JUST_PRESSED) {
                handler->key_status[idx] = KEY_PRESSED;
            }
            else if(handler->key_status[idx] == KEY_RELEASED || handler->key_status[idx] == KEY_JUST_RELEASED) {
                handler->key_status[idx] = KEY_JUST_PRESSED;
            }
        }
        else {
            if(handler->key_status[idx] == KEY_JUST_PRESSED || handler->key_status[idx] == KEY_PRESSED) {
                handler->key_status[idx] = KEY_JUST_RELEASED;
            }
            else {
                handler->key_status[idx] = KEY_RELEASED;
            }
        }
    }
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED) && key >= 65  && key < 91) {
        int idx = key-KEY_A;
        if(action == GLFW_PRESS) {
            if(handler->key_status[idx] == KEY_JUST_PRESSED) {
                handler->key_status[idx] = KEY_PRESSED;
            }
            else if(handler->key_status[idx] == KEY_RELEASED || handler->key_status[idx] == KEY_JUST_RELEASED) {
                handler->key_status[idx] = KEY_JUST_PRESSED;
            }
        }
        else if (action == GLFW_REPEAT) {
            if(handler->key_status[idx] != KEY_PRESSED) {
                handler->key_status[idx] = KEY_PRESSED;
            }
        }
        else {
            if(handler->key_status[idx] == KEY_JUST_PRESSED || handler->key_status[idx] == KEY_PRESSED) {
                handler->key_status[idx] = KEY_JUST_RELEASED;
            }
            else {
                handler->key_status[idx] = KEY_RELEASED;
            }
        }
    }
}

void update_key_state(input_handler *ih) {
    for(int i = 0; i < NUM_KEYS; i++) {
        if(ih->key_status[i] == KEY_JUST_PRESSED) ih->key_status[i] = KEY_PRESSED;
        if(ih->key_status[i] == KEY_JUST_RELEASED) ih->key_status[i] = KEY_RELEASED;
    }
}

void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        handler->mouseX = xpos;
        handler->mouseY = ypos;
    }
}

int cursor_in_bounds(input_handler *ih) {
    if(ih->mouseX < 0 || ih->mouseX >= 800) return 0;
    if(ih->mouseY < 0 || ih->mouseY >= 600) return 0;
    return 1;
}

