# HOW TO USE

* You must first install all the required packages, this wasn't statically linked :\P

```bash

sudo apt install libavcodec-dev libavformat-dev libavutil-dev libavdevice-dev libavfilter-dev libpostproc-dev libswscale-dev
libncurses-dev

```

```bash
cmake . && make ascii-renderer
```

## USAGE

```bash
ascii-renderer -v /path/to/video/file
```

* PS: I just made this today, so don't expect a lot of features xd it can just play videos in gray-scale with audio. With time i'd add pixel colors and maybe some video control, who knows :P
