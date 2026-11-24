#include "DialogueMode.hpp"
#include "SoulCaptureMode.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include <cmath>
#include <algorithm>

// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]


std::shared_ptr<InkStory> InkStory::get(std::string const &script_file) {
	static std::unordered_map< std::string, std::shared_ptr<InkStory> > stories;

	auto found = stories.find(script_file);
	if (found != stories.end()) return found->second;

	auto s = std::make_shared<InkStory>();
	s->story = ink::runtime::story::from_file(data_path(script_file).c_str());
	s->globals = s->story->new_globals();
	s->runner = s->story->new_runner(s->globals); // also initializes the globals
	stories.emplace(script_file, s);
	return s;
}

// strip leading/trailing whitespace
static std::string trim(std::string const &s) {
	size_t begin = s.find_first_not_of(" \t\r\n");
	if (begin == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(begin, end - begin + 1);
}

DialogueMode::DialogueMode(std::string const &script_file) {
	ink = InkStory::get(script_file);
	ink->paused = false;

	// player decided to retreat.
	// story stays where it is until battle is done.
	resume_battle = ink->battle_pending;

	auto &thread = ink->runner;
	if (!resume_battle && !thread->can_continue() && !thread->has_choices()) {
		if (!thread->move_to(ink->next_patient.c_str())) {
			std::cerr << "DialogueMode: resume knot '" << ink->next_patient << "' not found in " << script_file << std::endl;
		}
	}
}

DialogueMode::~DialogueMode() {
}

//parse the tags and sets the variables for control flow. 
void DialogueMode::process_tags() {
	auto &thread = ink->runner;
	for (ink::size_t i = 0; i < thread->num_tags(); ++i) {
		std::string tag = trim(thread->get_tag(i));
		std::cout << "tag: " << tag << std::endl;

		if (tag == "battle") {
			can_battle = true;
		} else if (tag == "pause") {
			pause_pending = true;
		}
	}

	// In final_script the patient knots put "# battle" on its own line after the last
	// line of text, and ink drops a tag that has no line to attach to. Until the script
	// is fixed, treat the last line before each battle as if it had the tag.
	static const char *battle_lines[] = {
		"\"I... I must finish.\"", // abigail
		"\"You... must be Satan. God cannot look like this\"", // sherry
		"\"Bro actually, I kinda dont gaf, can we just get this over with?\"", // keith
		"\"this aint an airport.\"", // david
		"\"...Goodbye Luca.\"", // twins
		"\"I hope he won't be mad at me for this.\"",
		"\"Im sorry that I couldn't do anything.\"",
		"\"LUCA PLEASE DON'T LEAVE ME...!\"",
		"A human cannot interfere with my job.",
	};
	std::string line = trim(curr_line);
	for (const char *b : battle_lines) {
		if (line == b) can_battle = true;
	}
}

void DialogueMode::leave() {
	auto self = shared_from_this();
	std::shared_ptr<Mode> back = return_to;
	Mode::set_current(back);
}

void DialogueMode::start_battle() {
	//the soul is currently being captured so the battle counts as unfinished
	ink->battle_pending = true;

	// difficulty comes from the script 
	//ink doesnt use unsigned so casting hell is below. 
	//for a failsafe, lets make 3 be the default
	int32_t level = 3; 
	auto difficulty = ink->get_var<int32_t>("battle_difficulty");
	if (difficulty.has_value()) level = *difficulty;
	level = std::max<int32_t>(1, std::min<int32_t>(int32_t(SoulCaptureMode::Levels), level));

	auto minigame = std::make_shared<SoulCaptureMode>(SoulCaptureMode::level(uint32_t(level)));
	//come back to this dialogue
	minigame->return_to = shared_from_this();
	if (setup_minigame) setup_minigame(*minigame);

	// the minigame calls this right before handing control back; update() acts on it
	std::weak_ptr<Mode> weak_self = shared_from_this();
	minigame->did_retreat = [weak_self](bool captured) {
		auto self = std::static_pointer_cast< DialogueMode >(weak_self.lock());
		if (!self) return;
		self->battle_finished = true;
		self->battle_captured = captured;
	};

	Mode::set_current(minigame);
}

void DialogueMode::choose(int i) {
	auto &thread = ink->runner;
	if (!thread->has_choices() || i >= int(thread->num_choices())) return;

	showing_choices = false;
	thread->choose(i);
	curr_line = thread->can_continue() ? thread->getline() : "";
	process_tags();

	//handles battles right after a choice is made. 
	if (can_battle && trim(curr_line).empty()) {
		can_battle = false;
		start_battle();
		return;
	}

	// clear the old text (the choice list) before showing the next line
	dialogue_processor.reset_line(last_drawable_size);
	dialogue_processor.handle_curr_line(curr_line);
}

//show the next line (what Space does), or start the battle / close the dialogue
void DialogueMode::advance() {
	auto &thread = ink->runner;

	//Space on the choice list picks choice 1, so a player who only presses Space never gets stuck:
	if (showing_choices && thread->has_choices()) {
		choose(0);
		return;
	}

	if (can_battle) {
		can_battle = false;
		start_battle();
		return;
	}

	if (pause_pending) {
		pause_pending = false;
		ink->paused = true;
		leave(); //close the dialogue
		return;
	}

	curr_line = ""; // restart line
	dialogue_processor.reset_line(last_drawable_size);

	// can the runner continue?
	// yes - get the next line
	// no - at choice or end
	if ( thread->can_continue() ){
		curr_line = thread->getline();
		process_tags();

		//handles if the battle tag isnt attached to text
		if (can_battle && trim(curr_line).empty()) {
			can_battle = false;
			start_battle();
			return;
		}

		// nothing to show: either a pause tag with no text, or the story
		// close right away when hit ->DONE or ->END
		bool stopped = !thread->can_continue() && !thread->has_choices();
		if (trim(curr_line).empty() && (pause_pending || stopped)) {
			pause_pending = false;
			ink->paused = true;
			leave();
			return;
		}

		//if there is an empty line before choice  show the choices instead of a blank box
		if (!(trim(curr_line).empty() && thread->has_choices())) {
			dialogue_processor.handle_curr_line(curr_line);
			return;
		}
	}

	if (thread->has_choices()) {
		// one choice per line, numbered to match the 1/2/3 keys
		curr_line = "";
		int number = 1;
		for ( const ink::runtime::choice c : *thread ) {
			curr_line.append(std::to_string(number) + ". " + c.text() + "\n");
			number++;
		}
		dialogue_processor.handle_curr_line(curr_line);
		showing_choices = true;
	}
	else {
		leave();
	}
}

bool DialogueMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	(void)window_size;
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE){ // process dialogue
			talk.pressed = true;
			advance();
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_SPACE) {
			talk.pressed = false;
			return true;
		}
		if (evt.key.key == SDLK_1) { 
			choose(0);
			return true;
		}
		if (evt.key.key == SDLK_2) { 
			choose(1);
			return true; 
		}
		if (evt.key.key == SDLK_3) { 
			choose(2);
			return true;
		}
	}
	return false;
};

void DialogueMode::update(float elapsed) {
	//reset button press counters:
	talk.downs = 0;

	if (resume_battle) {
		resume_battle = false;
		start_battle();
		return;
	}

	// back from the minigame
	if (battle_finished) {
		battle_finished = false;
		if (battle_captured) {
			ink->battle_pending = false;
			if (on_capture) on_capture();
		} 
		else {
			//retreat
			leave();
		}
		return;
	}

	//show the first line right away instead of an empty box. This waits until the box has
	//been drawn once, because the text texture is made at the size of the drawable:
	if (!started && last_drawable_size.x > 1) {
		started = true;
		advance();
		if (Mode::current.get() != this) return; //the dialogue closed or a battle started
	}

	// keep ghost floating
	return_to->update(elapsed);

	// timer for text effect
	dialogue_processor.curr_line.typing_elapsed += elapsed;
	// std::cout << "dialogue_processor.curr_line.typing_elapsed = " << dialogue_processor.curr_line.typing_elapsed << std::endl;
		
}

void DialogueMode::draw(glm::uvec2 const &drawable_size) {	
	// background --------------------------------------------------------------------------------------------
	return_to->draw(drawable_size);

	// textbox dimensions
	last_drawable_size = drawable_size;

	float scale = std::floor(std::min(drawable_size.x / 256.0f, drawable_size.y / 240.0f));
	if (scale < 1.0f) {
		scale = 1.0f;
	}
	float view_x = std::floor((drawable_size.x - 256.0f * scale) * 0.5f);
	float view_y = std::floor((drawable_size.y - 240.0f * scale) * 0.5f);

	unsigned int font_pixels = (unsigned int)(9.0f * scale);
	if (font_pixels != dialogue_font_pixels) {
		FT_Set_Pixel_Sizes(dialogue_processor.ft_face, 0, font_pixels);
		hb_ft_font_changed(dialogue_processor.hb_font);
		dialogue_font_pixels = font_pixels;
	}

	float textbox_x_start = view_x + 8.0f * scale;
	float textbox_y_start = view_y + 8.0f * scale;
	float textbox_x_end = view_x + (256.0f - 8.0f) * scale;
	float textbox_y_end = view_y + 64.0f * scale;

	// // speaker dimensions
	// speaker_processor.textbox_width = 10*FONT_SIZE;
	// speaker_processor.textbox_height = 2*FONT_SIZE;
	// speaker_processor.pen_x_start = textbox_x_start;
	// speaker_processor.pen_y_start = textbox_y_end;

	dialogue_processor.textbox_width = (textbox_x_end - textbox_x_start) - 16.0f * scale;
	dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	dialogue_processor.pen_x_start = textbox_x_start + 8.0f * scale;
	dialogue_processor.pen_y_start = textbox_y_end - 14.0f * scale;
	dialogue_processor.curr_line.is_typing = true;
	dialogue_processor.draw_curr_line(textbox_x_start, textbox_y_start, textbox_x_end, textbox_y_end, drawable_size);

	//speaker name on a dark tag just above the box ("Someone" is the narrator, so no tag)
	auto speaker = ink->get_var<const char *>("speaker");
	std::string name = speaker.has_value() ? std::string(*speaker) : "";
	if (!name.empty() && name != "Someone" && !dialogue_processor.curr_line.characters.empty()) {
		uint32_t pixels = uint32_t(9.0f * scale);
		bool new_font = !name_font || name_font->pixel_size != pixels;
		if (new_font) name_font = std::make_unique< Font >(data_path("LowresPixel-Regular.otf"), pixels);
		if (new_font || name_text != name) {
			name_image = render_text(*name_font, name);
			name_text = name;
		}
		glm::mat4 to_clip = glm::mat4(
			2.0f / drawable_size.x, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / drawable_size.y, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f
		);
		glm::vec2 tag_min = glm::vec2(textbox_x_start, textbox_y_end);
		glm::vec2 tag_max = tag_min + glm::vec2(float(name_image.advance) + 12.0f * scale, 14.0f * scale);
		{
			DrawLines lines(to_clip);
			lines.fill_rect(tag_min, tag_max, glm::u8vec4(20, 20, 30, 255));
		}
		draw_text(name_image, tag_min + glm::vec2(6.0f * scale, 4.0f * scale), glm::vec4(1.0f, 0.85f, 0.5f, 1.0f), drawable_size);
	}

	// // dialogue --------------------------------------------------------------------------------------------
	// float textbox_x_start = MARGIN;
	// float textbox_y_start = MARGIN;
	// float textbox_x_end = textbox_x_start + drawable_size.x - 2*MARGIN;
	// float textbox_y_end = textbox_y_start + drawable_size.y/4;
	// {
	// 	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
	// 		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f), // x
	// 		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f), // y
	// 		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), // z
	// 		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
	// 	);

	// 	DrawLines lines(OBJECT_TO_CLIP);

	// 	lines.fill_rect(
	// 		glm::vec2(textbox_x_start, textbox_y_start), // min
	// 		glm::vec2(textbox_x_end, textbox_y_end), // max
	// 		glm::u8vec4(20, 20, 30, 255) // colour
	// 	);

	// 	float line_thickness = 5.0f;

	// 	// bottom line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_start), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// top line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_end), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// left line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_start, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_start, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	// right line
	// 	lines.fill_line( 
	// 		glm::vec2(textbox_x_end, textbox_y_start), // point a
	// 		glm::vec2(textbox_x_end, textbox_y_end), // point b
	// 		line_thickness, // width
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);
	// }

	// // we can put all of this into one texture :3

	// // draw dialogue line
	// dialogue_processor.textbox_width = textbox_x_end - textbox_x_start;
	// dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	// dialogue_processor.pen_x_start = textbox_x_start + MARGIN;
	// dialogue_processor.pen_y_start = textbox_y_end - MARGIN;
	// dialogue_processor.draw_curr_line(drawable_size);

	// // draw current speaker
	// speaker_processor.textbox_width = 10*FONT_SIZE;
	// speaker_processor.textbox_height = 2*FONT_SIZE;
	// speaker_processor.pen_x_start = textbox_x_start;
	// speaker_processor.pen_y_start = textbox_y_end;

	// {
	// 	glm::mat4 OBJECT_TO_CLIP = glm::mat4(
	// 		glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
	// 		glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
	// 		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
	// 		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
	// 	);

	// 	DrawLines lines(OBJECT_TO_CLIP);

	// 	float speaker_x_end = speaker_processor.pen_x_start + speaker_processor.textbox_width;
	// 	float speaker_y_end = speaker_processor.pen_y_start + speaker_processor.textbox_height;

	// 	lines.fill_rect(
	// 		glm::vec2(speaker_processor.pen_x_start, speaker_processor.pen_y_start), // min
	// 		glm::vec2(speaker_x_end, speaker_y_end), // max
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);

	// 	lines.fill_triangle(
	// 		glm::vec3(speaker_x_end, speaker_processor.pen_y_start, 0), // point a
	// 		glm::vec3(speaker_x_end + 50.0f, speaker_processor.pen_y_start, 0), // point b
	// 		glm::vec3(speaker_x_end, speaker_y_end, 0), // point c
	// 		glm::u8vec4(0, 0, 0, 255) // colour
	// 	);
	// }
	
	// speaker_processor.pen_x_start = textbox_x_start + MARGIN/2;
	// speaker_processor.pen_y_start = textbox_y_end + MARGIN/4;
	// speaker_processor.handle_curr_line("SOMEBODY\n"); 
	// speaker_processor.draw_curr_line(drawable_size);

	// // draw hint (bottom-right)
	// hint_processor.textbox_width = 10*FONT_SIZE;
	// hint_processor.textbox_height = FONT_SIZE;
	// hint_processor.pen_x_start = textbox_x_end - hint_processor.textbox_width;
	// hint_processor.pen_y_start = textbox_y_start + hint_processor.textbox_height;
	// hint_processor.handle_curr_line("SPACE TO CONTINUE\n"); 
	// hint_processor.draw_curr_line(drawable_size);
}