#include "Sprites.hpp"

#include "gl_errors.hpp"
#include "gl_compile_program.hpp"
#include "load_save_png.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstddef>

//Batching: queue quads in a CPU-side vertex list, flush when the texture changes or at end of frame. Structure based on:
//https://jasonliang.js.org/batch-renderer.html
//https://gamedev.net/tutorials/3900/

//Buffer re-upload with glBufferData:
//https://www.opengl.org/wiki/Buffer_Object_Streaming

Sprites::Sprites() {
	program = gl_compile_program(
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"layout(location=0) in vec2 Position;\n"
		"layout(location=1) in vec2 TexCoord;\n"
		"layout(location=2) in vec4 Color;\n"
		"out vec2 texCoord;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	gl_Position = OBJECT_TO_CLIP * vec4(Position, 0.0, 1.0);\n"
		"	texCoord = TexCoord;\n"
		"	color = Color;\n"
		"}\n"
		,

		"#version 330\n"
		"uniform sampler2D Tex;\n"
		"in vec2 texCoord;\n"
		"in vec4 color;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = texture(Tex, texCoord) * color;\n"
		"	if (fragColor.a < 0.01) {\n"
		"		discard;\n"
		"	}\n"
		"}\n"
	);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glm::u8vec4 white_pixel = glm::u8vec4(255, 255, 255, 255);
	glGenTextures(1, &white.id);
	glBindTexture(GL_TEXTURE_2D, white.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white_pixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	white.size = glm::uvec2(1, 1);
	loaded.push_back(white.id);

	GL_ERRORS();
}

Sprites::~Sprites() {
	for (size_t i = 0; i < loaded.size(); i++) {
		glDeleteTextures(1, &loaded[i]);
	}
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
}

Sprites::Texture Sprites::load(std::string const &filename) {
	Texture tex;
	std::vector<glm::u8vec4> data;
	load_png(filename, &tex.size, &data, LowerLeftOrigin);

	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.size.x, tex.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	loaded.push_back(tex.id);
	return tex;
}

void Sprites::begin(glm::uvec2 const &drawable_size) {
	glViewport(0, 0, drawable_size.x, drawable_size.y);
	glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float scale_x = drawable_size.x / ScreenWidth;
	float scale_y = drawable_size.y / ScreenHeight;
	float scale = scale_x;
	if (scale_y < scale) {
		scale = scale_y;
	}
	scale = std::floor(scale);
	if (scale < 1.0f) {
		scale = 1.0f;
	}
	int view_w = int(ScreenWidth * scale);
	int view_h = int(ScreenHeight * scale);
	int view_x = (int(drawable_size.x) - view_w) / 2;
	int view_y = (int(drawable_size.y) - view_h) / 2;
	glViewport(view_x, view_y, view_w, view_h);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program);
	glm::mat4 projection = glm::ortho(0.0f, ScreenWidth, 0.0f, ScreenHeight);
	glUniformMatrix4fv(glGetUniformLocation(program, "OBJECT_TO_CLIP"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(program, "Tex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	batch.clear();
	batch_tex = 0;
}

void Sprites::draw(Texture const &tex, glm::vec2 pos, glm::vec2 size, glm::vec4 color, float angle) {
  //switching textures: draw everything queued
	if (tex.id != batch_tex) {
		flush();
		batch_tex = tex.id;
	}

	if (snap_to_pixels) {
		pos = glm::floor(pos);
	}

	glm::vec2 center = pos + size * 0.5f;
	glm::vec2 half = size * 0.5f;

	float c = std::cos(angle);
	float s = std::sin(angle);
	glm::vec2 right = glm::vec2(c, s) * half.x;
	glm::vec2 up = glm::vec2(-s, c) * half.y;

	glm::vec2 bl = center - right - up;
	glm::vec2 br = center + right - up;
	glm::vec2 tl = center - right + up;
	glm::vec2 tr = center + right + up;

	batch.push_back({ bl, glm::vec2(0.0f, 0.0f), color });
	batch.push_back({ br, glm::vec2(1.0f, 0.0f), color });
	batch.push_back({ tr, glm::vec2(1.0f, 1.0f), color });
	batch.push_back({ bl, glm::vec2(0.0f, 0.0f), color });
	batch.push_back({ tr, glm::vec2(1.0f, 1.0f), color });
	batch.push_back({ tl, glm::vec2(0.0f, 1.0f), color });
}

void Sprites::end(glm::uvec2 const &drawable_size) {
	flush();
	batch_tex = 0;

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);
	glViewport(0, 0, drawable_size.x, drawable_size.y);

	GL_ERRORS();
}

void Sprites::begin_world(glm::mat4 const &world_to_clip) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "OBJECT_TO_CLIP"), 1, GL_FALSE, glm::value_ptr(world_to_clip));
	glUniform1i(glGetUniformLocation(program, "Tex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	batch.clear();
	batch_tex = 0;
	snap_to_pixels = false;
}

void Sprites::end_world() {
	flush();
	batch_tex = 0;

	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);

	snap_to_pixels = true;
}

void Sprites::flush() {
	if (batch.empty()) {
		return;
	}
	glBindTexture(GL_TEXTURE_2D, batch_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(Vertex), batch.data(), GL_STREAM_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(batch.size()));
	batch.clear();
}
