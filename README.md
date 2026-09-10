# Ymir & Helga

<<<<<<< Updated upstream
Ymir & Helga is a two-player 2D platform game built with C++17, SFML 2,
CMake, and GoogleTest. The project uses object-oriented design and includes
enemy behavior, level progression, and local multiplayer controls.

## Download and run on Linux

The AppImage supports Linux x86_64. Download the
`Ymir-Helga-Linux-x86_64` artifact from a successful
[Linux AppImage workflow run](https://github.com/ediasv/ymir-helga/actions/workflows/appimage.yml),
extract it, and run:
=======
<<<<<<< HEAD
• A 2D platformer game, developed in pairs, implementing design patterns and mechanics such as enemy AI and level progression. Furthermore, the development process used unit tests to ensure code reliability and maintainability. This experience highlights my skills in collaborative development, object-oriented programming, and project organization.

• Tools used: C++, SFML, CMake, Catch2
=======
Ymir & Helga is a two-player 2D platform game where two heroes fight their
way through enemy-filled levels together — on the same keyboard.

Grab a friend, pick your hero, and jump into the adventure!

![Menu](assets/caverna_gh_display.jpeg)
*Caverna level*

![Plains](assets/planicie_gh_display.jpeg)
*Planície level*

## About the game

- **Local co-op** — two players share one keyboard and play side by side.
- **Combat** — each hero has their own attack; time it right to defeat enemies.
- **Multiple levels** — battle through different environments, each with its
  own look and challenges.
- **Enemy AI** — enemies react to the players as you progress.

The game was designed and built from scratch with object-oriented
architecture, covering everything from gameplay and physics to menus and
level progression.

## Controls

- Menus: `W`/`S` to move and `Enter` to select.
- Player 1: `A`/`D` to move, `W` to jump, and `Space` to attack.
- Player 2: arrow keys to move and jump, right `Shift` to attack.
- `Escape` pauses the game during a level.

## Play it

The game ships as a self-contained Linux AppImage — no installation or
dependencies needed. Download the latest
`Ymir-Helga-Linux-x86_64` artifact from the
[releases on GitHub Actions](https://github.com/ediasv/ymir-helga/actions/workflows/appimage.yml),
make it executable, and run it:
>>>>>>> Stashed changes

```sh
chmod +x Ymir_Helga-x86_64.AppImage
./Ymir_Helga-x86_64.AppImage
```

<<<<<<< Updated upstream
If FUSE is unavailable, use the extraction-based fallback:

```sh
./Ymir_Helga-x86_64.AppImage --appimage-extract-and-run
```

The AppImage bundles the required SFML libraries. Running it does not require
SFML, CMake, GoogleTest, a compiler, or other build tools to be installed.
AppImage provides portable distribution, not application sandboxing.

## Controls

- Menus: `W`/`S` to move and `Enter` to select.
- Player 1: `A`/`D` to move, `W` to jump, and `Space` to attack.
- Player 2: left/right arrows to move, up arrow to jump, and right `Shift` to
  attack.
- `Escape` pauses the game during a level.

## Build from source

Install a C++17 compiler, CMake 3.20 or newer, and the SFML 2 development
packages. GoogleTest is required only when `BUILD_TESTING=ON`.

Build and run the tests:

```sh
cmake -S . -B build/tests -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests --parallel
ctest --test-dir build/tests --output-on-failure
```

Build a release without configuring GoogleTest:

```sh
cmake -S . -B build/release -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
```

The development executable is written to `build/release/bin/ymir-helga` and
loads assets without depending on the current working directory.

## Build the AppImage locally

AppImage packaging is supported on Linux x86_64. It requires the source-build
dependencies above plus `file`, `ldd`, `readelf`, `sha256sum`, the official
[`appimagetool` 1.9.1](https://github.com/AppImage/appimagetool/releases/tag/1.9.1),
and the official
[Type-2 runtime 20251108](https://github.com/AppImage/type2-runtime/releases/tag/20251108).
The packaging script does not download tools or install host packages.

Download and verify the two pinned official files:

```sh
mkdir -p build/appimage-tools
curl -fL -o build/appimage-tools/appimagetool-x86_64.AppImage \
  https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage
curl -fL -o build/appimage-tools/runtime-x86_64 \
  https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64
printf '%s  %s\n' \
  ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0 \
  build/appimage-tools/appimagetool-x86_64.AppImage \
  2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d \
  build/appimage-tools/runtime-x86_64 | sha256sum --check --strict
chmod +x build/appimage-tools/appimagetool-x86_64.AppImage
```

Then build the AppImage:

```sh
APPIMAGETOOL="$PWD/build/appimage-tools/appimagetool-x86_64.AppImage" \
APPIMAGE_RUNTIME="$PWD/build/appimage-tools/runtime-x86_64" \
  packaging/appimage/build-appimage.sh
```

The script stages a CMake install, recursively bundles non-system runtime
libraries (including SFML) according to the reviewable policy in
`packaging/appimage/excluded-libraries.txt`, verifies the resulting image, and
writes:

```text
build/appimage/output/Ymir_Helga-x86_64.AppImage
build/appimage/output/Ymir_Helga-x86_64.AppImage.sha256
```
=======
## Built with

- **C++17** — core game logic and architecture
- **SFML 2** — graphics, audio, and input
- **CMake** — build system
- **GoogleTest** — automated testing
>>>>>>> Stashed changes

## License

Ymir & Helga is licensed under the [GNU General Public License v3.0](LICENSE).
<<<<<<< Updated upstream
=======
>>>>>>> 0e1ee14 (update readme)
>>>>>>> Stashed changes
