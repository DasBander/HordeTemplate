# Horde Template V2 (Native) - v2.7.3

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue)](https://www.unrealengine.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-Native-orange)](https://docs.unrealengine.com/5.7/en-US/programming-with-cplusplus-in-unreal-engine/)

> *"If You Love Something, Set It Free"* - March 2, 2015 Tim

A fully-featured, open-source **C++ framework** for creating 4-player cooperative horde/zombie survival games in Unreal Engine 5.7.

Originally a premium Unreal Marketplace asset, now released to the community for free under the MIT License.

**[Marketplace Page](https://www.unrealengine.com/marketplace/en-US/product/horde-template-v2-native-horde-template-framework)** | **[Documentation](https://docs.google.com/document/d/1iBH7lUJbGce0pt5V6JpFJs_xdPA5Gh6os2kZ5imvwf8/edit?tab=t.0#heading=h.5x0d5h95i329)** | **[Report Issues](https://github.com/DasBander/HordeTemplateV2Native/issues)**

---

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Core Systems](#core-systems)
- [Configuration](#configuration)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [License](#license)
- [Credits](#credits)

---

## Features

### Gameplay Systems
- **4-Player Cooperative Multiplayer** - Full network replication with Steam integration
- **Wave-Based Zombie Survival** - Configurable rounds with difficulty scaling
- **Advanced Lobby System** - Character selection, player management, ready states
- **Trader/Shop System** - Buy weapons and items between rounds
- **Seamless Map Travel** - Map rotation with server travel support

### Combat & Inventory
- **Modular Weapon System** - Support for multiple fire modes (Single, Burst, Full-Auto)
- **Projectile-Based Combat** - Headshot detection with bonus rewards
- **Advanced Inventory System** - Drop & pickup mechanics, ammo management
- **Health & Stamina** - Sprint mechanics with stamina drain/recovery

### AI & Enemies
- **Zombie AI with Behavior Trees** - Patrol, chase, and attack behaviors
- **AI Perception System** - Sight-based detection with configurable ranges
- **Spawn Point System** - Flexible zombie spawning with wave multipliers

### UI & HUD
- **Complete HUD System** - Health, ammo, interaction prompts, crosshair
- **In-Game Chat** - Lobby and game chat with command support
- **Scoreboard** - Kill tracking, headshots, and point system
- **Spectator Mode** - Watch teammates after death

### Technical
- **100% C++ Native** - No Blueprint dependencies for core systems
- **Fully Documented Code** - Extensive comments and documentation
- **Data-Driven Design** - DataTables for items, characters, and maps
- **Modular Architecture** - Easy to extend and customize

---

## Getting Started

### Requirements
- Unreal Engine 5.7
- Visual Studio 2022 (Windows) or Xcode (macOS)
- ~2GB disk space

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/DasBander/HordeTemplateV2Native.git
   ```

2. **Generate project files**
   - Right-click `HordeTemplateV2Native.uproject`
   - Select "Generate Visual Studio project files"

3. **Open in Unreal Engine**
   - Double-click `HordeTemplateV2Native.uproject`
   - Or open via Epic Games Launcher

4. **Build the project**
   - Press `Ctrl+Shift+B` in Visual Studio
   - Or use `Build > Build Solution`

### Quick Start

1. Open the **TestingMap** level (`Content/HordeTemplateBP/Maps/TestingMap`)
2. Press **Play** to test in editor
3. For multiplayer testing, use **Play > Advanced Settings > Number of Players**

---

## Project Structure

```
HordeTemplateV2Native/
├── Source/HordeTemplateV2Native/
│   ├── Public/                    # Header files
│   │   ├── AI/                    # Zombie AI (Pawn, Controller, Tasks)
│   │   ├── Animation/             # Animation instance classes
│   │   ├── Character/             # Player character & spectator
│   │   ├── FX/                    # Camera shakes & impact effects
│   │   ├── Gameplay/              # GameMode, GameState, Controllers
│   │   ├── HUD/                   # HUD & Widget classes
│   │   ├── Inventory/             # Inventory system
│   │   ├── Misc/                  # Doors, Trader, Triggers
│   │   ├── Projectiles/           # Projectile classes
│   │   └── Weapons/               # Firearm classes
│   └── Private/                   # Implementation files
├── Content/HordeTemplateBP/
│   ├── Maps/                      # Game levels
│   ├── Blueprint/                 # Character & Widget blueprints
│   ├── Data/                      # DataTables (Items, Characters, Maps)
│   └── Assets/                    # Meshes, Materials, Sounds, etc.
└── Config/                        # Engine & project configuration
```

---

## Core Systems

### Character System (`AHordeBaseCharacter`)
- Health & damage handling with death/respawn logic
- Stamina-based sprinting
- Weapon equipping and firing
- Interaction system for pickups and traders
- 3D player name/health display

### AI System (`AZedPawn` + `AZedAIController`)
- Behavior Tree-driven AI (`BT_Zed`)
- Patrol, chase, and attack behaviors
- Headshot detection for bonus points
- Configurable sight and chase ranges

### Inventory System (`UInventoryComponent`)
- Item pickup and dropping
- Ammo stacking and management
- Weapon slot system (Primary, Secondary, Healing)
- DataTable-driven item definitions

### Game Flow (`AHordeGameMode` + `AHordeGameState`)
- Lobby state with character selection
- Wave-based zombie spawning
- Round progression with pause/trade phases
- End-game scoring and map rotation

---

## Configuration

### Key Definitions (`HordeTemplateV2Native.h`)

```cpp
#define GAME_VERSION 2.7
#define STARTING_MONEY 1500
#define ZED_SIGHT_RADIUS 1000.f
#define ZED_LOSE_SIGHT_RADIUS 2500.f
```

### World Settings
Configure per-map settings via `AHordeWorldSettings`:
- Starting items
- Available characters
- Match mode (Linear/NonLinear)
- Round time and max rounds
- Zombie multiplier

### Input Bindings
Default controls (configurable in `Config/DefaultInput.ini`):
- **WASD** - Movement
- **Space** - Jump
- **Shift** - Sprint
- **E** - Interact
- **Q** - Drop item
- **R** - Reload
- **V** - Toggle fire mode
- **1/2/3** - Weapon slots
- **Tab** - Scoreboard

---

## Changelog

### Update 2.7.3
```
Spectator System Fixes:
- Fixed BaseSpectator using TObjectIterator instead of TActorIterator (was iterating ALL objects in memory, not just current world)
- Fixed BaseSpectator ClientFocusPlayer missing null check for Player parameter

AI System Fixes:
- Fixed AIAttackPlayer crash when GetAIOwner() returns null
- Fixed MoveToEnemy calling MoveToActor with potentially null BChar parameter
- Fixed MoveToPatrolPoint FMath::RandRange crash with empty PatrolLocations array (RandRange(0, -1))
- Fixed IsEnemyDead service crash when GetAIOwner() returns null (2 instances)

Gameplay System Fixes:
- Fixed HordeGameMode GameStart calling PC->Possess with null Character (moved inside null check)

Weapon System Fixes:
- Fixed BaseFirearm ServerToggleFireMode crash when FireModes array is empty
- Fixed BaseFirearm ServerToggleFireMode not handling case when current fire mode isn't in array

HUD System Fixes:
- Fixed HordeBaseHUD CloseTraderUI not checking if PlayerTraderWidget is null before RemoveFromParent

Projectile System Fixes:
- Fixed ExplosiveProjectile crash when GetOwner() returns null during OnProjectileImpact (owner destroyed before impact)

Controller System Fixes:
- Fixed HordeBaseController DisconnectFromServer calling Client RPC from client (RPC misuse - now uses QuitGame directly)

Replication Fixes:
- Fixed InventoryComponent OnRep_ActiveItemIndex accessing non-replicated Inventory array (now uses replicated firearm data)
- Fixed InventoryBaseItem mesh not appearing on clients (added OnRep_ItemInfo for proper client-side setup)

Replication Optimizations:
- Removed unnecessary replication of ZedPawn PatrolTag (only used server-side for AI behavior)
```

### Update 2.7.1
```
AI System Fixes:
- Fixed ZedPawn OnCharacterOutRange decrementing counter for dead players (mismatched with OnCharacterInRange)
- Fixed AISpawnPoint not tracking multiple characters correctly (same counter bug pattern)

Lobby & Trade Fixes:
- Fixed AcceptCharacterTrade crash when character indices are invalid (missing bounds check)
- Fixed multiple GetUniqueId() calls without null checks (HordeGameState, HordePlayerState)
- Fixed GetControllerByID crash when PlayerState is null

Weapon & Medical Fixes:
- Fixed Med_VAC StartHealing crash when AnimInstance is null

Replication Optimizations:
- Optimized Stamina replication to owner-only (reduces bandwidth)
- Optimized InventoryComponent ActiveItemIndex/AvailableAmmo to owner-only
- Optimized BaseFirearm LoadedAmmo/FireMode to owner-only
```

### Update 2.7.0 (UE5.7)
```
Engine & Configuration:
- Upgraded to Unreal Engine 5.7
- Removed deprecated MagicLeap plugins
- Updated project version to 2.7.0.0

Character System Fixes:
- Fixed crash in GetRandomPlayerSpawn when no spawn points exist
- Fixed null PlayerState access in RemoveHealth/AddHealth functions
- Fixed PlayAnimationAllClients ignoring the Montage parameter
- Fixed reload check comparing wrong ammo value (DefaultLoadedAmmo vs MaximumLoadedAmmo)
- Fixed FinishReload partial ammo bug (was replacing instead of adding to loaded ammo)
- Fixed null checks for GetHUD/GetHUDWidget in StartInteraction/StopInteraction

AI System Fixes:
- Fixed PlayerInRange tracking not handling multiple players correctly (added counter)
- Fixed ZedAIController sight timer logic (was only resetting if already active)
- Added dead player check when AI detects enemies

Inventory & Economy Fixes:
- Fixed BuyItem affordability check logic error (always evaluated to true)
- Fixed projectile damage not crediting kills to shooter (missing InstigatorController)

Weapon System Fixes:
- Fixed crash when firing weapon with null ProjectileClass

Lobby System Fixes:
- Fixed operator precedence bug in FreeupUnassignedCharacters (trade abort logic)
- Fixed ResetLobbyTime sending "interrupted" message when timer wasn't active
- Added mid-game player join handling (spawns as spectator instead of being stuck)

UI/HUD Fixes:
- Fixed memory leaks from heap-allocated FInputMode objects (3 instances)
- Fixed null checks for PlayerTravelWidget, PlayerScoreboardWidget, PlayerEscapeWidget, PlayerTraderWidget
- Fixed null check in UpdateLobbyPlayerList for LobbyWidget
- Fixed EndPlay crash from null widget pointers (added cleanup for PlayerEscapeWidget)
```

### Update 2.6 (UE5.6)
```
- Upgraded to Unreal Engine 5.6
- Fixed Font Issues in the HUD
- Fixed HUD Boxes being Rounded Boxes
- Fixed Animation Crash from bad references
- Fixed Inventory not being properly initialized
- Fixed Character spawning issues
- Changed SubCharacter to Blueprints
- Added failsafe changes for stability
```

---

## Asset Statistics

| Category | Count |
|----------|-------|
| C++ Classes | 60 |
| Widgets | 28 |
| Materials | 53 |
| Textures | 70 |
| Sounds | 70 |
| Animations | 25 |
| Meshes | 19 |
| Particle Systems | 5 |

**Supported Platforms:** Windows (Win64)

---

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

**MIT License**

```
Copyright (c) 2018-2025 Marc Fraedrich under FinalSpark Gamestudios

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Credits

**Author:** Marc Fraedrich ([FinalSpark Gamestudios](https://finalspark-gamestudios.com))

**Special Thanks:**
- The Unreal Engine community
- All contributors and testers

---

*In loving memory of Alexander Gregory Morano (Joviex)*
*May 28, 1973 - September 26, 2025*
*Voice of the Trader / Tester and helper*
