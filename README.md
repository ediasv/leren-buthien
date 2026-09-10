# Ymir & Helga

Ymir & Helga is a 2D local multiplayer platform game where two players share
one keyboard and one screen. Fight your way through enemies side by side,
explore each level together, and see how far teamwork takes you.

Built from scratch in **C++** using **SFML**, with an object-oriented design
that features enemy AI, level progression, and independent controls for both
players.

## Screenshots

**The Cavern**

![Cavern level](assets/caverna_gh_display.jpeg)

**The Plains**

![Plains level](assets/planicie_gh_display.jpeg)

## How to play

Download the game for Linux from a successful
[Linux AppImage build](https://github.com/ediasv/ymir-helga/actions/workflows/appimage.yml),
make it executable, and run it — no installation required:

```sh
chmod +x Ymir_Helga-x86_64.AppImage
./Ymir_Helga-x86_64.AppImage
```

### Controls

| Action | Player 1 | Player 2 |
| --- | --- | --- |
| Move | `A` / `D` | Left / Right arrows |
| Jump | `W` | Up arrow |
| Attack | `Space` | Right `Shift` |

Menus use `W`/`S` to navigate and `Enter` to select. Press `Escape` to pause
during a level.

## About the project

This project was developed as a university assignment and doubles as a
demonstration of practical C++ skills: object-oriented architecture, game loop
design, collision handling, and enemy behavior. It is continuously built and
packaged with GitHub Actions, producing a ready-to-run AppImage.

## License

Ymir & Helga is licensed under the
[GNU General Public License v3.0](LICENSE).
