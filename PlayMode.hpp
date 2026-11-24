#pragma once

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

#include "DialogueMode.hpp"

#include "PixelCanvas.hpp"
#include <map>
#include <memory>

// #define FONT_SIZE 36
// #define MARGIN (FONT_SIZE * 0.5)
#include <string>

struct SoulCaptureMode;

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
	} left, right, down, up, interact, advance;

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

	glm::vec2 player_at = glm::vec2(120.0f, 20.0f);
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
	Sprites::Texture floor_tex;
	Sprites::Texture patient1_tex;
	Sprites::Texture man1_tex;
	Sprites::Texture woman1_tex;
	Sprites::Texture partition_tex;
	Sprites::Texture cardiogram_tex;
	Sprites::Texture iv_tex;
	Sprites::Texture desk_tex;
	Sprites::Texture interact_tex;
	Sprites::Texture vignette_tex;
	Sprites::Texture light_tex;
	std::vector< glm::vec2 > lights;
	Sprites::Texture soul_tex;
	glm::vec2 soul_pos = glm::vec2(128.0f, 200.0f);
	Sprites::Texture patient_dead_tex;
	Sprites::Texture cardiogram_dead_tex;

	struct Prop {
		Sprites::Texture tex;
		glm::vec2 pos;
		char kind = '.';
	};
	std::vector<Prop> props;
	Sprites::Texture texture_for(Prop const &prop);
	bool soul_taking = false;

    bool diary_read = false;         
	bool soul_taken = false;
	std::shared_ptr< DialogueMode > active_dialogue;
	std::shared_ptr< SoulCaptureMode > active_minigame;
	void start_minigame();

	void reset_buttons();

	PlayMode(int room_number);
	int room_number = 1;
	std::shared_ptr< Mode > return_to;

	float fade_out = 0.0f;
	std::shared_ptr< Mode > leaving_to;
	void start_leaving(std::shared_ptr< Mode > next);

	float bed_center_x = 128.0f;

	void draw_window();

	enum TutorialStep {
		TUT_OFF,
		TUT_INTRO,
		TUT_MOVE,
		TUT_SOUL,
		TUT_TALK,
		TUT_FIRST_TRY,
		TUT_DIARY,
		TUT_FIND_DIARY,
		TUT_READ_DIARY,
		TUT_EXPLAIN,
		TUT_TALK_AGAIN,
		TUT_SECOND_TRY,
		TUT_FINISH,
		TUT_DONE,
	};
	TutorialStep tutorial_step = TUT_OFF;
	bool minigame_seen = false;
	int tutorial_page = 0;
	float moved_distance = 0.0f;
	float hint_alpha = 0.0f;
	std::string shown_hint;
	float spotlight_alpha = 0.0f;
	bool spotlight_on_diary = false;
	glm::vec2 diary_pos = glm::vec2(0.0f);

	//a short message in the hint box, e.g. after reading the diary:
	std::string note;
	float note_time = 0.0f;
	void show_note(std::string const &text, float seconds = 2.5f) { note = text; note_time = seconds; }

	bool tutorial_page_lines(int page, std::string lines[2]);
	void finish_pages();
	std::string tutorial_hint();
	bool is_spotlight_step();
	void draw_hint_box(std::string const lines[], int count, bool show_continue, float alpha);
	void draw_tutorial();
	void draw_soul();
	bool in_tutorial();  

	std::unique_ptr< Font > hint_font;
	std::map< std::string, TextImage > hint_cache;
	struct HintLine {
		std::string text;
		float center_x;
		float baseline_y;
		float alpha;
	};
	std::vector< HintLine > hint_lines;
	void draw_hint_text(glm::uvec2 const &drawable_size);
	//----- text stuff -----
	TextProcessor hint_processor;
	std::string dialogue_line;
	std::string hint_line;

	bool talked_patient = false;

	//----- sound stuff -----
	//swap the background track, fading the old one out and the new one in:
	void play_music(Sound::Sample const &sample, std::shared_ptr< Sound::PlayingSample > &slot);
	std::shared_ptr< Sound::PlayingSample > before_soul_loop;
	std::shared_ptr< Sound::PlayingSample > during_soul_loop;
	std::shared_ptr< Sound::PlayingSample > after_soul_loop;
};