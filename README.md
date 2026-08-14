# BurgerTime — Programming 4 Exam Project

Source repository: https://github.com/MireauLijbaert/minigin

## Controls

| Action | Player 1 | Player 2 (co-op / versus) |
|---|---|---|
| Move | WASD / Gamepad 0 D-pad | IJKL / Gamepad 1 D-pad |
| Pepper | Space / Gamepad 0 Y | , (comma) / Gamepad 1 Y |
| Pause | P / Gamepad Start | — |
| Mute | F2 | — |
| Skip level | F1 | — |

Title screen: WASD / arrow keys or D-pad to select mode, Enter / Space / A to confirm. Shortcut keys 1 / 2 / 3 jump straight to the matching mode. Name entry (game over screen) also uses WASD / arrow keys to navigate the letter grid.

## Game Modes

- **Single Player** - Peter Pepper vs AI enemies.
- **Co-op** - Peter Pepper (P1) and Mrs. Salt (P2, tinted blue) share lives and play simultaneously.
- **Versus** - P1 plays as Peter Pepper; P2 takes control of the first Hot Dog enemy.

## Engine Design

The engine is built on the **minigin** template and uses SDL3 for windowing, rendering, input, and audio.

### Component System

Every `GameObject` is a bag of `BaseComponent` subclasses. Each component implements `Update()` and/or `Render()`. The engine calls these each frame via the **Update Method** and **Game Loop** patterns, no component needs to manage its own loop.

### Observer / Subject

A lightweight publish-subscribe system (`Subject` + `Observer`) decouples game logic from UI and audio. Components that want to broadcast events own a `Subject`; interested components register as `Observer`s and receive typed `Event` structs. Used for score changes, life changes, pepper count, level completion, burger events, and enemy kills. Observers unregister in their destructor via `RemoveObserver` and null their subject pointer in `OnSubjectDestroyed`, so there are no dangling observer pointers when a subject is destroyed.

### Command / Input

`InputManager` maps keyboard scancodes and XInput gamepad buttons to `Command` objects in three states: Down, Up, Held. `LambdaCommand` wraps a `std::function` so a binding can be set up in one line without a dedicated class. Components that bind keys in their constructor overwrite the binding with a no-op lambda in their destructor, preventing dangling `this` captures across scene transitions.

### State Machines

Inline enum-based state machines are used throughout - `EnemyComponent` (9 states: Entering, Walking, Stunned, FallingWithBurger, Squished, Dead, Waiting, RecoveringFromBurger, ClimbingFromCup), `PlatformMovementComponent` (Alive, Dying, Dead), `BurgerPieceComponent` (Idle, Falling, Dead), and `VersusEnemyPlayerComponent` (6 states). This is the lightweight form of the State pattern from *Game Programming Patterns*: all transitions are visible in a single switch, no heap allocation per state, no virtual dispatch overhead.

### Service Locator + Threading

The sound system is accessed through `ServiceLocator`, which returns either the real `SdlSoundSystem` or a `NullSoundSystem` (silent fallback). `SdlSoundSystem` runs a dedicated **worker thread** with a mutex-protected request queue (the Event Queue pattern). The game thread enqueues fire-and-forget requests — `Play`, `PlayMusic`, `SetMuted`, etc. and the worker thread processes them and manages all SDL_mixer calls. This keeps audio off the main thread entirely.

### Level Format

Levels are plain-text `.txt` files. Each line is a row of the grid:

```
P = platform tile
L = ladder tile
B = burger ingredient (type encoded by column position)
C = cup (plate) position
S = enemy spawn marker (with encoded type and delay)
```

`LevelLoader` parses the file into a `LevelData` struct (platform rows, ladders, cup definitions, burger layout, spawn points, bonus position). An integer overload `LevelLoader::Load(int levelNum, ...)` handles level cycling and difficulty scaling, so `Main.cpp` only passes a level number and gets back a fully prepared `LevelData`.

### Rendering

Game logic never calls SDL render functions directly. All sprite display goes through `RenderComponent`, which supports `SetVisible(bool)` for show/hide without removing objects from the scene. The only exceptions are components with custom multi-segment rendering (burger pieces, HUD life icons, score popups) that do targeted SDL draws in their own `Render()` override.

### High Score

High scores are saved to `Data/highscores.txt` (plain text, one `NAME SCORE` entry per line, top 5 kept). Name entry uses an arcade-style letter grid navigable by arrow keys or D-pad — no keyboard typing required, matching how real arcade cabinets work.

## Project Structure

```
Minigin/     Engine library (static)
BurgerTime/  Game executable
Data/        Assets: textures, sounds, fonts, level .txt files
cmake/       FindSteamworks, Emscripten toolchain helpers
web/         Emscripten shell page
```
