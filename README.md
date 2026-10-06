# Demon's Cove

An arena shooter built with [ezEngine](https://ezengine.net) as an engine demo. It is a fan-made homage to [Devil Daggers](https://store.steampowered.com/app/422970/Devil_Daggers/) by Sorath, which the game idea comes from.

Survive as long as possible on a floating platform. Daggers kill the enemies, enemies drop gems, gems upgrade your hand.

![Screenshot](Screenshots/Screenshot1.jpg)

## Purpose of This Project

This demo was built in a day using Claude Code and [ezEngine](https://ezengine.net). Its main purpose is to demonstrate that ezEngine can be used productively with AI to develop games. This specific game was chosen because it was easy for the AI to make with little supervision. Using ezEngine's editor and MCP integration, the AI was able to import all required assets, build the scene and prefabs, run the game and iterate on the functionality, until it was playable. In total this took about 4 hours.

In no way is this meant to be a real, finished game. Turning this from a proof-of-concept prototype into a fully fleshed out, polished game, would still require a lot of manual work and creativity. It is merely meant to demonstrate, that ezEngine has the infrastructure to leverage AI tools for such purposes.

## Prebuilt Binaries

Download prebuilt binaries from the [Releases](https://github.com/jankrassnigg/demons-cove/releases), unpack the archive and launch the game with the `.bat` file.

## Building and Running the game

Demon's Cove needs a recent ezEngine build from the `dev` branch, with the editor and the MiniAudio, Jolt, RmlUi and particle plugins.

1. Open this folder as a project in ezEditor. The editor offers to compile the C++ plugin in `CppSource`; let it do that.
2. Open `Scenes/Arena.ezScene` and play it (*Play the Game*, Ctrl+F5).

To run it without the editor, export the project (*Project > Export Project*) and use the `.bat` file in the export folder to launch the game.

![Screenshot](Screenshots/Screenshot2.jpg)

## Controls

| Keyboard and mouse | Gamepad | Action |
| --- | --- | --- |
| WASD / mouse | Left stick / right stick | Move and look |
| Space | A or left shoulder | Jump |
| Left mouse click | Right trigger or right shoulder, tap | Shotgun burst |
| Left mouse hold | Right trigger or right shoulder, hold | Dagger stream |
| R | Y | Restart the round |
| ESC | Start | Menu (resume, settings, exit) |

The first round starts when you pick up the dagger floating in front of you.

## Enemies

| Enemy | Behavior |
| --- | --- |
| Squid I / II | Hover around the arena and release skull swarms. They only take damage at their glowing gems (one, or two for Squid II); the body blocks daggers. |
| Skull I | Swarm enemy, homes in on you. |
| Skull II | Circles you and dashes at you. |
| Skeleton | Groups that climb onto the platform and run at you. |
| Thief | Crawls over the floor and collects gems lying around. Kill it to get them back. |
| Centipede | Flies through the arena with a long body. Segments can be shot off, killing the head kills the whole body. |

Hand levels: level 2 at 10 gems (faster stream, bigger shotgun), level 3 at 50 gems (homing daggers).

During the endless phase (stage 6) the arena crumbles at the edge. Tiles turn red shortly before they fall.

The 10 best rounds are kept in a scoreboard in the user's app data folder (`ezPlayer/DemonCove/Scores.txt`, one `time kills gems` line per round). The death screen shows it and highlights the place of the round just played. Rounds with any of the debug CVars below don't count.

## Debug CVars

All of these can be set on the command line, for example `-DemonCove.GodMode 1 -DemonCove.StartStage 5`.

| CVar | Effect |
| --- | --- |
| `DemonCove.GodMode` | Enemies can't kill you (they disappear on contact), and a wall around the arena prevents falling off. Falling into a crumbled hole respawns you. |
| `DemonCove.StartStage` | Stage a new round starts at (1-6, 6 is the endless phase). |
| `DemonCove.StartGems` | Gems at the start of a round, which determines the hand level. |
| `DemonCove.DebugAutoAim` | Daggers fly towards the closest enemy (or its gem) and the view turns towards it. For automated tests. |
| `DemonCove.DebugSpawn` | Set to an enemy type index to spawn one: 0 Squid I, 1 Squid II, 5 Thief, 6 Centipede. Resets itself. |
| `DemonCove.DebugStress` | Set to 1 to spawn the worst case number of enemies and gems at once, for performance measurements. |
| `DemonCove.ShowStats` | Shows dagger, kill and per-enemy-type statistics. |
| `DemonCove.GemAttractRange` | Distance at which gems fly towards you. |

## Code

The game logic is in the C++ plugin in `CppSource/DemonCovePlugin`:

- `DcArenaComponent` runs a round: spawn timeline, enemies, daggers, gems, the tile floor, death and restart. Enemies, daggers and gems are plain data, their game objects (prefab instances) are only visuals. Collision checks are sphere tests.
- `DcPlayerComponent` handles input, movement through the Jolt character controller, and firing.
- `DemonCoveGameState` opens the RmlUi main menu on ESC.

The HUD (`UI/Hud.rml`) is an RmlUi document bound to a blackboard named `hud`, which the arena fills every frame.

## Credits

- Game idea: [Devil Daggers](https://store.steampowered.com/app/422970/Devil_Daggers/) by Sorath.
- 3D models: [Quaternius](https://quaternius.com) (CC0).
- Sounds and music: various authors on [Freesound](https://freesound.org), under CC0, CC BY and Sampling+ licenses.

[Credits.md](Credits.md) lists every model pack and every sound with its author and license.
