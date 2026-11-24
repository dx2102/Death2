#pragma once

#include "Mode.hpp"
#include "Sprites.hpp"
#include "PixelCanvas.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

struct PlayMode;

struct HallwayMode : Mode {
	HallwayMode();
	virtual ~HallwayMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, up, down, interact;

	glm::vec2 player_at = glm::vec2(120.0f, 24.0f);
	float hover_time = 0.0f;

	std::vector<std::string> hallway;

	struct Door {
		int number = 1;
		glm::vec2 pos;
	};
	std::vector<Door> doors;
	int nearby = -1;

	std::shared_ptr< PlayMode > rooms[5];

	bool tile_is_solid(int tx, int ty);
	bool can_move_to(glm::vec2 pos);
	void enter_door(int number);
	void reset_buttons();

	Sprites sprites;
	Sprites::Texture reaper_tex;
	Sprites::Texture floor_tex;
	Sprites::Texture interact_tex;
	Sprites::Texture vignette_tex;
	Sprites::Texture light_tex;

	float fade_out = 0.0f;
	std::shared_ptr< Mode > leaving_to;
	void start_leaving(std::shared_ptr< Mode > next);

	//hint box along the bottom: how to enter a room, or the ending once every soul is collected
	bool all_souls_taken();
	std::unique_ptr< Font > hint_font;
	std::string hint_texts[2];
	TextImage hint_images[2];
	void draw_hint_text(std::string const lines[2], int count, float first_baseline, glm::uvec2 const &drawable_size);
};