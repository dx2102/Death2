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

#define MARGIN (FONT_SIZE * 2)

struct DialogueMode : Mode {
	DialogueMode();
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
	// textbox size scales with screen size
	// pen position dependent on textbox
	float textbox_width;
	float textbox_height;
    float dialogue_pen_x_start; 
	float dialogue_pen_y_start;

	TextProcessor hint_processor;
	TextProcessor speaker_processor;

	//----- tags stuff -----
	bool can_battle;

	//----- story stuff -----

	// Load ink binary story, generated from the inkCPP compiler
	ink::runtime::story* inkDialogue;

	// Create a new thread
	ink::runtime::runner thread;

	// Keep track of current line
	std::string curr_line;

	std::shared_ptr< Mode > return_to;
};
