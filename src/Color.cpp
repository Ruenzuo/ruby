#include "Color.hpp"

Color::Color(uint32_t value) {
    red = ((GLubyte)(value & 0xff));
    green = ((GLubyte)((value >> 8) & 0xff));
    blue = ((GLubyte)((value >> 16) & 0xff));
}

Color::~Color() {

}