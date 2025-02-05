#include "../includes/ffmpeg-utils.h"

#include <SDL2/SDL.h>
#include <SDL_timer.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "../includes/logger.h"

#define FPS_CAP 75

ulong get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000) ;
}

void render_frames(AVFrame *frame);
pthread_mutex_t video_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audio_thread = PTHREAD_MUTEX_INITIALIZER;

AVFormatContext *load_video_file(const char *path) {
  AVFormatContext *context = avformat_alloc_context();
  if (avformat_open_input(&context, path, NULL, NULL) != 0) {
    LOG("Invalid media file. Error loading");
    return NULL;
  }
  LOG("File opened successfully!\n");
  LOG("Format: %s, Duration: %ld microseconds\n", context->iformat->name,
      context->duration);
  return context;
}

int check_media_stream_info(AVFormatContext *context) {
  if (!context) {
    return -1;
  }

  if (avformat_find_stream_info(context, NULL) < 0) {
    LOG("Couldn't find stream info in media file. Is that file corrupt?");
    return -1;
  }
  return 0;
}

void display_all_streams(AVFormatContext *context) {
  if (context) {
    for (int i = 0; i < context->nb_streams; i++) {
      AVStream *stream = context->streams[i];
      LOG("Stream: %d\t", i);
      if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        LOG("Video,");
        LOG("Resolution: %d X %d \n", stream->codecpar->width,
            stream->codecpar->height);
        LOG("Duration: %ld", stream->duration);
      } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        LOG("Video,");
        LOG("Sample rate: %d, Channels: %d\n", stream->codecpar->sample_rate,
            stream->codecpar->ch_layout.nb_channels);
      }

      const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
      if (codec) {
        printf("Codec: %s\n", codec->name);
      } else {
        printf("Unknown codec");
      }
    }
  }
}

void display_all_video_streams(AVFormatContext *context) {
  if (context) {
    for (int i = 0; i < context->nb_streams; i++) {
      AVStream *stream = context->streams[i];
      LOG("Stream: %d\t", i);
      if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        LOG("Video,");
        LOG("Resolution: %d X %d \n", stream->codecpar->width,
            stream->codecpar->height);
        LOG("Duration: %ld", stream->duration);
      }
      const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
      if (codec) {
        printf("Codec: %s\n", codec->name);
      } else {
        printf("Unknown codec");
      }
    }
  }
}

void display_all_audio_streams(AVFormatContext *context) {
  if (context) {
    for (int i = 0; i < context->nb_streams; i++) {
      AVStream *stream = context->streams[i];
      LOG("Stream: %d\t", i);

      if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        LOG("Audio,");
        LOG("Sample rate: %d, Channels: %d\n", stream->codecpar->sample_rate,
            stream->codecpar->ch_layout.nb_channels);
      }

      const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
      if (codec) {
        printf("Codec: %s\n", codec->name);
      } else {
        printf("Unknown codec");
      }
    }
  }
}

int *get_audio_stream_indices(AVFormatContext *context) {
  if (context) {
    int *streamArray = malloc(context->nb_streams);
    memset(streamArray, -1, context->nb_streams);

    if (!streamArray) {
      LOG("Could not allocated: %d bytes for stream processing",
          context->nb_streams);
      return NULL;
    }

    int audioStreamCount = 0;
    for (int i = 0; i < context->nb_streams; i++) {
      AVStream *stream = context->streams[i];
      LOG("Stream: %d\t", i);

      if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        LOG("Video,");
        LOG("Sample rate: %d, Channels: %d\n", stream->codecpar->sample_rate,
            stream->codecpar->ch_layout.nb_channels);
        streamArray[audioStreamCount++] = i;
      }
    }
    return streamArray;
  }
  return NULL;
}

int get_video_stream_index(AVFormatContext *context) {
  if (context) {
    for (int i = 0; i < context->nb_streams; i++) {
      AVStream *stream = context->streams[i];
      LOG("Stream: %d\t", i);

      if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        LOG("Video,");
        LOG("Sample rate: %d, Channels: %d\n", stream->codecpar->sample_rate,
            stream->codecpar->ch_layout.nb_channels);
        return i;
      }
    }
  }
  return 0;
}

void display_video_frames(AVFormatContext *context) {
  int videoStreamIndex = get_video_stream_index(context);
  AVCodecParameters *codecParams = context->streams[videoStreamIndex]->codecpar;
  const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
  AVCodecContext *codecContext = avcodec_alloc_context3(codec);
  avcodec_parameters_to_context(codecContext, codecParams);
  avcodec_open2(codecContext, codec, NULL);

  AVPacket pck;
  AVFrame *frame = av_frame_alloc();
  int frameCount = 0;

  while (av_read_frame(context, &pck) >= 0) {
    if (pck.stream_index == videoStreamIndex) {
      avcodec_send_packet(codecContext, &pck);
      if (avcodec_receive_frame(codecContext, frame) == 0) {
        LOG("Frame %d (PTS: %ld)\n", frameCount++, frame->pts);
      }
    }
    av_packet_unref(&pck);
  }

  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
}

void *init_video_proc(void *args) {
  struct video_thread_arg *arg = (struct video_thread_arg *)args;
  int width = arg->width;
  int height = arg->height;

  AVFormatContext *context = arg->context;

  AVStream *videoStream = NULL;
  int videoStreamIndex = -1;

  for (int i = 0; i < context->nb_streams; i++) {
    if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStream = context->streams[i];
      videoStreamIndex = i;
      break;
    }
  }

  if (!videoStream) return NULL;

  AVCodecParameters *streamParams = videoStream->codecpar;
  const AVCodec *videoCodec = avcodec_find_decoder(streamParams->codec_id);

  AVCodecContext *codecContext = avcodec_alloc_context3(videoCodec);
  avcodec_parameters_to_context(codecContext, streamParams);
  avcodec_open2(codecContext, videoCodec, NULL);

  AVPacket *packet = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();

  while (av_read_frame(context, packet) >= 0) {
    if (packet->stream_index != videoStreamIndex) continue;
    avcodec_send_packet(codecContext, packet);
    // av_packet_free(&packet);

    while (avcodec_receive_frame(codecContext, frame) == 0) {
      struct SwsContext *sws_ctx = sws_getContext(
          frame->width, frame->height, frame->format, width, height,
          AV_PIX_FMT_GRAY8, SWS_BILINEAR, NULL, NULL, NULL);

      AVFrame *resizedFrame = av_frame_alloc();
      resizedFrame->format = AV_PIX_FMT_GRAY8;
      resizedFrame->width = width;
      resizedFrame->height = height;
      av_frame_get_buffer(resizedFrame, 0);

      sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                resizedFrame->data, resizedFrame->linesize);

      render_frames(resizedFrame);

      sws_freeContext(sws_ctx);
      av_frame_free(&resizedFrame);
    }
  }
  return NULL;
}

void scale_and_draw_frames(AVFormatContext *context, int width, int height) {
  if (!context) {
    return;
  }

  video_thread_arg args = {context, width, height};
  pthread_t audio_r_thread, video_r_thread;

  if (pthread_create(&video_r_thread, NULL, init_video_proc, (void *)&args) !=
      0) {
    perror("Thread creation failed!");
    exit(-1);
  }

  pthread_join(video_r_thread, NULL);

  // init video playing on a new thread;

  // init video streaming
}

char grayscale_to_shading(uint8_t gray) {
  // const char *shading_boxes = "#@%*+=-:. ";
  const char *shading_boxes = " .*x@#";
  if (gray < 32) {
    return shading_boxes[0];
  } else if (gray < 64) {
    return shading_boxes[1];
  } else if (gray < 96) {
    return shading_boxes[2];
  } else if (gray < 128) {
    return shading_boxes[3];
  } else if (gray < 160) {
    return shading_boxes[4];
  } else
    return shading_boxes[5];
}

void render_frames(AVFrame *frame) {
  if (!frame) return;

  ulong frame_start = get_time();
  clear();
  start_color();
  use_default_colors();
  noecho();
  nodelay(stdscr, TRUE);

  // TIME TO DRAW

  int bytesPerRow = frame->linesize[0];
  uint8_t *rgb_data = frame->data[0];

  for (int y = 0; y < frame->height; y++) {
    uint8_t *px_ptr = frame->data[0] + y * bytesPerRow;
    for (int x = 0; x < frame->width; x++) {
      uint8_t gray = *px_ptr++;

      char pix = grayscale_to_shading(gray);

      mvaddch(y, x, pix);
    }
  }
  refresh();
  ulong frame_time = get_time() - frame_start;
  ulong delay = ((1000 / FPS_CAP) - frame_time);
  if ( delay > 0) {
    napms(delay);
  }
}