#include "GameAudio.hpp"

#include "Sound.hpp"
#include "Load.hpp"
#include "data_path.hpp"

#include <memory>

//The two music files are mastered very quietly (peaks around 0.05 of full scale) while the
//effects are close to full scale, so the music is turned up and the effects are turned down
//until they sit at about the same loudness, with the peaks still well under 1.
static constexpr float MusicVolume = 6.0f;
static constexpr float FadeSeconds = 1.0f;

static Load< Sound::Sample > ward_track(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Hospital_Theme.wav"));
});
static Load< Sound::Sample > battle_track(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("soul_capture.wav"));
});
static Load< Sound::Sample > scythe_sound(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("scythe.wav"));
});
static Load< Sound::Sample > hit_sound(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("hit.wav"));
});
static Load< Sound::Sample > miss_sound(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("miss.wav"));
});

static Sound::Sample const *current_track = nullptr;
static std::shared_ptr< Sound::PlayingSample > current_music;

static void play_track(Sound::Sample const &track) {
	if (current_track == &track) return; //already playing, keep it going
	if (current_music) current_music->stop(FadeSeconds);
	current_track = &track;
	current_music = Sound::loop(track, 0.0f);
	current_music->set_volume(MusicVolume, FadeSeconds);
}

void GameAudio::ward_music() { play_track(*ward_track); }
void GameAudio::battle_music() { play_track(*battle_track); }

void GameAudio::scythe() { Sound::play(*scythe_sound, 0.35f); }
void GameAudio::hit() { Sound::play(*hit_sound, 0.6f); }
void GameAudio::miss() { Sound::play(*miss_sound, 0.6f); }
