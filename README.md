# Death

You play Death on your first day on the job. The quota is one soul.

You can walk straight up to the patient and take what you came for, or read his
diary first and find out what he is holding on to. A patient whose story you know
puts up less of a fight.

## How to run

Grab the build for your system from the Releases page, unpack it, and run `game`
(`game.exe` on Windows). Each release lists a one line command that downloads,
unpacks and starts the game without tripping any security warning.

## Controls

- Ward: `WASD` to move, `E` to interact. The diary is on the bedside desk.
- Dialogue: `Space` to advance, number keys to pick a choice.
- Soul capture: `WASD` moves the scythe, the mouse aims it, `Space` or left click
  cuts. Cut across the yellow lifeline. The blade turns green when a cut would
  land. Five hits and the soul is yours. `Esc` steps back into the ward.

## Building it yourself

You need Node.js and a C++ compiler. On macOS and Linux:

```
./run.sh
```

That downloads nest-libs next to the repo on the first run, builds, and starts the
game. On Windows, see NEST.md.

inkCPP is built from the sources in `ink/runtime`, so there is nothing else to
fetch and no prebuilt library in the repo. Set `MACOS_UNIVERSAL=1` to build a
macOS binary that runs on both Apple silicon and Intel; it is off by default
because it compiles everything twice.

## Credits

- Soul capture minigame, pixel canvas: Deming Xu
- Dialogue system, text rendering, ward: Raeshana
- Pixel art and music: Mikhail Titov
- Script and fonts: Aren Davey

Dialogue runs on [inkCPP](https://github.com/JBenda/inkcpp) (MIT).
