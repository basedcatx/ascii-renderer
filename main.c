#include <curses.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <ncurses.h>
#include <stdio.h>

#include "includes/ffmpeg-utils.h"
#include "includes/logger.h"

int main(int argc, char *argv[]) {
  if (argc < 1) {
    LOG("USAGE: ./ascii-renderer -v video.mp4");
  }

  char *video_path = argv[1];

  if (!video_path) {
    LOG("Invalid Video SRC supplied, Check if the video exists");
  }

  initscr();
  curs_set(0);

  int term_width, term_height;
  getmaxyx(stdscr, term_height, term_width);
  AVFormatContext *ctx = load_video_file(video_path);
  scale_and_draw_frames(ctx, term_width, term_height);
  avformat_close_input(&ctx);
  avformat_free_context(ctx);
  endwin();
  return 0;
}