# Ikki kesgich bilan olingan MPU6050 yozuvi

Tahlil sanasi: 2026-10-28. Muallif: Yahyoxonov Asadulloh, M11-25 guruhi magistranti.

Manba: harakat_2026-10-27T08-20-39-983Z.csv. Asl fayl o‘zgartirilmadi.
13 277 qator, 26 ustun. Barcha qatorlarda sensor_valid=1, calibration_id=1, bitta sensor va gateway sessiyasi bor.
Bu CSV MPU6050 prototipining yozuvidir. TEAC 730ZT yoki PCB 333B30 signali emas.

## Kesgichlarni ajratish

Foydalanuvchi kesgich almashtirish tanaffuslarda bo‘lganini bildirdi. CSV ichida kesgich identifikatori yo‘q.
Shu sabab asosiy 111,486 soniyalik tanaffus ajratish chegarasi sifatida olindi.
Vaqt sensor_time_us ustunidan hisoblandi. Birinchi qatorda t=0. Tanaffus to‘ldirilmadi.

| Qism | Yozuv boshidan vaqt | Qatorlar | Talqin |
|---|---|---:|---|
| 1 | 0–101,391 s | 10 078 | Birinchi kesgich, tanaffus asosida |
| Tanaffus | 101,391–212,877 s | Yozuv yo‘q | Kesgich almashtirish oralig‘i deb qabul qilindi |
| 2 | 212,877–245,043 s | 3 199 | Ikkinchi kesgich, tanaffus asosida |

82,141–82,221 soniyada yana 0,080 soniyalik kichik uzilish bor. Bu kesgich almashtirish deb olinmadi.
Sensor va gateway sessiyalari hamda calibration_id asosiy tanaffusdan oldin va keyin o‘zgarmagan.
Katta tanaffusda sequence bo‘yicha 11 142 namuna, packet_id bo‘yicha 11 021 yozuv tashlab o‘tilgan.
Bu intervalni to‘liq radioaloqa nosozligi deb hisoblash to‘g‘ri emas: CSVga yozishdagi tanaffus ham bor.
Tanaffusdan tashqaridagi sequence bo‘shliqlari: birinchi qismda 57, ikkinchisida 17, jami 74 namuna (~0,55%).

## Natijalar

Xom ax, ay, az qiymatlarining har bir qism bo‘yicha o‘rtachasi ayirildi.
Shunda doimiy gravitatsiya va offsetning asosiy qismi olib tashlanadi. Bu usul tebranishdan sekin harakatni to‘liq ajratmaydi.
Vektor RMS = sqrt(mean((ax-mean(ax))²+(ay-mean(ay))²+(az-mean(az))²)).
Birlik g; 1 g=9,80665 m/s². Bu accel_norm_g ning RMS qiymati emas.

| Ko‘rsatkich | Birinchi kesgich qismi | Ikkinchi kesgich qismi |
|---|---:|---:|
| X RMS, g | 0,011776 | 0,024619 |
| Y RMS, g | 0,011382 | 0,016666 |
| Z RMS, g | 0,009407 | 0,017401 |
| Vektor RMS, g | 0,018887 | 0,034448 |
| Markazlangan vektor cho‘qqisi, g | 0,201442 | 0,233485 |
| 2 s oynalar RMS medianasi, g | 0,017142 | 0,018043 |
| 2 s oynalar RMS 95-persentili, g | 0,025824 | 0,065773 |
| Tahlilga yaroqli 2 s oynalar | 49 | 16 |

Ikkinchi qismda umumiy RMS 82,4% yuqori. Lekin 2 soniyalik oynalar medianasi faqat 5,3% yuqori.
Umumiy RMS farqi kuchli, qisqa epizodlarga sezgir. Ikkinchi qismning butun davomida tebranish 82% baland degan xulosa chiqarilmaydi.
Oynalar ichida alohida o‘rtacha ayirildi. Kamida 190 namuna talab qilindi va >30,1 ms bo‘shlig‘i bor oynalar chiqarildi.
Qismlar davomiyligi turlicha; ular ichida bo‘sh yurish va materialga kirish/chiqish aralashgan bo‘lishi mumkin.
Kesishning aniq boshlanish/tugash belgilari bo‘lmagani uchun teng sharoitli sof kesish davrlari alohida isbotlanmagan.

Kuchli epizodlar: birinchi qismda taxminan 66–70 s; ikkinchisida 215–219 s va 231–235 s.
Birinchi qismda 80 s atrofidan keyin tebranish sezilarli kamayadi. Bu kesish tugashi yoki bo‘sh yurishga mos kelishi mumkin, ammo CSVda buni tasdiqlovchi belgi yo‘q.
Ikkinchi qismning 232,877–234,877 s oynasida RMS 0,079181 g bo‘lib, qism ichidagi eng katta 2 s qiymat qayd etilgan.
Xom akselerometr kanallarida ±2 g chegarasiga yetish aniqlanmadi (|a|>=1,99 g mezoni bo‘yicha 0 qator).
sensor_valid=1 faqat sensor o‘qilganini bildiradi; chastota yoki montaj sifati kafolati emas.

## Chastota va o‘lchov cheklovlari

Median namuna oralig‘i 10 ms, ya’ni 100 Hz. Oraliqlarning 98,97 foizi 10 ms (1 mikrosoniya tolerans bilan).
Joriy sensor firmware faylida CONFIG=3, SMPLRT_DIV=9: akselerometr DLPF diapazoni 44 Hz.
100 Hz yozuvda Nyquist chegarasi 50 Hz. Filtr bu chegarada ideal kesmaydi, shuning uchun yuqori chastotalardan aliasing ehtimoli saqlanadi.
Mazkur yozuvdan yuqori chastotali kesish tebranishlari yoki akustik emissiyani baholab bo‘lmaydi.

Grafikdagi taxminiy spektr uchun >30,1 ms bo‘shliqlarda yozuv bo‘lindi. Har kichik uzluksiz bo‘lak 100 Hz tekis vaqtga chiziqli interpolatsiya qilindi.
256 namuna, Hann oynasi, 128 qadamli Welch usuli; har oynada o‘rtacha ayirildi, uch o‘q quvvati yig‘ildi.
Grafik 1–40 Hz bilan cheklangan. Qismlar bo‘yicha o‘rtacha spektrlarning eng katta nuqtalari 35,55 Hz va 10,94 Hz.
Bular faqat yozuvdagi spektral maksimumlar. Ularni aylanish chastotasi, chatter yoki yeyilish chastotasi deb atashga dalil yetarli emas.

Ishlab chiqaruvchi hujjati: https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf (CONFIG registri, 13-bet).

## Dashboarddagi ogohlantirish

Barcha 13 277 qatorda state_code=2. relative_tilt_deg o‘rtachasi 85,96°, oralig‘i 75,91–95,32°.
Joriy gateway firmware qiyalik >50° bo‘lganda 2-kodni beradi. Shu sabab bu yozuvdagi doimiy ogohlantirish qiyalik mezoni bilan tushuntiriladi.
Bu kesgich yeyilishi aniqlanganini bildirmaydi. Sensorning tayanch holati yoki montaj yo‘nalishi kalibrlash holatidan farq qilgan bo‘lishi mumkin.
Joriy dasturdagi qiyalik ogohlantirishi kesgich holatini baholash yorlig‘i sifatida ishlatilmasligi kerak.

## Ilmiy xulosa

Tanaffus asosida ikkinchi kesgichga tegishli deb olingan yozuvda kuchli tebranish epizodlari yaqqolroq.
Oddiy, zarbasiz epizodlar darajasining farqi ancha kichik. Natija ikkinchi yozuvdagi jarayon beqarorroq bo‘lganini ko‘rsatadi.
Qaysi kesgich o‘tmasroq ekanini faqat shu CSVdan tasdiqlab bo‘lmaydi: aylanish tezligi, surish, kesish chuqurligi, material va haqiqiy yeyilish o‘lchovi yo‘q.
Sensor o‘rnatilishi, materialga kirish, zarba yoki boshqa kesish sharoiti ham ko‘rsatilgan farqni yuzaga keltirishi mumkin.
Bu dastlabki kuzatuv sifatida foydali; tasdiqlangan yeyilish klassifikatori yoki statistik umumlashma emas.

Keyingi tajribada har kesgichni bir xil rejimda bir necha mustaqil kesishda sinash, kesish boshi/oxirini belgilash,
kesgich identifikatori va mikroskopdagi yeyilish natijasini yozish kerak. TEAC/PCB datchiklar bilan yuqoriroq chastotali signal yig‘ish mumkin.
Asl CSV va qurilma firmware fayllari ushbu tahlil davomida o‘zgartirilmadi.
