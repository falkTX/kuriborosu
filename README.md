# 🌰 koriborosu 🌰

A simple Audio and MIDI file render, powered by [Carla Plugin Host](https://github.com/falkTX/Carla)

Still very much work-in-progress, but the idea is simply:

* Specify 1 input file to read (can be an audio or MIDI file, optionally also just a number of seconds)
* Specify what plugins to run after, in order of processing

Koriborosu will run the plugins and generate an audio file at the end with its output.

This can be used to:

* Simple render of a MIDI file to WAV via softsynth
* Render a MIDI file, with added MIDI filters on top, then synths, then some FX because why not
* Apply some FX on an audio file
