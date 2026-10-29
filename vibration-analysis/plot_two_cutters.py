from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

out = Path(__file__).parent/'analysis-2026-10-27'
z = np.load(out/'plot-data.npz')
t, xyz, w = z['t'], z['xyz'], z['windows']
plt.rcParams.update({'font.family':'DejaVu Sans','font.size':11,'axes.spines.top':False,'axes.spines.right':False})
fig, axs = plt.subplots(3,1,figsize=(13,10),gridspec_kw={'height_ratios':[1.2,1.1,1]},constrained_layout=True)
colors=['#067a8a','#d97706']
for seg,color in zip([1,2],colors):
    mask = t<102 if seg==1 else t>212
    ix=np.flatnonzero(mask)
    # Break lines for missing intervals; use a mean-removed vector norm for display.
    value=np.linalg.norm(xyz[ix]-xyz[ix].mean(axis=0),axis=1)
    value[np.r_[False,np.diff(t[ix])>.0301]]=np.nan
    axs[0].plot(t[ix],value,color=color,lw=.65,label=f'Kesgich {seg} (tanaffus asosida)')
    ww=w[w[:,0]==seg]
    axs[1].plot(ww[:,1],ww[:,2],'.-',color=color,lw=1.4)
    axs[2].semilogy(z['freq'][1:],z['psd'][seg-1,1:],color=color,lw=1.6,label=f'Kesgich {seg}')
for ax in axs[:2]:
    ax.axvspan(101.391,212.877,color='#e5e7eb',alpha=.8)
    ax.set_xlim(0,246)
    ax.grid(alpha=.18)
    ax.set_xlabel('Yozuv boshidan vaqt (s)')
axs[0].set_ylabel('Markazlangan vektor\namplitudasi (g)')
axs[0].set_title('Ikki kesgich: MPU6050 yozuvidagi tebranishlar',loc='left',weight='bold',fontsize=17)
axs[0].legend(loc='upper left',fontsize=10)
axs[0].text(157,axs[0].get_ylim()[1]*.55,"111,486 s tanaffus\nMa'lumot yo'q",ha='center',color='#374151')
axs[1].set_ylabel('2 soniyalik oynada\nvektor RMS (g)')
axs[1].set_title('DC olib tashlangan; uzilishli yoki to‘liq bo‘lmagan oynalar chiqarilgan',fontsize=11,loc='left')
axs[2].set_xlim(1,40)
axs[2].set_xlabel('Chastota (Hz)')
axs[2].set_ylabel('Yig‘indi PSD (g²/Hz)')
axs[2].set_title('Taxminiy Welch spektri: faqat yozilgan past chastotalar',fontsize=11,loc='left')
axs[2].grid(alpha=.18)
axs[2].legend(fontsize=10)
fig.savefig(out/'kesgichlar-tahlili.png',dpi=160,facecolor='white')
print(out/'kesgichlar-tahlili.png')
