#include "DialogueMode.hpp"

#include <iostream>
#include <string>

// Referenced https://github.com/JBenda/inkcpp for inky-c++ integration [RAHHH!]

DialogueMode::DialogueMode() {
	// inky
	// ink::compiler::run("test.json", "test.bin");
	inkDialogue = ink::runtime::story::from_file(data_path("prototype_script.bin").c_str());
	thread = inkDialogue->new_runner();

    can_battle = false;
}

DialogueMode::~DialogueMode() {
}

bool DialogueMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE){ // process dialogue
            talk.pressed = true;

			curr_line = ""; // restart line
            
			// can the runner continue?
			// yes - get the next line
			// no - at choice or end
			if ( thread->can_continue() ){ 
				curr_line = thread->getline();
				// check for tags
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::string tag = thread->get_tag(i);
						std::cout << "tag: " << tag << "\n" << std::endl;

                        // set can battle status to true
                        if (tag == "battle") {
                            can_battle = true;
							std::cout << "can battle: " << can_battle << std::endl;
                        }
					}
				}
				dialogue_processor.handle_curr_line(curr_line);
                return true;
			}
			else if (thread->has_choices()) { // display as 3 different choices
				// check for tags
				if (thread->has_tags()){
					std::cout << "tag avalible: " << "\n" << std::endl;

					for (ink::size_t i = 0; i < thread->num_tags(); ++i){
						std::string tag = thread->get_tag(i);
						std::cout << "tag: " << tag << "\n" << std::endl;

                        // set can battle status to true
                        if (tag == "battle") {
                            can_battle = true;
							std::cout << "can battle: " << can_battle << std::endl;
                        }
					}
				}
				// iterate choices, numbering them so the player can see which key to press
				int id = 0;
				for ( const ink::runtime::choice c : *thread )
				{
					id++;
					curr_line.append(std::to_string(id) + ". " + c.text() + "   ");
				}
				dialogue_processor.handle_curr_line(curr_line);
                return true;
			}
			else { // at end
				// this will be moved under the actual choices
				// depending on tags
				// e.g. choice 1 will end in tag "battle", triggers battle
				// choice 2 will end in tag "acceptance", increases patient acceptance of death
				if (can_battle) {
					Mode::set_current(std::move(return_to));
				}
			}
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
        // use this for choice stuff instead of hard coding?
        if (evt.key.key ==SDLK_SPACE) {
            talk.pressed = false;
			return true;
        }
	    if (evt.key.key == SDLK_1) { // choice 1
			if(thread->has_choices()){
				thread->choose(0);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		} else if (evt.key.key == SDLK_2) { // choice 2
			if(thread->has_choices()){
				thread->choose(1);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		} else if (evt.key.key == SDLK_3) { // choice 3
			if(thread->has_choices()){
				thread->choose(2);
				curr_line = thread->getline();
				dialogue_processor.handle_curr_line(curr_line);
			}
            return true;
		}
	}
	return false;
};

void DialogueMode::update(float elapsed) {
	//reset button press counters:
	talk.downs = 0;

	// keep ghost floating
	return_to->update(elapsed);
}

void DialogueMode::draw(glm::uvec2 const &drawable_size) {	
	// draw background sprites
	if (!can_battle) return_to->draw(drawable_size);

	float textbox_x_start = MARGIN;
	float textbox_y_start = MARGIN;
	float textbox_x_end = textbox_x_start + drawable_size.x - 2*MARGIN;
	float textbox_y_end = textbox_y_start + drawable_size.y/4;

	// draw textbox
	{
		glm::mat4 OBJECT_TO_CLIP = glm::mat4(
			glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f), // x
			glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f), // y
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), // z
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
		);

		DrawLines lines(OBJECT_TO_CLIP);

		lines.fill_rect(
			glm::vec2(textbox_x_start, textbox_y_start), // min
			glm::vec2(textbox_x_end, textbox_y_end), // max
			glm::u8vec4(20, 20, 30, 255) // colour
		);

		float line_thickness = 5.0f;

		// bottom line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_start), // point a
			glm::vec2(textbox_x_end, textbox_y_start), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// top line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_end), // point a
			glm::vec2(textbox_x_end, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// left line
		lines.fill_line( 
			glm::vec2(textbox_x_start, textbox_y_start), // point a
			glm::vec2(textbox_x_start, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		// right line
		lines.fill_line( 
			glm::vec2(textbox_x_end, textbox_y_start), // point a
			glm::vec2(textbox_x_end, textbox_y_end), // point b
			line_thickness, // width
			glm::u8vec4(0, 0, 0, 255) // colour
		);
	}

	// draw dialogue line
	dialogue_processor.textbox_width = textbox_x_end - textbox_x_start;
	dialogue_processor.textbox_height = textbox_y_end - textbox_y_start;
	dialogue_processor.pen_x_start = textbox_x_start + MARGIN;
	dialogue_processor.pen_y_start = textbox_y_end - MARGIN;
	dialogue_processor.draw_curr_line(drawable_size);

	// draw current speaker
	speaker_processor.textbox_width = 10*FONT_SIZE;
	speaker_processor.textbox_height = 2*FONT_SIZE;
	speaker_processor.pen_x_start = textbox_x_start;
	speaker_processor.pen_y_start = textbox_y_end;

	{
		glm::mat4 OBJECT_TO_CLIP = glm::mat4(
			glm::vec4(2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 2.0f / float(drawable_size.y), 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f) // bottom-left
		);

		DrawLines lines(OBJECT_TO_CLIP);

		float speaker_x_end = speaker_processor.pen_x_start + speaker_processor.textbox_width;
		float speaker_y_end = speaker_processor.pen_y_start + speaker_processor.textbox_height;

		lines.fill_rect(
			glm::vec2(speaker_processor.pen_x_start, speaker_processor.pen_y_start), // min
			glm::vec2(speaker_x_end, speaker_y_end), // max
			glm::u8vec4(0, 0, 0, 255) // colour
		);

		lines.fill_triangle(
			glm::vec3(speaker_x_end, speaker_processor.pen_y_start, 0), // point a
			glm::vec3(speaker_x_end + 50.0f, speaker_processor.pen_y_start, 0), // point b
			glm::vec3(speaker_x_end, speaker_y_end, 0), // point c
			glm::u8vec4(0, 0, 0, 255) // colour
		);
	}
	
	speaker_processor.pen_x_start = textbox_x_start + MARGIN/2;
	speaker_processor.pen_y_start = textbox_y_end + MARGIN/4;
	speaker_processor.handle_curr_line("SOMEBODY\n"); 
	speaker_processor.draw_curr_line(drawable_size);

	// draw hint (bottom-right)
	hint_processor.textbox_width = 10*FONT_SIZE;
	hint_processor.textbox_height = FONT_SIZE;
	hint_processor.pen_x_start = textbox_x_end - hint_processor.textbox_width;
	hint_processor.pen_y_start = textbox_y_start + hint_processor.textbox_height;
	hint_processor.handle_curr_line(thread->has_choices() ? "PRESS 1 2 3 TO CHOOSE\n" : "SPACE TO CONTINUE\n"); 
	hint_processor.draw_curr_line(drawable_size);
}