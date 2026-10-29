# V2 tekshiruv — 2026-10-27

## Haqiqiy qurilmalar
- Gateway COM4 ga yangi dastur yozildi, flash hash tekshiruvi o'tdi. Hajm: 1 013 060 bayt (77%). Statik RAM: 55 872 bayt (17%); 512 yozuvli tarix ishga tushishda heap'dan ajratiladi.
- C3 COM14 dagi avvalgi ishlaydigan 100 Hz ESP-NOW dasturi o'zgartirilmadi; yangi gateway uning 41 bayt paket formatiga mos.
- Gateway qayta ishga tushirilib avtomatik kalibrlash tekshirildi. 300 ta tinch o'lchovdan so'ng `CAL OK id=1`, `saved=1` chiqdi.
- Tayanch vektor: [0.11601, -0.02231, 1.05454] g. Gyro nol siljishi: [-0.8152, 0.6030, -2.1792] daraja/s.
- Kalibrlashdan so'ng `state=normal`, qiyalik 0.04–0.07°, burchak tezligi moduli 0.046–0.069 daraja/s.
- So'nggi qabul 98–100 paket/s; ayrim paket yo'qotishlari hisoblagichda qayd etilgan. Bu kafolatlangan transport emas.
- LCD 0x27 da aniqlandi va monitoring davom etmoqda.
- Asosiy plataning yangi HTTP API'siga kompyuterdan so'rov tarmoq orqali yetib bormadi (kompyuter AP tarmog'iga ulanmagan). Yangi HTTP API'ning haqiqiy telefon sinovi foydalanuvchi tomonidan bajarilishi mumkin; serial orqali kalibrlash/qabul ishlashi tasdiqlangan.

## Brauzer va CSV sinovi — SUN'IY MA'LUMOT
- Mahalliy fixture'da panel ochildi, tugmalar ishladi. Avvalgi ishga tushishda kalibrlash tugmasi jarayon holatini ko'rsatdi va 3 soniyadan so'ng tugadi.
- Qaydni boshlash/to'xtatish va CSV yuklab olish brauzerda tekshirildi: 4 000 qator, 26 ustun, 10 000 mikrosekundlik sensor vaqt qadami. Bu sinov ma'lumoti, haqiqiy tajriba qaydi emas.
- JavaScript sintaksisi node --check bilan o'tdi.
- Telefon ko'rinishi 390 px oynada tekshirildi: sahifa eni 375 px, scrollWidth ham 375 px, gorizontal toshish yo'q.
- Desktop va mobil matnlar ko'zdan kechirildi. dashboard-v2-preview.jpg sun'iy fixture ko'rinishidir.
- MATLAB skripti va format yo'riqnomasi berildi. MATLAB bu kompyuterda ishga tushirilib tekshirilmagan.

Kalibrlash joriy holatni tayanch qiladi, gyro nol siljishini tuzatadi va tinch tezlanish modulini me'yorlaydi. To'liq olti holatli kalibrlash, sensor fusion yoki mutlaq orientatsiya kafolati berilmaydi.
