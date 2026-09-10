# AppImage implementation plan

## Purpose

Prepare **Ymir & Helga** for portfolio-friendly Linux distribution as a single x86_64 AppImage. A user should be able to download one file, mark it executable, and launch the game without installing SFML, CMake, GoogleTest, or a compiler.

This plan is intended to be executed by a separate coding agent on the `appimage` branch.

## Important product constraint

An AppImage bundles runtime dependencies and avoids persistent package installation, but it is **not a security sandbox**. Do not describe it as sandboxed or isolated in the README. Its benefits here are portability and a low-friction portfolio demo. Flatpak would be the appropriate follow-up if security isolation becomes a requirement.

## Branch and safety instructions

1. Verify that the working branch is `appimage` before making implementation changes:
   ```sh
   git branch --show-current
   git status --short
   ```
2. If necessary, switch to the existing `appimage` branch. Do not recreate or reset it.
3. Preserve unrelated user changes. Do not rewrite history, commit, tag, or publish a release unless explicitly requested.
4. Keep the implementation focused on Linux packaging and the minimum C++/CMake changes needed to support it.

## Current repository findings

Use these findings as the implementation baseline, but re-check them before editing:

- The project uses C++17 and CMake 3.20+.
- The game executable target is currently named `main`.
- `CMAKE_RUNTIME_OUTPUT_DIRECTORY` points into the source tree at `${CMAKE_SOURCE_DIR}/bin`.
- The game links SFML by raw library names in `src/CMakeLists.txt`.
- `sfml-audio` is linked, but no SFML audio APIs were found in the current source.
- Tests always configure because the root `CMakeLists.txt` unconditionally calls `enable_testing()` and `add_subdirectory(tests)`.
- `tests/CMakeLists.txt` requires GoogleTest and Threads.
- Runtime resource access is coupled to the checkout by the global definition `ROOT="${CMAKE_SOURCE_DIR}"`.
- Resource path construction is centralized in only three implementations:
  - `src/Ente.cpp` for textures;
  - `src/Gerenciadores/GerenciadorGrafico.cpp` for the font;
  - `src/Fases/Fase.cpp` for map files.
- Existing callers pass paths beginning with `/assets/`, for example `/assets/Menu.png`.
- The repository has no `.github` workflow, AppImage files, desktop entry, or CMake install rules.
- Existing possible icon art is only 48×48 (`assets/Personagens/Ymir.png` and similar). It is acceptable as an initial 48×48 desktop icon; do not invent new artwork.
- Canonical repository: `https://github.com/ediasv/ymir-helga`.
- Use application ID `io.github.ediasv.YmirHelga` consistently for desktop/AppStream metadata.

## Desired result

The implementation is complete when all of the following are true:

- A normal out-of-tree CMake build works.
- Tests can be enabled and run when GoogleTest is installed.
- A release build with `-DBUILD_TESTING=OFF` does not search for or require GoogleTest.
- Runtime resources load in both a developer build and an installed/AppImage layout.
- `cmake --install` creates a valid AppDir-style filesystem hierarchy.
- A reproducible script builds `Ymir_Helga-x86_64.AppImage` on Linux x86_64.
- The AppImage contains the game executable, all assets, desktop metadata, icon, and required non-system shared libraries such as SFML.
- The AppImage starts on a Linux graphical session without requiring SFML to be installed on the host.
- CI can build and upload the AppImage artifact.
- The README gives a reviewer short, accurate download-and-run instructions.

---

## Phase 1: Modernize the build/install boundary

### 1.1 Update the root `CMakeLists.txt`

Make these focused changes:

1. Keep C++17 and CMake 3.20.
2. Replace the unconditional testing setup with standard CTest handling:
   ```cmake
   include(CTest)
   ```
   `include(CTest)` provides the `BUILD_TESTING` option and enables testing when appropriate.
3. Wrap the tests subdirectory:
   ```cmake
   if(BUILD_TESTING)
     add_subdirectory(tests)
   endif()
   ```
4. Include `GNUInstallDirs` for portable install paths.
5. Add an explicit SFML 2.x package lookup before `add_subdirectory(src)`. Require only components actually used after verifying the source. At minimum, graphics/window/system are used. Keep audio only if source inspection or a successful link demonstrates it is needed.
6. Remove the global `ROOT` compile definition. No production resource lookup may concatenate paths with `ROOT` after this work.
7. Change runtime output from the source tree to the build tree, preferably `${CMAKE_BINARY_DIR}/bin`.
8. Define CMake install rules for:
   - the executable through the target’s own CMake file or the root file;
   - `assets/` under `${CMAKE_INSTALL_DATADIR}/ymir-helga/assets`;
   - the desktop file under `${CMAKE_INSTALL_DATADIR}/applications`;
   - the icon under `${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps`, renamed to `io.github.ediasv.YmirHelga.png`;
   - AppStream metadata under `${CMAKE_INSTALL_DATADIR}/metainfo` if Phase 3 includes it.

Do not install tests.

### 1.2 Update `src/CMakeLists.txt`

1. Preserve the `jogo` library/source organization except where needed to add the resource-path implementation.
2. Add the new resource-path `.cpp` file described in Phase 2 to `jogo`.
3. Replace raw/ad hoc SFML dependency handling with the targets or variables produced by the root `find_package`. Follow the installed SFML 2.x CMake package’s supported target names and verify on Ubuntu 22.04.
4. Remove `sfml-audio` only after confirming no source uses it and the project still configures, links, and launches.
5. Rename the user-facing executable target from `main` to `ymir-helga`, or retain an internal target name and set `OUTPUT_NAME` to `ymir-helga`. Prefer a final installed executable named `ymir-helga` because the desktop file will reference it.
6. Add:
   ```cmake
   install(TARGETS ymir-helga
     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   )
   ```
   Adapt the target name if `OUTPUT_NAME` is used instead.

### 1.3 Keep tests optional

`tests/CMakeLists.txt` may keep `find_package(GTest REQUIRED)` and `find_package(Threads REQUIRED)` because the entire directory will only be entered when `BUILD_TESTING=ON`.

Verify both modes in separate clean build directories:

```sh
cmake -S . -B build/tests -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests --parallel
ctest --test-dir build/tests --output-on-failure

cmake -S . -B build/release -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
```

The second configure must succeed in an environment where GoogleTest is not installed.

---

## Phase 2: Replace checkout-bound asset lookup

### 2.1 Add a small resource-path utility

Create a focused utility, using names consistent with the project’s existing Portuguese naming style, for example:

- `include/CaminhoRecursos.h`
- `src/CaminhoRecursos.cpp`

A single function should accept a resource-relative path and return a `std::filesystem::path` (or string if necessary for SFML 2 APIs). It must normalize existing inputs so both `"/assets/Menu.png"` and `"Menu.png"` cannot accidentally discard the selected base directory.

Recommended lookup order:

1. `YMIR_HELGA_ASSET_DIR`, when explicitly set. Treat this as the complete path to the `assets` directory. This is useful for tests and troubleshooting.
2. If `APPDIR` exists (set by the AppImage runtime), use:
   ```text
   $APPDIR/usr/share/ymir-helga/assets
   ```
3. Use an install-path fallback configured by CMake, such as `${CMAKE_INSTALL_FULL_DATADIR}/ymir-helga/assets`, for a conventional non-AppImage `cmake --install` installation.
4. Use a development-tree fallback configured by CMake, pointing to the repository’s `assets` directory, so the executable can run directly from `build/.../bin` during development.

Implementation requirements:

- Centralize all environment/macro handling in this utility.
- Use `std::filesystem` because the project requires C++17.
- Strip the legacy leading `/` and optional leading `assets/` from callers before joining to the selected asset root.
- Avoid depending on the current working directory.
- Do not use `/proc/self/exe` unless the environment/install strategy above proves insufficient; `APPDIR` is the standard AppImage mechanism and is simpler.
- Return or log the resolved full path in actionable resource-load error messages.
- Do not print every successful resource path during normal gameplay.

CMake may define quoted fallback paths **for this target only**, not globally. Ensure spaces in source/install paths are handled correctly.

### 2.2 Migrate the three resource-loading implementations

Update only the centralized implementations unless inspection reveals another direct file load:

1. `src/Ente.cpp`
   - Replace `ROOT + path` in `Ente::setTextura` with the resource utility.
   - On failure, include the resolved path in the error.
2. `src/Gerenciadores/GerenciadorGrafico.cpp`
   - Replace `ROOT + path` in `carregarFonte`.
   - Remove the unconditional debug print of the path.
   - Include the resolved path in the failure message.
3. `src/Fases/Fase.cpp`
   - Replace `ROOT + path` in `criarMapa`.
   - Include the resolved path in the map-open failure message.

Existing entity/menu/fase call sites may remain unchanged if the utility correctly normalizes their `/assets/...` strings. Changing all call sites to asset-relative strings is optional cleanup; avoid a broad mechanical edit unless it makes the API clearer and is covered by validation.

### 2.3 Add focused tests where practical

Add resource-path tests only if they can remain deterministic and do not require opening the SFML window. Good cases include:

- stripping a leading slash;
- stripping an `assets/` prefix;
- honoring `YMIR_HELGA_ASSET_DIR`;
- joining paths without allowing an absolute child path to replace the base.

If environment mutation would make tests brittle, factor path normalization/joining into a pure helper and test that helper. Do not significantly redesign the game for this.

---

## Phase 3: Add Linux desktop metadata

Create `packaging/appimage/` and add the following text files.

### 3.1 Desktop entry

Create `packaging/appimage/io.github.ediasv.YmirHelga.desktop` with at least:

```ini
[Desktop Entry]
Type=Application
Name=Ymir & Helga
Comment=Two-player 2D platform game
Exec=ymir-helga
Icon=io.github.ediasv.YmirHelga
Terminal=false
Categories=Game;ArcadeGame;
```

Validate it with `desktop-file-validate` when that tool is available.

### 3.2 Icon

Initially install `assets/Personagens/Ymir.png` as the 48×48 application icon, renamed to `io.github.ediasv.YmirHelga.png` by CMake. Do not modify the original asset.

Optionally create a larger nearest-neighbor scaled packaging icon if image tooling is already available in the packaging environment, but do not require an additional runtime dependency and do not present an upscaled sprite as newly designed artwork. A custom high-resolution icon is future portfolio polish, not a blocker.

### 3.3 AppStream metadata

Create `packaging/appimage/io.github.ediasv.YmirHelga.metainfo.xml` and install it under `share/metainfo`. Include:

- component type `desktop-application`;
- ID `io.github.ediasv.YmirHelga`;
- name and concise summary;
- GPL-3.0-only project license;
- an appropriate metadata license such as CC0-1.0;
- developer name `Eduardo Dias`;
- launchable desktop ID;
- project homepage/repository URL;
- one initial release entry.

Do not include screenshots or URLs to media that do not exist. Validate with `appstreamcli validate` where available.

---

## Phase 4: Build the AppImage reproducibly

### 4.1 Add `packaging/appimage/build-appimage.sh`

Create a POSIX-compatible or Bash script with strict error handling. It should:

1. Refuse unsupported hosts with a clear message; initially support Linux x86_64 only.
2. Resolve the repository root based on the script’s own location rather than the caller’s working directory.
3. Use disposable directories under `build/appimage/`:
   - CMake build directory;
   - `AppDir` staging directory;
   - final artifact/output directory.
4. Configure release mode with:
   ```sh
   -DCMAKE_BUILD_TYPE=Release
   -DBUILD_TESTING=OFF
   -DCMAKE_INSTALL_PREFIX=/usr
   ```
5. Build the game.
6. Stage installation with `DESTDIR=<...>/AppDir cmake --install ...`.
7. Verify before packaging that these paths exist:
   - `AppDir/usr/bin/ymir-helga`;
   - `AppDir/usr/share/ymir-helga/assets/...`;
   - desktop file;
   - application icon;
   - AppStream metadata if included.
8. Invoke `linuxdeploy` to collect non-system shared libraries and produce an AppImage. The script should accept the tool path through a documented `LINUXDEPLOY` environment variable or argument. Do not commit the `linuxdeploy` binary to the repository.
9. Set `ARCH=x86_64` and a deterministic output name such as `Ymir_Helga-x86_64.AppImage`.
10. Print the final artifact path and its SHA-256 checksum.

Do not use `sudo`, install host packages, or write outside the repository from this script.

### 4.2 Tool acquisition policy

For CI, download `linuxdeploy` from its official GitHub release location. Prefer an immutable tagged release. If only the continuous release is practical:

- document that choice;
- pin and verify a known SHA-256 checksum;
- fail if checksum verification fails;
- set `APPIMAGE_EXTRACT_AND_RUN=1` when executing packaging tools in environments without FUSE.

Do not silently download and execute an unverified binary in the local build script. A separate CI setup step can acquire it and pass its path through `LINUXDEPLOY`.

### 4.3 Shared-library checks

After producing the AppImage:

1. Extract it in a temporary build directory using:
   ```sh
   ./Ymir_Helga-x86_64.AppImage --appimage-extract
   ```
   If FUSE is unavailable, use `--appimage-extract-and-run` for execution.
2. Inspect `squashfs-root/usr/lib` and the executable’s dynamic dependencies.
3. Confirm SFML libraries are bundled.
4. Do not bundle host GPU drivers, glibc, or other libraries intentionally excluded by standard AppImage tooling.
5. Build on an older supported Linux baseline (Ubuntu 22.04 is a reasonable initial target) to avoid requiring a newer glibc than common user systems provide.

---

## Phase 5: Add CI artifact generation

Create `.github/workflows/appimage.yml`.

Recommended behavior:

1. Run on:
   - pushes to `appimage` while this branch is under development;
   - pull requests affecting C++, CMake, assets, or packaging files;
   - manual `workflow_dispatch`;
   - version tags such as `v*` if release upload is implemented.
2. Use a stable Ubuntu 22.04 environment or container as the compatibility baseline.
3. Install build-only CI packages, including:
   - compiler/build essentials;
   - CMake and Ninja;
   - SFML development package;
   - GoogleTest development package for the test job;
   - desktop/AppStream validators where available;
   - `curl`, `file`, and checksum tools;
   - Xvfb for an optional launch smoke test.
4. First configure/build with tests enabled and run `ctest --output-on-failure`.
5. Run desktop and AppStream metadata validation.
6. Acquire and verify `linuxdeploy` as described in Phase 4.
7. Run `packaging/appimage/build-appimage.sh`.
8. Upload `Ymir_Helga-x86_64.AppImage` and its `.sha256` file using the official `actions/upload-artifact` action pinned to a major version or immutable commit.
9. If tag-based GitHub Release publishing is included, use the repository `GITHUB_TOKEN` with minimal `contents: write` permission and the GitHub CLI. Do not publish releases on ordinary pushes. It is acceptable to leave actual release publishing as a documented manual step if no version/release policy exists yet.

Avoid hiding the build entirely inside opaque third-party actions. Keep the same packaging script usable locally and in CI.

---

## Phase 6: Documentation for portfolio users

Expand `README.md` without inventing project history or gameplay claims.

### Required README content

1. Keep the concise game description and technologies.
2. Add a prominent **Download and run on Linux** section:
   ```sh
   chmod +x Ymir_Helga-x86_64.AppImage
   ./Ymir_Helga-x86_64.AppImage
   ```
3. State that the initial release supports Linux x86_64.
4. Mention the FUSE-independent fallback:
   ```sh
   ./Ymir_Helga-x86_64.AppImage --appimage-extract-and-run
   ```
5. Link users to the GitHub Releases page once a release exists. Until then, accurately describe the CI artifact or local build path instead of linking to a nonexistent release.
6. State clearly that users do not need to install SFML or build tools to run the AppImage.
7. Do **not** claim that AppImage provides sandboxing.
8. Add source build/test instructions using the new out-of-tree paths and `BUILD_TESTING` behavior.
9. Add local AppImage build instructions, including required build dependencies and how to provide `LINUXDEPLOY`.
10. Mention the license (`GPL-3.0`) and link `LICENSE`.

### Optional portfolio polish

- Add controls by inspecting the actual keyboard bindings; do not guess.
- Add a screenshot or short gameplay GIF only if a real file is supplied or captured during a graphical test. Do not add placeholders or fabricate media.
- Add badges only after the associated workflow/release URLs exist.

---

## Phase 7: Validation matrix

Run as much of this matrix as the environment permits and report exact limitations.

### A. Development build with tests

On a Linux environment with SFML and GoogleTest development packages:

```sh
cmake -S . -B build/tests -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests --parallel
ctest --test-dir build/tests --output-on-failure
```

Expected: configure/build/tests pass.

### B. Release build without GoogleTest

Use a clean Linux environment that has SFML development files but not GoogleTest:

```sh
cmake -S . -B build/release -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
```

Expected: CMake never requires GTest and the game links.

### C. Staged install

```sh
cmake -S . -B build/install \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build/install --parallel
DESTDIR="$PWD/build/stage" cmake --install build/install
```

Expected staged content:

```text
build/stage/usr/bin/ymir-helga
build/stage/usr/share/ymir-helga/assets/
build/stage/usr/share/applications/io.github.ediasv.YmirHelga.desktop
build/stage/usr/share/icons/hicolor/48x48/apps/io.github.ediasv.YmirHelga.png
build/stage/usr/share/metainfo/io.github.ediasv.YmirHelga.metainfo.xml
```

### D. Resource lookup

Verify all three resource types from a directory unrelated to the checkout:

- font;
- at least one texture;
- a map file.

For staged/AppImage testing, set `APPDIR` to the staging/AppDir root and launch from another working directory. There must be no dependency on `${CMAKE_SOURCE_DIR}` or the current directory.

### E. Headless launch smoke test

If Xvfb is available, launch the installed/AppDir executable under it with a short timeout. Because the game is interactive and normally keeps running, a timeout can indicate a successful smoke launch. Capture output and fail if it contains resource-loading errors, an immediate crash, or a missing-library error. Do not treat every timeout as success without inspecting the process result/log.

### F. Real desktop test

On at least one actual Linux desktop:

- launch by executing the AppImage;
- verify the fullscreen window appears;
- verify keyboard input works;
- reach a menu/level that loads the font, textures, and map;
- close the game normally;
- verify no SFML host packages are required by testing on a clean VM/container-backed desktop if available.

A headless CI check cannot replace this final interactive test. If unavailable, state that manual graphical validation remains pending.

### G. Portability checks

At minimum test the AppImage on the Ubuntu 22.04 build baseline and one newer mainstream distribution. Record architecture and distribution results in release notes or the README only after verification.

---

## Expected file changes

Likely modified files:

- `CMakeLists.txt`
- `src/CMakeLists.txt`
- `src/Ente.cpp`
- `src/Gerenciadores/GerenciadorGrafico.cpp`
- `src/Fases/Fase.cpp`
- `tests/CMakeLists.txt` only if test target cleanup is needed
- `README.md`
- `.gitignore`

Likely new files:

- `include/CaminhoRecursos.h`
- `src/CaminhoRecursos.cpp`
- optional focused resource tests under `tests/`
- `packaging/appimage/io.github.ediasv.YmirHelga.desktop`
- `packaging/appimage/io.github.ediasv.YmirHelga.metainfo.xml`
- `packaging/appimage/build-appimage.sh`
- `.github/workflows/appimage.yml`

Do not commit generated content:

- `build/`
- `AppDir/`
- `squashfs-root/`
- downloaded `linuxdeploy` binaries
- generated `*.AppImage`
- checksum files generated during local packaging

Update `.gitignore` for these outputs if their chosen locations are not already covered.

## Scope controls and non-goals

- Do not port to SFML 3 as part of this work.
- Do not rewrite gameplay, ownership patterns, singleton behavior, or unrelated warnings.
- Do not fix unrelated duplicate source entries or latent bugs unless they block configuration/linking/packaging; if one blocks the work, make the smallest justified fix and document it.
- Do not add Flatpak, Docker GUI forwarding, Windows installers, macOS bundles, ARM64 AppImages, auto-updating, code signing, or a game launcher.
- Do not claim universal Linux compatibility. State the tested architecture/distributions.
- Do not add proprietary artwork or unverified third-party assets.

## Final handoff requirements

When implementation is complete, report:

1. exact files changed;
2. the resource lookup strategy and why it works inside AppImage;
3. build, test, install, metadata-validation, and packaging commands actually run;
4. the generated artifact name and SHA-256 if built;
5. which Linux environments were actually tested;
6. whether a real interactive graphical launch was completed;
7. any remaining manual action, especially creating a GitHub Release or adding gameplay media.
