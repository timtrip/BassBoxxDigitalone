# Bass Boxx Digital One — v0.9

A JUCE 8 virtual instrument prototype centered on one modeled bass string and a movable virtual pickup. It builds as a VST3 plug-in or a standalone instrument.

## Controls

- **Pickup:** turn the knob or drag the amber pickup marker along the string to change the harmonic balance.
- **Sub:** blends in a sub oscillator that follows the played note. Choose sine, triangle, square, or saw.
- **Drive:** adds saturation.
- **Pluck:** changes the excitation brightness.
- **Slide:** sets glide time between played pitches; zero gives immediate pitch changes.
- **Wah:** adds a slow, resonant band-pass sweep to the bass. Turn it fully down for the unfiltered sound.
- **Pitch bend:** responds to MIDI pitch wheel over a two-semitone range in either direction.

The instrument is monophonic by design. Play it from a MIDI keyboard or a DAW MIDI track. A note-on plucks the string; the matching note-off damps it.

## Build

On macOS, run the included one-click build script from Terminal:

```sh
cd ~/Downloads/BassBoxxDigitalOne
zsh ./build-mac.command
```

It builds the VST3 and standalone app, then installs the VST3 in `~/Library/Audio/Plug-Ins/VST3`. The first build downloads JUCE and needs an internet connection.

Requirements: CMake 3.22 or newer, a C++17 compiler, Git, and an internet connection for the first configure (CMake fetches JUCE 8.0.6).

### macOS or Linux

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Windows

Run the same configure command from a Visual Studio Developer PowerShell, then build with:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The VST3 bundle is produced under `build/BassBoxxDigitalOne_artefacts/Release/VST3`. The standalone app is in the same artefacts tree. Copy the VST3 bundle to your system's VST3 plug-in folder, then rescan plug-ins in your DAW.

## Project status

This source package contains the playable monophonic instrument, parameter automation/state, MIDI note and pitch-wheel input, and the first UI controls. It has not been compiled in this environment because CMake is unavailable here. Sample import, wah, kick transient, and additional string articulations remain future features.
