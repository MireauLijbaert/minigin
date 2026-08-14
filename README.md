# BurgerTime

Source repository: https://github.com/MireauLijbaert/minigin

## Controls

| Action | Player 1 | Player 2 (co-op / versus) |
|---|---|---|
| Move | WASD / Gamepad 0 D-pad | IJKL / Gamepad 1 D-pad |
| Pepper | Space / Gamepad 0 Y | , (comma) / Gamepad 1 Y |
| Pause | P / Gamepad Start | - |
| Mute | F2 | - |
| Skip level | F1 | - |

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

A lightweight publish-subscribe system (`Subject` + `Observer`) decouples game logic from UI and audio. Components that want to broadcast events own a `Subject` interested components register as `Observer`s and receive typed `Event` structs. Used for score changes, life changes, pepper count, level completion, burger events, and enemy kills. Observers unregister in their destructor via `RemoveObserver` and null their subject pointer in `OnSubjectDestroyed`, so there are no dangling observer pointers when a subject is destroyed.


### State Machines

Inline enum-based state machines are used throughout `EnemyComponent` (9 states: Entering, Walking, Stunned, FallingWithBurger, Squished, Dead, Waiting, RecoveringFromBurger, ClimbingFromCup), `PlatformMovementComponent` (Alive, Dying, Dead), `BurgerPieceComponent` (Idle, Falling, Dead), and `VersusEnemyPlayerComponent` (6 states).

### Level Format

Levels are plain-text `.txt` files. Each line is a row of the grid:

```
P = Player spawn
t,b,l,o,c = burger ingredients
C = cup (plate) position
S = enemy spawn marker
- = platform position (other markes also get platforms)
| = ladder position
```

### High Score

High scores are saved to `Data/highscores.txt` (plain text, one `NAME SCORE` entry per line, top 5 kept). Name entry uses an arcade-style letter grid navigable by arrow keys or D-pad no keyboard typing required, matching how real arcade cabinets work.
