#include "Mode.hpp"
#include "Sprites.hpp"

#include <glm/glm.hpp>

#include <vector>
#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <deque>

// text pipeline
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <string>

// #define FONT_SIZE 36
// #define MARGIN (FONT_SIZE * 0.5)
#include <string>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	static constexpr float ScreenWidth = 176.0f;
	static constexpr float ScreenHeight = 160.0f;


	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, interact;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

	// Initialize library and load font face
    FT_Library ft_library = nullptr; // handle to library
    FT_Face ft_face = nullptr;       // handle to face object

	// Create hb-ft font
    hb_font_t *hb_font = nullptr;

	// Story variables
	int current_line = 0;

	glm::vec2 player_at = glm::vec2(128.0f, 60.0f);
	float hover_time = 0.0f;

	std::vector<std::string> room;

	struct Interactable {
		std::string name;
		glm::vec2 pos;
		float radius;
	};
	std::vector<Interactable> interactables;
	int nearby = -1;

	bool tile_is_solid(int tx, int ty);
	bool can_move_to(glm::vec2 pos);
	void on_interact(Interactable const &thing);

	Sprites sprites;
	Sprites::Texture reaper_tex;
	Sprites::Texture bed_tex;
	glm::vec2 bed_pos = glm::vec2(0.0f);
	Sprites::Texture desk_tex;
	glm::vec2 desk_pos = glm::vec2(0.0f);
	Sprites::Texture floor_tex;
};