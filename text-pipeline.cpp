#include "data_path.hpp"

// font things
#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>
#include <algorithm>
#include <globals.h>
#include <story.h>
#include <runner.h>
#include <choice.h>
#include <memory.h>
#include <compiler.h>

#include <string>
#include <vector>
#include <iostream>

// rendering
#include "LitColorTextureProgram.hpp"
#include <glm/glm.hpp>
#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "text-pipeline.hpp"

// Referenced https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// Referenced https://freetype.org/freetype2/docs/tutorial/step1.html
// Referenced PPU466.cpp from project 1 to render bitmap textures

TextProcessor::TextProcessor(){
    // Abort if files not found/ not working
    if (FT_Init_FreeType( &ft_library )) {
        fprintf (stderr, "Could not find library\n");
        abort();
    }

    // https://www.1001fonts.com/lower-pixel-font.html
    // credit to IrbaCreative
	//look the font up next to the executable, like every other asset,
	//so the game works no matter which directory it is started from:
	std::string fontpath = data_path("LowresPixel-Regular.otf");
	const char *fontfile = fontpath.c_str();
    FT_Error error = FT_New_Face(ft_library, fontfile, 0, &ft_face);
    
    if (error) {
        std::cerr << "FT_New_Face failed: " << error
                << "\nFont path: " << fontfile << '\n';
        abort();
    }

    if ((FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0))){   
        fprintf (stderr, "Font size could not be set\n");
        abort();
    }

	// Create hb-ft font
    hb_font = hb_ft_font_create(ft_face, NULL);
	std::cout << "Font created\n";
}

TextProcessor::~TextProcessor(){
    if (hb_font) hb_font_destroy(hb_font);
    if (ft_face) FT_Done_Face(ft_face);
    if (ft_library) FT_Done_FreeType(ft_library);

}

/// @brief Takes an imported line from ink.bin file
///         And converts it into a Line
/// @param imported_line String from ink.bin
void TextProcessor::handle_curr_line(std::string imported_line) {
    // Create hb-buffer and populate
    hb_buffer_t *hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8 (hb_buffer, imported_line.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties (hb_buffer);

    // Shape text
    hb_shape(hb_font, hb_buffer, NULL, 0);

    // Get glyph info and pos out of the buffer
    unsigned int num_chars = hb_buffer_get_length (hb_buffer);
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);
    curr_line.characters.clear();
    float pen_x = pen_x_start;
    float pen_y = pen_y_start;

    for (unsigned int i = 0; i < num_chars; i++) { 
        // ignoring the null character
        // if i make i < num_chars-1 in the loopguard it crashes on comments in the ink file. 
        
        if (i == num_chars-1) continue;

        Character character;
        if (i == 0){
            // if guard for parsing 
        }

        FT_UInt glyph_index = info[i].codepoint;
        
        // Load glyph image into the slot (erase prev)
        if (FT_Load_Glyph (ft_face, glyph_index, FT_LOAD_DEFAULT)) {
            fprintf (stderr, "Error loading glyph image into slot\n");
            abort();
        }

        // Convert to an anti-aliased bitmap
        if (FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf (stderr, "Error converting glyph to an anti-aliased bitmap\n");
            abort();
        }
        FT_GlyphSlot slot = ft_face->glyph;

        // Get the character
        character.character = imported_line.at(i);
        character.width = slot->bitmap.width;
        character.rows = slot->bitmap.rows;
        character.bitmap_left = slot->bitmap_left;
        character.bitmap_top = slot->bitmap_top;
        character.x = pen_x + pos[i].x_offset/64.0f;
        character.y = pen_y + pos[i].y_offset/64.0f;

		// really bad text wrapping attempt
        if (character.x >= pen_x + textbox_width) {
            pen_x = pen_x_start;
            pen_y = pen_y_start - (pen_y_start/2);
        }

        character.pixels.resize(character.width*character.rows);
        
        for (unsigned int row = 0; row < character.rows; row++) {
            const unsigned char* src = slot->bitmap.buffer + row*slot->bitmap.pitch;
            std::copy(src, src + character.width, character.pixels.begin() + row*character.width);
        }

        curr_line.characters.emplace_back(std::move(character));

        pen_x += pos[i].x_advance/64.0f;
        pen_y += pos[i].y_advance/64.0f;
    }
    hb_buffer_destroy(hb_buffer);
}

void TextProcessor:: draw_curr_line(glm::uvec2 const &drawable_size){
    //set up screen scaling (ref. from Sprites.cpp)
	// float scale_x = drawable_size.x / ScreenWidth;
	// float scale_y = drawable_size.y / ScreenHeight;
	// float scale = scale_x;
	// if (scale_y < scale) {
	// 	scale = scale_y;
	// }
	// scale = std::floor(scale);
	// if (scale < 1.0f) {
	// 	scale = 1.0f;
	// }
	// int view_w = int(ScreenWidth * scale);
	// int view_h = int(ScreenHeight * scale);
	// int view_x = (int(drawable_size.x) - view_w) / 2;
	// int view_y = (int(drawable_size.y) - view_h) / 2;
	// glViewport(view_x, view_y, view_w, view_h);

    // text vertex shader
    const char *text_vertex_shader = R"(
        #version 330

        layout(location = 0) in vec2 Position;
        layout(location = 1) in vec2 TexCoord;

        out vec2 texCoord;

        uniform mat4 OBJECT_TO_CLIP;

        void main() {
            gl_Position = OBJECT_TO_CLIP * vec4(Position, 0.0, 1.0);
            texCoord = TexCoord;
        }
    )";

    // text fragment shader
    const char *text_fragment_shader = R"(
        #version 330

        in vec2 texCoord;

        uniform sampler2D glyphTexture;
        uniform vec3 textColor;

        out vec4 fragColor;

        void main() {
            float alpha = texture(glyphTexture, texCoord).r;
            fragColor = vec4(textColor, alpha);
        }
    )";

	if (text_program == 0) {
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &text_vertex_shader, nullptr);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &text_fragment_shader, nullptr);
		glCompileShader(fs);
		text_program = glCreateProgram();
		glAttachShader(text_program, vs);
		glAttachShader(text_program, fs);
		glLinkProgram(text_program);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

    for (unsigned int i = 0; i < curr_line.characters.size(); i++) {

		auto const &character = curr_line.characters.at(i);
		if (character.pixels.empty()) continue; 

		// // For testing
		// printf("glyph index: %u\n", glyph_index);
		// printf("bitmap: %d x %d\n", character.width, character.rows);
		// printf("pitch: %d\n", slot->bitmap.pitch);
		// printf("pixel mode: %d\n", slot->bitmap.pixel_mode);
		// printf("bitmap_left: %d, bitmap_top: %d\n", slot->bitmap_left, slot->bitmap_top);

		// Built off of ideas in PPU466.cpp from project 1 
		// Create texture for glyph
		GLuint glyph_tex = 0;

		glGenTextures(1, &glyph_tex);
		glBindTexture(GL_TEXTURE_2D, glyph_tex);

		// Freetype's bitmap is one byte per pixel
		// Keep in mind: 1 alpha channel
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//passing 'nullptr' to TexImage says "allocate memory but don't store anything there":
		// (textures will be uploaded later)
		// GL_RGBA8 to GL_R8 b/c 1 colour channel instead of 4
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, character.width, character.rows, 0, GL_RED, GL_UNSIGNED_BYTE, character.pixels.data());
		//make the texture have sharp pixels when magnified:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//when access past the edge, clamp to the edge:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// vertex
		struct Vertex {
			glm::vec2 Position;
			glm::vec2 TexCoord;
		};

		// need to account for the bitmap starting points
		float pen_x_offset = character.x + character.bitmap_left;
		float pen_y_offset = character.y + character.bitmap_top - float(character.rows);

		// build rectangle representing background and sprites (of bitmap):
		std::vector<Vertex> vertices = {
			// triangle 1
			{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset), glm::vec2(1.0f, 1.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)},
			// triangle 2
			{glm::vec2(pen_x_offset, pen_y_offset), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(pen_x_offset, pen_y_offset + character.rows), glm::vec2(0.0f, 0.0f)},
			{glm::vec2(pen_x_offset + character.width, pen_y_offset + character.rows), glm::vec2(1.0f, 0.0f)}
		};

		//vertex_buffer will (eventually) hold vertex data for drawing:
		GLuint vertex_buffer = 0;
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// vao
		GLuint vertex_array = 0;
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		// attribute array for position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

		// attribute array for text coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glUseProgram(text_program);

		// uniforms for shader programs
		glm::mat4 OBJECT_TO_CLIP = glm::mat4(
			glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f)
		);
		GLint object_to_clip = glGetUniformLocation(text_program, "OBJECT_TO_CLIP");
		glUniformMatrix4fv(object_to_clip, 1, GL_FALSE, glm::value_ptr(OBJECT_TO_CLIP));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glyph_tex);

		GLint glyph_texture = glGetUniformLocation(text_program, "glyphTexture");
		glUniform1i(glyph_texture, 0);

		glUniform3fv(glGetUniformLocation(text_program, "textColor"), 1, glm::value_ptr(color));

		glBindVertexArray(vertex_array);

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteVertexArrays(1, &vertex_array);
		glDeleteBuffers(1, &vertex_buffer);
		glDeleteTextures(1, &glyph_tex);
    }
    GL_ERRORS();
}

