Audio instructions
=============

In order to process audio on the development board, we can use sample_audio or ffmpeg or build alsa tools from source code.

# Use samle_audio

## Usage:

./sample_audio <index> --list
-r [sample_rate] -R [Chnsample_rate]
-c [channel] -p [preiod_size][*aac enc must 1024]
-C [codec 0:g726 1:g711A 2:g711Mu 3: adpcm 4.AAC]
-V [bVqeOn] -F [In/Out filename] -T [record time]

## Index

0 : Ai bind Aenc, save as file
1 : Ai unbind Aenc, save as file
2 : Adec bind ao, save as file
3 : Adec unbind ao, save as file
4 : recording frame by frame
5 : playing audio frame by frame
6 : SetVolume db test
7 : Audio Version
8 : GetVolume db test
9 : ioctl test
10: Aec test

## Examples

Aenc    eg : ./sample_audio 0 --list -r 8000 -R 8000 -c 2 -p 320 -C 1 -V 0 -F Cvi_8k_2chn.g711a -T 10
           : ./sample_audio 0 --list -r 8000 -R 8000 -c 2 -p 1024 -C 4 -V 0 -F Cvi_8k_2chn.aac -T 10
Adec    eg : ./sample_audio 2 --list -r 8000 -R 8000 -c 2 -p 320 -C 1 -V 0 -F Cvi_8k_2chn.g711a -T 10
Ai      eg : ./sample_audio 4 --list -r 8000 -R 8000 -c 2 -p 320 -C 0 -V 0 -F Cvi_8k_2chn.raw -T 10
Ao      eg : ./sample_audio 5 --list -r 8000 -R 8000 -c 2 -p 320 -C 0 -V 0 -F Cvi_8k_2chn.raw -T 10
SetVol  eg : ./sample_audio 6
GetVol  eg : ./sample_audio 8
AECtest eg : ./sample_audio 10 --list -r 8000 -R 8000 -c 2 -p 320 -C 0 -V 1 -F play.wav -T 10

## External codec

### AAC

AAC (Advanced Audio Coding) is a file compression format specifically designed for audio data. It utilizes a high compression ratio audio compression algorithm, supports multiple audio tracks, various sampling rates and bitrates, and has higher decoding efficiency, allowing for better sound quality while reducing space. For this implementation, the Fdk-aac encoding library is used, which offers more precise output bitrate control compared to other AAC libraries. The CVITEAK SDK PACKAGE only provides the interface; users must download and compile the required AAC library on their own.

#### Compile the AAC library

The version of Fdk_aac used is 2.0.1, download link: https://github.com/mstorsjo/fdkaac/releases

Place the fdk-aac-2.0.1.tar.gz in the middleware/v2/sample/audio/aac_sample directory and extract it, naming it fdkaac.

Then run `make clean & make`.

#### Test AAC codec

Before entering the codec test, please make sure that the basic test has passed.

Go to the middleware/v2/sample/audio/ directory and execute `make` to generate the sample_audio executable file (skip this step if it has already been generated).

Please ensure that the shared library generated in the first step is in the same directory as sample_audio or placed in a path supported by the LD_LIBRARY_PATH.

Run the following commands:
```
./sample_audio 0 --list -r 8000 -R 8000 -c 2 -p 1024 -C 4 -V 0 -F Cvi_8k_2chn.aac -T 10
./sample_audio 2 --list -r 8000 -R 8000 -c 2 -p 320 -C 4 -V 0 -F Cvi_8k_2chn.aac -T 10
```
Note: The cvitek sdk package, for the convenience of customers interfacing with Fdk_aac, will compile an empty AAC codec library. For actual testing, simply replace it with the real library compiled in the first step.

### MP3

MP3 (MPEG1 Layer-3) is a digital audio encoding format using lossy data compression. It is the third compression encoding format in the audio layer of MPEG(Moving Picture Experts Group). Compared to Layer-1 and Layer-2, MP3 encoding is more complex and is commonly used for high-quality audio transmission over the internet, with compression ratios as high as 1:10 or even 1:12.

Note: The Cvitek SDK does not support MP3, and Cvitek has no authorization or licensing rights to mad/libmad. Therefore, Cvitek only provides public interface sample code and is not responsible for any commercial use by any other owners or suppliers. Users should assume full responsibility when using mad/libmad.

Currently, only the MP3 decoder part is released. If encoding is needed, please contact FAE.

#### Libraries required for MP3 encoding and decoding

Mp3 encode:
libcvi_mp3enc.so、 libmp3lame.so、 libmp3lame.so.0、 libmp3lame.so.0.0.0

MP3 decode:
libcvi_mp3.so、 libmad.so、 libmad.so.0、 libmad.so.0.2.1

#### Test MP3 decode

Check if the required MP3 .so files are present in middleware/3rdparty/libmad. If not, consult FAE to ensure you obtain the MP3 lib.

Place the MP3 lib in middleware/lib/3rd.

Execute make clean & make cvi_mp3player.

This will generate cvi_mp3player in the current directory.

Transfer the generated MP3 executable files to the board for testing, remembering to link the MP3 lib.

# Use FFmpeg

## Command

  ffmpeg -f alsa <input_options> -i <input_device> ... output.wav

  ffmpeg -i input.wav -f alsa <output_options> <output_device>

To see the list of cards currently recognized by your system check the files /proc/asound/cards and /proc/asound/devices.

## Examples

To capture with ffmpeg from an ALSA device with card id 0, you may run the command:

  ffmpeg -f alsa -i hw:0 output.wav

To play a file on soundcard 1, audio device 7, you may run the command:

  ffmpeg -i input.wav -f alsa hw:1,7

* For more details, you can visit https://ffmpeg.org/documentation.html


# Build from source code

## Download source code

Download the source code here: https://www.alsa-project.org/wiki/Download#Detailed_package_descriptions

```
# tar -xvjf alsa-lib-1.2.9.tar.bz2
# tar -xvjf alsa-utils-1.2.9.tar.bz2
```

## Build alsa-lib

```
# cd alsa-lib-1.2.9/
# ./configure --prefix=<path_to_build_directory>/alsa-lib --host=aarch64-linux-gnu --with-configdir=/usr/share/alsa-arm
# make
# sudo make install
```

## Build alsa-utils

```
# cd alsa-utils-1.2.9/
# ./configure --prefix=<path_to_build_directory>/alsa-utils --host=aarch64-linux-gnu --with-alsa-inc-prefix=<path_to_build_directory>/alsa-lib/include/ --with-alsa-prefix=<path_to_build_directory>/alsa-lib/lib/ --disable-alsamixer --disable-xmlto
# make
# sudo make install
```

## Configure the board

### Copy files from the build directory to the root directory of the board.

```
# <path_to_build_directory>/alsa-lib/lib/*      -->  /usr/lib
# <path_to_build_directory>/alsa-utils/bin/*    -->  /usr/bin
# <path_to_build_directory>/alsa-utils/sbin/*   -->  /usr/bin
# <path_to_build_directory>/alsa-utils/share/*  -->  /usr/share
# /usr/share/alsa-arm/*                         -->  /usr/share/alsa-arm
```

### Set environment variables

```
# export ALSA_CONFIG_PATH=/usr/share/alsa-arm/alsa.conf
```

## Use tools

### Play audio

To see the list of playback hardware devices, you may run the command: aplay -l

For example, to play a file on soundcard 1, audio device 1, you may run the command:

  aplay -D hw:1,1 play.wav

### Record Audio

To see the list of capture hardware devices, you may run the command: arecord -l

For example, to capture a file on soundcard 0, audio device 1, you may run the command:

  arecord -D hw:0,1 -r 16000 -c 2 -f S16_LE test4.wav
