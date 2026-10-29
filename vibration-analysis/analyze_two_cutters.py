from pathlib import Path
import json
import numpy as np
import pandas as pd

import argparse
parser = argparse.ArgumentParser(description='Analyze an MPU6050 experiment CSV.')
parser.add_argument('csv', type=Path, help='Path to the original experiment CSV')
args = parser.parse_args()
SOURCE = args.csv
OUT = Path(__file__).parent / 'analysis-2026-10-27'
OUT.mkdir(exist_ok=True)
d = pd.read_csv(SOURCE)
t = (d.sensor_time_us.to_numpy() - d.sensor_time_us.iloc[0]) / 1e6
xyz = d[['raw_ax_g', 'raw_ay_g', 'raw_az_g']].to_numpy()
dt = np.diff(t)
breaks = np.flatnonzero(dt > 1) + 1
groups = np.split(np.arange(len(d)), breaks)
windows = []
segments = []
spectra = []
for number, ix in enumerate(groups, 1):
    tt, aa = t[ix], xyz[ix]
    ac = aa - aa.mean(axis=0)
    wrms = []
    for start in np.arange(tt[0], tt[-1] - 1.999, 2):
        wi = ix[(tt >= start) & (tt < start + 2)]
        if len(wi) < 190 or np.max(np.diff(t[wi])) > .0301:
            continue
        v = xyz[wi] - xyz[wi].mean(axis=0)
        rms = float(np.sqrt(np.mean(np.sum(v*v, axis=1))))
        peak = float(np.max(np.linalg.norm(v, axis=1)))
        windows.append([number, start + 1, rms, peak])
        wrms.append(rms)
    powers = []
    # Never fill the major gap. Split smaller acquisition gaps before FFT.
    runs = np.split(ix, np.flatnonzero(np.diff(tt) > .0301) + 1)
    for run in runs:
        if len(run) < 256:
            continue
        grid = np.arange(t[run[0]], t[run[-1]], .01)
        uniform = np.column_stack([np.interp(grid, t[run], xyz[run, j]) for j in range(3)])
        for k in range(0, len(uniform) - 255, 128):
            block = uniform[k:k+256]
            block = block - block.mean(axis=0)
            h = np.hanning(256)
            p = np.sum(np.abs(np.fft.rfft(block*h[:, None], axis=0))**2, axis=1)/(100*np.sum(h*h))
            p[1:-1] *= 2
            powers.append(p)
    freq = np.fft.rfftfreq(256, .01)
    psd = np.mean(powers, axis=0)
    spectra.append(psd)
    keep = (freq >= 1) & (freq <= 40)
    peak_freq = float(freq[keep][np.argmax(psd[keep])])
    seq = d.sequence.iloc[ix].to_numpy()
    entry = dict(segment=number, start_s=float(tt[0]), end_s=float(tt[-1]), samples=len(ix),
                 duration_s=float(tt[-1]-tt[0]), missing_sequence_inside=int(np.sum(np.maximum(np.diff(seq)-1,0))),
                 rms_axes_g=np.sqrt(np.mean(ac*ac, axis=0)).tolist(),
                 rms_vector_g=float(np.sqrt(np.mean(np.sum(ac*ac, axis=1)))),
                 centered_peak_vector_g=float(np.max(np.linalg.norm(ac, axis=1))),
                 window_rms_median_g=float(np.median(wrms)), window_rms_p95_g=float(np.quantile(wrms,.95)),
                 accepted_2s_windows=len(wrms), descriptive_psd_peak_1_40_hz=peak_freq,
                 mean_raw_accel_g=aa.mean(axis=0).tolist())
    segments.append(entry)
w = np.array(windows)
summary = dict(source=str(SOURCE), rows=len(d), span_s=float(t[-1]),
               valid_samples=int(d.sensor_valid.sum()),
               intervals_10ms_fraction=float(np.mean(np.isclose(dt,.01,atol=.000001))),
               median_sample_rate_hz=float(1/np.median(dt)),
               gaps=[dict(start_s=float(t[i]),end_s=float(t[i+1]),interval_s=float(dt[i]),
                          sequence_missing=int(d.sequence.iloc[i+1]-d.sequence.iloc[i]-1),
                          packet_ids_missing=int(d.packet_id.iloc[i+1]-d.packet_id.iloc[i]-1))
                     for i in np.flatnonzero(dt>.0301)],
               tilt_range_deg=[float(d.relative_tilt_deg.min()),float(d.relative_tilt_deg.max())],
               tilt_mean_deg=float(d.relative_tilt_deg.mean()),
               alarm_rows=int((d.state_code==2).sum()),
               saturated_rows=int((np.abs(xyz)>=1.99).any(axis=1).sum()), segments=segments)
summary['rms_ratio_second_over_first'] = segments[1]['rms_vector_g']/segments[0]['rms_vector_g']
summary['median_2s_rms_ratio_second_over_first'] = segments[1]['window_rms_median_g']/segments[0]['window_rms_median_g']
(OUT/'results.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
np.savez(OUT/'plot-data.npz',t=t,xyz=xyz,windows=w,freq=freq,psd=np.array(spectra))
pd.DataFrame(windows,columns=['segment','window_center_s','centered_vector_rms_g','centered_vector_peak_g']).to_csv(OUT/'window_metrics.csv',index=False)
print(json.dumps(summary,indent=2))
