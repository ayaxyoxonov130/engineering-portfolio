# Tekshiruv — 2026-10-26

- ESP32-C3 COM14, MAC [DEVICE-MAC-REMOVED]. ESP32 Dev Module COM4, MAC [DEVICE-MAC-REMOVED].
- Arduino ESP32 core 3.3.10 bilan ikkala dastur yig'ildi, esptool bilan yozildi va flash hash tekshiruvi o'tdi.
- MPU6050 SDA9/SCL8 da ishlaydi. Haqiqiy tezlanish moduli taxminan 0.99 g, qiyalik taxminan 58.8°.
- C3 uzatish quvvati 8.5 dBm ga tushirilgach ESP-NOW qabul ishladi. Yuqori quvvatda qabul bo'lmagan; Wi-Fi sinovida AUTH_EXPIRE chiqdi. Bu kuzatuv antenna nuqsonining isboti emas.
- Yakuniy ESP-NOW sinovi: RX 651 dan 1198 gacha taxminan 5.5 soniyada oshdi, deyarli 100 paket/soniya. Oraliqda ayrim yo'qotishlar bor. Broadcast tasdiqsiz.
- 41 bayt paketlar C3 MAC manzilidan olindi, valid=1. Oxirgi qabuldan beri vaqt 1–9 ms. Bu bir tomonlama radio kechikishi emas.
- Sensor qiyaligi sabab DANGER holati ishladi. LED chiqishlari R25/G26/Y27 uchun sozlangan; ranglarning fizik yonishi alohida ko'z bilan tasdiqlanmagan.
- LCD 0x27 manzilida topildi va foydalanuvchi yozuvni tasdiqladi (xira). Oxirgi restartda 0x00 qaytdi; displey topilmaganda har 2 soniyada qayta qidirish qo'shildi. Ulanish/ta'minot/kontrast tekshiruvi kerak bo'lishi mumkin.
- UDP vaqtinchalik diagnostika uchun ishlatildi va yakuniy koddan olib tashlandi. Asosiy yo'l faqat ESP-NOW.
- Panel JavaScript sintaksisi node --check bilan o'tdi. Foydalanuvchi telefonida Magistr-MPU6050 tarmog`i ko`ringanini va panel ochilganini tasdiqladi. API ma`lumotlari brauzerda alohida kuzatilmagan.
- sensor-runtime.log va gateway-runtime.log apparatdagi oxirgi saqlangan tekshiruvni beradi.

Ogohlantirish chegaralari sinov qiymatlari; magistrlik mavzusi va tajriba mezonlariga moslashtirish kerak.
Oxirgi yuklashdan keyin: LCD=0x27 qayta topildi, RX 968 dan 1219 gacha taxminan 2.5 soniyada oshdi. LCD yangilanishidan keyin chiqarilgan diagnostika snapshot yoshi 60–68 ms bo'ldi; oldingi 1–9 ms o'lchovi LCD aniqlanmagan davrga tegishli. LCD yozuvi asosiy siklda vaqt oladi, radio callback esa shu paytda ham yangi paket qabul qiladi. Yangi apparat logi gateway-runtime.log ichida. Panel telefonda ochilishi foydalanuvchi tomonidan tasdiqlandi.
