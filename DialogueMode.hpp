#pragma once
#include "Mode.hpp"


#include "Scene.hpp"
#include "Sound.hpp"
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

#include "text-pipeline.hpp"
#include "PixelCanvas.hpp"
#include "DrawLines.hpp"

#include <memory>
#include <functional>

struct SoulCaptureMode;

// the ink story for the entire game. 
struct InkStory {
	ink::runtime::story *story = nullptr;
	//variables set in the story
	ink::runtime::globals globals;
	//the story thread
	ink::runtime::runner runner;
	// handling the pause tag.
	bool paused = false;
	// a "# battle" tag was read but the soul hasn't been captured yet (e.g. the player
	// retreated with Escape). the next dialogue that opens goes straight into the battle.
	bool battle_pending = false;
	//whose dialogue the story is in, so a battle left pending in one room is not started in another
	std::string battle_patient;
	//the knot that handles the next patient
	std::string next_patient = "next_patient";

	// script loading. 
	static std::shared_ptr<InkStory> get(std::string const &script_file);

	// reading and writing the ink variables
	// InkStory::get("prototype_script.bin")->set_var<bool>("start_abigail", true);
	template<typename T>
	bool set_var(const char *name, T const &val) {
		bool ok = globals->set<T>(name, val);
		if (!ok) std::cerr << "InkStory: could not set ink variable '" << name << "' (missing or wrong type)" << std::endl;
		return ok;
	}
	template<typename T>
	ink::optional<T> get_var(const char *name) const {
		return globals->get<T>(name);
	}
};

struct DialogueMode : Mode {
	DialogueMode(std::string const &script_file);
	virtual ~DialogueMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	// virtual bool handle_dialogue(SDL_Event const &, glm::uvec2 const &window_size);
	// virtual void update_dialogue(float elapsed);
	// virtual void draw_dialogue(glm::uvec2 const &drawable_size);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} talk;

	//----- text stuff -----

	PixelCanvas canvas;
	TextProcessor dialogue_processor;
	float typing_elapsed;
	float typing_max = 2;

	// textbox size scales with screen size
	// pen position dependent on textbox
	float textbox_width;
	float textbox_height;
    float dialogue_pen_x_start; 
	float dialogue_pen_y_start;

	TextProcessor hint_processor;
	TextProcessor speaker_processor;

	//----- tags stuff -----
	bool can_battle = false;    //the line on screen has "# battle": next input starts the minigame
	bool pause_pending = false; //next input closes dialogue

	//read the current line's tags (call after every getline())
	void process_tags();
	//pick choice i, then show the line that follows it
	void choose(int i);
	//show the next line, what Space does
	void advance();
	bool started = false; //the first line has been shown
	bool showing_choices = false; //the choice list is on screen: Space picks the first one
	//close dialogue
	void leave();

	//----- battle stuff -----
	//switch to the soul capture minigame, difficulty from the ink VAR battle_difficulty
	void start_battle();
	bool resume_battle = false;   //opened while ink->battle_pending: start the battle on first update
	bool battle_finished = false; //minigame just handed control back (handled in update)
	bool battle_captured = false; //...and whether the soul was captured
	//called when a soul is captured (PlayMode uses it to set soul_taken)
	std::function< void() > on_capture;
	std::function< void(SoulCaptureMode &) > setup_minigame;
	//----- story stuff -----

	std::shared_ptr<InkStory> ink;

	// Keep track of current line
	std::string curr_line;

	std::shared_ptr< Mode > return_to;

	bool is_typing = false;

	glm::uvec2 last_drawable_size = glm::uvec2(1, 1);
	unsigned int dialogue_font_pixels = 0;
};
