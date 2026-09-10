# Ymir & Helga

Ymir & Helga is a two-player 2D platform game built with C++17, SFML 2,
CMake, and GoogleTest. The project uses object-oriented design and includes
enemy behavior, level progression, and local multiplayer controls.

## Download and run on Linux

The AppImage supports Linux x86_64. Download the
`Ymir-Helga-Linux-x86_64` artifact from a successful
[Linux AppImage workflow run](https://github.com/ediasv/ymir-helga/actions/workflows/appimage.yml),
extract it, and run:

```sh
chmod +x Ymir_Helga-x86_64.AppImage
./Ymir_Helga-x86_64.AppImage
```

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

## License

Ymir & Helga is licensed under the [GNU General Public License v3.0](LICENSE).
