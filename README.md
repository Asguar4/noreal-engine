# noreal-engine

A small C++17 / OpenGL 3.3 core engine in the spirit of Doom-era renderers: the level is compiled into a BSP tree and
walked front-to-back. Right now the game draws a top-down automap of a test level with the player as a dot.

## Features

- **Platform layer** — GLFW window with an OpenGL 3.3 core context, event pump and optional vsync (`riz::Window`).
- **Input** — GLFW-free per-frame keyboard/mouse state with press/release latches and mouse delta (`riz::Input`).
- **Game loop** — fixed timestep (60 Hz) with a frame clock, frame-time smoothing and render interpolation (`engine/loop.hpp`).
- **Renderer** — batched screen-space quads (`riz::Renderer`, `riz::ScreenQuad`).
- **Top-down camera** — orthographic view with position, rotation and zoom, world ↔ screen ↔ pixel transforms.
- **BSP** — builder that splits segments into nodes / subsectors / segs, point location and front-to-back traversal.
- **Game** — test level, player movement step and automap rendering on top of the engine and BSP libraries.

## Layout

```
src/
  engine/       riz_engine: platform, input, loop, movement, render, topDownCamera, io
  containers/   riz_bsp: BSP tree build and queries
  game/         riz_game: level, player, automap
  main.cpp      game executable
shaders/        GLSL shaders loaded at runtime
cmake/          header staging and layering / include-surface lint scripts
tests/          GoogleTest suites
```

Each module exposes one public header (`engine/render.hpp`, `containers/bsp.hpp`, ...). Internal headers are named
`i<Name>.hpp` and are not reachable from other modules: CMake stages only the public headers into the include path.

## Building

Requirements: Windows, MSVC x64, CMake 3.16+.

GLEW, GLFW and GLM are vendored and not stored in the repo. Put them into the project root:

```
include/   GL/, GLFW/, glm/
lib/       glew32s.lib, glfw3_mt.lib
```

or point CMake at another directory with `-DRIZ_DEPS_DIR=<dir containing include/ and lib/>`.

```
cmake -S . -B build -A x64
cmake --build build --config Release
```

GoogleTest is fetched automatically. Pass `-DRIZ_BUILD_TESTS=OFF` to skip the tests.

## Running

Run from the project root so the shaders are found:

```
build\Release\game.exe
```

| Input  | Action               |
|--------|----------------------|
| W / S  | move forward / back  |
| A / D  | strafe left / right  |
| Mouse  | turn                 |
| Esc    | quit                 |

`game.exe --smoke-frames N` runs N frames in a hidden window and exits (used for smoke testing).

## Tests

```
ctest --test-dir build -C Release
```

Tests that need a real GL context are labeled `gl`. Skip them on headless machines with `-LE gl`.
