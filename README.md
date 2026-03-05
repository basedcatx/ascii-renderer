```markdown
# ascii-renderer

A terminal-based video player that decodes video files and renders them as ASCII art in real time. Frames are decoded with FFmpeg, downscaled to fit your terminal dimensions, converted to grayscale, and mapped to a 6-character ASCII brightness palette — all displayed via ncurses.

## How it works

```
Video File → FFmpeg demux → Decode → swscale (resize + grayscale) → ASCII map → ncurses render
```

1. The video file is opened and demuxed with `libavformat`.
2. The video stream is decoded frame-by-frame with `libavcodec`.
3. Each frame is scaled to the terminal's character dimensions and converted to `AV_PIX_FMT_GRAY8` using `libswscale` (bilinear filtering).
4. Each grayscale pixel is mapped to one of 6 ASCII characters based on brightness:
   ```
   " .*x@#"
   (dark)  →  (bright)
   ```
5. The frame is drawn to the terminal using ncurses `mvaddch`.
6. Frame pacing uses SDL2 timing (`SDL_GetTicks` / `SDL_Delay`) to sync output to the source video's presentation timestamps.

Video decoding and rendering runs on a dedicated pthread. Audio processing is architecturally separated but not yet enabled.

## Requirements

- **OS:** Linux (uses POSIX APIs)
- **Compiler:** C99-compatible (gcc, clang)
- **CMake** >= 3.20
- **FFmpeg** libraries: `libavcodec`, `libavformat`, `libavutil`, `libswscale`
- **ncurses**
- **SDL2**

### Install dependencies

**Debian/Ubuntu:**
```bash
sudo apt install build-essential cmake pkg-config \
  libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
  libncurses-dev libsdl2-dev
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake pkgconf ffmpeg ncurses sdl2
```

**Fedora:**
```bash
sudo dnf install gcc cmake pkgconf-pkg-config \
  ffmpeg-free-devel ncurses-devel SDL2-devel
```

## Build

```bash
cmake -B build
cmake --build build
```

This produces the `ascii-renderer` binary in the `build/` directory.

## Usage

```bash
./build/ascii-renderer <video_file>
```

Example:
```bash
./build/ascii-renderer ~/videos/sample.mp4
```

The video will play in your terminal at the source frame rate. Resize your terminal before running to control the output resolution — the renderer reads terminal dimensions at startup via ncurses `getmaxyx`.

## Project structure

```
.
├── main.c                    # Entry point: argument parsing, ncurses init, orchestration
├── utils/
│   └── ffmpeg-utils.c        # Video pipeline: demux, decode, scale, render, frame pacing
├── includes/
│   ├── ffmpeg-utils.h        # Public API declarations and video_thread_arg struct
│   └── logger.h              # LOG() macro (stderr)
└── CMakeLists.txt            # Build configuration
```

## Limitations

- Audio playback is not yet functional (the thread architecture exists but the audio path is disabled).
- Terminal dimensions are read once at startup; resizing the terminal mid-playback is not handled.
- The `swscale` context is recreated per frame rather than reused, which adds overhead.
- No seek, pause, or playback controls.
