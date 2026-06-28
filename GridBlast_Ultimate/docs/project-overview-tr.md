# Proje Açıklaması - GridBlast Ultimate

GridBlast Ultimate, Win32 tabanlı ders oyun motoru üzerinde geliştirilmiş grid tabanlı bir GridBlast varyasyonudur. Proje, classic grid-based bomb-maze döngüsünü korur: oyuncu labirentte hareket eder, kırılabilir duvarları patlatır, düşmanları yok eder, gizli kapıyı bulur ve bölümü tamamlar. Bunun üzerine Tron izi, Sticky C4, tripwire, portal, Ghost Puller ve çekiçli bonus bölüm gibi ekstra mekanikler eklenmiştir.

## Mimari

- `GameEngine.cpp/.h`: Dersten gelen motor katmanı. Win32 penceresi, mesaj yakalama, frame rate ve input callback çağrılarını yönetir.
- `GridBlastUltimate.cpp/.h`: Oyun mantığı. State machine, level üretimi, düşman AI, bomba/patlama, bonus, çizim ve skor işlemleri burada bulunur.
- `Resource.h` ve `.rc`: İkon ve ses kaynaklarını bağlar.

## Oyun Akışı

1. Menü açılır.
2. Oyuncu normal oyun veya test modunu seçer.
3. Normal oyunda 5 level oynanır.
4. Her levelde maze, duvarlar, düşmanlar, power-up, portal ve gizli kapı yeniden üretilir.
5. Tüm düşmanlar öldürülüp kapıdan geçilince sonraki levele geçilir.
6. 5. levelden sonra Hammer Rush bonus bölümü oynanır.
7. Oyun tamamlanınca isim istenir ve skor tablosuna kaydedilir.

## Kodda Özellikle Bilinmesi Gerekenler

- Oyun grid mantığıyla çalışır; mantıksal pozisyonlar hücre bazlıdır.
- Smooth hareket için çizim pozisyonları animasyon timer ile ara değerlerden hesaplanır.
- Düşman AI deterministik ve rastgele kararların karışımıdır. Hunter/Spooky/Ghost belirli olasılıkla A* kullanır.
- Patlama sistemi aktif explosion cell listesi üretir; oyuncu ve düşman hasarı bu liste üzerinden kontrol edilir.
- Bonus bölüm normal bombadan ayrılmıştır; çekiç vuruşu bitişik hücreye uygulanır.
