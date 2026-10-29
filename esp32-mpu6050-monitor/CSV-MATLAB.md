# MATLAB uchun o'lchov qaydi

1. Panelda kalibrlash tugashini kuting, keyin **Qayd etishni boshlash** tugmasini bosing.
2. Tajribani bajaring va **Qaydni to'xtatish** tugmasini bosing.
3. **CSV yuklab olish** tugmasi orqali faylni saqlang.
4. MATLAB'da `analyze_harakat.m` faylini ishga tushiring va CSV'ni tanlang.

```matlab
T = readtable('harakat.csv', 'VariableNamingRule', 'preserve');
```

Eksport brauzerning kartalarni yangilash tezligida emas, gateway olgan sensor paketlari bo'yicha yig'iladi. Maqsad 100 Hz; radio va tarmoq yo'qotishlari bo'lishi mumkin. Gateway taxminan 5 soniyalik tarix saqlaydi, brauzer esa qayd yoqilganida so'nggi 30 000 qatorni saqlaydi. Sahifa yopilsa yoki yangilansa saqlanmagan CSV qaydi yo'qoladi. Qaydni to'xtatib, keyin davom ettirish orasida vaqt bo'shlig'i qoladi.

## Ustunlar

| Ustun | Ma'nosi |
|---|---|
| gateway_session | Asosiy plataning ishga tushish identifikatori |
| browser_batch_utc | Paketlar to'plami brauzerga kelgan UTC vaqti; o'lchov vaqti emas |
| packet_id | Gateway qabul tartibi |
| sensor_session | Sensor qayta ishga tushsa o'zgaradi |
| sequence | C3 paketining tartib raqami; uzilishlarni tekshirish uchun |
| sensor_time_us | C3 o'lchov jadvalining mikrosekund vaqti; 32 bit aylanishi gateway'da davom ettirilgan |
| receiver_time_ms | Paket gateway'ga kelgan payt, asosiy plata ishga tushganidan beri millisekund |
| sensor_valid | 1 — sensor o'qildi, 0 — sensor xatosi |
| raw_ax_g, raw_ay_g, raw_az_g | Asl akselerometr o'qlari, g |
| raw_gx_dps, raw_gy_dps, raw_gz_dps | Asl giroskop o'qlari, daraja/soniya |
| sensor_temp_c | MPU6050 chip harorati, °C |
| ax_g, ay_g, az_g | Tinch holatdagi vektor moduli 1 g ga me'yorlangan o'qlar |
| gx_dps, gy_dps, gz_dps | Giroskopning o'lchangan o'rtacha nol siljishi ayirilgan qiymatlar |
| accel_norm_g | Me'yorlangan tezlanish vektori moduli |
| relative_tilt_deg | Boshlang'ich tayanch vektor bilan hozirgi vektor orasidagi burchak |
| deviation_g | `abs(accel_norm_g - 1)`; dinamik tezlanish vektorining to'liq moduli emas |
| calibration_id | 0 — kalibrlanmagan, musbat — ushbu gateway sessiyasidagi tayanch ID |
| state_code | 0 — me'yorda, 1 — diqqat, 2 — chegara oshgan, 3 — kalibrlanmoqda, 4 — sensor xatosi |

`calibration_id=0` qatorlarida tuzatilgan qiymatlar kalibrlanmagan; tahlil uchun `sensor_valid=1` va `calibration_id>0` ni tanlang. Burchak tezligi integratsiyasidan orientatsiya hisoblanmaydi; qiyalik akselerometr vektori orqali topiladi va faol harakatda inertsiya xatosi bor.

Har xil gateway/sensor sessiyalari va kalibrlash ID'larini bitta uzluksiz signal deb olmang. Vaqt qadamini CSV'dan hisoblang: 100 Hz deb ko'r-ko'rona qabul qilmang. FFT uchun notekis vaqt oralig'i yoki katta bo'shliqlarni tekshiring. Skript eng oxirgi sessiya va kalibrlashni tanlaydi; spektr uchun uzluksiz qismni taxminiy interpolatsiya qiladi.

Manbalar: [MathWorks readtable](https://www.mathworks.com/help/matlab/ref/readtable.html), [MathWorks: notekis vaqtli signal spektri](https://www.mathworks.com/matlabcentral/answers/92117-how-can-i-find-the-fft-of-a-time-series-when-the-samples-are-not-equally-spaced).
