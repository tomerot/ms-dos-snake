<p align="center">
  <img src="https://github.com/user-attachments/assets/9bc8e87b-0583-42f1-b2e4-67c97024779b" alt="Snake" width="450">
</p>

A variant of the classic Snake game for MS-DOS, written in C89 and built with
Borland Turbo C++ 3.0. It drives the PC's hardware directly, with no graphics,
sound or input library.

## About the Project

This is an academic project for a real-time programming course. A real-time
system must act on schedule: here, the snake must step at fixed intervals and
react to a key before its next step. Snake is soft real-time: a late response
to an action can make the game feel slightly laggy, but it won't cause a
failure. In most cases, the player won't even notice the delay.

Real-time systems typically rely on interrupts: the CPU is alerted the moment
an event happens, even in the middle of other work, instead of checking the
hardware over and over (polling). On a desktop operating system such as
Windows or Linux, only the kernel and its drivers can install interrupt
handlers; under MS-DOS, any program can, much like bare-metal firmware.

The program is built as a superloop, a common design for small systems:
interrupt handlers in the foreground only record what happened, such as a
clock tick or a key press, and the main loop in the background checks those
records and acts on them.

The same direct access also lets the program draw the screen by writing
straight into video memory (memory-mapped I/O), and play sound by programming
the timer (whose output drives the speaker) at the register level, with
bitwise operations.

## Contents

- [The Game](#the-game)
- [Running the Game](#running-the-game)
- [Background](#background)
- [Architecture](#architecture)
- [Building from Source](#building-from-source)

## The Game

<p align="center">
  <img src="https://github.com/user-attachments/assets/470211eb-beb5-4c1a-bc84-03a90c483b73" alt="Snake gameplay" width="700">
  <br>
  <sub>Gameplay (sped up)</sub>
</p>

The snake moves on its own, and the arrow keys steer it. Each level places
food on the field; reaching it before the time runs out starts the next
level. If the snake hits a wall or the border, or the time runs out before
it reaches the food, the snake dies and the game is over. Unlike classic
Snake:

- **The snake never grows** - It is always 9 pieces long.
- **It turns only when straight** - A 90° turn works only while all its
  pieces lie in one line, so the snake can never run into itself.
- **A wall guards the food** - A V-shaped wall stands beside the food, its
  tip toward the snake, so the snake has to go around it.
- **Each level has a time limit** - It counts down on the top line of the
  screen.

Later levels speed the snake up (from 3 to 10 steps a second), shorten the
time limit (from 60 to 20 seconds) and shrink the food (from 3×3 to 1×1).

### Controls

| Key | Action |
|---|---|
| Arrow keys | Steer the snake |
| Enter | Restart, on the game-over screen |
| Esc | Exit, on the game-over screen |

## Running the Game

64-bit Windows, macOS and Linux can't run DOS programs directly, so Snake
runs in [DOSBox-X](https://dosbox-x.com/), an emulator of a DOS PC. It was
tested in DOSBox-X 2025.05.03.

1. Install DOSBox-X from its
   [releases page](https://github.com/joncampbell123/dosbox-x/releases).
2. Download `SNAKE.EXE` from this repository's
   [latest release](../../releases/latest) into a folder, for example
   `C:\Games\Snake`.
3. Start DOSBox-X and type:

   ```
   MOUNT C C:\Games\Snake
   C:
   SNAKE
   ```

   On macOS or Linux, use the folder's path, for example
   `MOUNT C /home/you/games/snake`.

The game takes over the keyboard while it runs, so leave it from the
game-over screen, with Esc.

## Background

The game is written for the IBM PC AT (1984) and the PCs compatible with it,
which kept the same ports, interrupts and BIOS services through the Pentium
era of DOS. It needs an AT because the real-time clock and the second
interrupt controller it relies on first appeared there. The timer chip every
PC has could tick too, but DOS keeps its time of day on it, so the program
uses the real-time clock instead. It was tested in DOSBox-X, not on original
hardware.

This section explains that machine, and the Turbo C++ functions and keywords
used throughout the project.

### Real Mode

DOS runs the CPU in real mode, the only mode of Intel's 8086, the first x86
CPU, which every later x86 CPU kept for compatibility. In real mode, the CPU
can address 1 MB, with no protection: a program lives in only part of that
space, but nothing stops it from reading or writing the rest, or from talking
to any hardware directly.

### Text Mode

The game uses the color text mode of IBM's Color Graphics Adapter (CGA),
which the later EGA and VGA cards reproduce, with the same mode number, memory
address and cell layout, so it runs on all three. The screen is a grid of
80 × 25 character cells, each stored as two bytes in the video card's memory:
a character code, and a color byte with 4 bits of foreground color, 3 of
background and a blink bit.

The game draws shapes, not text, so each cell only needs to be one solid
color. Coloring a cell's background would allow just 8 colors, since the
background has only 3 bits. To solve this, a trick was devised: each cell is
filled with the full block character `█` (code 219), which covers the whole
cell in the foreground color, and so reaches all 16.

### PC Architecture

The CPU's 1 MB of addresses is not all RAM: several pieces of hardware share
it, each answering its own range. The RAM on the motherboard answers the
addresses below 640 KB, and the video card and the ROMs answer addresses
above it. Addresses are written as segment:offset, two 16-bit numbers that the
CPU combines as segment × 16 + offset, so `B800:0000` is address `0xB8000`;
the table gives the combined form.

<table>
<tr><th>Component</th><th>Element</th><th>Description</th></tr>
<tr>
<td>CPU</td>
<td>Registers</td>
<td>Small storage slots inside the CPU, holding the values it is working on. In 16-bit code each holds 16 bits, so a 32-bit value takes two steps to read. One bit, the interrupt flag, switches hardware interrupts off and on. In this code, <code>disable</code> and <code>enable</code> flip it, for example just for the moment it takes to read the 32-bit tick count, so a tick can't land between its two halves. A tick that arrives meanwhile isn't lost: the PIC holds it (see IRQ lines and PICs below), and the CPU takes it as soon as interrupts are switched back on.</td>
</tr>
<tr>
<td rowspan="2">Motherboard</td>
<td>I/O ports</td>
<td>A second set of addresses, 65,536 of them, separate from memory, through which the CPU talks to the chips. Each chip's registers are reached through its own ports, and decoding logic on the motherboard routes each port to its chip. In this code, <code>inportb</code> and <code>outportb</code> read and write them.</td>
</tr>
<tr>
<td>IRQ lines and PICs</td>
<td>Each device has its own wire, an IRQ (interrupt request line), to ask for the CPU's attention. Two programmable interrupt controllers (PIC, Intel 8259A) pass these requests to the CPU one at a time, most urgent first, each as an interrupt number. They hold back that IRQ, and every less urgent one, until the handler reports it is done with an end of interrupt (EOI).</td>
</tr>
<tr>
<td rowspan="2">ROM</td>
<td>BIOS</td>
<td>The PC's built-in code on the motherboard, at <code>0xF0000</code>. It starts the machine and offers services that programs call with a software interrupt: they trigger an interrupt themselves, with the inputs in registers. In this code, <code>int86</code> does this.</td>
</tr>
<tr>
<td>Video BIOS</td>
<td>The video services that programs call through interrupt 0x10, such as setting the text mode. For a CGA, this code is part of the motherboard's BIOS. EGA and VGA came later, with new video modes the motherboard's BIOS doesn't know, so each card carries its own code, in a ROM at <code>0xC0000</code>, and takes over interrupt 0x10 at startup, the old CGA modes included.</td>
</tr>
<tr>
<td>RAM</td>
<td>IVT (interrupt vector table)</td>
<td>At address <code>0x00000</code>: 256 slots, one per interrupt number, each holding the address of that interrupt's handler. On every interrupt, the CPU reads the slot and jumps to that address: into ROM for the BIOS's handlers, or into RAM for those of DOS or of a program that installed its own, as this one does.</td>
</tr>
<tr>
<td>Video card</td>
<td>Video memory</td>
<td>The card's own memory, holding what the screen shows. It sits at memory addresses from <code>0xB8000</code> (memory-mapped I/O, MMIO), so an ordinary memory write there changes the screen at once.</td>
</tr>
</table>

### Hardware Used by the Game

| Hardware | Reached through | Used for | File |
|---|---|---|---|
| Video card (CGA text mode) | The video BIOS (interrupt 0x10), and video memory at `0xB8000` | Setting text mode, hiding the cursor, drawing | [cga.c](hardware/cga.c) |
| Intel 8042 keyboard controller | Port 0x60, IRQ 1 (interrupt 0x09) | Key presses and releases | [keyboard.c](hardware/keyboard.c) |
| Motorola MC146818 real-time clock (RTC) | Ports 0x70 and 0x71, IRQ 8 (interrupt 0x70) | A tick 1024 times a second | [rtc.c](hardware/rtc.c) |
| Intel 8254 programmable interval timer (PIT) | Ports 0x42 and 0x43 | A square wave on channel 2, which sets the tone | [pit.c](hardware/pit.c) |
| PC speaker | Port 0x61 | Turning the tone on and off | [speaker.c](hardware/speaker.c) |
| Two Intel 8259A programmable interrupt controllers (PIC) | Ports 0x20 and 0xA0 | End-of-interrupt commands | [pic.c](hardware/pic.c) |

Port 0x61 is named in [ppi.h](hardware/ppi.h) after the Intel 8255
programmable peripheral interface (PPI), which owned it on the IBM PC and XT;
the AT dropped the chip but kept the port. Each hardware file starts with a
description of its chip: what it can do in a PC, and which parts of it the
file uses.

### Turbo C++ Functions and Keywords

| Name | What it does | In the code |
|---|---|---|
| `inportb(port)`, `outportb(port, value)` | Read or write a byte at an I/O port. | `code = inportb(KEYBOARD_DATA_PORT);` |
| `int86(n, &in, &out)` | Calls interrupt `n` with the CPU's registers set from `in`, and returns them in `out`. Used to call the BIOS. | `int86(VIDEO_BIOS_VECTOR, &regs, &regs);` |
| `getvect(n)`, `setvect(n, handler)` | Read or replace entry `n` of the interrupt vector table. | `setvect(RTC_VECTOR, rtc_handler);` |
| `disable()`, `enable()` | Block or allow hardware interrupts. | `disable(); tick = tick_count; enable();` |
| `interrupt` | Makes a function an interrupt handler: it saves every register, and returns so that the interrupted code continues exactly where it stopped. | `static void interrupt rtc_handler(void)` |
| `far` | A 32-bit segment:offset pointer, which can reach any address; here, a pointer to the bytes of video memory. A normal pointer holds only a 16-bit offset into the program's own data, so it can't reach video memory. | `static byte far *const vram = (byte far *)CGA_MEMORY_ADDRESS;` |
| `volatile` | Standard C: the variable is read from memory every time, since an interrupt handler can change it at any moment. | `static volatile unsigned long tick_count;` |

## Architecture

The code is split into three layers, from the hardware up to the game:

```
├── main.c             starts the devices, runs the main loop, stops the devices
├── common.h           shared types: bool, byte, pos
├── MAKEFILE
├── hardware/          one driver per chip, named after the chip
│   ├── cga.c/.h       video card: text mode and video memory
│   ├── keyboard.c/.h  keyboard controller
│   ├── rtc.c/.h       real-time clock
│   ├── pit.c/.h       programmable interval timer
│   ├── speaker.c/.h   PC speaker
│   ├── pic.c/.h       interrupt controllers
│   └── ppi.h          port 0x61's bits
├── platform/          the reusable platform, one module per job
│   ├── video.c/.h     pieces, text, big letters
│   ├── sound.c/.h     melodies on the speaker
│   ├── clock.c/.h     waits and interval timers
│   └── input.c/.h     key presses and held keys
└── game/              Snake
    ├── gameplay.c/.h  rules and state machine
    ├── layout.c/.h    positions and shapes, and drawing them
    ├── controls.c/.h  keys to commands
    ├── melodies.c/.h  the game's melodies
    └── style.h        the game's colors
```

Three rules keep the layers apart:

- **Each layer talks only to the one below it** - The game asks the platform
  for things, such as "draw a piece here", and only the platform talks to the
  hardware files. The game never touches a port or a register itself. Even
  the color and key names it uses (`BLUE`, `KEY_UP`), which are defined in
  the hardware files, reach it through the platform's headers.
- **The platform knows nothing about Snake** - No file in `platform/` or
  `hardware/` mentions a snake, food or walls. The test for each file is
  whether a different DOS program, such as a Tetris game or a text editor,
  could use it unchanged. That is what makes the platform reusable (see
  below).
- **The game decides what, the platform knows how** - Snake's colors,
  melodies and choice of keys live in `game/`, and the game hands them to the
  platform at the moment it uses them. For example, in
  `set_piece(p, SNAKE_COLOR)`, the platform's `set_piece` knows how to draw a
  block at position `p`, and the game supplies the color, `SNAKE_COLOR`,
  which it defines as blue.

### Reusing the Platform

To build another program on top of the platform, replace `game/` and the game
calls in [main.c](main.c); `platform/` and `hardware/` stay as they are. Each
module's functions are in its header: [video.h](platform/video.h),
[sound.h](platform/sound.h), [clock.h](platform/clock.h) and
[input.h](platform/input.h). As `main.c` shows, video, clock and input must
be started first, sound needs the clock, and all three must be stopped, in
reverse order, before the program exits; otherwise the clock's and keyboard's
next interrupts would jump into freed memory. Replacing `game/` changes the
files to build, so the MAKEFILE must be updated too; see
[Changing the MAKEFILE](#changing-the-makefile).

## Building from Source

Requirements:

- Borland Turbo C++ 3.0. Its command-line compiler, TCC, compiles `.c` files
  as C.
- MS-DOS, or DOSBox-X with a drive that holds both Turbo C++ and the
  project.

Keep the project folder's name to 8 characters or fewer, with no spaces, as
DOS requires. The build was tested with Turbo C++ 3.0 in DOSBox-X 2025.05.03.

### Building

The MAKEFILE reads the Turbo C++ folder from the `TC` variable. From the
project folder, either set it once:

```
SET TC=C:\TC
MAKE
```

or pass it to MAKE:

```
MAKE -DTC=C:\TC
```

Replace `C:\TC` with the folder Turbo C++ is installed in, without a trailing
backslash. `SET` lasts until DOS or DOSBox-X closes; to keep it, add the line
to `AUTOEXEC.BAT`, or to the `[autoexec]` section of DOSBox-X's configuration
file. If Turbo C++'s `BIN` folder is not on the `PATH`, run MAKE by its full
path, such as `C:\TC\BIN\MAKE.EXE`.

The build produces `SNAKE.EXE`. `MAKE clean` deletes the files the build
created; `TC` must be set for it too.

### Changing the MAKEFILE

The MAKEFILE needs a change only when the code does:

- **A new or removed `.c` file** - Add or remove its `.obj` in `OBJS` and in
  the `echo` lines that write `link.rsp`, and add or delete its rule. A
  renamed file counts as one removed and one new.
- **A new or removed `#include` of a project header** - Add or remove the
  header in the rule of every `.obj` whose `.c` includes it, directly or
  through another header. A header missing from a rule means MAKE doesn't
  rebuild that `.obj` when the header changes; a deleted header still listed
  stops MAKE with an error.
