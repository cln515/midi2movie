# midi2movie

Auto movie generator from Midi file (.mid) and Wav file (.wav)

## Environments
Windows 10

## Usage
First, install FFmpeg (https://www.ffmpeg.org/). Type "ffmpeg" at the command prompt, and the program should start.

descrive setting in .json file and run

```$midi2movie.exe input.json```

Important parameter
- "midi": path to input midi file
- "wav": path to input wav file
- "output": output movie file (.mp4) 
- "mode": rendering mode (0 or 1, 0: midi note projection in flat plane. 1: in cylindrical surface)
- "width": movie width
- "height": movie height

## dependency for build
- Midifile (https://github.com/craigsapp/midifile)
- OpenCV
- Json Perser (https://github.com/nlohmann/json)
- Eigen3

