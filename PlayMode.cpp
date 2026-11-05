#include "PlayMode.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"

#include <iostream>
#include <cmath>
#include <cstring>

const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

static const char *ROOM_MAP[MapHeight] = {
	"################################",
	"#..............................#",
	"#..............................#",
	"#.............BBBB.............#",
	"#...........P.BBBB.............#",
	"#.............BBBB..TT.........#",
	"#.............BBBB..TT.........#",
	"#.............BBBB...D.........#",
	"#.............BBBB.............#",
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
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"###############==###############",
};

PlayMode::PlayMode() {
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
				diary.radius = 10.0f;
				interactables.push_back(diary);
			}
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	bed_tex = sprites.load(data_path("bed.png"));
	bool found_bed = false;
		for (int y = 0; y < MapHeight && !found_bed; y++) {
		for (int x = 0; x < MapWidth && !found_bed; x++) {
			if (room[y][x] == 'B') {
				bed_pos = glm::vec2(x * TileSize, y * TileSize);
				found_bed = true;
			}
		}
	}

	desk_tex = sprites.load(data_path("desk.png"));
	bool found_desk = false;
	for (int y = 0; y < MapHeight && !found_desk; y++) {
		for (int x = 0; x < MapWidth && !found_desk; x++) {			if (room[y][x] == 'T') {
				desk_pos = glm::vec2(x * TileSize, y * TileSize);
				found_desk = true;
			}
		}
	}

	floor_tex = sprites.load(data_path("tiles.png"));
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
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
		} else if (evt.key.key == SDLK_E) {
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
		} else if (evt.key.key == SDLK_E) {
			interact.pressed = false;
			return true;
		}
	}

	return false;
}

bool PlayMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = room[ty][tx];
	if (c == '#' || c == 'B' || c == 'T' || c == '=') {
		return true;
	}
	return false;
}

bool PlayMode::can_move_to(glm::vec2 pos) {
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

void PlayMode::on_interact(Interactable const &thing) {
	if (thing.name == "patient") {
		std::cout << "You approach the patient. (TODO: Reap / Talk choice)" << std::endl;
	} else if (thing.name == "diary") {
		std::cout << "You open the diary. (TODO: show diary text)" << std::endl;
	}
}

void PlayMode::update(float elapsed) {
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
	glm::vec2 player_center = player_at + glm::vec2(ReaperSize * 0.5f);
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

void PlayMode::draw(glm::uvec2 const &drawable_size) {
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

	float hover = 2.0f * std::sin(hover_time * 3.0f);
	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));
	sprites.draw(bed_tex, bed_pos, glm::vec2(bed_tex.size));
	sprites.draw(desk_tex, desk_pos, glm::vec2(desk_tex.size));

	if (nearby != -1) {
		glm::vec4 alert_color = glm::vec4(1.0f, 0xd0 / 255.0f, 0x20 / 255.0f, 1.0f);
		glm::vec2 alert_pos = glm::vec2(player_at.x + 6.0f, player_at.y + ReaperSize + 3.0f + hover);
		sprites.draw(sprites.white, alert_pos, glm::vec2(4.0f, 6.0f), alert_color);
	}

	sprites.end(drawable_size);
}