# GridBlast Ultimate - Kod Kontrol Sunum Rehberi
Bu dosya kod kontrolünde projeyi sistematik anlatmak için hazırlanmıştır. Kod dosyalarının içinde fonksiyon başlarına **Amaç / Girdi / Çıktı** yorumları eklenmiştir.
## 1. Projenin genel yapısı
- Proje, derste kullanılan Win32 oyun motoru callback yapısını korur. `GameEngine.cpp/.h` pencere, mesaj döngüsü, frame timing ve input yönlendirmesini sağlar.
- Asıl oyun mantığı `GridBlastUltimate.cpp/.h` içindedir. Harita, düşmanlar, bombalar, bonus, skor ve çizim bu dosyada yönetilir.
- Oyun 5 normal level + 1 bonus level yapısındadır. Normal bölümlerde amaç düşmanları yenmek, gizli kapıyı bulmak ve çıkışa ulaşmaktır. Bonus bölümde bomba yerine çekiç kullanılır.
- Harita 36x17 grid ve 32px tile ölçüsüyle 16:9 oranına uygun tutulur. HUD üst ve alt panel olarak oyun alanından ayrılmıştır.

## 2. Kod kontrolünde anlatılacak temel özellikler
- **Branch maze generation:** Harita önce blok yapı olarak hazırlanır, sonra bağlı yollar branch/tree mantığıyla açılır. Bu sayede oyuncunun hareket edebileceği ana path garanti edilir.
- **Kırılabilir duvar ve gizli kapı:** Kapı doğrudan açıkta değildir; kırılabilir bir duvarın altında saklanır. Kapı konumu oyuncuya çok yakın olmayacak aday duvarlar arasından rastgele seçilir.
- **Düşman AI:** Normal, Runner, Hunter, Spooky ve Ghost düşmanları farklı davranır. Hunter/Spooky/Ghost belirli olasılıkla A* kullanır; böylece akıllı ama kaçışsız olmayan takip oluşur.
- **A* pathfinding:** A* fonksiyonu mevcut düşmanın geçebileceği hücrelere göre bir sonraki adımı bulur. Ghost gibi özel düşmanlarda walkable kuralı farklıdır.
- **Bomba sistemi:** Normal bomba, remote detonator, Sticky C4 ve tripwire ayrı mekaniklerdir. Patlama hücreleri oluşturulur, sonra oyuncu/düşman hasarı bu hücrelere göre kontrol edilir.
- **Tron trail:** Oyuncu açıkken arkasında iz bırakır. Bu iz oyuncuyu öldürmez, düşmanlar için engel gibi davranır.
- **Ghost puller:** Ghost düşman duvarlardan geçebilir, arkasında 5 hücrelik iz bırakır. İz bölgesi oyuncunun hareketini kısıtlar ve Ghost için özel tehdit oluşturur.
- **Bonus Hammer Rush:** Bonus bölümde bombalar kapatılır, oyuncu çekiç ile altın kasaları kırar, normal kasalardan skor toplar ve bonus düşmanlarını öldürür.
- **Scoreboard:** Run tamamlandığında isim alınır ve skorlar dosyadan yüklenip dosyaya kaydedilir.

## 3. Fonksiyon grupları
### Motor bağlantısı ve frame döngüsü
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void GameStart(HWND hWindow)` | Ders motoru nesnesini oluşturur, pencere boyutunu ayarlar ve frame rate değerini belirler. |
| `void GameEnd()` | Müzik, GDI ve motor kaynaklarını serbest bırakır. |
| `void GameActivate(HWND hWindow)` | Pencere aktif olduğunda çalışacak oyun bağlantı noktasıdır. |
| `void GameDeactivate(HWND hWindow)` | Pencere pasif olduğunda çalışacak oyun bağlantı noktasıdır. |
| `void GamePaint(HDC hDC)` | Geçerli oyun durumunu offscreen DC üzerine çizer. |
| `void GameRenderFrame(HWND hWindow)` | Offscreen buffer içeriğini pencereye 16:9 oranını koruyarak aktarır. |
| `void GameCycle()` | Bir frame içinde oyun simülasyonunu günceller ve sonucu ekrana çizer. |
| `void HandleKeys()` | Menü, oyun, bonus ve isim giriş durumları için klavye girişini işler. |

### Menü, state ve level akışı
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void NewGame()` | Oyunu ana menü durumuna alır ve menü geçiş değişkenlerini sıfırlar. |
| `void NewTestGame()` | Test modunda seçilen level veya bonus bölümünü başlatır. |
| `void CompleteRun()` | Oyunun tamamlandığını işaretler ve isim giriş ekranına geçer. |
| `void AwardLevelClearBonus()` | Kalan süreye göre level bitirme skor bonusunu ekler. |
| `void NextLevel()` | Bir sonraki normal veya bonus level için tüm gameplay state değerlerini hazırlar. |
| `void DrawMainMenu(HDC hDC)` | Ana menüyü çizer. |
| `void DrawTestMenu(HDC hDC)` | Test level seçim ekranını çizer. |
| `void DrawCredits(HDC hDC)` | Proje ve ekip bilgilerini çizer. |
| `void DrawNameEntry(HDC hDC)` | Skor kaydı için isim giriş ekranını çizer. |
| `void DrawScoreboard(HDC hDC)` | Scoreboard kayıtlarını çizer. |
| `void HandleNameEntryKeys()` | Menü, test, credits ve scoreboard tuşlarını işler. |

### Harita üretimi ve yerleşim
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void GenerateBranchingMaze()` | Başlangıçtan erişilebilir branch/tree tabanlı harita üretir. |
| `void CarveBranchMaze(int startCol, int startRow)` | Lab mantığına yakın dallanan yol yapısını harita üzerinde açar. |
| `void OpenExtraBranches()` | Haritada ek bağlantılar açarak koridor sıkışmasını azaltır. |
| `BOOL IsPathLikeCell(int col, int row)` | Harita üretim sürecini başlatır. |
| `void PlaceBreakableWalls()` | Kırılabilir duvarları ve power-up saklayan duvarları yerleştirir. |
| `void PlaceExitGate()` | Çıkış kapısını uygun uzaklıktaki rastgele kırılabilir duvarın altına saklar. |
| `void PlaceHiddenPowerups()` | Power-up içeren özel duvarları belirler. |
| `void PlacePortals()` | Level içindeki portal çiftlerini uygun yol hücrelerine yerleştirir. |
| `void PlaceEnemies()` | Level zorluğuna göre düşman tiplerini ve konumlarını üretir. |

### Düşman AI ve A*
| Fonksiyon | Ne işe yarar? |
|---|---|
| `BOOL IsGhostTrailCell(int col, int row)` | A* algoritması için düşman tipine göre geçilebilir hücre kontrolü yapar. |
| `BOOL FindNextStepAStar(int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow)` | A* aramasıyla düşmanın hedefe doğru atacağı ilk adımı bulur. |
| `void UpdateEnemies()` | Düşman hareketini, A* takibini, runner davranışını ve çarpışmalarını günceller. |
| `void UpdateGhostPullEffects()` | Ghost izine giren oyuncuyu adım adım ghost yönüne çeker. |
| `int GetAStarChanceForEnemy(int enemyType)` | Düşman tipi ve level değerine göre A* takip olasılığını verir. |
| `int GetAStarRangeForEnemy(int enemyType)` | Düşman tipi ve level değerine göre A* takip menzilini verir. |

### Bomba, C4, tripwire ve patlama
| Fonksiyon | Ne işe yarar? |
|---|---|
| `BOOL IsBombAt(int col, int row)` | Verilen grid hücresinde aktif bomba olup olmadığını kontrol eder. |
| `void UpdateBombs()` | Oyuncunun bulunduğu hücreye normal bomba bırakır. |
| `void DetonateRemoteBombs()` | Remote özellik açıkken aktif normal bombaları elle patlatır. |
| `void ThrowStickyBomb()` | Oyuncunun baktığı yöne hareket eden C4 bombasını başlatır. |
| `void PlaceTripwireAnchor()` | Tripwire için birinci veya ikinci anchor noktasını yerleştirir. |
| `void UpdateStickyBombs()` | Hareket eden veya yapışmış C4 bombalarının durumunu günceller. |
| `void UpdateTripwires()` | Aktif tripwire hatlarını kontrol eder ve tetiklenirse patlatır. |
| `void Explode(int col, int row)` | Bomba patlamasını grid üzerinde yayar, duvarları kırar ve hasar uygular. |
| `void KillEnemyAtCell(int col, int row)` | Belirtilen hücredeki düşmanı öldürür ve skor ekler. |
| `void CheckEnemyExplosionDamage()` | Aktif patlama hücrelerindeki düşmanları her frame kontrol eder. |
| `void UpdateExplosions()` | Patlama görsellerinin ömrünü azaltır. |
| `void CheckPlayerDeath()` | Oyuncunun düşman veya patlama nedeniyle ölmesini kontrol eder. |
| `void KillPlayer()` | Can, kalkan, invincibility ve game over akışını yönetir. |

### Power-up, Tron ve portal sistemi
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void CheckPlayerPickups()` | Oyuncunun portal, çıkış kapısı ve power-up etkileşimlerini işler. |
| `void UpdatePowerUpTimers()` | Süreli power-up etkilerini günceller. |
| `void ClearTrail()` | Tron izini açar veya kapatır. |
| `void AddTrailCell(int col, int row)` | Oyuncunun eski konumunu Tron izi listesine ekler. |
| `BOOL IsTrailCell(int col, int row)` | Bir hücrenin aktif Tron izi olup olmadığını döndürür. |
| `void UpdateTrailEffects()` | Tron izinin düşmanlara engel olarak davranması için gereken etkileri günceller. |
| `void DrawTrail(HDC hDC)` | Tron izini neon çizgi olarak çizer. |
| `void DrawPortals(HDC hDC)` | Portal çiftlerini çizer. |
| `void DrawPowerUps(HDC hDC)` | Açık power-up simgelerini çizer. |

### Bonus Hammer Rush
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void GenerateBonusArena()` | Çekiç tabanlı bonus bölümü başlatır. |
| `void PlaceBonusEnemies()` | Bonus bölümde çekiçle öldürülebilecek düşmanları yerleştirir. |
| `void HandleBonusArenaAction()` | Bonus bölümde oyuncu aksiyonunu işler. |
| `void SwingBonusHammer()` | Çekiç vuruşunu uygular, kasa kırar veya düşman öldürür. |
| `void ToggleMineFlag()` | ToggleMineFlag fonksiyonunun oyun içindeki ilgili işlemini yürütür. |
| `int CountAdjacentMines(int col, int row)` | CountAdjacentMines fonksiyonunun oyun içindeki ilgili işlemini yürütür. |
| `void RevealMineCell(int col, int row)` | RevealMineCell fonksiyonunun oyun içindeki ilgili işlemini yürütür. |
| `void CheckBonusTargetHit(int col, int row)` | CheckBonusTargetHit fonksiyonunun oyun içindeki ilgili işlemini yürütür. |
| `void DrawBonusArena(HDC hDC)` | Bonus bölüm sahnesini çizer. |

### Çizim/HUD
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void DrawFloorBackground(HDC hDC)` | Düşmanın normal hareket için bir hücreye girip giremeyeceğini kontrol eder. |
| `void DrawMaze(HDC hDC)` | Sabit duvarları, kırılabilir duvarları ve özel kasaları çizer. |
| `void DrawExitGate(HDC hDC)` | Gizli çıkış bulunduysa kapı görselini çizer. |
| `void GetPlayerDrawPosition(int* px, int* py)` | Oyuncunun grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar. |
| `void GetEnemyDrawPosition(int index, int* px, int* py)` | Düşmanın grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar. |
| `void DrawPlayer(HDC hDC)` | Oyuncu karakterini ve aktif aura efektlerini çizer. |
| `void DrawBombs(HDC hDC)` | Normal bombaları çizer. |
| `void DrawStickyBombs(HDC hDC)` | C4 bombalarını ve hareket durumlarını çizer. |
| `void DrawGhostTrails(HDC hDC)` | Ghost puller izlerini çizer. |
| `void DrawTripwires(HDC hDC)` | Tripwire anchor ve hatlarını çizer. |
| `void DrawExplosions(HDC hDC)` | Aktif patlama hücrelerini çizer. |
| `void DrawEnemies(HDC hDC)` | Düşman tiplerini kendi siluet ve renkleriyle çizer. |
| `void DrawHUD(HDC hDC)` | Skor, süre, level, can ve özellik bilgilerini gösterir. |
| `static void DrawCenteredTextLine(HDC hDC, int y, const char* text, int height, COLORREF color)` | Tek satır metni yatay merkezli olarak çizer. |

### Skor dosyası
| Fonksiyon | Ne işe yarar? |
|---|---|
| `void SafeCopyPlayerName(char* dest, int destSize, const char* src)` | Skor tablosu için oyuncu adını güvenli şekilde kopyalar. |
| `void LoadScoreboard()` | Scoreboard dosyasını okur. |
| `void SaveScoreboard()` | Scoreboard dosyasını yazar. |
| `void AddScoreEntry(const char* name, int score, int level)` | Yeni skor kaydını sıralı top listeye ekler. |

## 4. Hocaya kısa anlatım sırası
1. Önce `GameEngine.cpp` dosyasının sadece motor katmanı olduğunu söyle: Win32 pencere, mesaj döngüsü, frame zamanı ve callback çağrıları.
2. Asıl oyunun `GridBlastUltimate.cpp` içinde olduğunu belirt: state machine, harita üretimi, düşman AI, bomba sistemi, çizim ve skor.
3. Harita üretimini anlat: 36x17 grid, branch maze, kırılabilir duvar, rastgele gizli kapı ve power-up yerleşimi.
4. Düşman sistemini anlat: tipler, hız/karar gecikmesi, A* olasılığı, Ghost özel izi.
5. Bomba sistemini anlat: normal bomba, remote, C4, tripwire ve patlama hücrelerinin hasar mantığı.
6. Bonus bölümü anlat: bomba değil çekiç; hedef kasalar ve bonus düşmanları skor amaçlıdır.
7. Son olarak skor, isim girişi, scoreboard ve 5+1 level tamamlanma akışını göster.

## 5. Kritik kod kontrol sorularına kısa cevaplar
- **Neden global değişken kullanıldı?** Ders motoru C/Win32 callback yapısıyla çalıştığı için GameInitialize, GameCycle, GamePaint gibi global fonksiyonlar ana giriş noktasıdır. Bu yapı içinde oyun state verileri global tutuldu.
- **A* nerede?** `FindNextStepAStar()` fonksiyonunda. `IsAStarWalkable()` düşman tipine göre geçilebilir hücreleri belirler.
- **Kapı neden her zaman aynı yerde değil?** `PlaceExitGate()` uygun uzaklıktaki kırılabilir duvarları listeleyip rastgele seçim yapar.
- **Bomba öldürme nasıl çalışıyor?** `Explode()` patlama hücreleri üretir. Sonra `CheckPlayerDeath()` ve `CheckEnemyExplosionDamage()` bu aktif hücrelere göre hasarı uygular.
- **Bonus neden ayrı?** Normal oyun bomba tabanlıdır, bonus bölüm ise aynı grid/movement sistemini koruyup çekiçle farklı bir mini-game deneyimi verir.
- **Smooth hareket nasıl yapıldı?** Oyuncu ve düşmanların grid konumu ayrı, çizim konumu ayrı hesaplanır. `prevCol/prevRow` ve animasyon timer ile hücreler arasında ara pozisyon çizilir.
