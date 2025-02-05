#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct video_thread_arg {
	AVFormatContext *context;
	int width;
	int height;
} video_thread_arg;

AVFormatContext *load_video_file(const char *path);
int check_media_stream_info(AVFormatContext *context);
void display_all_streams(AVFormatContext *context);
void display_all_video_streams(AVFormatContext *context);
void display_all_audio_streams(AVFormatContext *context);
int *get_audio_stream_indices(AVFormatContext *context);
int get_video_stream_index(AVFormatContext *context);
void display_video_frames(AVFormatContext *context);
void scale_and_draw_frames(AVFormatContext *context, int width, int height);

#endif