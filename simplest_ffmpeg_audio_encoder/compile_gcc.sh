#! /bin/sh
gcc simplest_ffmpeg_audio_encoder.cpp -g -o simplest_ffmpeg_audio_encoder.out \
-I /usr/local/include -L /usr/local/lib -lavformat -lavcodec -lavutil
