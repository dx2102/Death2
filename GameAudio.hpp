#pragma once

//Background music and sound effects for the game.
//Only one music track plays at a time; switching tracks fades the old one out and the new one in.

namespace GameAudio {
	void ward_music();   //the hospital theme, for the rooms and the hallway
	void battle_music(); //the soul capture theme, for the minigame

	void scythe(); //the blade swings
	void hit();    //the cut went across the lifeline
	void miss();   //the cut missed, was blocked, or was shielded
}
