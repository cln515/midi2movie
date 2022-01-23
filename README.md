# midi2movie

Auto movie generator from Midi file (.mid) and Wav file (.wav)
https://www.youtube.com/watch?v=WWxiGiIM5lU

## Environments
Windows 10

## Usage
First, install FFmpeg (https://www.ffmpeg.org/). Type "FFmpeg" at the command prompt, and the program should start.

describe setting in .json file and run

```$midi2movie.exe input.json```

Important parameter
- "wav": path to input wav file
- "output": output movie file (.mp4) 
- "width": movie width
- "height": movie height
- "midi": midi notes setting
- "background": background image setting
- "text": text setting

detail (Japanese): (https://sounds-cocktail.com/tools/midi2movie.html)

## dependency for build
- Midifile (https://github.com/craigsapp/midifile)
- OpenCV
- Json Parser (https://github.com/nlohmann/json)
- Eigen3
- GLFW
