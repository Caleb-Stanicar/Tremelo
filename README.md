# Tremolo / Auto-Pan / Wah Plugin

A JUCE audio plugin (VST3 + AU) combining a BPM-synced stereo tremolo/auto-pan and a BPM-synced wah effect.

---

## Signal Chain

```
Input → Tremolo/Auto-pan → Wah (if enabled) → Output
```

---

## Effects

### Tremolo / Auto-pan
Modulates amplitude using a sine LFO. Left and right channels are 180° out of phase,
creating an auto-pan effect. The LFO can run free (Hz) or locked to BPM subdivisions.

**Parameters**
| Name | Range | Description |
|---|---|---|
| Sync to BPM | on/off | Switch between free Hz mode and BPM-locked divisions |
| Rate | 0.1–20 Hz | LFO speed in free mode |
| Division | 1/32–2/1 | LFO speed in sync mode |
| Depth | 0–1 | How deep the volume dips on each cycle |

### Wah
A resonant bandpass filter whose center frequency is swept by a BPM-synced LFO.
Implements the Chamberlin State Variable Filter (SVF) — a stable two-pole filter
that produces lowpass, bandpass, and highpass outputs simultaneously from the same
memory. The bandpass output is used as the wah signal.

Center frequency sweeps exponentially between 300–3000 Hz. Exponential (not linear)
because human pitch perception is logarithmic — equal ratios sound like equal intervals.

**Parameters**
| Name | Range | Description |
|---|---|---|
| Wah On | on/off | Bypasses the filter entirely when off |
| Depth | 0–1 | Width of the frequency sweep |
| Rate | 1/16–1/1 | LFO speed, always BPM-synced |
| Offset | None/1/16/1/8/1/4 | Phase shift of the wah LFO relative to the bar |

---

## DSP Notes

### State Variable Filter (SVF)
The filter maintains two state values per channel (`svfLow`, `svfBand`), each acting
as a leaky accumulator — a weighted history of past input biased toward recent samples.
The tuning coefficient `f = 2 * sin(π * fc / fs)` controls how fast the accumulators
update: low center frequency = slow updates = long memory; high = fast = short memory.

Resonance (Q) is implemented by feeding the bandpass output back into the highpass
computation. This creates a self-reinforcing peak at the center frequency — the
"honk" characteristic of a wah pedal.

```cpp
float high   = input - svfLow[ch] - q * svfBand[ch];
svfBand[ch] += f * high;   // bandpass accumulator
svfLow[ch]  += f * svfBand[ch];  // lowpass accumulator (one step further removed)
output = svfBand[ch];
```

### Frequency Sweep
```cpp
float centerFreq = midFreq * std::exp(wahDepth * (lfo - 0.5f) * logRange);
```
`midFreq` is the geometric mean of 300 and 3000 Hz (~949 Hz). Multiplying by `exp()`
converts from log-space (where human perception is linear) back to Hz.

---

## Build

Requires CMake 3.22+ and the JUCE submodule.

```bash
cmake -B build
cmake --build build --config Debug
```

Output: `build/Tremolo_artefacts/Debug/VST3/` and `.../AU/`

---

## Install to Ableton Live (macOS)

Build the plugin first, then copy the formats you want into the system plugin folders.

**AU (recommended for Ableton):**
```bash
cp -r "build/Tremolo_artefacts/Debug/AU/Tremolo Auto-Pan Wah.component" \
      ~/Library/Audio/Plug-Ins/Components/
```

**VST3:**
```bash
cp -r "build/Tremolo_artefacts/Debug/VST3/Tremolo Auto-Pan Wah.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/
```

After copying, rescan in Ableton:
1. Open Ableton → **Preferences** → **Plug-ins** tab
2. Enable **Use Audio Units** and/or **Use VST3 Plug-In Custom Folder**
3. Click **Rescan** (or restart Ableton)
4. The plugin will appear under **Audio Effects → Audio Units → MyPlugins** (AU) or in the VST3 browser

For a Release build, replace `Debug` with `Release` in the paths above.
