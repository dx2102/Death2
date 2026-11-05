#pragma once
#include "data_path.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

// text pipeline
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <string>

#include <globals.h>
#include <story.h>
#include <runner.h>
#include <choice.h>
#include <memory.h>
#include <compiler.h>

#include <vector>

#define FONT_SIZE 28

struct Character {
    std::vector<unsigned char> pixels; // width*rows bytes, tightly packed
    unsigned int width = 0;
    unsigned int rows = 0;
    int bitmap_left = 0;
    int bitmap_top = 0;
    float x = 0.0f;   
    float y = 0.0f;
    char character;     // sanity check (is the char the same?)
};

struct Line {
    std::vector <Character> characters; 
};

struct TextProcessor {
    TextProcessor();
    ~TextProcessor();

    // Textbox sizes
    // don't want these to be fixed in case we want text to change
    // in different circumstances
    // e.g. dialogue vs. hints
    float textbox_width;
	float textbox_height;
    float pen_x_start;
    float pen_y_start;

    // drawing colour; defaults to white so existing callers are unaffected
    glm::vec3 color = glm::vec3(1.0f);

    // Freetype and Harbuff 
    FT_Library ft_library = nullptr; // handle to library
    FT_Face ft_face = nullptr;       // handle to face object 
    hb_font_t *hb_font = nullptr;   // created font

    Line curr_line; // stores the next Line to be printed

    // functions
    void handle_curr_line(std::string imported_line);
    void draw_curr_line(glm::uvec2 const &drawable_size);
    
    GLuint text_program = 0;
};



