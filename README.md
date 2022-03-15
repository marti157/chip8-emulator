# Chip8 Emulator/Interpreter

## Reference
All system specs are derived from the following document.
[http://devernay.free.fr/hacks/chip8/C8TECH10.HTM](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)

## Requirements
Windows is not supported.

**SDL2** graphic library is required to run the emulator, and MacOS users should use Xcode.

### Linux
SDL2: [https://wiki.libsdl.org/Installation](https://wiki.libsdl.org/Installation)

### MacOS
SDL2: [https://wiki.libsdl.org/Installation](https://wiki.libsdl.org/Installation)
To install with Xcode: [https://lazyfoo.net/tutorials/SDL/01_hello_SDL/mac/index.php](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/mac/index.php)

## Example Programs
You can find example Chip8 programs all around the web, make sure they're compiled before using them with the emulator/interpreter.

[https://johnearnest.github.io/chip8Archive/](https://johnearnest.github.io/chip8Archive/)

[https://github.com/kripod/chip8-roms](https://github.com/kripod/chip8-roms)

## TODO
* Use different timing system, because SDL GetTicks is only accurate to milliseconds, thus introducing some timing issues with DT and cycle frequency.
* Add debug screen to view memory/register values.
