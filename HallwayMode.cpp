#include "HallwayMode.hpp"
#include "PlayMode.hpp"

#include "data_path.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;
const float FloorTileSize = 32.0f;

const int MapWidth = 32;
const int MapHeight = 30;

const float HintFontSize = 8.0f;
const float HintLineHeight = 11.0f;

static void draw_floor_shadow(Sprites &sprites, float center_x, float floor_y, float width) {
	glm::vec4 shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.4f);

	float narrow = width - 4.0f;
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y + 1.0f), glm::vec2(narrow, 1.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - width * 0.5f, floor_y - 1.0f), glm::vec2(width, 2.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y - 2.0f), glm::vec2(narrow, 1.0f), shadow_color);
}

static const char *HALLWAY_MAP[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"################################",
	"################################",
	"###11####22####33####44####55###",
	"###11####22####33####44####55###",
	"###11####22####33####44####55###",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"#############......#############",
	"################################",
};

static bool is_door_char(char c) {
	if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5') {
		return true;
	}
	return false;
}

HallwayMode::HallwayMode() {
	hallway.resize(MapHeight);
	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(HALLWAY_MAP[row]) != MapWidth) {
			throw std::runtime_error("HALLWAY_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}
		int y = MapHeight - 1 - row;
		hallway[y] = HALLWAY_MAP[row];
	}

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = hallway[y][x];
			if (!is_door_char(c)) {
				continue;
			}
			if (x > 0 && hallway[y][x - 1] == c) {
				continue;
			}
			if (y > 0 && hallway[y - 1][x] == c) {
				continue;
			}
			Door door;
			door.number = c - '0';
			door.pos = glm::vec2(x * TileSize + 8.0f, y * TileSize);
			doors.push_back(door);
		}
	}

	reaper_tex = sprites.load(data_path("reaper.png"));
	floor_tex = sprites.load(data_path("tiles.png"));
	interact_tex = sprites.load(data_path("interact.png"));
	vignette_tex = sprites.load(data_path("vignette.png"));
	light_tex = sprites.load(data_path("light.png"));
}

HallwayMode::~HallwayMode() {
}

bool HallwayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
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

bool HallwayMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = hallway[ty][tx];
	if (c == '#' || is_door_char(c)) {
		return true;
	}
	return false;
}

bool HallwayMode::can_move_to(glm::vec2 pos) {
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

void HallwayMode::reset_buttons() {
	left.pressed = false;
	right.pressed = false;
	up.pressed = false;
	down.pressed = false;
	interact.pressed = false;
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

void HallwayMode::enter_door(int number) {
	int index = number - 1;

	if (!rooms[index]) {
		std::shared_ptr< PlayMode > room = std::make_shared< PlayMode >(number);
		room->return_to = shared_from_this();
		rooms[index] = room;
	}

	reset_buttons();
	start_leaving(rooms[index]);
}

void HallwayMode::update(float elapsed) {

	const float FadeTime = 0.4f;
	fade_in = fade_in - elapsed / FadeTime;
	if (fade_in < 0.0f) {
		fade_in = 0.0f;
	}

	if (leaving_to) {
		fade_out = fade_out + elapsed / FadeTime;
		if (fade_out >= 1.0f) {
			auto self = shared_from_this(); 
			std::shared_ptr< Mode > next = leaving_to;
			leaving_to.reset();
			fade_out = 0.0f;
			Mode::set_current(next);
		}
		return;
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

	const float DoorRadius = 16.0f;
	nearby = -1;
	float best_dist = 1e9f;
	glm::vec2 player_center = player_at + glm::vec2(ReaperSize * 0.5f);
	for (size_t i = 0; i < doors.size(); i++) {
		float dist = glm::length(doors[i].pos - player_center);
		if (dist < DoorRadius && dist < best_dist) {
			best_dist = dist;
			nearby = int(i);
		}
	}

	if (interact.downs > 0 && nearby != -1) {
		enter_door(doors[nearby].number);
		return;
	}

	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	interact.downs = 0;
}

static bool hall_is_floor(std::vector< std::string > const &hallway, int x, int y) {
	if (x < 0 || y < 0 || x >= MapWidth || y >= MapHeight) {
		return false;
	}
	char c = hallway[y][x];
	if (c == '#' || is_door_char(c)) {
		return false;
	}
	return true;
}

static int floor_distance_below(std::vector< std::string > const &hallway, int x, int y) {
	int distance = 1;
	for (int yy = y - 1; yy >= 0; yy--) {
		if (hall_is_floor(hallway, x, yy)) {
			return distance;
		}
		distance = distance + 1;
	}
	return 1000;
}

static bool touches_floor(std::vector< std::string > const &hallway, int x, int y) {
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			if (hall_is_floor(hallway, x + dx, y + dy)) {
				return true;
			}
		}
	}
	return false;
}

void HallwayMode::draw(glm::uvec2 const &drawable_size) {
	sprites.begin(drawable_size);

	glm::vec4 floor_tint = glm::vec4(0.52f, 0.58f, 0.62f, 1.0f);
	for (float y = 0.0f; y < MapHeight * TileSize; y += FloorTileSize) {
		for (float x = 0.0f; x < MapWidth * TileSize; x += FloorTileSize) {
			sprites.draw(floor_tex, glm::vec2(x, y), glm::vec2(FloorTileSize), floor_tint);
		}
	}

	glm::vec4 wall_color      = glm::vec4(0x60 / 255.0f, 0x70 / 255.0f, 0x78 / 255.0f, 1.0f);
	glm::vec4 wall_face_color = glm::vec4(0x3b / 255.0f, 0x46 / 255.0f, 0x4b / 255.0f, 1.0f);
	glm::vec4 baseboard_color = glm::vec4(0x26 / 255.0f, 0x2d / 255.0f, 0x32 / 255.0f, 1.0f);
	glm::vec4 wall_edge_color = glm::vec4(0x7a / 255.0f, 0x88 / 255.0f, 0x8c / 255.0f, 1.0f);
	glm::vec4 wall_shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);
	glm::vec4 ceiling_color   = glm::vec4(0x1d / 255.0f, 0x23 / 255.0f, 0x28 / 255.0f, 1.0f);

	const int WallFaceHeight = 3;

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			if (hallway[y][x] != '#') {
				continue;
			}
			glm::vec2 pos = glm::vec2(x * TileSize, y * TileSize);
			int dist = floor_distance_below(hallway, x, y);

			if (dist <= WallFaceHeight) {
				sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_face_color);
				if (dist == 1) {
					sprites.draw(sprites.white, pos, glm::vec2(TileSize, 2.0f), baseboard_color);
					sprites.draw(sprites.white, pos + glm::vec2(0.0f, -2.0f), glm::vec2(TileSize, 2.0f), wall_shadow_color);
				}
				if (dist == WallFaceHeight) {
					sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
				}
			} else if (touches_floor(hallway, x, y)) {
				sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_color);
				if (hall_is_floor(hallway, x, y + 1)) {
					sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
				}
			} else {
				sprites.draw(sprites.white, pos, glm::vec2(TileSize), ceiling_color);
			}
		}
	}

	glm::vec4 frame_color  = glm::vec4(0x2c / 255.0f, 0x33 / 255.0f, 0x38 / 255.0f, 1.0f);
	glm::vec4 panel_color  = glm::vec4(0x8a / 255.0f, 0x5a / 255.0f, 0x3a / 255.0f, 1.0f);
	glm::vec4 panel_light  = glm::vec4(0xa8 / 255.0f, 0x74 / 255.0f, 0x50 / 255.0f, 1.0f);
	glm::vec4 glass_color  = glm::vec4(0xa9 / 255.0f, 0xc6 / 255.0f, 0xcc / 255.0f, 1.0f);
	glm::vec4 glass_dark   = glm::vec4(0x1a / 255.0f, 0x1f / 255.0f, 0x23 / 255.0f, 1.0f);
	glm::vec4 handle_color = glm::vec4(0xc9 / 255.0f, 0xb2 / 255.0f, 0x7a / 255.0f, 1.0f);
	glm::vec4 plate_color  = glm::vec4(0xd7 / 255.0f, 0xe4 / 255.0f, 0xe8 / 255.0f, 1.0f);

	for (size_t i = 0; i < doors.size(); i++) {
		glm::vec2 bl = glm::vec2(doors[i].pos.x - 8.0f, doors[i].pos.y);
		int index = doors[i].number - 1;

		bool done = false;
		if (rooms[index] && rooms[index]->soul_taken) {
			done = true;
		}

		sprites.draw(sprites.white, bl, glm::vec2(16.0f, 24.0f), frame_color);
		sprites.draw(sprites.white, bl + glm::vec2(1.0f, 0.0f), glm::vec2(14.0f, 23.0f), panel_color);
		sprites.draw(sprites.white, bl + glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 23.0f), panel_light);

		glm::vec4 window_color = glass_color;
		if (done) {
			window_color = glass_dark;
		}
		sprites.draw(sprites.white, bl + glm::vec2(4.0f, 13.0f), glm::vec2(8.0f, 6.0f), window_color);
		sprites.draw(sprites.white, bl + glm::vec2(7.0f, 13.0f), glm::vec2(2.0f, 6.0f), frame_color);

		sprites.draw(sprites.white, bl + glm::vec2(11.0f, 9.0f), glm::vec2(2.0f, 2.0f), handle_color);

		for (int d = 0; d < doors[i].number; d++) {
			sprites.draw(sprites.white, bl + glm::vec2(3.0f + d * 2.0f, 20.0f), glm::vec2(1.0f, 1.0f), plate_color);
		}

		sprites.draw(sprites.white, bl, glm::vec2(16.0f, 1.0f), frame_color);
	}

	glm::vec2 door_lights[5] = {
		glm::vec2(32.0f, 168.0f),
		glm::vec2(80.0f, 168.0f),
		glm::vec2(128.0f, 168.0f),
		glm::vec2(176.0f, 168.0f),
		glm::vec2(224.0f, 168.0f),
	};
	glm::vec4 door_light_tint = glm::vec4(1.0f, 0.9f, 0.7f, 0.3f);
	glm::vec2 door_light_size = glm::vec2(light_tex.size) * 0.3f;
	for (int i = 0; i < 5; i++) {
		if (rooms[i] && rooms[i]->soul_taken) {
			continue;
		}
		sprites.draw(light_tex, door_lights[i] - door_light_size * 0.5f, door_light_size, door_light_tint);
	}

	glm::vec2 screen_size = glm::vec2(MapWidth * TileSize, MapHeight * TileSize);

	glm::vec4 cold_tint = glm::vec4(0.44f, 0.65f, 0.77f, 0.06f);
	sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, cold_tint);

	sprites.draw(vignette_tex, glm::vec2(0.0f, 0.0f), screen_size);

	float hover = 2.0f * std::sin(hover_time * 3.0f);
	float shadow_width = 12.0f - hover;
	draw_floor_shadow(sprites, player_at.x + ReaperSize * 0.5f, player_at.y - 3.0f, shadow_width);
	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));


	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(
			player_at.x + (ReaperSize - marker_size.x) * 0.5f,
			player_at.y + ReaperSize + 2.0f + hover
		);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	//hint box, the same look as the one in the rooms. It sits at the top because the
	//player starts at the bottom of the corridor:
	std::string hint_lines[2];
	int hint_count = 1;
	if (all_souls_taken()) {
		hint_lines[0] = "All souls collected. Your shift is over.";
		hint_lines[1] = "Thank you for playing!";
		hint_count = 2;
	} else if (nearby != -1) {
		hint_lines[0] = "Press E to enter room " + std::to_string(doors[nearby].number) + ".";
	} else {
		hint_lines[0] = "Walk up to a door and press E.";
	}
	float box_height = 10.0f + hint_count * HintLineHeight;
	glm::vec2 box_pos = glm::vec2(12.0f, 230.0f - box_height);
	glm::vec2 box_size = glm::vec2(232.0f, box_height);
	sprites.draw(sprites.white, box_pos - glm::vec2(1.0f), box_size + glm::vec2(2.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	sprites.draw(sprites.white, box_pos, box_size, glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));

	float fade = fade_in;
	if (fade_out > fade) {
		fade = fade_out;
	}
	if (fade > 0.0f) {
		glm::vec4 fade_color = glm::vec4(0.0f, 0.0f, 0.0f, fade);
		sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, fade_color);
	}

	sprites.end(drawable_size);

	//text goes on top at screen resolution, and not while fading (it would sit on top of the fade):
	if (fade <= 0.0f) {
		draw_hint_text(hint_lines, hint_count, box_pos.y + box_height - 4.0f - HintFontSize, drawable_size);
	}
}

bool HallwayMode::all_souls_taken() {
	for (int i = 0; i < 5; i++) {
		if (!rooms[i] || !rooms[i]->soul_taken) {
			return false;
		}
	}
	return true;
}

void HallwayMode::draw_hint_text(std::string const lines[2], int count, float first_baseline, glm::uvec2 const &drawable_size) {
	float scale = std::floor(std::min(drawable_size.x / 256.0f, drawable_size.y / 240.0f));
	if (scale < 1.0f) {
		scale = 1.0f;
	}
	float view_x = std::floor((drawable_size.x - 256.0f * scale) * 0.5f);
	float view_y = std::floor((drawable_size.y - 240.0f * scale) * 0.5f);

	uint32_t pixels = uint32_t(HintFontSize * scale);
	bool new_font = !hint_font || hint_font->pixel_size != pixels;
	if (new_font) {
		hint_font = std::make_unique< Font >(data_path("LowresPixel-Regular.otf"), pixels);
	}

	glViewport(0, 0, drawable_size.x, drawable_size.y);
	for (int i = 0; i < count; i++) {
		if (new_font || hint_texts[i] != lines[i]) {
			hint_images[i] = render_text(*hint_font, lines[i]);
			hint_texts[i] = lines[i];
		}
		float text_x = std::floor(view_x + 128.0f * scale - hint_images[i].advance * 0.5f);
		float baseline = std::floor(view_y + (first_baseline - i * HintLineHeight) * scale);
		draw_text(hint_images[i], glm::vec2(text_x, baseline), glm::vec4(1.0f), drawable_size);
	}
}

void HallwayMode::start_leaving(std::shared_ptr< Mode > next) {
	leaving_to = next;
	fade_out = 0.0f;
	reset_buttons();
}