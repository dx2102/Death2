#include "RoomMode.hpp"

#include "DrawLines.hpp"

#include <iostream>
#include <cmath>
#include <cstring>

#include "gl_errors.hpp"
#include "Load.hpp"

#include <glm/gtc/type_ptr.hpp>

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static const char *ROOM_MAP[MapHeight] = {
	"################################",
	"#..............................#",
	"#..............................#",
	"#..II.BBBB.CC.||...............#",
	"#..II.BBBB.CC.||...............#",
	"#..II.BBBB.CC.||...............#",
	"#.....BBBB....||...............#",
	"#.....BBBB....||...............#",
	"#.....BBBB....||...............#",
	"#......P...TT..................#",
	"#..........TT..................#",
	"#..........D...................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"###############==###############",
};

Load< Sound::Sample > after_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("after_soul_loop.opus"));
});

Load< Sound::Sample > during_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("during_soul_loop.opus"));
});

Load< Sound::Sample > before_soul_capture(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("before_soul_loop.opus"));
});

RoomMode::RoomMode() {
	// sprites --------------------------------------------------------------------------------------------
	room.resize(MapHeight);
	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(ROOM_MAP[row]) != MapWidth) {
			throw std::runtime_error("ROOM_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}

		int y = MapHeight - 1 - row;
		room[y] = ROOM_MAP[row];

		for (int x = 0; x < MapWidth; x++) {
			char c = ROOM_MAP[row][x];
			if (c == 'P') {
				Interactable patient;
				patient.name = "patient";
				patient.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				patient.radius = 20.0f;
				interactables.push_back(patient);
			} else if (c == 'D') {
				Interactable diary;
				diary.name = "diary";
				diary.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				diary.radius = 16.0f;
				interactables.push_back(diary);
			}
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	floor_tex = sprites.load(data_path("tiles.png"));
	interact_tex = sprites.load(data_path("interact.png"));

	Sprites::Texture patient_tex = sprites.load(data_path("patient1.png"));
	Sprites::Texture cardiogram_tex = sprites.load(data_path("cardiogram.png"));
	Sprites::Texture iv_tex = sprites.load(data_path("IV.png"));
	Sprites::Texture partition_tex = sprites.load(data_path("partition.png"));
	Sprites::Texture desk_tex = sprites.load(data_path("desk.png"));

	//walk the map and place one image per block of identical tiles:
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];

			Sprites::Texture tex;
			if (c == 'B') {
				tex = patient_tex;
			} else if (c == 'C') {
				tex = cardiogram_tex;
			} else if (c == 'I') {
				tex = iv_tex;
			} else if (c == '|') {
				tex = partition_tex;
			} else if (c == 'T') {
				tex = desk_tex;
			} else {
				continue;
			}

			int block_left = x;
			while (block_left > 0 && room[y][block_left - 1] == c) {
				block_left = block_left - 1;
			}
			int block_bottom = y;
			while (block_bottom > 0 && room[block_bottom - 1][x] == c) {
				block_bottom = block_bottom - 1;
			}

			int image_w = int(tex.size.x) / int(TileSize);
			int image_h = int(tex.size.y) / int(TileSize);
			if ((x - block_left) % image_w != 0) {
				continue;
			}
			if ((y - block_bottom) % image_h != 0) {
				continue;
			}

			Prop prop;
			prop.tex = tex;
			prop.pos = glm::vec2(x * TileSize, y * TileSize);
			props.push_back(prop);
		}
	}

	hint_line = "WASD MOVE   E INTERACT   Talk to the patient first.\n";


	// text --------------------------------------------------------------------------------------------
	// ink::compiler::run("test.json", "test.bin");
	myInk = ink::runtime::story::from_file(data_path("prototype_script.bin").c_str());
	thread = myInk->new_runner();

	// sound --------------------------------------------------------------------------------------------
	play_music(*before_soul_capture, before_soul_loop);
}

RoomMode::~RoomMode() {
}

bool RoomMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	(void)window_size;

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_E) { // interaction
			interact.downs += 1;
			interact.pressed = true;
			return true;
		} 

	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_E) { // interaction
			interact.pressed = false;
			return true;
		}
	}
	return false;
}

bool RoomMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = room[ty][tx];
	if (c == '#' || c == 'B' || c == 'T' || c == '=' || c == 'C' || c == 'I' || c == '|') {
		return true;
	}
	return false;
}

bool RoomMode::can_move_to(glm::vec2 pos) {
	float left_x = pos.x;
	float right_x = pos.x + ReaperSize - 1.0f;
	float bottom_y = pos.y;
	float top_y = pos.y + ReaperSize - 1.0f;

	if (tile_is_solid(int(left_x) / 8, int(bottom_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(right_x) / 8, int(bottom_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(left_x) / 8, int(top_y) / 8)) {
		return false;
	}
	if (tile_is_solid(int(right_x) / 8, int(top_y) / 8)) {
		return false;
	}
	return true;
}

void RoomMode::play_music(Sound::Sample const &sample, std::shared_ptr< Sound::PlayingSample > &slot) {
	for (auto *other : { &before_soul_loop, &during_soul_loop, &after_soul_loop }) {
		if (*other) {
			(*other)->stop(1.0f);
			*other = nullptr;
		}
	}
	slot = Sound::loop(sample, 0.0f, 0.0f);
	slot->set_volume(1.0f, 1.0f);
}

void RoomMode::on_interact(Interactable const &thing) {
	if (soul_taken) return;

	if (thing.name == "patient") {
		// std::cout << "can battle room mode: " << can_battle << std::endl;
		// interacting with patient started dialogue
		// dialogue will be handled using dialogue_started
		if (!dialogue) {
			auto self = shared_from_this();
			dialogue = std::make_shared< DialogueMode >();
			dialogue->return_to = self;
			Mode::set_current(dialogue);
		}

		// interacting with patient after exhausting dialogue
		// enters minigame battle
		if (dialogue && dialogue->can_battle){
			auto self = shared_from_this();
			play_music(*during_soul_capture, during_soul_loop);
			capture = std::make_shared< SoulCaptureMode >(read_diary ? SoulCaptureMode::normal() : SoulCaptureMode::hard());
			capture->return_to = self;
			Mode::set_current(capture);
		}
		// talked_patient ? hint = "Space to capture soul" : hint = "Space to talk";
	} else if (thing.name == "diary") {
		read_diary = true;
		hint_line = "THE PATIENT SEEMS CALMER   GO TALK TO HIM\n";
	}
}

void RoomMode::update(float elapsed) {
	//keep the music and the hint in step with what has happened:
	if (capture && capture->captured) {
		if (!soul_taken) {
			soul_taken = true;
			hint_line = "TODAY'S QUOTA IS MET.  THANK YOU FOR PLAYING.\n";
		}
		if (!after_soul_loop) play_music(*after_soul_capture, after_soul_loop);
	} else if (!before_soul_loop) {
		//the player backed out of the capture, so the ward music comes back:
		play_music(*before_soul_capture, before_soul_loop);
	}

	hover_time = hover_time + elapsed;
	if (hover_time > 1000.0f) {
		hover_time = hover_time - 1000.0f;
	}

	const float PlayerSpeed = 60.0f;

	glm::vec2 move = glm::vec2(0.0f);
	if (left.pressed) {
		move.x -= 1.0f;
	}
	if (right.pressed) {
		move.x += 1.0f;
	}
	if (down.pressed) {
		move.y -= 1.0f;
	}
	if (up.pressed) {
		move.y += 1.0f;
	}
	if (move != glm::vec2(0.0f)) {
		move = glm::normalize(move) * PlayerSpeed * elapsed;
	}

	glm::vec2 try_x = glm::vec2(player_at.x + move.x, player_at.y);
	if (can_move_to(try_x)) {
		player_at.x = try_x.x;
	}
	glm::vec2 try_y = glm::vec2(player_at.x, player_at.y + move.y);
	if (can_move_to(try_y)) {
		player_at.y = try_y.y;
	}

	nearby = -1;
	float best_dist = 1e9f;
	glm::vec2 player_center = player_at + glm::vec2(4.0f, 4.0f);
	for (size_t i = 0; i < interactables.size(); i++) {
		float dist = glm::length(interactables[i].pos - player_center);
		if (dist < interactables[i].radius && dist < best_dist) {
			best_dist = dist;
			nearby = int(i);
		}
	}

	if (interact.downs > 0 && nearby != -1) {
		on_interact(interactables[nearby]);
	}

	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

void RoomMode::draw(glm::uvec2 const &drawable_size) {
	// sprites --------------------------------------------------------------------------------------------
	sprites.begin(drawable_size);

	const float FloorTileSize = 32.0f;
	for (float y = 0.0f; y < MapHeight * TileSize; y += FloorTileSize) {
		for (float x = 0.0f; x < MapWidth * TileSize; x += FloorTileSize) {
			sprites.draw(floor_tex, glm::vec2(x, y), glm::vec2(FloorTileSize));
		}
	}

	glm::vec4 wall_color = glm::vec4(0x60 / 255.0f, 0x70 / 255.0f, 0x78 / 255.0f, 1.0f);
	glm::vec4 door_color = glm::vec4(0x40 / 255.0f, 0x48 / 255.0f, 0x50 / 255.0f, 1.0f);
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];
			if (c == '#') {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), wall_color);
			} else if (c == '=') {
				sprites.draw(sprites.white, glm::vec2(x * TileSize, y * TileSize), glm::vec2(TileSize), door_color);
			}
		}
	}

	for (size_t i = 0; i < props.size(); i++) {
		sprites.draw(props[i].tex, props[i].pos, glm::vec2(props[i].tex.size));
	}

	float hover = 2.0f * std::sin(hover_time * 3.0f);
	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));

	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(player_at.x + (ReaperSize - marker_size.x) * 0.5f, player_at.y + ReaperSize + 2.0f + hover);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	sprites.end(drawable_size);

	//controls hint along the bottom of the screen:
	hint_processor.textbox_width = float(drawable_size.x);
	hint_processor.textbox_height = FONT_SIZE;
	hint_processor.pen_x_start = (soul_taken ? drawable_size.x * 0.5f - 11.0f * FONT_SIZE : float(FONT_SIZE));
	hint_processor.pen_y_start = (soul_taken ? drawable_size.y * 0.5f : FONT_SIZE * 1.5f);
	hint_processor.handle_curr_line(hint_line);
	hint_processor.draw_curr_line(drawable_size);

	GL_ERRORS();
}