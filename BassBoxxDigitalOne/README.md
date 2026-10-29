# Bass Boxx Digital One

An early JUCE instrument prototype built around one modeled bass string and one pickup.

## First playable controls

- **Pickup:** changes where the virtual pickup listens along the string, changing the harmonic balance.
- **Sub:** blends in a sine sub oscillator that follows the played note.
- **Drive:** adds saturation.
- **Pluck:** changes the excitation brightness.
- **MIDI:** note-on plucks the string; note-off damps it. It is intentionally monophonic: one note at a time, one string.

## Build

Requires CMake, a C++17 compiler, and an internet connection on the first configure so CMake can fetch JUCE 8.0.6.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The project creates VST3 and Standalone targets. Build and host compatibility still need to be checked on the target computer/DAW.

## Direction

Keep the instrument centered on the single string. Next refinements should make pickup position directly draggable on the string, then add pitch bends/slides and optional sample excitation that still follows the same string.
