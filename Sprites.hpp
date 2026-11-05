#pragma once

#include "GL.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Sprites {
	static constexpr float ScreenWidth = 256.0f;
	static constexpr float ScreenHeight = 240.0f;

	struct Texture {
		GLuint id = 0;
		glm::uvec2 size = glm::uvec2(0);
	};

	Sprites();
	~Sprites();

	Sprites(Sprites const &) = delete;
	Sprites &operator=(Sprites const &) = delete;

	Texture load(std::string const &filename);
	Texture white;

	void begin(glm::uvec2 const &drawable_size);

	void draw(Texture const &tex, glm::vec2 pos, glm::vec2 size,
		glm::vec4 color = glm::vec4(1.0f), float angle = 0.0f);

	void end(glm::uvec2 const &drawable_size);

	//same batching, but drawing into whatever coordinate system the caller is using:
	void begin_world(glm::mat4 const &world_to_clip);
	void end_world();

	private:
	struct Vertex {
		glm::vec2 pos;
		glm::vec2 uv;
		glm::vec4 color;
	};

	GLuint program = 0;
	GLuint vao = 0;
	GLuint vbo = 0;

	std::vector<Vertex> batch;
	GLuint batch_tex = 0;
	std::vector<GLuint> loaded;

	void flush();

	bool snap_to_pixels = true;
};