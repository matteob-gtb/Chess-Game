//class to handle scrollwheel input through GLFW callbacks
#pragma once
#include "GLFW/glfw3.h"


namespace ScrollWheel{
    static float x_off = 0;
    static float y_off = 0;
};

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    const float scrollSpeed = 115.0f;

    ScrollWheel::x_off += xoffset * scrollSpeed;
    ScrollWheel::y_off += yoffset * scrollSpeed;
}
