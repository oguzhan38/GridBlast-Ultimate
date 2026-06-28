# Fonksiyon Özeti

| İmza | Amaç | Girdi | Çıktı |
|---|---|---|---|
| `void StartBackgroundMusic()` | Belirtilen kaynak kimliğine sahip kısa WAV efektini çalar. | int soundId. | Yok. */ // ============================================================ // BÖLÜM 1: Ses Sistemi // MIDI arka plan müziği ve kısa WAV efektleri burada yönetilir. // ============================================================ void PlayEffect(int soundId) { PlaySound((LPCSTR)soundId, _hInstance, SND_ASYNC | SND_RESOURCE); } /* Amaç: Arka plan MIDI müziğini başlatır ve döngüde çalar. Girdi: Yok. Çıktı: Yok. |
| `void StopBackgroundMusic()` | Arka plan MIDI müziğini durdurur ve MCI kaynağını kapatır. | Yok. | Yok. |
| `void GameStart(HWND hWindow)` | Ders motoru nesnesini oluşturur, pencere boyutunu ayarlar ve frame rate değerini belirler. | HINSTANCE hInstance. | BOOL. */ // ============================================================ // BÖLÜM 2: Oyun Motoru Yaşam Döngüsü // Chapter motorundaki GameInitialize/GameStart/GameCycle/GamePaint // bağlantı noktaları grid-based bomb-maze mantığına burada bağlanır. // ============================================================ BOOL GameInitialize(HINSTANCE hInstance) { _pGame = new GameEngine(hInstance, TEXT("GridBlast Ultimate"), TEXT("GridBlast Ultimate"), IDI_SPACEOUT, IDI_SPACEOUT_SM, MAZE_COLS * TILE_SIZE, MAZE_ROWS * TILE_SIZE + HUD_TOP + HUD_BOTTOM); if (_pGame == NULL) return FALSE; _pGame->SetFrameRate(30); _hInstance = hInstance; return TRUE; } /* Amaç: Offscreen çizim tamponunu, skor tablosunu ve başlangıç menüsünü hazırlar. Girdi: HWND hWindow. Çıktı: Yok. |
| `void GameEnd()` | Müzik, GDI ve motor kaynaklarını serbest bırakır. | Yok. | Yok. |
| `void GameActivate(HWND hWindow)` | Pencere aktif olduğunda çalışacak oyun bağlantı noktasıdır. | HWND hWindow. | Yok. |
| `void GameDeactivate(HWND hWindow)` | Pencere pasif olduğunda çalışacak oyun bağlantı noktasıdır. | HWND hWindow. | Yok. |
| `void GamePaint(HDC hDC)` | Geçerli oyun durumunu offscreen DC üzerine çizer. | HDC hDC. | Yok. |
| `void GameRenderFrame(HWND hWindow)` | Offscreen buffer içeriğini pencereye 16:9 oranını koruyarak aktarır. | HWND hWindow. | Yok. |
| `void GameCycle()` | Bir frame içinde oyun simülasyonunu günceller ve sonucu ekrana çizer. | Yok. | Yok. |
| `void HandleKeys()` | Menü, oyun, bonus ve isim giriş durumları için klavye girişini işler. | Yok. | Yok. |
| `void MouseButtonDown(int x, int y, BOOL bLeft)` | Sol veya sağ fare basma olayını karşılayan boş bağlantı noktasıdır. | int x, int y, BOOL bLeft. | Yok. |
| `void MouseButtonUp(int x, int y, BOOL bLeft)` | Sol veya sağ fare bırakma olayını karşılayan boş bağlantı noktasıdır. | int x, int y, BOOL bLeft. | Yok. |
| `void MouseMove(int x, int y)` | Fare hareketi olayını karşılayan boş bağlantı noktasıdır. | int x, int y. | Yok. |
| `void HandleJoystick(JOYSTATE jsJoystickState)` | Joystick durumunu karşılayan boş bağlantı noktasıdır. | JOYSTATE jsJoystickState. | Yok. |
| `int GetStageInCycle()` | Yaşayan düşman sayısını hesaplar. | Yok. | int. */ // ============================================================ // BÖLÜM 3: Level, Zorluk ve Skor Yardımcıları // 5 normal level + 1 bonus yapısı, süre ve zorluk ölçekleme // fonksiyonları bu bölümde toplanmıştır. // ============================================================ int CountAliveEnemies() { int alive = 0; for (int i = 0; i < _iNumEnemies; i++) if (_enemies[i].alive) alive++; return alive; } /* Amaç: Level değerinden 5 normal + 1 bonus döngüsündeki aşamayı hesaplar. Girdi: Yok. Çıktı: int. |
| `int GetReferenceGridBlastLevel()` | Beş level yapısını classic grid-based bomb-maze zorluk eğrisine göre referans seviyeye eşler. | Yok. | int. |
| `int GetBalancedLevelTimeSeconds()` | Level, düşman ve duvar yoğunluğuna göre adil süreyi saniye cinsinden hesaplar. | Yok. | int. |
| `int GetAStarChanceForEnemy(int enemyType)` | Düşman tipi ve level değerine göre A* takip olasılığını verir. | int enemyType. | int. |
| `int GetAStarRangeForEnemy(int enemyType)` | Düşman tipi ve level değerine göre A* takip menzilini verir. | int enemyType. | int. |
| `BOOL IsBombAt(int col, int row)` | Verilen grid hücresinde aktif bomba olup olmadığını kontrol eder. | int col, int row. | BOOL. |
| `BOOL IsEnemyAtCell(int col, int row)` | Verilen grid hücresinde yaşayan düşman olup olmadığını kontrol eder. | int col, int row. | BOOL. |
| `BOOL IsTripwireLineCell(int col, int row, int wireIndex)` | Verilen hücrenin aktif tripwire hattı üzerinde olup olmadığını kontrol eder. | int col, int row, int wireIndex. | BOOL. |
| `void CheckPlayerPickups()` | Oyuncunun portal, çıkış kapısı ve power-up etkileşimlerini işler. | Yok. | Yok. |
| `void NewGame()` | Oyunu ana menü durumuna alır ve menü geçiş değişkenlerini sıfırlar. | Yok. | Yok. */ // ============================================================ // BÖLÜM 4: Oyun Durumları ve Menü Akışı // Ana menü, test modu, yeni oyun, run complete ve level geçişleri // oyun durum makinesi üzerinden yönetilir. // ============================================================ void GoToMainMenu() { _iGameState = STATE_MENU; _bGameOver = FALSE; _bTestMode = FALSE; if (_iTestStartLevel < 1 || _iTestStartLevel > 6) _iTestStartLevel = 1; _iActionCooldown = 0; } /* Amaç: Normal oyun akışını level 1den başlatır. Girdi: Yok. Çıktı: Yok. |
| `void NewTestGame()` | Test modunda seçilen level veya bonus bölümünü başlatır. | Yok. | Yok. |
| `void CompleteRun()` | Oyunun tamamlandığını işaretler ve isim giriş ekranına geçer. | Yok. | Yok. |
| `void AwardLevelClearBonus()` | Kalan süreye göre level bitirme skor bonusunu ekler. | Yok. | Yok. |
| `void NextLevel()` | Bir sonraki normal veya bonus level için tüm gameplay state değerlerini hazırlar. | Yok. | Yok. |
| `BOOL IsPathLikeCell(int col, int row)` | Harita üretim sürecini başlatır. | Yok. | Yok. */ // ============================================================ // BÖLÜM 5: Harita Üretimi ve Objelerin Yerleşimi // Branch/tree maze üretimi, kırılabilir duvarlar, kapı, power-up, // portal ve düşman yerleşimleri burada yapılır. // ============================================================ void GenerateMaze() { GenerateBranchingMaze(); } /* Amaç: Bir hücrenin yol gibi davranıp davranmadığını döndürür. Girdi: int col, int row. Çıktı: BOOL. |
| `void CarveBranchMaze(int startCol, int startRow)` | Lab mantığına yakın dallanan yol yapısını harita üzerinde açar. | int startCol, int startRow. | Yok. |
| `void OpenExtraBranches()` | Haritada ek bağlantılar açarak koridor sıkışmasını azaltır. | Yok. | Yok. |
| `void GenerateBranchingMaze()` | Başlangıçtan erişilebilir branch/tree tabanlı harita üretir. | Yok. | Yok. |
| `void PlaceBreakableWalls()` | Kırılabilir duvarları ve power-up saklayan duvarları yerleştirir. | Yok. | Yok. |
| `void PlaceExitGate()` | Çıkış kapısını uygun uzaklıktaki rastgele kırılabilir duvarın altına saklar. | Yok. | Yok. |
| `void PlaceHiddenPowerups()` | Power-up içeren özel duvarları belirler. | Yok. | Yok. |
| `void PlacePortals()` | Level içindeki portal çiftlerini uygun yol hücrelerine yerleştirir. | Yok. | Yok. |
| `void PlaceEnemies()` | Level zorluğuna göre düşman tiplerini ve konumlarını üretir. | Yok. | Yok. |
| `BOOL IsGhostTrailCell(int col, int row)` | A* algoritması için düşman tipine göre geçilebilir hücre kontrolü yapar. | int col, int row, int enemyType. | BOOL. */ // ============================================================ // BÖLÜM 6: Düşman AI ve A* Pathfinding // Düşmanların hareket edebileceği hücreler, A* sonraki adım // seçimi ve düşman güncelleme mantığı bu bölümde yer alır. // ============================================================ BOOL IsAStarWalkable(int col, int row, int enemyType) { if (col < 1 || col >= MAZE_COLS - 1 || row < 1 || row >= MAZE_ROWS - 1) return FALSE; if (IsTrailCell(col, row)) return FALSE; if (enemyType != ENEMY_GHOST && IsGhostTrailCell(col, row)) return FALSE; if (enemyType == ENEMY_GHOST) return TRUE; if (_maze[col][row] == CELL_WALL) return FALSE; if (_maze[col][row] == CELL_BREAKABLE && enemyType != ENEMY_SPOOKY) return FALSE; if (IsBombAt(col, row)) return FALSE; return TRUE; } /* Amaç: Ghost düşmanlarının bıraktığı iz hücresini tespit eder. Girdi: int col, int row. Çıktı: BOOL. |
| `BOOL FindNextStepAStar(int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow)` | A* aramasıyla düşmanın hedefe doğru atacağı ilk adımı bulur. | int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow. | BOOL. |
| `void UpdateEnemies()` | Düşman hareketini, A* takibini, runner davranışını ve çarpışmalarını günceller. | Yok. | Yok. |
| `void UpdateGhostPullEffects()` | Ghost izine giren oyuncuyu adım adım ghost yönüne çeker. | Yok. | Yok. |
| `void UpdateBombs()` | Oyuncunun bulunduğu hücreye normal bomba bırakır. | Yok. | Yok. */ // ============================================================ // BÖLÜM 7: Bomba ve Patlama Mekanikleri // Normal bomba, remote bomba, C4, tripwire, patlama yayılımı ve // oyuncu/düşman hasar kontrolü burada yönetilir. // ============================================================ void PlaceBomb() { for (int i = 0; i < _iMaxBombs; i++) if (_bombs[i].active && _bombs[i].col == _iPlayerCol && _bombs[i].row == _iPlayerRow) return; for (int i = 0; i < _iMaxBombs; i++) { if (!_bombs[i].active) { _bombs[i].col = _iPlayerCol; _bombs[i].row = _iPlayerRow; _bombs[i].timer = BOMB_TIMER; _bombs[i].active = TRUE; PlayEffect(IDW_SMEXPLODE); return; } } } /* Amaç: Normal bomba zamanlayıcılarını günceller ve süresi dolanları patlatır. Girdi: Yok. Çıktı: Yok. |
| `void DetonateRemoteBombs()` | Remote özellik açıkken aktif normal bombaları elle patlatır. | Yok. | Yok. |
| `void ThrowStickyBomb()` | Oyuncunun baktığı yöne hareket eden C4 bombasını başlatır. | Yok. | Yok. |
| `void PlaceTripwireAnchor()` | Tripwire için birinci veya ikinci anchor noktasını yerleştirir. | Yok. | Yok. |
| `void UpdateStickyBombs()` | Hareket eden veya yapışmış C4 bombalarının durumunu günceller. | Yok. | Yok. |
| `void UpdateTripwires()` | Aktif tripwire hatlarını kontrol eder ve tetiklenirse patlatır. | Yok. | Yok. |
| `void Explode(int col, int row)` | Bomba patlamasını grid üzerinde yayar, duvarları kırar ve hasar uygular. | int col, int row. | Yok. |
| `void KillEnemyAtCell(int col, int row)` | Belirtilen hücredeki düşmanı öldürür ve skor ekler. | int col, int row. | Yok. |
| `void CheckEnemyExplosionDamage()` | Aktif patlama hücrelerindeki düşmanları her frame kontrol eder. | Yok. | Yok. |
| `void UpdateExplosions()` | Patlama görsellerinin ömrünü azaltır. | Yok. | Yok. |
| `void UpdatePowerUpTimers()` | Süreli power-up etkilerini günceller. | Yok. | Yok. |
| `void CheckPlayerDeath()` | Oyuncunun düşman veya patlama nedeniyle ölmesini kontrol eder. | Yok. | Yok. |
| `void KillPlayer()` | Can, kalkan, invincibility ve game over akışını yönetir. | Yok. | Yok. |
| `void ClearTrail()` | Tron izini açar veya kapatır. | Yok. | Yok. */ // ============================================================ // BÖLÜM 8: Tron Trail Mekaniği // Oyuncunun arkasında bıraktığı 10 hücrelik iz düşmanlar için // engel gibi davranır, oyuncuya zarar vermez. // ============================================================ void ToggleTrail() { _bTrailEnabled = !_bTrailEnabled; ClearTrail(); } /* Amaç: Tron izi belleğini temizler. Girdi: Yok. Çıktı: Yok. |
| `void AddTrailCell(int col, int row)` | Oyuncunun eski konumunu Tron izi listesine ekler. | int col, int row. | Yok. |
| `BOOL IsTrailCell(int col, int row)` | Bir hücrenin aktif Tron izi olup olmadığını döndürür. | int col, int row. | BOOL. |
| `void UpdateTrailEffects()` | Tron izinin düşmanlara engel olarak davranması için gereken etkileri günceller. | Yok. | Yok. |
| `void DrawTrail(HDC hDC)` | Tron izini neon çizgi olarak çizer. | HDC hDC. | Yok. |
| `void GenerateBonusArena()` | Çekiç tabanlı bonus bölümü başlatır. | Yok. | Yok. */ // ============================================================ // BÖLÜM 9: Bonus Hammer Rush // Bonus bölümde bomba yerine çekiç kullanılır; altın kasalar, // normal kasalar ve bonus düşmanları skor hedefi olarak çalışır. // ============================================================ void StartBonusArenaLevel() { _iNumEnemies = 0; _iLevelTime = 100 * 30; _iLevelTimeLeft = _iLevelTime; _iMaxBombs = 0; _iExplosionRange = 0; GenerateBonusArena(); PlaceBonusEnemies(); } /* Amaç: Bonus bölüm kasalarını, hedeflerini ve açık yollarını oluşturur. Girdi: Yok. Çıktı: Yok. |
| `void PlaceBonusEnemies()` | Bonus bölümde çekiçle öldürülebilecek düşmanları yerleştirir. | Yok. | Yok. |
| `void HandleBonusArenaAction()` | Bonus bölümde oyuncu aksiyonunu işler. | Yok. | Yok. |
| `void SwingBonusHammer()` | Çekiç vuruşunu uygular, kasa kırar veya düşman öldürür. | Yok. | Yok. |
| `void ToggleMineFlag()` | ToggleMineFlag fonksiyonunun oyun içindeki ilgili işlemini yürütür. | Yok. | Yok. |
| `int CountAdjacentMines(int col, int row)` | CountAdjacentMines fonksiyonunun oyun içindeki ilgili işlemini yürütür. | int col, int row. | int. |
| `void RevealMineCell(int col, int row)` | RevealMineCell fonksiyonunun oyun içindeki ilgili işlemini yürütür. | int col, int row. | Yok. |
| `void CheckBonusTargetHit(int col, int row)` | CheckBonusTargetHit fonksiyonunun oyun içindeki ilgili işlemini yürütür. | int col, int row. | Yok. |
| `void DrawBonusArena(HDC hDC)` | Bonus bölüm sahnesini çizer. | HDC hDC. | Yok. |
| `void DrawFloorBackground(HDC hDC)` | Düşmanın normal hareket için bir hücreye girip giremeyeceğini kontrol eder. | int col, int row, int enemyType. | BOOL. */ // ============================================================ // BÖLÜM 10: Çizim ve Görsel Katman // Board, maze, oyuncu, düşman, patlama, HUD, menü, skor ve // bonus ekran çizimleri bu bölümden itibaren yapılır. // ============================================================ BOOL IsEnemyWalkable(int col, int row, int enemyType) { if (col < 1 || col >= MAZE_COLS - 1 || row < 1 || row >= MAZE_ROWS - 1) return FALSE; if (IsTrailCell(col, row)) return FALSE; if (enemyType != ENEMY_GHOST && IsGhostTrailCell(col, row)) return FALSE; if (enemyType == ENEMY_GHOST) return TRUE; if (_maze[col][row] == CELL_WALL) return FALSE; if (enemyType != ENEMY_SPOOKY && _maze[col][row] == CELL_BREAKABLE) return FALSE; if (IsBombAt(col, row)) return FALSE; return TRUE; } /* Amaç: Board zeminini ve grid dokusunu çizer. Girdi: HDC hDC. Çıktı: Yok. |
| `void DrawMaze(HDC hDC)` | Sabit duvarları, kırılabilir duvarları ve özel kasaları çizer. | HDC hDC. | Yok. |
| `void DrawExitGate(HDC hDC)` | Gizli çıkış bulunduysa kapı görselini çizer. | HDC hDC. | Yok. |
| `void DrawPortals(HDC hDC)` | Portal çiftlerini çizer. | HDC hDC. | Yok. |
| `void DrawPowerUps(HDC hDC)` | Açık power-up simgelerini çizer. | HDC hDC. | Yok. |
| `void GetPlayerDrawPosition(int* px, int* py)` | Oyuncunun grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar. | int* px, int* py. | Yok. |
| `void GetEnemyDrawPosition(int index, int* px, int* py)` | Düşmanın grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar. | int index, int* px, int* py. | Yok. |
| `void DrawPlayer(HDC hDC)` | Oyuncu karakterini ve aktif aura efektlerini çizer. | HDC hDC. | Yok. |
| `void DrawBombs(HDC hDC)` | Normal bombaları çizer. | HDC hDC. | Yok. |
| `void DrawStickyBombs(HDC hDC)` | C4 bombalarını ve hareket durumlarını çizer. | HDC hDC. | Yok. |
| `void DrawGhostTrails(HDC hDC)` | Ghost puller izlerini çizer. | HDC hDC. | Yok. |
| `void DrawTripwires(HDC hDC)` | Tripwire anchor ve hatlarını çizer. | HDC hDC. | Yok. |
| `void DrawExplosions(HDC hDC)` | Aktif patlama hücrelerini çizer. | HDC hDC. | Yok. |
| `void DrawEnemies(HDC hDC)` | Düşman tiplerini kendi siluet ve renkleriyle çizer. | HDC hDC. | Yok. |
| `void DrawHUD(HDC hDC)` | Skor, süre, level, can ve özellik bilgilerini gösterir. | HDC hDC. | Yok. |
| `static void DrawCenteredTextLine(HDC hDC, int y, const char* text, int height, COLORREF color)` | Tek satır metni yatay merkezli olarak çizer. | HDC hDC, int y, const char* text, int height, COLORREF color. | Yok. |
| `void DrawMainMenu(HDC hDC)` | Ana menüyü çizer. | HDC hDC. | Yok. |
| `void DrawTestMenu(HDC hDC)` | Test level seçim ekranını çizer. | HDC hDC. | Yok. |
| `void DrawCredits(HDC hDC)` | Proje ve ekip bilgilerini çizer. | HDC hDC. | Yok. |
| `void DrawNameEntry(HDC hDC)` | Skor kaydı için isim giriş ekranını çizer. | HDC hDC. | Yok. |
| `void DrawScoreboard(HDC hDC)` | Scoreboard kayıtlarını çizer. | HDC hDC. | Yok. |
| `void HandleNameEntryKeys()` | Menü, test, credits ve scoreboard tuşlarını işler. | Yok. | Yok. */ // ============================================================ // BÖLÜM 11: Menü, İsim Girişi ve Scoreboard Kontrolleri // Menü tuşları, isim yazma, skor yükleme/kaydetme işlemleri // bu final bölümünde toplanmıştır. // ============================================================ void HandleMenuKeys() { if (_iActionCooldown > 0) _iActionCooldown--; if (GetAsyncKeyState(VK_RETURN) < 0) { if (_iGameState == STATE_CREDITS) GoToMainMenu(); else if (_iGameState == STATE_TEST_MENU) NewTestGame(); else NewGame(); _iActionCooldown = 8; return; } if (_iActionCooldown > 0) return; if (_iGameState == STATE_TEST_MENU) { if (GetAsyncKeyState(VK_LEFT) < 0) { _iTestStartLevel--; if (_iTestStartLevel < 1) _iTestStartLevel = 6; _iActionCooldown = 8; return; } if (GetAsyncKeyState(VK_RIGHT) < 0) { _iTestStartLevel++; if (_iTestStartLevel > 6) _iTestStartLevel = 1; _iActionCooldown = 8; return; } if (GetAsyncKeyState(VK_RETURN) < 0) { NewTestGame(); return; } } if (GetAsyncKeyState(VK_ESCAPE) < 0) { if (_iGameState == STATE_MENU) PostQuitMessage(0); else GoToMainMenu(); _iActionCooldown = 10; return; } if (_iGameState == STATE_MENU && GetAsyncKeyState('T') < 0) { if (_iTestStartLevel < 1 || _iTestStartLevel > 6) _iTestStartLevel = 1; _iGameState = STATE_TEST_MENU; _iActionCooldown = 10; return; } if (_iGameState == STATE_MENU && GetAsyncKeyState('S') < 0) { _iGameState = STATE_SCOREBOARD; _iActionCooldown = 10; return; } if (_iGameState == STATE_MENU && GetAsyncKeyState('C') < 0) { _iGameState = STATE_CREDITS; _iActionCooldown = 10; return; } } /* Amaç: Oyuncu adını alır ve skor kaydını tamamlar. Girdi: Yok. Çıktı: Yok. |
| `void SafeCopyPlayerName(char* dest, int destSize, const char* src)` | Skor tablosu için oyuncu adını güvenli şekilde kopyalar. | char* dest, int destSize, const char* src. | Yok. |
| `void LoadScoreboard()` | Scoreboard dosyasını okur. | Yok. | Yok. |
| `void SaveScoreboard()` | Scoreboard dosyasını yazar. | Yok. | Yok. |
| `void AddScoreEntry(const char* name, int score, int level)` | Yeni skor kaydını sıralı top listeye ekler. | const char* name, int score, int level. | Yok. |
