You play Death on your first day on the job. The quota is one soul.

Walk straight up to the patient and take what you came for, or read his diary first and find out what he is holding on to. A patient whose story you know puts up less of a fight.

## Running it

Every platform has two ways: paste one command, or download the archive by hand and click through it. The command is a single line that downloads, unpacks and starts the game, and it does not trip any security warning, because files fetched from a terminal are not marked as downloaded.

**macOS** (universal, runs on Apple silicon and Intel):

```
cd ~/Downloads && curl -fLO https://github.com/dx2102/Death2/releases/download/RELEASE_TAG/death2-macos-RELEASE_TAG.tar.gz && tar xzf death2-macos-RELEASE_TAG.tar.gz && ./death2-macos-RELEASE_TAG/game
```

**Linux**:

```
cd ~/Downloads && curl -fLO https://github.com/dx2102/Death2/releases/download/RELEASE_TAG/death2-linux-RELEASE_TAG.tar.gz && tar xzf death2-linux-RELEASE_TAG.tar.gz && ./death2-linux-RELEASE_TAG/game
```

**Windows** (PowerShell):

```
cd ~\Downloads; curl.exe -fLO https://github.com/dx2102/Death2/releases/download/RELEASE_TAG/death2-windows-RELEASE_TAG.zip; tar -xf death2-windows-RELEASE_TAG.zip; .\death2-windows-RELEASE_TAG\game.exe
```

The other way, by hand:

- Download the archive for your system from the list below and unpack it into a folder.
- Run `game`, or `game.exe` on Windows. Do not run it from inside the zip viewer, the game will not find its assets.
- Windows may show a SmartScreen warning: More info, then Run anyway.
- macOS refuses to open anything a browser downloaded. Clear that flag and start the game with one command:

```
xattr -dr com.apple.quarantine ~/Downloads/death2-macos-RELEASE_TAG && ~/Downloads/death2-macos-RELEASE_TAG/game
```

## Credits

- Soul capture minigame, pixel canvas: Deming Xu
- Dialogue system, text rendering, ward: Raeshana
- Pixel art and music: Mikhail Titov
- Script and fonts: Aren Davey

Dialogue runs on [inkCPP](https://github.com/JBenda/inkcpp) (MIT), built from the sources in `ink/runtime`.
