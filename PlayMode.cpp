#include "PlayMode.hpp"
#include "data_path.hpp"
#include "gl_compile_program.hpp"

#include <iostream>
#include <cmath>
#include <cstring>
#include "SoulCaptureMode.hpp"

#include "text-pipeline.hpp"


//room size in tiles
const int MapWidth = 32;
const int MapHeight = 30;

const float TileSize = 8.0f;
const float ReaperSize = 16.0f;

//3 thin rectangles for small oval shadow under the reaper
//ideas for shadows and lights: https://pixel-beef.itch.io/xdasher/devlog/192949/4-things-we-did-to-add-light-and-depth-to-our-pixel-art-game
static void draw_floor_shadow(Sprites &sprites, float center_x, float floor_y, float width) {
	glm::vec4 shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.4f);
	float narrow = width - 4.0f;
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y + 1.0f), glm::vec2(narrow, 1.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - width * 0.5f, floor_y - 1.0f), glm::vec2(width, 2.0f), shadow_color);
	sprites.draw(sprites.white, glm::vec2(center_x - narrow * 0.5f, floor_y - 2.0f), glm::vec2(narrow, 1.0f), shadow_color);
}

//room maps
static const char *ROOM_MAP_0[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"#..............................#",
	"#..............................#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#...............P..............#",
	"#..............................#",
	"#.....TT.......................#",
	"#.....TT.......................#",
	"#.....D........................#",
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
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_1[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"#..............................#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#...............P..............#",
	"#..............................#",
	"#.....TT.......................#",
	"#.....TT.......................#",
	"#.....D........................#",
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
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_2[MapHeight] = {
	"################################",
	"################################",
	"################################",
	"#..............................#",
	"#.IIBBBBCC.....................#",
	"#.IIBBBBCC.....................#",
	"#.IIBBBBCC.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_3[MapHeight] = {
	"################################",
	"#..............................#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#...........IIBBBBCC...........#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#.............BBBB.............#",
	"#...............P..............#",
	"#..............................#",
	"#.....TT.......................#",
	"#.....TT.......................#",
	"#.....D........................#",
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
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_4[MapHeight] = {
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#..............................#",
	"#..............................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const char *ROOM_MAP_5[MapHeight] = {
	"################################",
	"#..............................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.....................#",
	"#.CCBBBBII.................TT..#",
	"#...BBBB...................TT..#",
	"#...BBBB...................D...#",
	"#...BBBB.......................#",
	"#.....P........................#",
	"#..............................#",
	"#..............................#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#....................MMM.......#",
	"#..............................#",
	"#..............................#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#....................WWW.......#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............................#",
	"#..............X...............#",
	"###############==###############",
};

static const int IntroPageCount = 3;
static const std::string INTRO_LINES[IntroPageCount][2] = {
	{"Welcome to your first day on the job.", "You are a reaper."},
	{"Today's quota: at least one soul.", "We have stationed you in a hospital."},
	{"Your first client is right here.", "Let's start with the basics."},
};

static const int SoulPageCount = 2;
static const std::string SOUL_LINES[SoulPageCount][2] = {
	{"See that light above the bed?", "That is your client's soul."},
	{"It is already loosening from the body.", "Your job is to cut it free."},
};

static const int DiaryPageCount = 2;
static const std::string DIARY_LINES[DiaryPageCount][2] = {
	{"The soul fought back. It is not ready to go.", "Something is still holding it here."},
	{"Learn about your client first.", "Their diary is on the desk."},
};

static const int DiaryTextPageCount = 2;
static const std::string DIARY_TEXT[DiaryTextPageCount][2] = {
	{"\"Stacy and the grandbabies visited again.\"", "\"Jack turns 14 soon. Alice loves her friends.\""},
	{"\"Stacy looks just like Dee did.\"", "\"If only Dee could see them now.\""},
};

static const int ExplainPageCount = 3;
static const std::string EXPLAIN_LINES[ExplainPageCount][2] = {
	{"The better you understand someone,", "the less they resist."},
	{"Reading diaries and talking to people", "makes their soul calmer and easier to catch."},
	{"Now try again.", ""},
};

static const int FinishPageCount = 2;
static const std::string FINISH_LINES[FinishPageCount][2] = {
	{"Your first soul. Well done.", "The rest of the ward is waiting."},
	{"Leave through the door at the bottom", "when you are ready."},
};

//all rooms
static const char **ROOM_MAPS[6] = {
	ROOM_MAP_0,
	ROOM_MAP_1,
	ROOM_MAP_2,
	ROOM_MAP_3,
	ROOM_MAP_4,
	ROOM_MAP_5,
};

PlayMode::PlayMode(int room_number_) : room_number(room_number_) {
	if (room_number < 0 || room_number > 5) {
		throw std::runtime_error("No map for room " + std::to_string(room_number));
	}

	//read the map
	const char **room_map = ROOM_MAPS[room_number];

	room.resize(MapHeight);

	for (int row = 0; row < MapHeight; row++) {
		if (std::strlen(room_map[row]) != MapWidth) {
			throw std::runtime_error("ROOM_MAP row " + std::to_string(row) + " is not " + std::to_string(MapWidth) + " characters long.");
		}

		int y = MapHeight - 1 - row;
		room[y] = room_map[row];

		for (int x = 0; x < MapWidth; x++) {
			char c = room_map[row][x];
			if (c == 'P') {
				Interactable patient;
				patient.name = "patient";
				patient.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				patient.radius = 20.0f;
				interactables.push_back(patient);
				bed_center_x = x * TileSize;
				soul_pos = glm::vec2(x * TileSize + 0.5f, y * TileSize + 32.0f);
				lights.push_back(glm::vec2(x * TileSize, y * TileSize + 8.0f));

			} else if (c == 'D') {
				Interactable diary;
				diary.name = "diary";
				diary.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				diary.radius = 10.0f;
				interactables.push_back(diary);
				diary_pos = diary.pos;
			} else if (c == 'F') {
				Interactable father;
				father.name = "father";
				father.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				father.radius = 10.0f;
				interactables.push_back(father);
			} else if (c == 'N') {
				Interactable nurse;
				nurse.name = "nurse";
				nurse.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				nurse.radius = 10.0f;
				interactables.push_back(nurse);
			} else if (c == 'X') {
				Interactable exit;
				exit.name = "exit";
				exit.pos = glm::vec2(x * TileSize + 4.0f, y * TileSize + 4.0f);
				exit.radius = 20.0f;
				interactables.push_back(exit);
			}
		}
	}

	//start the tutorial intro
	if (room_number == 0) {
		tutorial_step = TUT_INTRO;
		player_at = glm::vec2(MapWidth * TileSize * 0.5f - ReaperSize * 0.5f, MapHeight * TileSize * 0.5f - ReaperSize * 0.5f);
	}

	//load textures
	reaper_tex = sprites.load(data_path("reaper.png"));
	patient1_tex = sprites.load(data_path("patient" + std::to_string(room_number) + ".png"));
	man1_tex = sprites.load(data_path("man1.png"));
	woman1_tex = sprites.load(data_path("woman1.png"));
	partition_tex = sprites.load(data_path("partition.png"));
	cardiogram_tex = sprites.load(data_path("cardiogram.png"));
	iv_tex = sprites.load(data_path("IV.png"));
	desk_tex = sprites.load(data_path("desk.png"));
	interact_tex = sprites.load(data_path("interact.png"));
	vignette_tex = sprites.load(data_path("vignette.png"));
	light_tex = sprites.load(data_path("light.png"));
	soul_tex = sprites.load(data_path("soul.png"));
	patient_dead_tex = sprites.load(data_path("patient" + std::to_string(room_number) + "_dead.png"));
	cardiogram_dead_tex = sprites.load(data_path("cardiogram_dead.png"));

	//build props
	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];

			Sprites::Texture tex;
			if (c == 'B') {
				tex = patient1_tex;
			} else if (c == 'M') {
				tex = man1_tex;
			} else if (c == 'W') {
				tex = woman1_tex;
			} else if (c == '|') {
				tex = partition_tex;
			} else if (c == 'C') {
				tex = cardiogram_tex;
			} else if (c == 'I') {
				tex = iv_tex;
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
			prop.kind = c;
			props.push_back(prop);
		}
	}

	//one extra ceiling light in the middle of the room
	lights.push_back(glm::vec2(MapWidth * TileSize * 0.5f, MapHeight * TileSize * 0.4f));

	floor_tex = sprites.load(data_path("tiles.png"));
	light_tex = sprites.load(data_path("light.png"));

	floor_tex = sprites.load(data_path("tiles.png"));
}

//which image to draw for a prop
Sprites::Texture PlayMode::texture_for(Prop const &prop) {

	if (soul_taken) {
		if (prop.kind == 'B') {
			return patient_dead_tex;
		}
		if (prop.kind == 'C') {
			return cardiogram_dead_tex;
		}
	}
	
	return prop.tex;
}

PlayMode::~PlayMode() {
}

//tutorial state machine
//reference: https://gameprogrammingpatterns.com/state.html
bool PlayMode::tutorial_page_lines(int page, std::string lines[2]) {

	if (tutorial_step == TUT_INTRO) {
		if (page >= IntroPageCount) {
			return false;
		}
		lines[0] = INTRO_LINES[page][0];
		lines[1] = INTRO_LINES[page][1];
		return true;
	}
	if (tutorial_step == TUT_SOUL) {
		if (page >= SoulPageCount) {
			return false;
		}
		lines[0] = SOUL_LINES[page][0];
		lines[1] = SOUL_LINES[page][1];
		return true;
	}
	if (tutorial_step == TUT_DIARY) {
		if (page >= DiaryPageCount) {
			return false;
		}
		lines[0] = DIARY_LINES[page][0];
		lines[1] = DIARY_LINES[page][1];
		return true;
	}
	if (tutorial_step == TUT_READ_DIARY) {
		if (page >= DiaryTextPageCount) {
			return false;
		}
		lines[0] = DIARY_TEXT[page][0];
		lines[1] = DIARY_TEXT[page][1];
		return true;
	}
	if (tutorial_step == TUT_EXPLAIN) {
		if (page >= ExplainPageCount) {
			return false;
		}
		lines[0] = EXPLAIN_LINES[page][0];
		lines[1] = EXPLAIN_LINES[page][1];
		return true;
	}
	if (tutorial_step == TUT_FINISH) {
		if (page >= FinishPageCount) {
			return false;
		}
		lines[0] = FINISH_LINES[page][0];
		lines[1] = FINISH_LINES[page][1];
		return true;
	}
	return false;
}

//go to the next step if all the lines were read
void PlayMode::finish_pages() {
	tutorial_page = 0;
	if (tutorial_step == TUT_INTRO) {
		tutorial_step = TUT_MOVE;
	} else if (tutorial_step == TUT_SOUL) {
		tutorial_step = TUT_TALK;
	} else if (tutorial_step == TUT_DIARY) {
		tutorial_step = TUT_FIND_DIARY;
	} else if (tutorial_step == TUT_READ_DIARY) {
		tutorial_step = TUT_EXPLAIN;
	} else if (tutorial_step == TUT_EXPLAIN) {
		tutorial_step = TUT_TALK_AGAIN;
	} else if (tutorial_step == TUT_FINISH) {
		tutorial_step = TUT_DONE;
	}
}

//oneline hints
std::string PlayMode::tutorial_hint() {

	if (tutorial_step == TUT_MOVE) {
		return "Use W A S D to move.";
	}
	if (tutorial_step == TUT_TALK) {
		return "Walk up to the patient and press E to talk.";
	}
	if (tutorial_step == TUT_FIND_DIARY) {
		return "Find the diary on the desk and press E.";
	}
	if (tutorial_step == TUT_TALK_AGAIN) {
		return "Talk to the patient again.";
	}
	return "";
}

//room dims and items are highlighted
bool PlayMode::is_spotlight_step() {

	if (tutorial_step == TUT_SOUL || tutorial_step == TUT_DIARY) {
		return true;
	}
	return false;
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
		} else if (evt.key.key == SDLK_SPACE) {
			advance.downs += 1;
			advance.pressed = true;
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
		} else if (evt.key.key == SDLK_SPACE) {
			advance.pressed = false;
			return true;
		}
	}

	return false;
}

//collision
//reference: https://jonathanwhiting.com/tutorial/collision/
bool PlayMode::tile_is_solid(int tx, int ty) {
	if (tx < 0 || ty < 0 || tx >= MapWidth || ty >= MapHeight) {
		return true;
	}
	char c = room[ty][tx];
	if (c == '#' || c == 'B' || c == 'T' || c == '=' || c == '|' || c == 'C' || c == 'I' || c == 'M' || c == 'W') {
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

//interaction when E is pressed
void PlayMode::on_interact(Interactable const &thing) {
	if (thing.name == "patient") {
		if (in_tutorial() && tutorial_step != TUT_TALK && tutorial_step != TUT_TALK_AGAIN) {
			return;
		}
		if (soul_taken) {
			std::cout << "This soul has already been collected." << std::endl;
			return;
		}
		if (tutorial_step == TUT_TALK) {
			tutorial_step = TUT_FIRST_TRY;
		} else if (tutorial_step == TUT_TALK_AGAIN) {
			tutorial_step = TUT_SECOND_TRY;
		}

		// tell the story which patient we are talking to before it resumes
		static const char *patients[6] = { "henry", "abigail", "sherry", "keith", "david", "twins" };
		std::string patient = patients[room_number];

		std::shared_ptr< InkStory > story = InkStory::get("final_script.bin");
		for (int i = 1; i < 6; i++) {
			story->set_var< bool >(("start_" + std::string(patients[i])).c_str(), patient == patients[i]);
		}
		if (diary_read) {
			story->set_var< bool >(("diary_" + patient).c_str(), true);
		}

		if (tutorial_step == TUT_FIRST_TRY && !minigame_seen) {
			story->runner->move_to("henry");
		}

		std::shared_ptr< DialogueMode > dialogue = std::make_shared< DialogueMode >("final_script.bin");
		dialogue->return_to = shared_from_this();
		// the dialogue starts the battle itself when it reads "# battle"; we just hear about the capture
		std::weak_ptr<Mode> weak_room = shared_from_this();
		dialogue->on_capture = [weak_room]() {
			auto owner = std::static_pointer_cast< PlayMode >(weak_room.lock());
			if (owner) owner->soul_taken = true;
		};

		//the dialogue starts the battle; in the tutorial we set up its minigame
		PlayMode *self = this;
		dialogue->setup_minigame = [self](SoulCaptureMode &minigame) {
			self->minigame_seen = true;
			if (self->tutorial_step == TUT_FIRST_TRY) {
				minigame.difficulty = SoulCaptureMode::level(1);
				minigame.tutorial = true;
				minigame.resist_after_first_hit = true;
			}
			if (self->tutorial_step == TUT_SECOND_TRY) {
				minigame.difficulty = SoulCaptureMode::level(1);
				minigame.tutorial = true;
			}
		};

		active_dialogue = dialogue;

		reset_buttons();
		Mode::set_current(dialogue);
	} else if (thing.name == "diary") {
		diary_read = true;
		std::cout << "You read the diary. The patient seems calmer now." << std::endl;
		if (tutorial_step == TUT_FIND_DIARY) {
			tutorial_step = TUT_READ_DIARY;
			tutorial_page = 0;
		}
	} else if (thing.name == "father") {
		std::cout << "You talked to the father. (TODO: tell story)" << std::endl;
	} else if (thing.name == "nurse") {
		std::cout << "You talked to the nurse. (TODO: tell story)" << std::endl;
	} else if (thing.name == "exit") {
		if (in_tutorial()) {
			return;
		}
		start_leaving(return_to);
	}
}

//drawing the walls
static bool room_is_floor(std::vector< std::string > const &room, int x, int y) {
	if (x < 0 || y < 0 || x >= MapWidth || y >= MapHeight) {
		return false;
	}
	char c = room[y][x];
	if (c == '#' || c == '=') {
		return false;
	}
	return true;
}

static int room_floor_distance_below(std::vector< std::string > const &room, int x, int y) {
	int distance = 1;
	for (int yy = y - 1; yy >= 0; yy--) {
		if (room_is_floor(room, x, yy)) {
			return distance;
		}
		distance = distance + 1;
	}
	return 1000;
}

void PlayMode::update(float elapsed) {

	if (active_dialogue && Mode::current.get() == this) {
		active_dialogue.reset();
		if (tutorial_step == TUT_FIRST_TRY) {
			if (minigame_seen) {
				tutorial_step = TUT_DIARY;
			} else {
				tutorial_step = TUT_TALK;
			}
			tutorial_page = 0;
		} else if (tutorial_step == TUT_SECOND_TRY) {
			if (soul_taken) {
				tutorial_step = TUT_FINISH;
			} else {
				tutorial_step = TUT_TALK_AGAIN;
			}
			tutorial_page = 0;
		}
	}

	//fade in after entering this room
	const float FadeTime = 0.5f;
	fade_in = fade_in - elapsed / FadeTime;
	if (fade_in < 0.0f) {
		fade_in = 0.0f;
	}

	//fading out
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

	std::string hint = tutorial_hint();
	if (hint != "" && spotlight_alpha <= 0.0f) {
		shown_hint = hint;
		hint_alpha = std::min(1.0f, hint_alpha + elapsed * 3.0f);
	} else {
		hint_alpha = std::max(0.0f, hint_alpha - elapsed * 2.0f);
	}

	//spotlight
	if (tutorial_step == TUT_SOUL) {
		spotlight_on_diary = false;
	}
	if (tutorial_step == TUT_DIARY) {
		spotlight_on_diary = true;
	}
	if (is_spotlight_step() && hint_alpha <= 0.0f) {
		spotlight_alpha = std::min(1.0f, spotlight_alpha + elapsed * 2.0f);
	} else if (!is_spotlight_step()) {
		spotlight_alpha = std::max(0.0f, spotlight_alpha - elapsed * 2.0f);
	}

	std::string page_lines[2];
	if (tutorial_page_lines(tutorial_page, page_lines)) {
		bool ready = true;
		if (hint_alpha > 0.0f) {
			ready = false;
		}
		if (is_spotlight_step() && spotlight_alpha < 1.0f) {
			ready = false;
		}
		if (advance.downs > 0 && ready) {
			tutorial_page = tutorial_page + 1;
			if (!tutorial_page_lines(tutorial_page, page_lines)) {
				finish_pages();
			}
		}
		advance.downs = 0;
		left.downs = 0;
		right.downs = 0;
		up.downs = 0;
		down.downs = 0;
		interact.downs = 0;
		return;
	}

	//movement
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

	glm::vec2 before_move = player_at;

	glm::vec2 try_x = glm::vec2(player_at.x + move.x, player_at.y);
	if (can_move_to(try_x)) {
		player_at.x = try_x.x;
	}
	glm::vec2 try_y = glm::vec2(player_at.x, player_at.y + move.y);
	if (can_move_to(try_y)) {
		player_at.y = try_y.y;
	}

	//go to the soul step after moving a bit
	if (tutorial_step == TUT_MOVE) {
		moved_distance = moved_distance + glm::length(player_at - before_move);
		if (moved_distance > 40.0f) {
			tutorial_step = TUT_SOUL;
			tutorial_page = 0;
		}
	}

	//find the closest thing in range and show the ! marker
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
	advance.downs = 0;
}

//use rectangles to draw window above the bed
void PlayMode::draw_window() {
	glm::vec4 frame_dark = glm::vec4(0x2a / 255.0f, 0x32 / 255.0f, 0x36 / 255.0f, 1.0f);
	glm::vec4 night_sky  = glm::vec4(0x18 / 255.0f, 0x21 / 255.0f, 0x3a / 255.0f, 1.0f);
	glm::vec4 moon_color = glm::vec4(0xd9 / 255.0f, 0xe2 / 255.0f, 0xc8 / 255.0f, 1.0f);
	glm::vec4 star_color = glm::vec4(0x9f / 255.0f, 0xb0 / 255.0f, 0xd0 / 255.0f, 1.0f);
	glm::vec4 sill_color = glm::vec4(0x7a / 255.0f, 0x88 / 255.0f, 0x8c / 255.0f, 1.0f);

	glm::vec2 win = glm::vec2(std::floor(bed_center_x - 12.0f), 228.0f);
	sprites.draw(sprites.white, win, glm::vec2(24.0f, 10.0f), frame_dark);
	sprites.draw(sprites.white, win + glm::vec2(1.0f, 1.0f), glm::vec2(22.0f, 8.0f), night_sky);
	sprites.draw(sprites.white, win + glm::vec2(16.0f, 4.0f), glm::vec2(3.0f, 3.0f), moon_color);
	sprites.draw(sprites.white, win + glm::vec2(17.0f, 5.0f), glm::vec2(2.0f, 2.0f), night_sky);
	sprites.draw(sprites.white, win + glm::vec2(4.0f, 6.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(8.0f, 3.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(21.0f, 2.0f), glm::vec2(1.0f, 1.0f), star_color);
	sprites.draw(sprites.white, win + glm::vec2(11.0f, 1.0f), glm::vec2(2.0f, 8.0f), frame_dark);
	sprites.draw(sprites.white, win + glm::vec2(-1.0f, -1.0f), glm::vec2(26.0f, 1.0f), sill_color);
}

const float HintFontSize = 8.0f;
const float HintLineHeight = 11.0f;

void PlayMode::draw_hint_box(std::string const lines[], int count, bool show_continue, float alpha) {
	glm::vec4 border = glm::vec4(0.5f, 0.5f, 0.5f, alpha);
	glm::vec4 inside = glm::vec4(0.00f, 0.00f, 0.00f, 0.85f * alpha);

	float height = 10.0f + count * HintLineHeight;
	glm::vec2 box_pos = glm::vec2(12.0f, 10.0f);
	glm::vec2 box_size = glm::vec2(232.0f, height);

	sprites.draw(sprites.white, box_pos - glm::vec2(1.0f), box_size + glm::vec2(2.0f), border);
	sprites.draw(sprites.white, box_pos, box_size, inside);

	for (int i = 0; i < count; i++) {
		HintLine line;
		line.text = lines[i];
		line.center_x = 128.0f;
		line.baseline_y = box_pos.y + height - 4.0f - HintFontSize - i * HintLineHeight;
		line.alpha = alpha;
		hint_lines.push_back(line);
	}

	if (show_continue && std::sin(hover_time * 5.0f) > -0.3f) {
		glm::vec4 arrow = glm::vec4(0.56f, 0.88f, 0.84f, alpha);
		glm::vec2 a = box_pos + glm::vec2(box_size.x - 8.0f, 3.0f);
		sprites.draw(sprites.white, a, glm::vec2(1.0f, 5.0f), arrow);
		sprites.draw(sprites.white, a + glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 3.0f), arrow);
		sprites.draw(sprites.white, a + glm::vec2(2.0f, 2.0f), glm::vec2(1.0f, 1.0f), arrow);
	}
}

//spotlight and the text boxes
void PlayMode::draw_tutorial() {
	if (spotlight_alpha > 0.0f) {
		glm::vec4 dim = glm::vec4(0.02f, 0.03f, 0.06f, 0.65f * spotlight_alpha);
		sprites.draw(sprites.white, glm::vec2(0.0f), glm::vec2(256.0f, 240.0f), dim);

		glm::vec2 arrow_target;
		if (spotlight_on_diary) {
			glm::vec2 size = glm::vec2(light_tex.size) * 0.4f;
			glm::vec4 warm = glm::vec4(1.0f, 0.9f, 0.7f, 0.6f * spotlight_alpha);
			sprites.draw(light_tex, diary_pos - size * 0.5f, size, warm);
			arrow_target = diary_pos + glm::vec2(0.0f, 8.0f);
		} else {
			if (!soul_taken) {
				draw_soul();
			}
			arrow_target = soul_pos + glm::vec2(0.0f, 16.0f);
		}

		float arrow_bob = std::floor(2.0f * std::sin(hover_time * 6.0f));
		glm::vec4 arrow = glm::vec4(0.56f, 0.88f, 0.84f, spotlight_alpha);
		glm::vec2 tip = glm::vec2(std::floor(arrow_target.x), std::floor(arrow_target.y + arrow_bob));
		sprites.draw(sprites.white, tip + glm::vec2(-3.0f, 3.0f), glm::vec2(7.0f, 1.0f), arrow);
		sprites.draw(sprites.white, tip + glm::vec2(-2.0f, 2.0f), glm::vec2(5.0f, 1.0f), arrow);
		sprites.draw(sprites.white, tip + glm::vec2(-1.0f, 1.0f), glm::vec2(3.0f, 1.0f), arrow);
		sprites.draw(sprites.white, tip, glm::vec2(1.0f, 1.0f), arrow);
	}

	std::string page_lines[2];
	if (hint_alpha <= 0.0f && tutorial_page_lines(tutorial_page, page_lines)) {
		float alpha = 1.0f;
		bool can_continue = true;
		if (is_spotlight_step()) {
			alpha = spotlight_alpha;
			can_continue = (spotlight_alpha >= 1.0f);
		}
		if (alpha > 0.0f) {
			draw_hint_box(page_lines, 2, can_continue, alpha);
		}
	}

	if (hint_alpha > 0.0f) {
		std::string lines[1] = {shown_hint};
		draw_hint_box(lines, 1, false, hint_alpha);
	}
}

void PlayMode::draw_hint_text(glm::uvec2 const &drawable_size) {
	if (hint_lines.empty()) {
		return;
	}

	float scale = std::floor(std::min(drawable_size.x / 256.0f, drawable_size.y / 240.0f));
	if (scale < 1.0f) {
		scale = 1.0f;
	}
	float view_x = std::floor((drawable_size.x - 256.0f * scale) * 0.5f);
	float view_y = std::floor((drawable_size.y - 240.0f * scale) * 0.5f);

	uint32_t pixels = uint32_t(HintFontSize * scale);
	if (!hint_font || hint_font->pixel_size != pixels) {
		hint_font = std::make_unique< Font >(data_path("LowresPixel-Regular.otf"), pixels);
		hint_cache.clear();
	}

	glViewport(0, 0, drawable_size.x, drawable_size.y);

	for (size_t i = 0; i < hint_lines.size(); i++) {
		HintLine const &line = hint_lines[i];

		auto found = hint_cache.find(line.text);
		if (found == hint_cache.end()) {
			found = hint_cache.emplace(line.text, render_text(*hint_font, line.text)).first;
		}
		TextImage const &image = found->second;

		float text_x = std::floor(view_x + line.center_x * scale - image.advance * 0.5f);
		float base = std::floor(view_y + line.baseline_y * scale);
		draw_text(image, glm::vec2(text_x, base), glm::vec4(1.0f, 1.0f, 1.0f, line.alpha), drawable_size);
	}

	hint_lines.clear();
}

//the patient's soul
void PlayMode::draw_soul() {
	float pulse = 0.5f + 0.5f * std::sin(hover_time * 2.0f);
	float bob = std::floor(1.5f * std::sin(hover_time * 1.5f));

	glm::vec2 glow_size = glm::vec2(light_tex.size) * 0.5f;
	glm::vec4 glow_tint = glm::vec4(0.56f, 0.88f, 0.84f, 0.25f + 0.25f * pulse);
	sprites.draw(light_tex, soul_pos - glow_size * 0.5f + glm::vec2(0.0f, bob), glow_size, glow_tint);

	glm::vec2 soul_size = glm::vec2(soul_tex.size) * 0.2f;
	glm::vec4 soul_tint = glm::vec4(1.0f, 1.0f, 1.0f, 0.7f + 0.3f * pulse);
	sprites.draw(soul_tex, soul_pos - soul_size * 0.5f + glm::vec2(0.0f, bob), soul_size, soul_tint);
}

bool PlayMode::in_tutorial() {
	if (tutorial_step == TUT_OFF || tutorial_step == TUT_DONE) {
		return false;
	}
	return true;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	sprites.begin(drawable_size);

	const float FloorTileSize = 32.0f;

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
	glm::vec4 door_color      = glm::vec4(0x40 / 255.0f, 0x48 / 255.0f, 0x50 / 255.0f, 1.0f);

	for (int y = 0; y < MapHeight; y++) {
		for (int x = 0; x < MapWidth; x++) {
			char c = room[y][x];
			glm::vec2 pos = glm::vec2(x * TileSize, y * TileSize);

			if (c == '#') {
				const int WallFaceHeight = 2;
				int dist = room_floor_distance_below(room, x, y);

				if (dist <= WallFaceHeight) {
					sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_face_color);
					if (dist == 1) {
						sprites.draw(sprites.white, pos, glm::vec2(TileSize, 2.0f), baseboard_color);
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, -2.0f), glm::vec2(TileSize, 2.0f), wall_shadow_color);
					}
					if (dist == WallFaceHeight) {
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
					}
				} else {
					sprites.draw(sprites.white, pos, glm::vec2(TileSize), wall_color);
					if (room_is_floor(room, x, y + 1)) {
						sprites.draw(sprites.white, pos + glm::vec2(0.0f, TileSize - 1.0f), glm::vec2(TileSize, 1.0f), wall_edge_color);
					}
				}
			} else if (c == '=') {
				sprites.draw(sprites.white, pos, glm::vec2(TileSize), door_color);
			}
		}
	}

	if (!soul_taken) {
		draw_window();
	}

	glm::vec4 frame_dark  = glm::vec4(0x2a / 255.0f, 0x32 / 255.0f, 0x36 / 255.0f, 1.0f);
	glm::vec4 paper_color = glm::vec4(0xc9 / 255.0f, 0xc4 / 255.0f, 0xb4 / 255.0f, 1.0f);
	glm::vec4 ink_color   = glm::vec4(0x7a / 255.0f, 0x74 / 255.0f, 0x66 / 255.0f, 1.0f);
	glm::vec4 clock_face  = glm::vec4(0xd8 / 255.0f, 0xdc / 255.0f, 0xd6 / 255.0f, 1.0f);

	glm::vec2 chart = glm::vec2(std::floor(bed_center_x - 30.0f), 228.0f);
	sprites.draw(sprites.white, chart, glm::vec2(6.0f, 9.0f), paper_color);
	sprites.draw(sprites.white, chart + glm::vec2(2.0f, 8.0f), glm::vec2(2.0f, 2.0f), frame_dark);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 6.0f), glm::vec2(4.0f, 1.0f), ink_color);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 4.0f), glm::vec2(3.0f, 1.0f), ink_color);
	sprites.draw(sprites.white, chart + glm::vec2(1.0f, 2.0f), glm::vec2(4.0f, 1.0f), ink_color);

	glm::vec2 clock = glm::vec2(std::floor(bed_center_x + 24.0f), 229.0f);
	sprites.draw(sprites.white, clock, glm::vec2(7.0f, 7.0f), frame_dark);
	sprites.draw(sprites.white, clock + glm::vec2(1.0f, 1.0f), glm::vec2(5.0f, 5.0f), clock_face);
	sprites.draw(sprites.white, clock + glm::vec2(3.0f, 3.0f), glm::vec2(1.0f, 3.0f), frame_dark);
	sprites.draw(sprites.white, clock + glm::vec2(3.0f, 3.0f), glm::vec2(2.0f, 1.0f), frame_dark);


	//light pools
	//reference: https://catlikecoding.com/godot/true-top-down-2d/4-light-and-shadow/
	if (!soul_taken) {
		glm::vec4 light_tint = glm::vec4(1.0f, 1.0f, 1.0f, 0.35f);
		for (size_t i = 0; i < lights.size(); i++) {
			glm::vec2 size = glm::vec2(light_tex.size);
			sprites.draw(light_tex, lights[i] - size * 0.5f, size, light_tint);
		}
	}

	//shadows
	float hover = 2.0f * std::sin(hover_time * 3.0f);

	glm::vec4 prop_shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.35f);
	glm::vec2 prop_shadow_offset = glm::vec2(2.0f, -2.0f);
	for (size_t i = 0; i < props.size(); i++) {
		Sprites::Texture t = texture_for(props[i]);
		sprites.draw(t, props[i].pos + prop_shadow_offset, glm::vec2(t.size), prop_shadow_color);
	}

	float shadow_width = 12.0f - hover;
	draw_floor_shadow(sprites, player_at.x + ReaperSize * 0.5f, player_at.y - 3.0f, shadow_width);

	//props and reaper
	for (size_t i = 0; i < props.size(); i++) {
		if (props[i].pos.y >= player_at.y) {
			Sprites::Texture t = texture_for(props[i]);
			sprites.draw(t, props[i].pos, glm::vec2(t.size));
		}
	}

	sprites.draw(reaper_tex, glm::vec2(player_at.x, player_at.y + hover), glm::vec2(ReaperSize));

	for (size_t i = 0; i < props.size(); i++) {
		if (props[i].pos.y < player_at.y) {
			sprites.draw(texture_for(props[i]), props[i].pos, glm::vec2(texture_for(props[i]).size));
		}
	}

	glm::vec2 screen_size = glm::vec2(MapWidth * TileSize, MapHeight * TileSize);

	//darker room after death
	if (soul_taken) {
		glm::vec4 darkness = glm::vec4(0.02f, 0.03f, 0.06f, 0.45f);
		sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, darkness);
	}

	glm::vec4 cold_tint = glm::vec4(0.44f, 0.65f, 0.77f, 0.06f);
	sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, cold_tint);

	sprites.draw(vignette_tex, glm::vec2(0.0f, 0.0f), screen_size);

	if (soul_taken) {
		glm::vec2 moon_size = glm::vec2(light_tex.size) * 0.5f;
		glm::vec4 moon_tint = glm::vec4(0.6f, 0.7f, 1.0f, 0.4f);
		sprites.draw(light_tex, glm::vec2(bed_center_x, 210.0f) - moon_size * 0.5f, moon_size, moon_tint);

		draw_window();
	}

	if (!soul_taken) {
		draw_soul();
	}

	//interact marker
	if (nearby != -1) {
		glm::vec2 marker_size = glm::vec2(interact_tex.size);
		glm::vec2 marker_pos = glm::vec2(player_at.x + (ReaperSize - marker_size.x) * 0.5f, player_at.y + ReaperSize + 2.0f + hover);
		sprites.draw(interact_tex, marker_pos, marker_size);
	}

	draw_tutorial();

	float fade = fade_in;
	if (fade_out > fade) {
		fade = fade_out;
	}
	if (fade > 0.0f) {
		glm::vec4 fade_color = glm::vec4(0.0f, 0.0f, 0.0f, fade);
		sprites.draw(sprites.white, glm::vec2(0.0f, 0.0f), screen_size, fade_color);
	}

	sprites.end(drawable_size);

	if (fade_in <= 0.0f && fade_out <= 0.0f) {
		draw_hint_text(drawable_size);
	} else {
		hint_lines.clear();
	}
}


void PlayMode::reset_buttons() {
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

//start the soul minigame
void PlayMode::start_minigame() {
	SoulCaptureMode::Difficulty difficulty = SoulCaptureMode::hard();
	if (diary_read) {
		difficulty = SoulCaptureMode::normal();
	}
	if (tutorial_step == TUT_FIRST_TRY || tutorial_step == TUT_SECOND_TRY) {
		difficulty = SoulCaptureMode::normal();
	}

	std::shared_ptr< SoulCaptureMode > minigame = std::make_shared< SoulCaptureMode >(difficulty);
	minigame->return_to = shared_from_this();

	if (tutorial_step == TUT_FIRST_TRY) {
		minigame->tutorial = true;
		minigame->resist_after_first_hit = true;
	}
	if (tutorial_step == TUT_SECOND_TRY) {
		minigame->tutorial = true;
	}

	active_minigame = minigame;
	start_leaving(minigame);
}

//remember where to go and start fading out
void PlayMode::start_leaving(std::shared_ptr< Mode > next) {
	leaving_to = next;
	fade_out = 0.0f;
	reset_buttons();
}