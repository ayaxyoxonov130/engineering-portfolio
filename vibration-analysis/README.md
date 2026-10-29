# MPU6050 Vibration Analysis

Analysis scripts from Asadulloh Yahyoxonov's master's research prototype, developed with AI assistance. This complements the ESP32 monitoring project; it is not a separate commercial engagement.

## Contents

- `analyze_two_cutters.py`: segment-aware RMS, peak, acquisition-gap and spectral analysis of an exported experiment CSV.
- `plot_two_cutters.py`: plots the analysis output.
- `analyze_harakat.m`: interactive MATLAB import and analysis. Not executed in MATLAB during the recorded checks.
- `analysis-2026-10-27/TAHLIL.md`: original Uzbek analysis report, including assumptions and limitations.
- `analysis-2026-10-27/kesgichlar-tahlili.png`: figure from the recorded experiment.

## Run

Install Python packages listed in `requirements.txt`, then run:

```sh
python analyze_two_cutters.py path/to/experiment.csv
python plot_two_cutters.py
```

Supply a CSV exported by the matching ESP32 gateway. Raw experiment data is not bundled. Analysis writes `results.json`, `window_metrics.csv` and `plot-data.npz` in `analysis-2026-10-27/`.

The recorded experiment contained 13,277 rows. Two sections were separated using a major recording gap and the user's explanation that cutters were changed during pauses. The CSV had no cutter labels. Different section lengths and unknown cutting conditions prevent a controlled tool-wear comparison. Higher RMS in one section does not prove that its cutter was worn. The 100 Hz acquisition and approximately 44 Hz accelerometer filter restrict usable vibration bandwidth.

This is an exploratory analysis, not a trained or validated diagnostic model.
