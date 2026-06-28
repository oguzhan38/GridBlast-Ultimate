

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "GridBlastUltimate.h"
#include <stdio.h>
#include <string.h>
#include <mmsystem.h>

/*
Amaç: Belirtilen kaynak kimliğine sahip kısa WAV efektini çalar.
Girdi: int soundId.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 1: Ses Sistemi
// MIDI arka plan müziği ve kısa WAV efektleri burada yönetilir.
// ============================================================
void PlayEffect(int soundId)
{

  
  PlaySound((LPCSTR)soundId, _hInstance, SND_ASYNC | SND_RESOURCE);
}

/*
Amaç: Arka plan MIDI müziğini başlatır ve döngüde çalar.
Girdi: Yok.
Çıktı: Yok.
*/
void StartBackgroundMusic()
{

  
  mciSendString(TEXT("close bgm"), NULL, 0, NULL);
  if (mciSendString(TEXT("open \"Music.mid\" type sequencer alias bgm"), NULL, 0, NULL) == 0)
    mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
}

/*
Amaç: Arka plan MIDI müziğini durdurur ve MCI kaynağını kapatır.
Girdi: Yok.
Çıktı: Yok.
*/
void StopBackgroundMusic()
{
  mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
  mciSendString(TEXT("close bgm"), NULL, 0, NULL);
}

/*
Amaç: Ders motoru nesnesini oluşturur, pencere boyutunu ayarlar ve frame rate değerini belirler.
Girdi: HINSTANCE hInstance.
Çıktı: BOOL.
*/
// ============================================================
// BÖLÜM 2: Oyun Motoru Yaşam Döngüsü
// Chapter motorundaki GameInitialize/GameStart/GameCycle/GamePaint
// bağlantı noktaları grid-based bomb-maze mantığına burada bağlanır.
// ============================================================
BOOL GameInitialize(HINSTANCE hInstance)
{
  _pGame = new GameEngine(hInstance, TEXT("GridBlast Ultimate"),
    TEXT("GridBlast Ultimate"), IDI_SPACEOUT, IDI_SPACEOUT_SM,
    MAZE_COLS * TILE_SIZE, MAZE_ROWS * TILE_SIZE + HUD_TOP + HUD_BOTTOM);
  if (_pGame == NULL)
    return FALSE;

  _pGame->SetFrameRate(30);
  _hInstance = hInstance;
  return TRUE;
}

/*
Amaç: Offscreen çizim tamponunu, skor tablosunu ve başlangıç menüsünü hazırlar.
Girdi: HWND hWindow.
Çıktı: Yok.
*/
void GameStart(HWND hWindow)
{
  srand((unsigned int)GetTickCount64());

  HDC hWndDC = GetDC(hWindow);
  _hOffscreenDC = CreateCompatibleDC(hWndDC);
  _hOffscreenBitmap = CreateCompatibleBitmap(hWndDC,
    _pGame->GetWidth(), _pGame->GetHeight());
  SelectObject(_hOffscreenDC, _hOffscreenBitmap);

  

  ReleaseDC(hWindow, hWndDC);
  LoadScoreboard();
  GoToMainMenu();
  StartBackgroundMusic();
}

/*
Amaç: Müzik, GDI ve motor kaynaklarını serbest bırakır.
Girdi: Yok.
Çıktı: Yok.
*/
void GameEnd()
{
  StopBackgroundMusic();
  DeleteObject(_hOffscreenBitmap);
  DeleteDC(_hOffscreenDC);
  delete _pGame;
}

/*
Amaç: Pencere aktif olduğunda çalışacak oyun bağlantı noktasıdır.
Girdi: HWND hWindow.
Çıktı: Yok.
*/
void GameActivate(HWND hWindow)
{
}
/*
Amaç: Pencere pasif olduğunda çalışacak oyun bağlantı noktasıdır.
Girdi: HWND hWindow.
Çıktı: Yok.
*/
void GameDeactivate(HWND hWindow)
{
}

/*
Amaç: Geçerli oyun durumunu offscreen DC üzerine çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void GamePaint(HDC hDC)
{
  RECT rect = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH hBrush = CreateSolidBrush(RGB(10, 14, 26));
  FillRect(hDC, &rect, hBrush);
  DeleteObject(hBrush);

  if (_iGameState == STATE_MENU)
  {
    DrawMainMenu(hDC);
    return;
  }
  if (_iGameState == STATE_NAME_ENTRY)
  {
    DrawNameEntry(hDC);
    return;
  }
  if (_iGameState == STATE_SCOREBOARD)
  {
    DrawScoreboard(hDC);
    return;
  }
  if (_iGameState == STATE_CREDITS)
  {
    DrawCredits(hDC);
    return;
  }
  if (_iGameState == STATE_TEST_MENU)
  {
    DrawTestMenu(hDC);
    return;
  }

  POINT oldViewport;
  SetViewportOrgEx(hDC, 0, HUD_TOP, &oldViewport);

  DrawFloorBackground(hDC);
  DrawMaze(hDC);

  if (_bMineBonusLevel)
  {
    DrawBonusArena(hDC);
    DrawEnemies(hDC);
  }
  else
  {
    DrawExitGate(hDC);
    DrawPortals(hDC);
    DrawPowerUps(hDC);
    DrawTrail(hDC);
    DrawGhostTrails(hDC);
    DrawTripwires(hDC);
    DrawExplosions(hDC);
    DrawBombs(hDC);
    DrawStickyBombs(hDC);
    DrawEnemies(hDC);
  }

  DrawPlayer(hDC);
  SetViewportOrgEx(hDC, oldViewport.x, oldViewport.y, NULL);
  DrawHUD(hDC);

}

/*
Amaç: Offscreen buffer içeriğini pencereye 16:9 oranını koruyarak aktarır.
Girdi: HWND hWindow.
Çıktı: Yok.
*/
void GameRenderFrame(HWND hWindow)
{
  if (hWindow == NULL || _hOffscreenDC == NULL)
    return;

  HDC hWndDC = GetDC(hWindow);
  if (hWndDC == NULL)
    return;

  GamePaint(_hOffscreenDC);

  RECT rcClient;
  GetClientRect(hWindow, &rcClient);
  int clientW = rcClient.right - rcClient.left;
  int clientH = rcClient.bottom - rcClient.top;
  int gameW = _pGame->GetWidth();
  int gameH = _pGame->GetHeight();

  if (clientW <= 0 || clientH <= 0 || gameW <= 0 || gameH <= 0)
  {
    ReleaseDC(hWindow, hWndDC);
    return;
  }

  static HDC hPresentDC = NULL;
  static HBITMAP hPresentBitmap = NULL;
  static int presentW = 0;
  static int presentH = 0;

  if (hPresentDC == NULL || hPresentBitmap == NULL ||
      presentW != clientW || presentH != clientH)
  {
    if (hPresentBitmap != NULL)
    {
      DeleteObject(hPresentBitmap);
      hPresentBitmap = NULL;
    }
    if (hPresentDC != NULL)
    {
      DeleteDC(hPresentDC);
      hPresentDC = NULL;
    }

    hPresentDC = CreateCompatibleDC(hWndDC);
    hPresentBitmap = CreateCompatibleBitmap(hWndDC, clientW, clientH);
    SelectObject(hPresentDC, hPresentBitmap);
    presentW = clientW;
    presentH = clientH;
  }

  int drawW = clientW;
  int drawH = clientW * gameH / gameW;
  if (drawH > clientH)
  {
    drawH = clientH;
    drawW = clientH * gameW / gameH;
  }

  int drawX = (clientW - drawW) / 2;
  int drawY = (clientH - drawH) / 2;

  HBRUSH hBlack = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hPresentDC, &rcClient, hBlack);
  DeleteObject(hBlack);

  SetStretchBltMode(hPresentDC, COLORONCOLOR);
  StretchBlt(hPresentDC, drawX, drawY, drawW, drawH,
    _hOffscreenDC, 0, 0, gameW, gameH, SRCCOPY);

  BitBlt(hWndDC, 0, 0, clientW, clientH, hPresentDC, 0, 0, SRCCOPY);
  ReleaseDC(hWindow, hWndDC);
}

/*
Amaç: Bir frame içinde oyun simülasyonunu günceller ve sonucu ekrana çizer.
Girdi: Yok.
Çıktı: Yok.
*/
void GameCycle()
{
  if (_iGameState != STATE_PLAYING)
  {
    GameRenderFrame(_pGame->GetWindow());
    return;
  }

  if (_iActionCooldown > 0)
    _iActionCooldown--;

  if (_iHammerSwingTimer > 0)
    _iHammerSwingTimer--;

  if (_iPlayerAnimTimer > 0)
    _iPlayerAnimTimer--;

  for (int ai = 0; ai < _iNumEnemies; ai++)
    if (_enemies[ai].animTimer > 0)
      _enemies[ai].animTimer--;

  if (!_bGameOver)
  {
    if (_bMineBonusLevel)
    {
      if (_iLevelTimeLeft > 0)
        _iLevelTimeLeft--;
      else
      {
        KillPlayer();
        _iLevelTimeLeft = _iLevelTime;
      }

      UpdateEnemies();
      CheckPlayerDeath();

      if (_iSafeCellsLeft <= 0)
      {
        _iScore += 1200 + max(0, _iLevelTimeLeft / 30) * 15;
        CompleteRun();
      }
    }
    else
    {
      if (_iLevelTimeLeft > 0)
        _iLevelTimeLeft--;
      else
      {
        KillPlayer();
        _iLevelTimeLeft = _iLevelTime;
      }

      UpdateBombs();
      UpdateStickyBombs();
      UpdateTripwires();

      

      CheckPlayerDeath();
      UpdateExplosions();

      UpdateEnemies();

      

      CheckEnemyExplosionDamage();
      UpdateGhostPullEffects();
      UpdateTrailEffects();
      UpdatePowerUpTimers();
      CheckPlayerDeath();
      CheckEnemyExplosionDamage();

      if (_exitGate.revealed && CountAliveEnemies() <= 0 &&
          _iPlayerCol == _exitGate.col && _iPlayerRow == _exitGate.row)
      {
        AwardLevelClearBonus();
        NextLevel();
      }
    }

    if (_iInvincibleTimer > 0)
      _iInvincibleTimer--;
    else
      _bPlayerInvincible = FALSE;

    if (_iMoveDelay > 0)
      _iMoveDelay--;
  }

  GameRenderFrame(_pGame->GetWindow());
}

/*
Amaç: Menü, oyun, bonus ve isim giriş durumları için klavye girişini işler.
Girdi: Yok.
Çıktı: Yok.
*/
void HandleKeys()
{

  
  if (_iGameState == STATE_PLAYING && GetAsyncKeyState(VK_ESCAPE) < 0)
  {
    GoToMainMenu();
    _iActionCooldown = 12;
    return;
  }

  if (_iGameState == STATE_MENU || _iGameState == STATE_SCOREBOARD || _iGameState == STATE_CREDITS || _iGameState == STATE_TEST_MENU)
  {
    HandleMenuKeys();
    return;
  }
  if (_iGameState == STATE_NAME_ENTRY)
  {
    if (GetAsyncKeyState(VK_ESCAPE) < 0)
    {
      GoToMainMenu();
      _iActionCooldown = 12;
      return;
    }
    HandleNameEntryKeys();
    return;
  }
  if (_bGameOver)
  {
    GoToMainMenu();
    return;
  }

  if (_iActionCooldown <= 0)
  {
    if (!_bMineBonusLevel && GetAsyncKeyState('T') < 0)
    {
      ToggleTrail();
      _iActionCooldown = ACTION_COOLDOWN;
    }
    else if (_bMineBonusLevel && GetAsyncKeyState('F') < 0)
    {
      ToggleMineFlag();
      _iActionCooldown = ACTION_COOLDOWN;
    }
    else if (_bMineBonusLevel && GetAsyncKeyState(VK_SPACE) < 0)
    {
      HandleBonusArenaAction();
      _iActionCooldown = ACTION_COOLDOWN;
    }
    else if (!_bMineBonusLevel && _bRemoteUnlocked && GetAsyncKeyState('R') < 0)
    {
      DetonateRemoteBombs();
      PlayEffect(IDW_TMISSILE);
      _iActionCooldown = ACTION_COOLDOWN;
    }
    else if (!_bMineBonusLevel && _bStickyUnlocked && GetAsyncKeyState('C') < 0)
    {
      ThrowStickyBomb();
      PlayEffect(IDW_BMISSILE);
      _iActionCooldown = ACTION_COOLDOWN;
    }
    else if (!_bMineBonusLevel && _bTripwireUnlocked && GetAsyncKeyState('V') < 0)
    {
      PlaceTripwireAnchor();
      PlayEffect(IDW_JMISSILE);
      _iActionCooldown = ACTION_COOLDOWN;
    }
  }

  if (_iMoveDelay > 0)
    return;

  // Grid pozisyonu değişmeden önce mevcut ara geçiş animasyonunun bitmesini bekler.
  // Böylece oyuncu hareketi tile atlama gibi görünmez.
  if (_iPlayerAnimTimer > 0)
    return;

  int newCol = _iPlayerCol;
  int newRow = _iPlayerRow;

  if (GetAsyncKeyState(VK_LEFT) < 0)        { newCol--; _iFacingDX = -1; _iFacingDY = 0; }
  else if (GetAsyncKeyState(VK_RIGHT) < 0)  { newCol++; _iFacingDX = 1; _iFacingDY = 0; }
  else if (GetAsyncKeyState(VK_UP) < 0)     { newRow--; _iFacingDX = 0; _iFacingDY = -1; }
  else if (GetAsyncKeyState(VK_DOWN) < 0)   { newRow++; _iFacingDX = 0; _iFacingDY = 1; }

  if (newCol != _iPlayerCol || newRow != _iPlayerRow)
  {
    BOOL canMove = FALSE;
    if (newCol >= 0 && newCol < MAZE_COLS && newRow >= 0 && newRow < MAZE_ROWS)
    {
      if (_bMineBonusLevel)
      {

        
        canMove = (_maze[newCol][newRow] != CELL_WALL &&
                   _maze[newCol][newRow] != CELL_BREAKABLE &&
                   !IsGhostTrailCell(newCol, newRow));
      }
      else
      {
        canMove = (_maze[newCol][newRow] != CELL_WALL &&
                   ((_maze[newCol][newRow] != CELL_BREAKABLE) || (_iWallPassTimer > 0)) &&
                   !IsBombAt(newCol, newRow) &&
                   !IsGhostTrailCell(newCol, newRow));
      }
    }

    if (canMove)
    {
      int oldCol = _iPlayerCol;
      int oldRow = _iPlayerRow;

      _iPlayerPrevCol = oldCol;
      _iPlayerPrevRow = oldRow;
      _iPlayerCol = newCol;
      _iPlayerRow = newRow;
      _iMoveDelay = (_iSpeedTimer > 0) ? 2 : 3;
      _iPlayerAnimTotal = (_iSpeedTimer > 0) ? 2 : 3;
      _iPlayerAnimTimer = _iPlayerAnimTotal;

      if (_bMineBonusLevel)
      {

        CheckPlayerDeath();
      }
      else
      {
        if (_bTrailEnabled)
          AddTrailCell(oldCol, oldRow);

        CheckPlayerPickups();
      }
    }
  }

  if (!_bMineBonusLevel && GetAsyncKeyState(VK_SPACE) < 0)
    PlaceBomb();
}

/*
Amaç: Sol veya sağ fare basma olayını karşılayan boş bağlantı noktasıdır.
Girdi: int x, int y, BOOL bLeft.
Çıktı: Yok.
*/
void MouseButtonDown(int x, int y, BOOL bLeft)
{
}
/*
Amaç: Sol veya sağ fare bırakma olayını karşılayan boş bağlantı noktasıdır.
Girdi: int x, int y, BOOL bLeft.
Çıktı: Yok.
*/
void MouseButtonUp(int x, int y, BOOL bLeft)
{
}
/*
Amaç: Fare hareketi olayını karşılayan boş bağlantı noktasıdır.
Girdi: int x, int y.
Çıktı: Yok.
*/
void MouseMove(int x, int y)
{
}
/*
Amaç: Joystick durumunu karşılayan boş bağlantı noktasıdır.
Girdi: JOYSTATE jsJoystickState.
Çıktı: Yok.
*/
void HandleJoystick(JOYSTATE jsJoystickState)
{
}

/*
Amaç: Yaşayan düşman sayısını hesaplar.
Girdi: Yok.
Çıktı: int.
*/
// ============================================================
// BÖLÜM 3: Level, Zorluk ve Skor Yardımcıları
// 5 normal level + 1 bonus yapısı, süre ve zorluk ölçekleme
// fonksiyonları bu bölümde toplanmıştır.
// ============================================================
int CountAliveEnemies()
{
  int alive = 0;
  for (int i = 0; i < _iNumEnemies; i++)
    if (_enemies[i].alive)
      alive++;
  return alive;
}

/*
Amaç: Level değerinden 5 normal + 1 bonus döngüsündeki aşamayı hesaplar.
Girdi: Yok.
Çıktı: int.
*/
int GetStageInCycle()
{
  return ((_iLevel - 1) % 6) + 1;
}

/*
Amaç: Beş level yapısını classic grid-based bomb-maze zorluk eğrisine göre referans seviyeye eşler.
Girdi: Yok.
Çıktı: int.
*/
int GetReferenceGridBlastLevel()
{

  

  

  switch (GetStageInCycle())
  {
  case 1: return 1;
  case 2: return 5;
  case 3: return 13;
  case 4: return 20;
  case 5: return 25;
  default: return 25;
  }
}

/*
Amaç: Level, düşman ve duvar yoğunluğuna göre adil süreyi saniye cinsinden hesaplar.
Girdi: Yok.
Çıktı: int.
*/
int GetBalancedLevelTimeSeconds()
{

  
  switch (GetStageInCycle())
  {
  case 1: return 150;
  case 2: return 165;
  case 3: return 185;
  case 4: return 205;
  case 5: return 225;
  default: return 150;
  }
}

/*
Amaç: Düşman tipi ve level değerine göre A* takip olasılığını verir.
Girdi: int enemyType.
Çıktı: int.
*/
int GetAStarChanceForEnemy(int enemyType)
{

  
  int stage = GetStageInCycle();
  if (enemyType == ENEMY_HUNTER)
  {
    int table[6] = { 0, 18, 38, 58, 74, 88 };
    return table[stage];
  }
  if (enemyType == ENEMY_SPOOKY)
  {
    int table[6] = { 0, 0, 22, 48, 70, 84 };
    return table[stage];
  }
  if (enemyType == ENEMY_GHOST)
  {
    int table[6] = { 0, 0, 0, 18, 45, 68 };
    return table[stage];
  }
  return 0;
}

/*
Amaç: Düşman tipi ve level değerine göre A* takip menzilini verir.
Girdi: int enemyType.
Çıktı: int.
*/
int GetAStarRangeForEnemy(int enemyType)
{
  int stage = GetStageInCycle();
  if (enemyType == ENEMY_HUNTER) return 8 + stage * 2;
  if (enemyType == ENEMY_SPOOKY) return 9 + stage * 2;
  if (enemyType == ENEMY_GHOST) return 7 + stage * 2;
  return 0;
}

/*
Amaç: Verilen grid hücresinde aktif bomba olup olmadığını kontrol eder.
Girdi: int col, int row.
Çıktı: BOOL.
*/
BOOL IsBombAt(int col, int row)
{
  for (int i = 0; i < MAX_BOMBS; i++)
    if (_bombs[i].active && _bombs[i].col == col && _bombs[i].row == row)
      return TRUE;
  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
    if (_stickyBombs[i].active && _stickyBombs[i].col == col && _stickyBombs[i].row == row)
      return TRUE;
  return FALSE;
}

/*
Amaç: Verilen grid hücresinde yaşayan düşman olup olmadığını kontrol eder.
Girdi: int col, int row.
Çıktı: BOOL.
*/
BOOL IsEnemyAtCell(int col, int row)
{
  for (int i = 0; i < _iNumEnemies; i++)
    if (_enemies[i].alive && _enemies[i].col == col && _enemies[i].row == row)
      return TRUE;
  return FALSE;
}

/*
Amaç: Verilen hücrenin aktif tripwire hattı üzerinde olup olmadığını kontrol eder.
Girdi: int col, int row, int wireIndex.
Çıktı: BOOL.
*/
BOOL IsTripwireLineCell(int col, int row, int wireIndex)
{
  if (wireIndex < 0 || wireIndex >= MAX_TRIPWIRES || !_tripwires[wireIndex].active || _tripwires[wireIndex].armed < 2)
    return FALSE;

  Tripwire& w = _tripwires[wireIndex];
  if (w.row1 == w.row2 && row == w.row1)
  {
    int a = min(w.col1, w.col2);
    int b = max(w.col1, w.col2);
    return (col >= a && col <= b);
  }
  if (w.col1 == w.col2 && col == w.col1)
  {
    int a = min(w.row1, w.row2);
    int b = max(w.row1, w.row2);
    return (row >= a && row <= b);
  }
  return FALSE;
}

/*
Amaç: Oyuncunun portal, çıkış kapısı ve power-up etkileşimlerini işler.
Girdi: Yok.
Çıktı: Yok.
*/
void CheckPlayerPickups()
{
  int cell = _maze[_iPlayerCol][_iPlayerRow];

  if (cell >= CELL_PORTAL_A && cell <= CELL_PORTAL_C)
  {
    int portalIdx = cell - CELL_PORTAL_A;
    if (portalIdx >= 0 && portalIdx < MAX_PORTAL_PAIRS && _portals[portalIdx].active)
    {
      Portal& p = _portals[portalIdx];
      if (_iPlayerCol == p.col1 && _iPlayerRow == p.row1)
      {
        _iPlayerCol = p.col2;
        _iPlayerRow = p.row2;
      }
      else
      {
        _iPlayerCol = p.col1;
        _iPlayerRow = p.row1;
      }
      _iMoveDelay = 8;
    }
  }

  if (_exitGate.revealed && _iPlayerCol == _exitGate.col && _iPlayerRow == _exitGate.row && CountAliveEnemies() <= 0)
  {
    AwardLevelClearBonus();
    NextLevel();
    return;
  }

  for (int i = 0; i < MAX_POWERUPS; i++)
  {
    if (_powerups[i].active &&
        _powerups[i].col == _iPlayerCol && _powerups[i].row == _iPlayerRow)
    {
      _powerups[i].active = FALSE;
      _iScore += 50;
      PlayEffect(IDW_SMEXPLODE);

      switch (_powerups[i].type)
      {
      case POWERUP_SPEED:
        _iSpeedTimer = 180;
        break;
      case POWERUP_RANGE:
        _iRangeTimer = 360;
        break;
      case POWERUP_SHIELD:
        _bHasShield = TRUE;
        break;
      case POWERUP_BOMB:
        _iMaxBombs = min(_iMaxBombs + 1, MAX_BOMBS);
        break;
      case POWERUP_WALLPASS:
        _iWallPassTimer = 360;
        break;
      }
    }
  }
}

/*
Amaç: Oyunu ana menü durumuna alır ve menü geçiş değişkenlerini sıfırlar.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 4: Oyun Durumları ve Menü Akışı
// Ana menü, test modu, yeni oyun, run complete ve level geçişleri
// oyun durum makinesi üzerinden yönetilir.
// ============================================================
void GoToMainMenu()
{
  _iGameState = STATE_MENU;
  _bGameOver = FALSE;
  _bTestMode = FALSE;
  if (_iTestStartLevel < 1 || _iTestStartLevel > 6)
    _iTestStartLevel = 1;
  _iActionCooldown = 0;
}

/*
Amaç: Normal oyun akışını level 1den başlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void NewGame()
{
  _bTestMode = FALSE;
  _bTestUnlockAll = FALSE;
  _iScore = 0;
  _iNumLives = 3;
  _iLevel = 0;
  _bGameOver = FALSE;
  _bLastRunGameOver = FALSE;
  _iActionCooldown = 10;
  _iGameState = STATE_PLAYING;
  NextLevel();
  GameRenderFrame(_pGame->GetWindow());
}

/*
Amaç: Test modunda seçilen level veya bonus bölümünü başlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void NewTestGame()
{
  _bTestMode = TRUE;
  _bTestUnlockAll = TRUE;
  _iScore = 0;
  _iNumLives = 9;
  if (_iTestStartLevel < 1) _iTestStartLevel = 1;
  if (_iTestStartLevel > 6) _iTestStartLevel = 6;
  _iLevel = _iTestStartLevel - 1;
  _bGameOver = FALSE;
  _bLastRunGameOver = FALSE;
  _iActionCooldown = 10;
  _iGameState = STATE_PLAYING;
  NextLevel();
  GameRenderFrame(_pGame->GetWindow());
}

/*
Amaç: Oyunun tamamlandığını işaretler ve isim giriş ekranına geçer.
Girdi: Yok.
Çıktı: Yok.
*/
void CompleteRun()
{

  

  
  _iGameState = STATE_NAME_ENTRY;
  _bGameOver = FALSE;
  _iActionCooldown = 12;
  lstrcpyA(_szPlayerName, "PLAYER");
  _iNameLength = lstrlenA(_szPlayerName);
  PlayEffect(IDW_SMEXPLODE);
}

/*
Amaç: Kalan süreye göre level bitirme skor bonusunu ekler.
Girdi: Yok.
Çıktı: Yok.
*/
void AwardLevelClearBonus()
{
  int secondsLeft = max(0, _iLevelTimeLeft / 30);
  int stage = GetStageInCycle();

  

  int clearBonus = 300 + stage * 75 + secondsLeft * 10;
  _iScore += clearBonus;
}

/*
Amaç: Bir sonraki normal veya bonus level için tüm gameplay state değerlerini hazırlar.
Girdi: Yok.
Çıktı: Yok.
*/
void NextLevel()
{
  if (_iLevel >= 6)
  {
    CompleteRun();
    return;
  }

  _iLevel++;
  PlayEffect(IDW_MISSILE);

  ClearTrail();
  _bTrailEnabled = FALSE;
  _bMineBonusLevel = (GetStageInCycle() == 6);

  _iPlayerCol = 1;
  _iPlayerRow = 1;
  _iPlayerPrevCol = _iPlayerCol;
  _iPlayerPrevRow = _iPlayerRow;
  _iPlayerAnimTimer = 0;
  _iPlayerAnimTotal = 0;
  _iMoveDelay = 0;
  _iActionCooldown = 0;
  _iFacingDX = 1;
  _iFacingDY = 0;

  
  _iLevelTime = 120 * 30;
  _iLevelTimeLeft = _iLevelTime;
  _bRemoteUnlocked = (_iLevel >= 3) || _bTestUnlockAll;
  _bStickyUnlocked = (_iLevel >= 4) || _bTestUnlockAll;
  _bTripwireUnlocked = (_iLevel >= 5) || _bTestUnlockAll;
  _bPlayerInvincible = TRUE;
  _iInvincibleTimer = _bTestMode ? 120 : 60;
  _bHasShield = _bTestMode ? TRUE : FALSE;
  _iSpeedTimer = _bTestMode ? 300 : 0;
  _iRangeTimer = 0;
  _iWallPassTimer = _bTestMode ? 300 : 0;
  _iGhostPullCooldown = 0;
  _iHammerSwingTimer = 0;
  _iHammerSwingCol = 0;
  _iHammerSwingRow = 0;
  _iHammerSwingDX = 0;
  _iHammerSwingDY = 0;
  _iMaxBombs = 3;
  _iExplosionRange = 2;
  _exitGate.col = 0;
  _exitGate.row = 0;
  _exitGate.revealed = FALSE;
  _iBreakableCount = 0;

  for (int i = 0; i < MAX_BOMBS; i++)
    _bombs[i].active = FALSE;
  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
    _stickyBombs[i].active = FALSE;
  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    _tripwires[i].active = FALSE;
    _tripwires[i].armed = 0;
    _tripwires[i].armDelay = 0;
  }
  for (int i = 0; i < MAX_EXPLOSIONS; i++)
    _explosions[i].active = FALSE;
  for (int i = 0; i < MAX_POWERUPS; i++)
    _powerups[i].active = FALSE;
  for (int i = 0; i <= MAX_POWERUP_TYPES; i++)
    _powerupSpawned[i] = FALSE;
  for (int hx = 0; hx < MAZE_COLS; hx++)
    for (int hy = 0; hy < MAZE_ROWS; hy++)
      _hiddenPowerupType[hx][hy] = POWERUP_NONE;
  for (int i = 0; i < MAX_ENEMIES; i++)
    _enemies[i].alive = FALSE;
  for (int i = 0; i < MAX_PORTAL_PAIRS; i++)
    _portals[i].active = FALSE;

  if (_bMineBonusLevel)
  {
    StartBonusArenaLevel();
    return;
  }

  GenerateMaze();
  PlaceBreakableWalls();
  PlaceExitGate();
  PlaceHiddenPowerups();
  PlacePortals();
  PlaceEnemies();

  

  {
    int seconds = GetBalancedLevelTimeSeconds();
    seconds += CountAliveEnemies() * 4;
    seconds += _iBreakableCount / 6;
    _iLevelTime = seconds * 30;
    _iLevelTimeLeft = _iLevelTime;
  }

  _maze[1][1] = CELL_PATH;
  _maze[2][1] = CELL_PATH;
  _maze[1][2] = CELL_PATH;
  _maze[2][2] = CELL_PATH;
  _maze[1][3] = CELL_PATH;
  _maze[3][1] = CELL_PATH;
}

/*
Amaç: Harita üretim sürecini başlatır.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 5: Harita Üretimi ve Objelerin Yerleşimi
// Branch/tree maze üretimi, kırılabilir duvarlar, kapı, power-up,
// portal ve düşman yerleşimleri burada yapılır.
// ============================================================
void GenerateMaze()
{
  GenerateBranchingMaze();
}

/*
Amaç: Bir hücrenin yol gibi davranıp davranmadığını döndürür.
Girdi: int col, int row.
Çıktı: BOOL.
*/
BOOL IsPathLikeCell(int col, int row)
{
  if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS)
    return FALSE;
  return (_maze[col][row] != CELL_WALL && _maze[col][row] != CELL_BREAKABLE);
}

/*
Amaç: Lab mantığına yakın dallanan yol yapısını harita üzerinde açar.
Girdi: int startCol, int startRow.
Çıktı: Yok.
*/
void CarveBranchMaze(int startCol, int startRow)
{

  

  
  int stackCol[MAZE_COLS * MAZE_ROWS];
  int stackRow[MAZE_COLS * MAZE_ROWS];
  int top = 0;

  _maze[startCol][startRow] = CELL_PATH;
  _visited[startCol][startRow] = TRUE;
  stackCol[top] = startCol;
  stackRow[top] = startRow;
  top++;

  while (top > 0)
  {
    int cx = stackCol[top - 1];
    int cy = stackRow[top - 1];

    int dirs[4][2] = { {2,0}, {-2,0}, {0,2}, {0,-2} };
    for (int i = 3; i > 0; i--)
    {
      int j = rand() % (i + 1);
      int tx = dirs[i][0], ty = dirs[i][1];
      dirs[i][0] = dirs[j][0]; dirs[i][1] = dirs[j][1];
      dirs[j][0] = tx; dirs[j][1] = ty;
    }

    BOOL carved = FALSE;
    for (int d = 0; d < 4; d++)
    {
      int nx = cx + dirs[d][0];
      int ny = cy + dirs[d][1];
      if (nx <= 0 || nx >= MAZE_COLS - 1 || ny <= 0 || ny >= MAZE_ROWS - 1)
        continue;
      if (_visited[nx][ny])
        continue;

      _maze[cx + dirs[d][0] / 2][cy + dirs[d][1] / 2] = CELL_PATH;
      _maze[nx][ny] = CELL_PATH;
      _visited[nx][ny] = TRUE;
      stackCol[top] = nx;
      stackRow[top] = ny;
      top++;
      carved = TRUE;
      break;
    }

    if (!carved)
      top--;
  }
}

/*
Amaç: Haritada ek bağlantılar açarak koridor sıkışmasını azaltır.
Girdi: Yok.
Çıktı: Yok.
*/
void OpenExtraBranches()
{

  

  int openings = 24 + min(_iLevel * 2, 14);
  for (int k = 0; k < openings; k++)
  {
    int x = 1 + rand() % (MAZE_COLS - 2);
    int y = 1 + rand() % (MAZE_ROWS - 2);
    if (_maze[x][y] != CELL_WALL)
      continue;

    BOOL horizontalBridge = (x > 1 && x < MAZE_COLS - 2 && IsPathLikeCell(x - 1, y) && IsPathLikeCell(x + 1, y));
    BOOL verticalBridge   = (y > 1 && y < MAZE_ROWS - 2 && IsPathLikeCell(x, y - 1) && IsPathLikeCell(x, y + 1));
    if (horizontalBridge || verticalBridge || (rand() % 100) < 30)
      _maze[x][y] = CELL_PATH;
  }
}

/*
Amaç: Başlangıçtan erişilebilir branch/tree tabanlı harita üretir.
Girdi: Yok.
Çıktı: Yok.
*/
void GenerateBranchingMaze()
{
  for (int x = 0; x < MAZE_COLS; x++)
    for (int y = 0; y < MAZE_ROWS; y++)
    {
      _maze[x][y] = CELL_WALL;
      _visited[x][y] = FALSE;
    }

  CarveBranchMaze(1, 1);
  OpenExtraBranches();

  for (int x = 1; x <= 4 && x < MAZE_COLS - 1; x++)
    for (int y = 1; y <= 4 && y < MAZE_ROWS - 1; y++)
      _maze[x][y] = CELL_PATH;
}

/*
Amaç: Kırılabilir duvarları ve power-up saklayan duvarları yerleştirir.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceBreakableWalls()
{
  _iBreakableCount = 0;

  

  int candidates[MAZE_COLS * MAZE_ROWS][2];
  int candidateCount = 0;
  for (int x = 1; x < MAZE_COLS - 1; x++)
    for (int y = 1; y < MAZE_ROWS - 1; y++)
    {
      if (_maze[x][y] != CELL_WALL)
        continue;
      if (x <= 4 && y <= 4)
        continue;

      int pathNeighbors = 0;
      if (IsPathLikeCell(x - 1, y)) pathNeighbors++;
      if (IsPathLikeCell(x + 1, y)) pathNeighbors++;
      if (IsPathLikeCell(x, y - 1)) pathNeighbors++;
      if (IsPathLikeCell(x, y + 1)) pathNeighbors++;
      if (pathNeighbors >= 1)
      {
        candidates[candidateCount][0] = x;
        candidates[candidateCount][1] = y;
        candidateCount++;
      }
    }

  for (int i = candidateCount - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);
    int tx = candidates[i][0], ty = candidates[i][1];
    candidates[i][0] = candidates[j][0]; candidates[i][1] = candidates[j][1];
    candidates[j][0] = tx; candidates[j][1] = ty;
  }

  int target = min(candidateCount * 32 / 100, 46);
  for (int i = 0; i < candidateCount && _iBreakableCount < target; i++)
  {
    int x = candidates[i][0];
    int y = candidates[i][1];
    _maze[x][y] = CELL_BREAKABLE;
    _iBreakableCount++;
  }
}

/*
Amaç: Çıkış kapısını uygun uzaklıktaki rastgele kırılabilir duvarın altına saklar.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceExitGate()
{

  

  

  int candidates[MAZE_COLS * MAZE_ROWS][2];
  int candidateCount = 0;
  int minimumDistance = (MAZE_COLS + MAZE_ROWS) / 4;

  for (int x = 1; x < MAZE_COLS - 1; x++)
    for (int y = 1; y < MAZE_ROWS - 1; y++)
    {
      if (_maze[x][y] != CELL_BREAKABLE)
        continue;
      if (abs(x - 1) + abs(y - 1) < minimumDistance)
        continue;

      
      int openNeighbors = 0;
      if (IsPathLikeCell(x - 1, y)) openNeighbors++;
      if (IsPathLikeCell(x + 1, y)) openNeighbors++;
      if (IsPathLikeCell(x, y - 1)) openNeighbors++;
      if (IsPathLikeCell(x, y + 1)) openNeighbors++;
      if (openNeighbors <= 0)
        continue;

      candidates[candidateCount][0] = x;
      candidates[candidateCount][1] = y;
      candidateCount++;
    }

  
  if (candidateCount == 0)
  {
    for (int x = 1; x < MAZE_COLS - 1; x++)
      for (int y = 1; y < MAZE_ROWS - 1; y++)
        if (_maze[x][y] == CELL_BREAKABLE && abs(x - 1) + abs(y - 1) >= 7)
        {
          candidates[candidateCount][0] = x;
          candidates[candidateCount][1] = y;
          candidateCount++;
        }
  }

  if (candidateCount > 0)
  {
    int idx = rand() % candidateCount;
    _exitGate.col = candidates[idx][0];
    _exitGate.row = candidates[idx][1];
  }
  else
  {

    
    int placed = FALSE;
    for (int tries = 0; tries < 500 && !placed; tries++)
    {
      int x = 2 + rand() % (MAZE_COLS - 4);
      int y = 2 + rand() % (MAZE_ROWS - 4);
      if (_maze[x][y] == CELL_PATH && abs(x - 1) + abs(y - 1) >= 7)
      {
        _maze[x][y] = CELL_BREAKABLE;
        _iBreakableCount++;
        _exitGate.col = x;
        _exitGate.row = y;
        placed = TRUE;
      }
    }
    if (!placed)
    {
      _exitGate.col = MAZE_COLS / 2;
      _exitGate.row = MAZE_ROWS / 2;
    }
  }

  _exitGate.revealed = FALSE;
}

/*
Amaç: Power-up içeren özel duvarları belirler.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceHiddenPowerups()
{

  
  int types[3] = { POWERUP_SPEED, POWERUP_SHIELD, POWERUP_WALLPASS };

  for (int t = 0; t < 3; t++)
  {
    int chosenX = -1, chosenY = -1;
    int tries = 0;

    do {
      int x = 1 + rand() % (MAZE_COLS - 2);
      int y = 1 + rand() % (MAZE_ROWS - 2);
      tries++;

      if (_maze[x][y] == CELL_BREAKABLE &&
          _hiddenPowerupType[x][y] == POWERUP_NONE &&
          !(x == _exitGate.col && y == _exitGate.row) &&
          abs(x - 1) + abs(y - 1) >= 5)
      {
        chosenX = x;
        chosenY = y;
      }
    } while (tries < 800 && chosenX < 0);

    if (chosenX < 0)
    {
      for (int x = 1; x < MAZE_COLS - 1 && chosenX < 0; x++)
        for (int y = 1; y < MAZE_ROWS - 1 && chosenX < 0; y++)
          if (_maze[x][y] == CELL_BREAKABLE &&
              _hiddenPowerupType[x][y] == POWERUP_NONE &&
              !(x == _exitGate.col && y == _exitGate.row))
          {
            chosenX = x;
            chosenY = y;
          }
    }

    if (chosenX >= 0)
    {
      _hiddenPowerupType[chosenX][chosenY] = types[t];
      _powerupSpawned[types[t]] = TRUE;
    }
  }
}

/*
Amaç: Level içindeki portal çiftlerini uygun yol hücrelerine yerleştirir.
Girdi: Yok.
Çıktı: Yok.
*/
void PlacePortals()
{
  int numPairs = min(1 + _iLevel / 4, MAX_PORTAL_PAIRS);
  for (int i = 0; i < MAX_PORTAL_PAIRS; i++)
    _portals[i].active = FALSE;

  for (int i = 0; i < numPairs; i++)
  {
    int c1 = 0, r1 = 0, c2 = 0, r2 = 0;
    int tries = 0;

    do {
      c1 = 1 + rand() % (MAZE_COLS - 2);
      r1 = 1 + rand() % (MAZE_ROWS - 2);
      tries++;
    } while (tries < 400 &&
            (_maze[c1][r1] != CELL_PATH || IsBombAt(c1, r1) || abs(c1 - 1) + abs(r1 - 1) < 6 ||
             (c1 == _exitGate.col && r1 == _exitGate.row)));

    tries = 0;
    do {
      c2 = 1 + rand() % (MAZE_COLS - 2);
      r2 = 1 + rand() % (MAZE_ROWS - 2);
      tries++;
    } while (tries < 400 &&
            (_maze[c2][r2] != CELL_PATH || abs(c2 - c1) + abs(r2 - r1) < 10 ||
             (c2 == _exitGate.col && r2 == _exitGate.row)));

    if (_maze[c1][r1] == CELL_PATH && _maze[c2][r2] == CELL_PATH)
    {
      int cellType = CELL_PORTAL_A + i;
      _maze[c1][r1] = cellType;
      _maze[c2][r2] = cellType;
      _portals[i].col1 = c1; _portals[i].row1 = r1;
      _portals[i].col2 = c2; _portals[i].row2 = r2;
      _portals[i].cellType = cellType;
      _portals[i].active = TRUE;
    }
  }
}

/*
Amaç: Level zorluğuna göre düşman tiplerini ve konumlarını üretir.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceEnemies()
{

  

  

  int stage = GetStageInCycle();
  int typeSlots[MAX_ENEMIES];
  int targetEnemies = 0;

  switch (stage)
  {
  case 1:

    targetEnemies = 5;
    typeSlots[0] = ENEMY_NORMAL; typeSlots[1] = ENEMY_NORMAL;
    typeSlots[2] = ENEMY_RUNNER; typeSlots[3] = ENEMY_RUNNER;
    typeSlots[4] = ENEMY_HUNTER;
    break;
  case 2:

    targetEnemies = 6;
    typeSlots[0] = ENEMY_NORMAL; typeSlots[1] = ENEMY_RUNNER;
    typeSlots[2] = ENEMY_RUNNER; typeSlots[3] = ENEMY_HUNTER;
    typeSlots[4] = ENEMY_HUNTER; typeSlots[5] = ENEMY_SPOOKY;
    break;
  case 3:

    targetEnemies = 6;
    typeSlots[0] = ENEMY_NORMAL; typeSlots[1] = ENEMY_RUNNER;
    typeSlots[2] = ENEMY_RUNNER; typeSlots[3] = ENEMY_HUNTER;
    typeSlots[4] = ENEMY_HUNTER; typeSlots[5] = ENEMY_SPOOKY;
    break;
  case 4:

    targetEnemies = 8;
    typeSlots[0] = ENEMY_RUNNER; typeSlots[1] = ENEMY_RUNNER;
    typeSlots[2] = ENEMY_HUNTER; typeSlots[3] = ENEMY_HUNTER;
    typeSlots[4] = ENEMY_HUNTER; typeSlots[5] = ENEMY_SPOOKY;
    typeSlots[6] = ENEMY_SPOOKY; typeSlots[7] = ENEMY_GHOST;
    break;
  default:

    

    targetEnemies = 12;
    typeSlots[0] = ENEMY_RUNNER; typeSlots[1] = ENEMY_RUNNER;
    typeSlots[2] = ENEMY_HUNTER; typeSlots[3] = ENEMY_HUNTER;
    typeSlots[4] = ENEMY_HUNTER; typeSlots[5] = ENEMY_SPOOKY;
    typeSlots[6] = ENEMY_SPOOKY; typeSlots[7] = ENEMY_SPOOKY;
    typeSlots[8] = ENEMY_GHOST; typeSlots[9] = ENEMY_GHOST;
    typeSlots[10] = ENEMY_HUNTER; typeSlots[11] = ENEMY_GHOST;
    break;
  }

  if (_bTestMode)
    targetEnemies = min(targetEnemies + 1, MAX_ENEMIES);

  _iNumEnemies = min(targetEnemies, MAX_ENEMIES);
  for (int i = 0; i < MAX_ENEMIES; i++)
    _enemies[i].alive = FALSE;

  int placed = 0;
  for (int i = 0; i < _iNumEnemies; i++)
  {
    int ex = 1, ey = 1, tries = 0;
    BOOL foundSpawn = FALSE;

    do {
      ex = 1 + rand() % (MAZE_COLS - 2);
      ey = 1 + rand() % (MAZE_ROWS - 2);
      tries++;

      int openNeighbors = 0;
      if (_maze[ex - 1][ey] != CELL_WALL && _maze[ex - 1][ey] != CELL_BREAKABLE) openNeighbors++;
      if (_maze[ex + 1][ey] != CELL_WALL && _maze[ex + 1][ey] != CELL_BREAKABLE) openNeighbors++;
      if (_maze[ex][ey - 1] != CELL_WALL && _maze[ex][ey - 1] != CELL_BREAKABLE) openNeighbors++;
      if (_maze[ex][ey + 1] != CELL_WALL && _maze[ex][ey + 1] != CELL_BREAKABLE) openNeighbors++;

      BOOL farFromOtherEnemies = TRUE;
      for (int e = 0; e < placed; e++)
        if (_enemies[e].alive && abs(_enemies[e].col - ex) + abs(_enemies[e].row - ey) < 4)
          farFromOtherEnemies = FALSE;

      foundSpawn = (_maze[ex][ey] != CELL_WALL &&
                    _maze[ex][ey] != CELL_BREAKABLE &&
                    _maze[ex][ey] != CELL_EXIT &&
                    !IsBombAt(ex, ey) &&
                    openNeighbors >= 2 &&
                    farFromOtherEnemies &&
                    (abs(ex - 1) + abs(ey - 1) >= 8));
    } while (tries < 900 && !foundSpawn);

    
    if (!foundSpawn)
    {
      for (int x = 1; x < MAZE_COLS - 1 && !foundSpawn; x++)
        for (int y = 1; y < MAZE_ROWS - 1 && !foundSpawn; y++)
          if (_maze[x][y] != CELL_WALL && _maze[x][y] != CELL_BREAKABLE &&
              abs(x - 1) + abs(y - 1) >= 8)
          {
            ex = x; ey = y; foundSpawn = TRUE;
          }
    }

    if (!foundSpawn)
      continue;

    int enemyType;
    if (_bTestMode && i == targetEnemies - 1)
      enemyType = ENEMY_GHOST;
    else
      enemyType = typeSlots[min(i, targetEnemies - 1)];

    _enemies[placed].col = ex;
    _enemies[placed].row = ey;
    _enemies[placed].type = enemyType;
    _enemies[placed].level = GetReferenceGridBlastLevel();

    

    int baseDelay = 10;
    if (enemyType == ENEMY_NORMAL) baseDelay = 11;
    else if (enemyType == ENEMY_RUNNER) baseDelay = 7;
    else if (enemyType == ENEMY_HUNTER) baseDelay = 8;
    else if (enemyType == ENEMY_SPOOKY) baseDelay = 7;
    else if (enemyType == ENEMY_GHOST) baseDelay = 9;

    if (stage >= 3) baseDelay -= 1;
    if (stage >= 4) baseDelay -= 1;
    if (stage >= 5) baseDelay -= 1;
    _enemies[placed].moveDelay = max((stage >= 5) ? 3 : 4, baseDelay);
    _enemies[placed].moveTimer = rand() % max(1, _enemies[placed].moveDelay);
    _enemies[placed].prevCol = ex;
    _enemies[placed].prevRow = ey;
    _enemies[placed].animTimer = 0;
    _enemies[placed].animTotal = 0;

    int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int d = rand() % 4;
    _enemies[placed].dirX = dirs[d][0];
    _enemies[placed].dirY = dirs[d][1];

    for (int gt = 0; gt < GHOST_TRAIL_LENGTH; gt++)
    {
      _enemies[placed].ghostTrailCol[gt] = ex;
      _enemies[placed].ghostTrailRow[gt] = ey;
    }
    _enemies[placed].trailCount = 0;
    _enemies[placed].alive = TRUE;
    placed++;
  }

  _iNumEnemies = placed;
}

/*
Amaç: A* algoritması için düşman tipine göre geçilebilir hücre kontrolü yapar.
Girdi: int col, int row, int enemyType.
Çıktı: BOOL.
*/
// ============================================================
// BÖLÜM 6: Düşman AI ve A* Pathfinding
// Düşmanların hareket edebileceği hücreler, A* sonraki adım
// seçimi ve düşman güncelleme mantığı bu bölümde yer alır.
// ============================================================
BOOL IsAStarWalkable(int col, int row, int enemyType)
{
  if (col < 1 || col >= MAZE_COLS - 1 || row < 1 || row >= MAZE_ROWS - 1)
    return FALSE;

  if (IsTrailCell(col, row))
    return FALSE;

  if (enemyType != ENEMY_GHOST && IsGhostTrailCell(col, row))
    return FALSE;

  if (enemyType == ENEMY_GHOST)
    return TRUE;

  if (_maze[col][row] == CELL_WALL)
    return FALSE;

  if (_maze[col][row] == CELL_BREAKABLE && enemyType != ENEMY_SPOOKY)
    return FALSE;

  if (IsBombAt(col, row))
    return FALSE;

  return TRUE;
}

/*
Amaç: Ghost düşmanlarının bıraktığı iz hücresini tespit eder.
Girdi: int col, int row.
Çıktı: BOOL.
*/
BOOL IsGhostTrailCell(int col, int row)
{
  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive || _enemies[i].type != ENEMY_GHOST)
      continue;

    for (int t = 0; t < _enemies[i].trailCount && t < GHOST_TRAIL_LENGTH; t++)
      if (_enemies[i].ghostTrailCol[t] == col && _enemies[i].ghostTrailRow[t] == row)
        return TRUE;
  }
  return FALSE;
}

/*
Amaç: A* aramasıyla düşmanın hedefe doğru atacağı ilk adımı bulur.
Girdi: int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow.
Çıktı: BOOL.
*/
BOOL FindNextStepAStar(int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow)
{

  

  
  int gCost[MAZE_COLS][MAZE_ROWS];
  int hCost[MAZE_COLS][MAZE_ROWS];
  int parentCol[MAZE_COLS][MAZE_ROWS];
  int parentRow[MAZE_COLS][MAZE_ROWS];
  BOOL opened[MAZE_COLS][MAZE_ROWS];
  BOOL closed[MAZE_COLS][MAZE_ROWS];

  for (int x = 0; x < MAZE_COLS; x++)
    for (int y = 0; y < MAZE_ROWS; y++)
    {
      gCost[x][y] = 99999;
      hCost[x][y] = abs(x - targetCol) + abs(y - targetRow);
      parentCol[x][y] = -1;
      parentRow[x][y] = -1;
      opened[x][y] = FALSE;
      closed[x][y] = FALSE;
    }

  gCost[startCol][startRow] = 0;
  opened[startCol][startRow] = TRUE;

  for (int iteration = 0; iteration < MAZE_COLS * MAZE_ROWS; iteration++)
  {
    int bestCol = -1, bestRow = -1, bestF = 999999, bestH = 999999;
    for (int x = 1; x < MAZE_COLS - 1; x++)
      for (int y = 1; y < MAZE_ROWS - 1; y++)
        if (opened[x][y] && !closed[x][y])
        {
          int f = gCost[x][y] + hCost[x][y];
          if (f < bestF || (f == bestF && hCost[x][y] < bestH))
          {
            bestF = f;
            bestH = hCost[x][y];
            bestCol = x;
            bestRow = y;
          }
        }

    if (bestCol < 0)
      break;

    if (bestCol == targetCol && bestRow == targetRow)
    {
      int cx = targetCol;
      int cy = targetRow;
      int px = parentCol[cx][cy];
      int py = parentRow[cx][cy];

      while (px != -1 && !(px == startCol && py == startRow))
      {
        cx = px;
        cy = py;
        px = parentCol[cx][cy];
        py = parentRow[cx][cy];
      }

      if (cx == startCol && cy == startRow)
        return FALSE;

      *nextCol = cx;
      *nextRow = cy;
      return TRUE;
    }

    closed[bestCol][bestRow] = TRUE;

    int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    for (int d = 0; d < 4; d++)
    {
      int nx = bestCol + dirs[d][0];
      int ny = bestRow + dirs[d][1];

      if (!(nx == targetCol && ny == targetRow) && !IsAStarWalkable(nx, ny, enemyType))
        continue;
      if (closed[nx][ny])
        continue;

      int tentativeG = gCost[bestCol][bestRow] + 1;
      if (!opened[nx][ny] || tentativeG < gCost[nx][ny])
      {
        opened[nx][ny] = TRUE;
        gCost[nx][ny] = tentativeG;
        parentCol[nx][ny] = bestCol;
        parentRow[nx][ny] = bestRow;
      }
    }
  }

  return FALSE;
}

/*
Amaç: Düşman hareketini, A* takibini, runner davranışını ve çarpışmalarını günceller.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateEnemies()
{
  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive)
      continue;

    _enemies[i].moveTimer++;
    if (_enemies[i].moveTimer < _enemies[i].moveDelay)
      continue;
    _enemies[i].moveTimer = 0;

    int nx = _enemies[i].col;
    int ny = _enemies[i].row;
    int diffX = _iPlayerCol - _enemies[i].col;
    int diffY = _iPlayerRow - _enemies[i].row;
    int dist = abs(diffX) + abs(diffY);
    BOOL usedAStar = FALSE;

    

    

    
    if (_enemies[i].type == ENEMY_GHOST ||
        _enemies[i].type == ENEMY_SPOOKY ||
        _enemies[i].type == ENEMY_HUNTER)
    {
      int chaseChance = GetAStarChanceForEnemy(_enemies[i].type);
      int chaseRange = GetAStarRangeForEnemy(_enemies[i].type);
      usedAStar = (dist <= chaseRange && (rand() % 100) < chaseChance &&
                   FindNextStepAStar(_enemies[i].col, _enemies[i].row,
                     _iPlayerCol, _iPlayerRow, _enemies[i].type, &nx, &ny));
    }

    if (!usedAStar)
    {
      int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

      if (_enemies[i].type == ENEMY_RUNNER)
      {

        

        int tx = _enemies[i].col + _enemies[i].dirX;
        int ty = _enemies[i].row + _enemies[i].dirY;
        if (IsEnemyWalkable(tx, ty, _enemies[i].type))
        {
          nx = tx; ny = ty;
        }
        else
        {
          int start = rand() % 4;
          for (int d = 0; d < 4; d++)
          {
            int idx = (start + d) % 4;
            tx = _enemies[i].col + dirs[idx][0];
            ty = _enemies[i].row + dirs[idx][1];
            if (IsEnemyWalkable(tx, ty, _enemies[i].type))
            {
              _enemies[i].dirX = dirs[idx][0];
              _enemies[i].dirY = dirs[idx][1];
              nx = tx; ny = ty;
              break;
            }
          }
        }
      }
      else
      {

        
        int start = rand() % 4;
        for (int d = 0; d < 4; d++)
        {
          int idx = (start + d) % 4;
          int tx = _enemies[i].col + dirs[idx][0];
          int ty = _enemies[i].row + dirs[idx][1];
          if (IsEnemyWalkable(tx, ty, _enemies[i].type))
          {
            _enemies[i].dirX = dirs[idx][0];
            _enemies[i].dirY = dirs[idx][1];
            nx = tx; ny = ty;
            break;
          }
        }
      }
    }

    if (IsEnemyWalkable(nx, ny, _enemies[i].type))
    {
      if (_enemies[i].type == ENEMY_GHOST && (nx != _enemies[i].col || ny != _enemies[i].row))
      {
        for (int gt = GHOST_TRAIL_LENGTH - 1; gt > 0; gt--)
        {
          _enemies[i].ghostTrailCol[gt] = _enemies[i].ghostTrailCol[gt - 1];
          _enemies[i].ghostTrailRow[gt] = _enemies[i].ghostTrailRow[gt - 1];
        }
        _enemies[i].ghostTrailCol[0] = _enemies[i].col;
        _enemies[i].ghostTrailRow[0] = _enemies[i].row;
        if (_enemies[i].trailCount < GHOST_TRAIL_LENGTH)
          _enemies[i].trailCount++;
      }
      if (nx != _enemies[i].col || ny != _enemies[i].row)
      {
        _enemies[i].prevCol = _enemies[i].col;
        _enemies[i].prevRow = _enemies[i].row;
        _enemies[i].animTotal = min(max(2, _enemies[i].moveDelay - 1), 5);
        _enemies[i].animTimer = _enemies[i].animTotal;
      }
      _enemies[i].col = nx;
      _enemies[i].row = ny;
    }
  }
}

/*
Amaç: Ghost izine giren oyuncuyu adım adım ghost yönüne çeker.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateGhostPullEffects()
{
  if (_bPlayerInvincible || _bMineBonusLevel)
    return;

  if (_iGhostPullCooldown > 0)
  {
    _iGhostPullCooldown--;
    return;
  }

  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive || _enemies[i].type != ENEMY_GHOST)
      continue;

    BOOL insideTrail = FALSE;
    for (int t = 0; t < _enemies[i].trailCount && t < GHOST_TRAIL_LENGTH; t++)
    {
      if (_iPlayerCol == _enemies[i].ghostTrailCol[t] && _iPlayerRow == _enemies[i].ghostTrailRow[t])
      {
        insideTrail = TRUE;
        break;
      }
    }

    if (!insideTrail)
      continue;

    

    int dx = 0, dy = 0;
    int diffX = _enemies[i].col - _iPlayerCol;
    int diffY = _enemies[i].row - _iPlayerRow;
    if (abs(diffX) >= abs(diffY) && diffX != 0)
      dx = (diffX > 0) ? 1 : -1;
    else if (diffY != 0)
      dy = (diffY > 0) ? 1 : -1;

    int nx = _iPlayerCol + dx;
    int ny = _iPlayerRow + dy;
    if (nx >= 1 && nx < MAZE_COLS - 1 && ny >= 1 && ny < MAZE_ROWS - 1 &&
        _maze[nx][ny] != CELL_WALL && _maze[nx][ny] != CELL_BREAKABLE && !IsBombAt(nx, ny))
    {
      _iPlayerCol = nx;
      _iPlayerRow = ny;
      _iMoveDelay = max(_iMoveDelay, 5);
      _iGhostPullCooldown = 10;
      PlayEffect(IDW_TMISSILE);
      CheckPlayerDeath();
    }
    return;
  }
}

/*
Amaç: Oyuncunun bulunduğu hücreye normal bomba bırakır.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 7: Bomba ve Patlama Mekanikleri
// Normal bomba, remote bomba, C4, tripwire, patlama yayılımı ve
// oyuncu/düşman hasar kontrolü burada yönetilir.
// ============================================================
void PlaceBomb()
{
  for (int i = 0; i < _iMaxBombs; i++)
    if (_bombs[i].active && _bombs[i].col == _iPlayerCol && _bombs[i].row == _iPlayerRow)
      return;

  for (int i = 0; i < _iMaxBombs; i++)
  {
    if (!_bombs[i].active)
    {
      _bombs[i].col = _iPlayerCol;
      _bombs[i].row = _iPlayerRow;
      _bombs[i].timer = BOMB_TIMER;
      _bombs[i].active = TRUE;
      PlayEffect(IDW_SMEXPLODE);
      return;
    }
  }
}

/*
Amaç: Normal bomba zamanlayıcılarını günceller ve süresi dolanları patlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateBombs()
{
  for (int i = 0; i < MAX_BOMBS; i++)
  {
    if (!_bombs[i].active)
      continue;

    _bombs[i].timer--;
    if (_bombs[i].timer <= 0)
    {
      _bombs[i].active = FALSE;
      Explode(_bombs[i].col, _bombs[i].row);
    }
  }
}

/*
Amaç: Remote özellik açıkken aktif normal bombaları elle patlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void DetonateRemoteBombs()
{
  if (!_bRemoteUnlocked)
    return;

  for (int i = 0; i < MAX_BOMBS; i++)
  {
    if (_bombs[i].active)
    {
      _bombs[i].active = FALSE;
      Explode(_bombs[i].col, _bombs[i].row);
    }
  }
}

/*
Amaç: Oyuncunun baktığı yöne hareket eden C4 bombasını başlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void ThrowStickyBomb()
{
  if (!_bStickyUnlocked)
    return;

  int slot = -1;
  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
    if (!_stickyBombs[i].active)
    {
      slot = i;
      break;
    }
  if (slot < 0)
    return;

  int startCol = _iPlayerCol + _iFacingDX;
  int startRow = _iPlayerRow + _iFacingDY;

  

  if (startCol < 1 || startCol >= MAZE_COLS - 1 || startRow < 1 || startRow >= MAZE_ROWS - 1)
    return;
  if (_maze[startCol][startRow] == CELL_WALL || _maze[startCol][startRow] == CELL_BREAKABLE || IsBombAt(startCol, startRow))
    return;

  _stickyBombs[slot].col = startCol;
  _stickyBombs[slot].row = startRow;
  _stickyBombs[slot].dx = _iFacingDX;
  _stickyBombs[slot].dy = _iFacingDY;
  _stickyBombs[slot].moveTimer = STICKY_TRAVEL_DELAY;
  _stickyBombs[slot].timer = BOMB_TIMER + 25;
  _stickyBombs[slot].moving = TRUE;
  _stickyBombs[slot].active = TRUE;
}


/*
Amaç: Aktif veya kurulma aşamasındaki tripwire sayısını döndürür.
Girdi: Yok.
Çıktı: Aktif tripwire sayısı.
*/
int CountActiveTripwires()
{
  int count = 0;
  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    if (_tripwires[i].active)
      count++;
  }
  return count;
}

/*
Amaç: Tripwire anchor koyulacak hücrenin uygun olup olmadığını kontrol eder.
Girdi: int col, int row.
Çıktı: Uygunsa TRUE, değilse FALSE.
*/
BOOL IsTripwireAnchorCellValid(int col, int row)
{
  if (col < 1 || col >= MAZE_COLS - 1 || row < 1 || row >= MAZE_ROWS - 1)
    return FALSE;

  if (_maze[col][row] == CELL_WALL || _maze[col][row] == CELL_BREAKABLE)
    return FALSE;

  if (IsBombAt(col, row) || IsEnemyAtCell(col, row))
    return FALSE;

  return TRUE;
}

/*
Amaç: İki tripwire anchor noktası arasında duvar, kırılabilir duvar veya bomba olup olmadığını kontrol eder.
Girdi: Başlangıç ve bitiş anchor koordinatları.
Çıktı: Hat kurulabilir ise TRUE, aksi halde FALSE.
*/
BOOL IsTripwirePathClear(int c1, int r1, int c2, int r2)
{
  if (c1 != c2 && r1 != r2)
    return FALSE;

  if (!IsTripwireAnchorCellValid(c1, r1) || !IsTripwireAnchorCellValid(c2, r2))
    return FALSE;

  if (c1 == c2)
  {
    int a = min(r1, r2);
    int b = max(r1, r2);
    for (int r = a; r <= b; r++)
    {
      if (_maze[c1][r] == CELL_WALL || _maze[c1][r] == CELL_BREAKABLE || IsBombAt(c1, r))
        return FALSE;
    }
  }
  else
  {
    int a = min(c1, c2);
    int b = max(c1, c2);
    for (int c = a; c <= b; c++)
    {
      if (_maze[c][r1] == CELL_WALL || _maze[c][r1] == CELL_BREAKABLE || IsBombAt(c, r1))
        return FALSE;
    }
  }

  return TRUE;
}

/*
Amaç: Tripwire için birinci veya ikinci anchor noktasını yerleştirir.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceTripwireAnchor()
{
  if (!_bTripwireUnlocked)
    return;

  if (!IsTripwireAnchorCellValid(_iPlayerCol, _iPlayerRow))
    return;

  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    if (_tripwires[i].active && _tripwires[i].armed == 1)
    {
      if (_tripwires[i].col1 == _iPlayerCol && _tripwires[i].row1 == _iPlayerRow)
        return;

      if (!IsTripwirePathClear(_tripwires[i].col1, _tripwires[i].row1, _iPlayerCol, _iPlayerRow))
        return;

      _tripwires[i].col2 = _iPlayerCol;
      _tripwires[i].row2 = _iPlayerRow;
      _tripwires[i].armed = 2;
      _tripwires[i].armDelay = 12;
      return;
    }
  }

  if (CountActiveTripwires() >= MAX_TRIPWIRES)
    return;

  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    if (!_tripwires[i].active)
    {
      _tripwires[i].col1 = _iPlayerCol;
      _tripwires[i].row1 = _iPlayerRow;
      _tripwires[i].col2 = _iPlayerCol;
      _tripwires[i].row2 = _iPlayerRow;
      _tripwires[i].armed = 1;
      _tripwires[i].armDelay = 0;
      _tripwires[i].active = TRUE;
      return;
    }
  }
}

/*
Amaç: Hareket eden veya yapışmış C4 bombalarının durumunu günceller.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateStickyBombs()
{
  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
  {
    if (!_stickyBombs[i].active)
      continue;

    if (_stickyBombs[i].moving)
    {
      _stickyBombs[i].moveTimer--;
      if (_stickyBombs[i].moveTimer > 0)
        continue;

      _stickyBombs[i].moveTimer = STICKY_TRAVEL_DELAY;
      int nextCol = _stickyBombs[i].col + _stickyBombs[i].dx;
      int nextRow = _stickyBombs[i].row + _stickyBombs[i].dy;

      

      

      if (nextCol < 1 || nextCol >= MAZE_COLS - 1 || nextRow < 1 || nextRow >= MAZE_ROWS - 1 ||
          _maze[nextCol][nextRow] == CELL_WALL || _maze[nextCol][nextRow] == CELL_BREAKABLE ||
          IsBombAt(nextCol, nextRow) || IsEnemyAtCell(nextCol, nextRow))
      {
        _stickyBombs[i].moving = FALSE;
      }
      else
      {
        _stickyBombs[i].col = nextCol;
        _stickyBombs[i].row = nextRow;
      }
      continue;
    }

    _stickyBombs[i].timer--;
    if (_stickyBombs[i].timer <= 0)
    {
      _stickyBombs[i].active = FALSE;
      Explode(_stickyBombs[i].col, _stickyBombs[i].row);
    }
  }
}

/*
Amaç: Aktif tripwire hatlarını kontrol eder ve tetiklenirse patlatır.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateTripwires()
{
  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    if (!_tripwires[i].active || _tripwires[i].armed < 2)
      continue;

    if (_tripwires[i].armDelay > 0)
    {
      _tripwires[i].armDelay--;
      continue;
    }

    BOOL triggered = FALSE;
    if (IsTripwireLineCell(_iPlayerCol, _iPlayerRow, i))
      triggered = TRUE;

    for (int e = 0; e < _iNumEnemies; e++)
      if (_enemies[e].alive && IsTripwireLineCell(_enemies[e].col, _enemies[e].row, i))
        triggered = TRUE;

    if (triggered)
    {
      Tripwire w = _tripwires[i];
      _tripwires[i].active = FALSE;
      _tripwires[i].armed = 0;
      _tripwires[i].armDelay = 0;

      if (w.row1 == w.row2)
      {
        int a = min(w.col1, w.col2);
        int b = max(w.col1, w.col2);
        for (int c = a; c <= b; c++)
        {
          int cellBefore = _maze[c][w.row1];
          if (cellBefore == CELL_WALL)
            break;
          ExplodeTripwireCell(c, w.row1);
          if (cellBefore == CELL_BREAKABLE)
            break;
        }
      }
      else if (w.col1 == w.col2)
      {
        int a = min(w.row1, w.row2);
        int b = max(w.row1, w.row2);
        for (int r = a; r <= b; r++)
        {
          int cellBefore = _maze[w.col1][r];
          if (cellBefore == CELL_WALL)
            break;
          ExplodeTripwireCell(w.col1, r);
          if (cellBefore == CELL_BREAKABLE)
            break;
        }
      }
    }
  }
}

/*
Amaç: Bomba patlamasını grid üzerinde yayar, duvarları kırar ve hasar uygular.
Girdi: int col, int row.
Çıktı: Yok.
*/
void ExplodeTripwireCell(int col, int row)
{
  if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS)
    return;

  if (_maze[col][row] == CELL_WALL)
    return;

  if (_maze[col][row] == CELL_BREAKABLE)
  {
    _maze[col][row] = CELL_PATH;
    if (_iBreakableCount > 0)
      _iBreakableCount--;

    if (!_bMineBonusLevel && col == _exitGate.col && row == _exitGate.row)
    {
      _exitGate.revealed = TRUE;
      PlayEffect(IDW_TMISSILE);
    }

    if (_hiddenPowerupType[col][row] != POWERUP_NONE)
    {
      for (int p = 0; p < MAX_POWERUPS; p++)
      {
        if (!_powerups[p].active)
        {
          _powerups[p].col = col;
          _powerups[p].row = row;
          _powerups[p].type = _hiddenPowerupType[col][row];
          _powerups[p].active = TRUE;
          _hiddenPowerupType[col][row] = POWERUP_NONE;
          break;
        }
      }
    }
  }

  AddExplosionCellSafe(col, row);
  ApplyExplosionDamageAtCell(col, row);
  TriggerBombChainAtCell(col, row);
}

/*
Amaç: Patlama hücresini listeye güvenli şekilde ekler.
Girdi: int col, int row.
Çıktı: Hücre eklendiyse TRUE, aksi durumda FALSE.
*/
BOOL AddExplosionCellSafe(int col, int row)
{
  if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS)
    return FALSE;

  if (_maze[col][row] == CELL_WALL)
    return FALSE;

  for (int i = 0; i < MAX_EXPLOSIONS; i++)
  {
    if (_explosions[i].active && _explosions[i].col == col && _explosions[i].row == row)
      return TRUE;
  }

  for (int i = 0; i < MAX_EXPLOSIONS; i++)
  {
    if (!_explosions[i].active)
    {
      _explosions[i].col = col;
      _explosions[i].row = row;
      _explosions[i].timer = EXPLOSION_TIMER;
      _explosions[i].active = TRUE;
      return TRUE;
    }
  }

  return FALSE;
}

/*
Amaç: Sadece aktif patlama hücresindeki oyuncu, düşman ve bonus hedef hasarını uygular.
Girdi: int col, int row.
Çıktı: Yok.
*/
void ApplyExplosionDamageAtCell(int col, int row)
{
  KillEnemyAtCell(col, row);
  CheckBonusTargetHit(col, row);

  if (_iPlayerCol == col && _iPlayerRow == row)
    KillPlayer();
}

/*
Amaç: Patlama hücresindeki normal bomba veya C4 varsa zincirleme patlatır.
Girdi: int col, int row.
Çıktı: Yok.
*/
void TriggerBombChainAtCell(int col, int row)
{
  for (int i = 0; i < MAX_BOMBS; i++)
  {
    if (_bombs[i].active && _bombs[i].col == col && _bombs[i].row == row)
    {
      _bombs[i].active = FALSE;
      Explode(_bombs[i].col, _bombs[i].row);
    }
  }

  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
  {
    if (_stickyBombs[i].active && _stickyBombs[i].col == col && _stickyBombs[i].row == row)
    {
      _stickyBombs[i].active = FALSE;
      Explode(_stickyBombs[i].col, _stickyBombs[i].row);
    }
  }
}

/*
Amaç: Bomba patlamasını grid üzerinde yayar, duvarları kırar ve hasar uygular.
Girdi: int col, int row.
Çıktı: Yok.
*/
void Explode(int col, int row)
{
  PlayEffect(IDW_LGEXPLODE);

  int range = max(1, min(_iExplosionRange, 5));
  int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

  if (AddExplosionCellSafe(col, row))
  {
    ApplyExplosionDamageAtCell(col, row);
    TriggerBombChainAtCell(col, row);
  }

  for (int d = 0; d < 4; d++)
  {
    for (int step = 1; step <= range; step++)
    {
      int ex = col + dirs[d][0] * step;
      int ey = row + dirs[d][1] * step;

      if (ex < 0 || ex >= MAZE_COLS || ey < 0 || ey >= MAZE_ROWS)
        break;

      if (_maze[ex][ey] == CELL_WALL)
        break;

      if (_maze[ex][ey] == CELL_BREAKABLE)
      {
        _maze[ex][ey] = CELL_PATH;
        if (_iBreakableCount > 0)
          _iBreakableCount--;

        if (!_bMineBonusLevel && ex == _exitGate.col && ey == _exitGate.row)
        {
          _exitGate.revealed = TRUE;
          PlayEffect(IDW_TMISSILE);
        }

        if (_hiddenPowerupType[ex][ey] != POWERUP_NONE)
        {
          for (int p = 0; p < MAX_POWERUPS; p++)
          {
            if (!_powerups[p].active)
            {
              _powerups[p].col = ex;
              _powerups[p].row = ey;
              _powerups[p].type = _hiddenPowerupType[ex][ey];
              _powerups[p].active = TRUE;
              _hiddenPowerupType[ex][ey] = POWERUP_NONE;
              break;
            }
          }
        }

        AddExplosionCellSafe(ex, ey);
        ApplyExplosionDamageAtCell(ex, ey);
        TriggerBombChainAtCell(ex, ey);
        break;
      }

      if (AddExplosionCellSafe(ex, ey))
      {
        ApplyExplosionDamageAtCell(ex, ey);
        TriggerBombChainAtCell(ex, ey);
      }
    }
  }
}

/*
Amaç: Belirtilen hücredeki düşmanı öldürür ve skor ekler.
Girdi: int col, int row.
Çıktı: Yok.
*/
void KillEnemyAtCell(int col, int row)
{
  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive)
      continue;

    if (_enemies[i].col == col && _enemies[i].row == row)
    {
      int baseScore = (_enemies[i].type == ENEMY_GHOST) ? 220 :
                      ((_enemies[i].type == ENEMY_SPOOKY) ? 175 :
                      ((_enemies[i].type == ENEMY_RUNNER) ? 130 :
                      ((_enemies[i].type == ENEMY_HUNTER) ? 150 : 100)));
      int secondsLeft = max(0, _iLevelTimeLeft / 30);
      int earlyKillBonus = min(180, secondsLeft * 2);
      _enemies[i].alive = FALSE;
      _iScore += baseScore + earlyKillBonus;
    }
  }
}

/*
Amaç: Aktif patlama hücrelerindeki düşmanları her frame kontrol eder.
Girdi: Yok.
Çıktı: Yok.
*/
void CheckEnemyExplosionDamage()
{
  for (int e = 0; e < _iNumEnemies; e++)
  {
    if (!_enemies[e].alive)
      continue;

    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
      if (_explosions[i].active &&
          _explosions[i].col == _enemies[e].col &&
          _explosions[i].row == _enemies[e].row)
      {
        KillEnemyAtCell(_enemies[e].col, _enemies[e].row);
        break;
      }
    }
  }
}

/*
Amaç: Patlama görsellerinin ömrünü azaltır.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateExplosions()
{
  for (int i = 0; i < MAX_EXPLOSIONS; i++)
  {
    if (!_explosions[i].active)
      continue;

    _explosions[i].timer--;
    if (_explosions[i].timer <= 0)
      _explosions[i].active = FALSE;
  }
}

/*
Amaç: Süreli power-up etkilerini günceller.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdatePowerUpTimers()
{
  if (_iSpeedTimer > 0) _iSpeedTimer--;
  if (_iRangeTimer > 0) _iRangeTimer--;
  if (_iWallPassTimer > 0) _iWallPassTimer--;
}

/*
Amaç: Oyuncunun düşman veya patlama nedeniyle ölmesini kontrol eder.
Girdi: Yok.
Çıktı: Yok.
*/
void CheckPlayerDeath()
{
  if (_bPlayerInvincible)
    return;

  BOOL dead = FALSE;

  for (int i = 0; i < _iNumEnemies; i++)
    if (_enemies[i].alive &&
        _enemies[i].col == _iPlayerCol && _enemies[i].row == _iPlayerRow)
      dead = TRUE;

  for (int i = 0; i < MAX_EXPLOSIONS; i++)
    if (_explosions[i].active &&
        _explosions[i].col == _iPlayerCol && _explosions[i].row == _iPlayerRow)
      dead = TRUE;

  if (dead)
    KillPlayer();
}

/*
Amaç: Can, kalkan, invincibility ve game over akışını yönetir.
Girdi: Yok.
Çıktı: Yok.
*/
void KillPlayer()
{
  if (_bPlayerInvincible)
    return;

  if (_bHasShield)
  {
    _bHasShield = FALSE;
    PlayEffect(IDW_JMISSILE);
    _bPlayerInvincible = TRUE;
    _iInvincibleTimer = 30;
    return;
  }

  PlayEffect(IDW_JMISSILE);
  _iNumLives--;
  if (_iNumLives <= 0)
  {
    _bGameOver = TRUE;
    _bLastRunGameOver = TRUE;
    _iGameState = STATE_MENU;
    PlayEffect(IDW_GAMEOVER);
  }
  else
  {
    _iPlayerCol = 1;
    _iPlayerRow = 1;
    _bPlayerInvincible = TRUE;
    _iInvincibleTimer = 60;
    ClearTrail();
    if (_bMineBonusLevel && _mineGrid[1][1])
      _mineGrid[1][1] = FALSE;
  }
}

/*
Amaç: Tron izini açar veya kapatır.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 8: Tron Trail Mekaniği
// Oyuncunun arkasında bıraktığı 10 hücrelik iz düşmanlar için
// engel gibi davranır, oyuncuya zarar vermez.
// ============================================================
void ToggleTrail()
{
  _bTrailEnabled = !_bTrailEnabled;
  ClearTrail();
}

/*
Amaç: Tron izi belleğini temizler.
Girdi: Yok.
Çıktı: Yok.
*/
void ClearTrail()
{
  _iTrailCount = 0;
  for (int i = 0; i < MAX_TRAIL_CELLS; i++)
  {
    _trail[i].active = FALSE;
    _trail[i].col = 0;
    _trail[i].row = 0;
  }
}

/*
Amaç: Oyuncunun eski konumunu Tron izi listesine ekler.
Girdi: int col, int row.
Çıktı: Yok.
*/
void AddTrailCell(int col, int row)
{
  if (!_bTrailEnabled)
    return;

  for (int i = MAX_TRAIL_CELLS - 1; i > 0; i--)
    _trail[i] = _trail[i - 1];

  _trail[0].col = col;
  _trail[0].row = row;
  _trail[0].active = TRUE;

  if (_iTrailCount < MAX_TRAIL_CELLS)
    _iTrailCount++;
}

/*
Amaç: Bir hücrenin aktif Tron izi olup olmadığını döndürür.
Girdi: int col, int row.
Çıktı: BOOL.
*/
BOOL IsTrailCell(int col, int row)
{
  if (!_bTrailEnabled)
    return FALSE;

  for (int i = 0; i < _iTrailCount; i++)
    if (_trail[i].active && _trail[i].col == col && _trail[i].row == row)
      return TRUE;

  return FALSE;
}

/*
Amaç: Tron izinin düşmanlara engel olarak davranması için gereken etkileri günceller.
Girdi: Yok.
Çıktı: Yok.
*/
void UpdateTrailEffects()
{

  
  if (!_bTrailEnabled)
    return;
}

/*
Amaç: Tron izini neon çizgi olarak çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawTrail(HDC hDC)
{
  if (!_bTrailEnabled)
    return;

  

  HPEN hGlowPen = CreatePen(PS_SOLID, 6, RGB(0, 95, 170));
  HPEN hCorePen = CreatePen(PS_SOLID, 2, RGB(150, 245, 255));
  for (int i = _iTrailCount - 1; i >= 0; i--)
  {
    if (!_trail[i].active)
      continue;

    int cx = _trail[i].col * TILE_SIZE + TILE_SIZE / 2;
    int cy = _trail[i].row * TILE_SIZE + TILE_SIZE / 2;
    int fade = 40 + (MAX_TRAIL_CELLS - i) * 18;
    if (fade > 220) fade = 220;

    HBRUSH hGlow = CreateSolidBrush(RGB(0, fade / 2, fade));
    HPEN hBorder = CreatePen(PS_SOLID, 1, RGB(80, 230, 255));
    SelectObject(hDC, hGlow);
    SelectObject(hDC, hBorder);
    RoundRect(hDC, cx - 9, cy - 9, cx + 9, cy + 9, 8, 8);
    DeleteObject(hGlow);
    DeleteObject(hBorder);

    if (i > 0 && _trail[i - 1].active)
    {
      int nx = _trail[i - 1].col * TILE_SIZE + TILE_SIZE / 2;
      int ny = _trail[i - 1].row * TILE_SIZE + TILE_SIZE / 2;
      if (abs(nx - cx) + abs(ny - cy) == TILE_SIZE)
      {
        SelectObject(hDC, hGlowPen);
        MoveToEx(hDC, cx, cy, NULL);
        LineTo(hDC, nx, ny);
        SelectObject(hDC, hCorePen);
        MoveToEx(hDC, cx, cy, NULL);
        LineTo(hDC, nx, ny);
      }
    }
  }
  DeleteObject(hGlowPen);
  DeleteObject(hCorePen);
}

/*
Amaç: Çekiç tabanlı bonus bölümü başlatır.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 9: Bonus Hammer Rush
// Bonus bölümde bomba yerine çekiç kullanılır; altın kasalar,
// normal kasalar ve bonus düşmanları skor hedefi olarak çalışır.
// ============================================================
void StartBonusArenaLevel()
{
  _iNumEnemies = 0;
  _iLevelTime = 100 * 30;
  _iLevelTimeLeft = _iLevelTime;
  _iMaxBombs = 0;
  _iExplosionRange = 0;
  GenerateBonusArena();
  PlaceBonusEnemies();
}

/*
Amaç: Bonus bölüm kasalarını, hedeflerini ve açık yollarını oluşturur.
Girdi: Yok.
Çıktı: Yok.
*/
void GenerateBonusArena()
{
  for (int x = 0; x < MAZE_COLS; x++)
    for (int y = 0; y < MAZE_ROWS; y++)
    {
      _maze[x][y] = CELL_PATH;
      _mineGrid[x][y] = FALSE;      
      _mineRevealed[x][y] = FALSE;  
      _mineFlagged[x][y] = FALSE;   
      _hiddenPowerupType[x][y] = POWERUP_NONE;
    }

  for (int x = 0; x < MAZE_COLS; x++)
  {
    _maze[x][0] = CELL_WALL;
    _maze[x][MAZE_ROWS - 1] = CELL_WALL;
  }
  for (int y = 0; y < MAZE_ROWS; y++)
  {
    _maze[0][y] = CELL_WALL;
    _maze[MAZE_COLS - 1][y] = CELL_WALL;
  }

  for (int x = 5; x < MAZE_COLS - 5; x += 6)
    for (int y = 3; y < MAZE_ROWS - 3; y += 5)
      _maze[x][y] = CELL_WALL;

  _iPlayerCol = 1;
  _iPlayerRow = 1;
  _maze[1][1] = CELL_PATH;
  _maze[2][1] = CELL_PATH;
  _maze[1][2] = CELL_PATH;
  _maze[2][2] = CELL_PATH;
  _maze[3][1] = CELL_PATH;
  _maze[1][3] = CELL_PATH;

  _iMineCount = 12;
  _iSafeCellsLeft = _iMineCount;
  _iBreakableCount = 0;

  int placedGold = 0;
  int attempts = 0;
  while (placedGold < _iMineCount && attempts < 4000)
  {
    attempts++;
    int x = 3 + rand() % (MAZE_COLS - 6);
    int y = 2 + rand() % (MAZE_ROWS - 4);
    if (_maze[x][y] != CELL_PATH || abs(x - 1) + abs(y - 1) < 6)
      continue;

    _maze[x][y] = CELL_BREAKABLE;
    _mineGrid[x][y] = TRUE;
    _iBreakableCount++;
    placedGold++;
  }

  
  int ordinary = 20;
  int placedOrdinary = 0;
  attempts = 0;
  while (placedOrdinary < ordinary && attempts < 4000)
  {
    attempts++;
    int x = 2 + rand() % (MAZE_COLS - 4);
    int y = 2 + rand() % (MAZE_ROWS - 4);
    if (_maze[x][y] != CELL_PATH || abs(x - 1) + abs(y - 1) < 5)
      continue;
    _maze[x][y] = CELL_BREAKABLE;
    _iBreakableCount++;
    placedOrdinary++;
  }
}

/*
Amaç: Bonus bölümde çekiçle öldürülebilecek düşmanları yerleştirir.
Girdi: Yok.
Çıktı: Yok.
*/
void PlaceBonusEnemies()
{
  _iNumEnemies = 5;
  for (int i = 0; i < MAX_ENEMIES; i++)
    _enemies[i].alive = FALSE;

  int placed = 0;
  int attempts = 0;
  while (placed < _iNumEnemies && attempts < 3000)
  {
    attempts++;
    int x = 4 + rand() % (MAZE_COLS - 8);
    int y = 3 + rand() % (MAZE_ROWS - 6);
    if (_maze[x][y] != CELL_PATH || abs(x - 1) + abs(y - 1) < 8)
      continue;

    _enemies[placed].col = x;
    _enemies[placed].row = y;
    _enemies[placed].type = (placed % 2 == 0) ? ENEMY_NORMAL : ENEMY_RUNNER;
    _enemies[placed].level = 1;
    _enemies[placed].moveDelay = 12;
    _enemies[placed].moveTimer = 0;
    _enemies[placed].dirX = (placed % 2 == 0) ? 1 : -1;
    _enemies[placed].dirY = 0;
    for (int gt = 0; gt < GHOST_TRAIL_LENGTH; gt++)
    {
      _enemies[placed].ghostTrailCol[gt] = x;
      _enemies[placed].ghostTrailRow[gt] = y;
    }
    _enemies[placed].trailCount = 0;
    _enemies[placed].alive = TRUE;
    placed++;
  }
  _iNumEnemies = placed;
}

/*
Amaç: Bonus bölümde oyuncu aksiyonunu işler.
Girdi: Yok.
Çıktı: Yok.
*/
void HandleBonusArenaAction()
{
  SwingBonusHammer();
}

/*
Amaç: Çekiç vuruşunu uygular, kasa kırar veya düşman öldürür.
Girdi: Yok.
Çıktı: Yok.
*/
void SwingBonusHammer()
{
  int tx = _iPlayerCol + _iFacingDX;
  int ty = _iPlayerRow + _iFacingDY;
  if (tx < 1 || tx >= MAZE_COLS - 1 || ty < 1 || ty >= MAZE_ROWS - 1)
    return;

  _iHammerSwingTimer = 8;
  _iHammerSwingCol = tx;
  _iHammerSwingRow = ty;
  _iHammerSwingDX = _iFacingDX;
  _iHammerSwingDY = _iFacingDY;

  BOOL hitSomething = FALSE;

  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (_enemies[i].alive && _enemies[i].col == tx && _enemies[i].row == ty)
    {
      _enemies[i].alive = FALSE;
      _iScore += 220 + max(0, _iLevelTimeLeft / 30);
      hitSomething = TRUE;
      PlayEffect(IDW_SMEXPLODE);
      break;
    }
  }

  if (_maze[tx][ty] == CELL_BREAKABLE)
  {
    _maze[tx][ty] = CELL_PATH;
    if (_iBreakableCount > 0)
      _iBreakableCount--;

    if (_mineGrid[tx][ty] && !_mineRevealed[tx][ty])
    {
      _mineRevealed[tx][ty] = TRUE;
      _mineGrid[tx][ty] = FALSE;
      if (_iSafeCellsLeft > 0)
        _iSafeCellsLeft--;
      _iScore += 140 + max(0, _iLevelTimeLeft / 30) * 2;
    }
    else
    {
      _iScore += 35;
    }
    hitSomething = TRUE;
    PlayEffect(IDW_BMISSILE);
  }

  if (!hitSomething)
    PlayEffect(IDW_MISSILE);

  if (_iSafeCellsLeft <= 0)
  {
    _iScore += 1200 + max(0, _iLevelTimeLeft / 30) * 15;
    CompleteRun();
  }
}

/*
Amaç: ToggleMineFlag fonksiyonunun oyun içindeki ilgili işlemini yürütür.
Girdi: Yok.
Çıktı: Yok.
*/
void ToggleMineFlag()
{
}
/*
Amaç: CountAdjacentMines fonksiyonunun oyun içindeki ilgili işlemini yürütür.
Girdi: int col, int row.
Çıktı: int.
*/
int CountAdjacentMines(int col, int row)
{
  return 0;
}
/*
Amaç: RevealMineCell fonksiyonunun oyun içindeki ilgili işlemini yürütür.
Girdi: int col, int row.
Çıktı: Yok.
*/
void RevealMineCell(int col, int row)
{
}
/*
Amaç: CheckBonusTargetHit fonksiyonunun oyun içindeki ilgili işlemini yürütür.
Girdi: int col, int row.
Çıktı: Yok.
*/
void CheckBonusTargetHit(int col, int row)
{
}

/*
Amaç: Bonus bölüm sahnesini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawBonusArena(HDC hDC)
{
  int tx = (_iHammerSwingTimer > 0) ? _iHammerSwingCol : (_iPlayerCol + _iFacingDX);
  int ty = (_iHammerSwingTimer > 0) ? _iHammerSwingRow : (_iPlayerRow + _iFacingDY);
  if (tx < 1 || tx >= MAZE_COLS - 1 || ty < 1 || ty >= MAZE_ROWS - 1)
  {
    tx = _iPlayerCol;
    ty = _iPlayerRow;
  }

  int px = tx * TILE_SIZE;
  int py = ty * TILE_SIZE;

  HPEN hTargetPen = CreatePen(PS_SOLID, 2, RGB(255, 235, 130));
  HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
  SelectObject(hDC, hTargetPen);
  SelectObject(hDC, hNull);
  Rectangle(hDC, px + 5, py + 5, px + TILE_SIZE - 5, py + TILE_SIZE - 5);
  DeleteObject(hTargetPen);

  int phase = _iHammerSwingTimer;
  int ox = 0;
  int oy = 0;
  if (phase > 5)
    oy = -14;
  else if (phase > 2)
    oy = -6;
  else if (phase > 0)
    oy = 2;

  int cx = px + TILE_SIZE / 2;
  int topY = py + 5 + oy;
  int bottomY = py + TILE_SIZE - 7;

  HPEN hHandle = CreatePen(PS_SOLID, 4, RGB(142, 94, 45));
  HPEN hHead = CreatePen(PS_SOLID, 7, RGB(225, 232, 238));
  HPEN hSpark = CreatePen(PS_SOLID, 2, RGB(255, 236, 80));

  SelectObject(hDC, hHandle);
  MoveToEx(hDC, cx - 6, bottomY, NULL);
  LineTo(hDC, cx + 4, topY + 12);

  SelectObject(hDC, hHead);
  MoveToEx(hDC, cx - 10, topY + 8, NULL);
  LineTo(hDC, cx + 13, topY + 3);

  if (phase > 0 && phase <= 3)
  {
    SelectObject(hDC, hSpark);
    MoveToEx(hDC, px + 7, py + 8, NULL);
    LineTo(hDC, px + 2, py + 2);
    MoveToEx(hDC, px + TILE_SIZE - 7, py + 8, NULL);
    LineTo(hDC, px + TILE_SIZE - 2, py + 2);
    MoveToEx(hDC, cx, py + 4, NULL);
    LineTo(hDC, cx, py - 4);
  }

  DeleteObject(hHandle);
  DeleteObject(hHead);
  DeleteObject(hSpark);
}

/*
Amaç: Düşmanın normal hareket için bir hücreye girip giremeyeceğini kontrol eder.
Girdi: int col, int row, int enemyType.
Çıktı: BOOL.
*/
// ============================================================
// BÖLÜM 10: Çizim ve Görsel Katman
// Board, maze, oyuncu, düşman, patlama, HUD, menü, skor ve
// bonus ekran çizimleri bu bölümden itibaren yapılır.
// ============================================================
BOOL IsEnemyWalkable(int col, int row, int enemyType)
{
  if (col < 1 || col >= MAZE_COLS - 1 || row < 1 || row >= MAZE_ROWS - 1)
    return FALSE;

  if (IsTrailCell(col, row))
    return FALSE;
  if (enemyType != ENEMY_GHOST && IsGhostTrailCell(col, row))
    return FALSE;
  if (enemyType == ENEMY_GHOST)
    return TRUE;
  if (_maze[col][row] == CELL_WALL)
    return FALSE;
  if (enemyType != ENEMY_SPOOKY && _maze[col][row] == CELL_BREAKABLE)
    return FALSE;
  if (IsBombAt(col, row))
    return FALSE;
  return TRUE;
}

/*
Amaç: Board zeminini ve grid dokusunu çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawFloorBackground(HDC hDC)
{
  for (int x = 0; x < MAZE_COLS; x++)
    for (int y = 0; y < MAZE_ROWS; y++)
    {
      int px = x * TILE_SIZE;
      int py = y * TILE_SIZE;
      RECT rc = { px, py, px + TILE_SIZE, py + TILE_SIZE };

      COLORREF base = ((x + y) % 2 == 0) ? RGB(22, 32, 46) : RGB(18, 27, 40);
      HBRUSH hBr = CreateSolidBrush(base);
      FillRect(hDC, &rc, hBr);
      DeleteObject(hBr);

      HPEN hGrid = CreatePen(PS_SOLID, 1, RGB(34, 48, 66));
      SelectObject(hDC, hGrid);
      MoveToEx(hDC, px, py + TILE_SIZE - 1, NULL);
      LineTo(hDC, px + TILE_SIZE - 1, py + TILE_SIZE - 1);
      LineTo(hDC, px + TILE_SIZE - 1, py);
      DeleteObject(hGrid);
    }
}

/*
Amaç: Sabit duvarları, kırılabilir duvarları ve özel kasaları çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawMaze(HDC hDC)
{
  for (int x = 0; x < MAZE_COLS; x++)
    for (int y = 0; y < MAZE_ROWS; y++)
    {
      int px = x * TILE_SIZE;
      int py = y * TILE_SIZE;

      if (_maze[x][y] == CELL_WALL)
      {

        RECT outer = { px + 1, py + 1, px + TILE_SIZE - 1, py + TILE_SIZE - 1 };
        RECT inner = { px + 5, py + 5, px + TILE_SIZE - 5, py + TILE_SIZE - 5 };
        HBRUSH hOuter = CreateSolidBrush(RGB(36, 63, 91));
        HBRUSH hInner = CreateSolidBrush(RGB(74, 111, 148));
        FillRect(hDC, &outer, hOuter);
        FillRect(hDC, &inner, hInner);
        DeleteObject(hOuter);
        DeleteObject(hInner);

        HPEN hLight = CreatePen(PS_SOLID, 2, RGB(128, 172, 205));
        HPEN hDark = CreatePen(PS_SOLID, 2, RGB(16, 30, 48));
        SelectObject(hDC, hLight);
        MoveToEx(hDC, px + 2, py + TILE_SIZE - 3, NULL);
        LineTo(hDC, px + 2, py + 2);
        LineTo(hDC, px + TILE_SIZE - 3, py + 2);
        SelectObject(hDC, hDark);
        MoveToEx(hDC, px + 3, py + TILE_SIZE - 3, NULL);
        LineTo(hDC, px + TILE_SIZE - 3, py + TILE_SIZE - 3);
        LineTo(hDC, px + TILE_SIZE - 3, py + 3);
        DeleteObject(hLight);
        DeleteObject(hDark);
      }
      else if (_maze[x][y] == CELL_BREAKABLE)
      {
        BOOL hasHiddenPower = (_hiddenPowerupType[x][y] != POWERUP_NONE);
        BOOL isBonusTarget = (_bMineBonusLevel && _mineGrid[x][y] && !_mineRevealed[x][y]);
        BOOL isBonusFuse = (_bMineBonusLevel && _mineFlagged[x][y]);
        RECT rc = { px + 2, py + 2, px + TILE_SIZE - 2, py + TILE_SIZE - 2 };
        COLORREF fillColor = isBonusTarget ? RGB(224, 168, 42) : (isBonusFuse ? RGB(210, 46, 58) : (hasHiddenPower ? RGB(76, 183, 106) : RGB(184, 84, 44)));
        COLORREF lineColor = isBonusTarget ? RGB(118, 75, 12) : (isBonusFuse ? RGB(92, 16, 22) : (hasHiddenPower ? RGB(20, 92, 48) : RGB(105, 42, 22)));
        HBRUSH hBr = CreateSolidBrush(fillColor);
        FillRect(hDC, &rc, hBr);
        DeleteObject(hBr);

        HPEN hPen = CreatePen(PS_SOLID, 2, lineColor);
        SelectObject(hDC, hPen);
        for (int by = py + 9; by < py + TILE_SIZE; by += 8)
        {
          MoveToEx(hDC, px + 3, by, NULL);
          LineTo(hDC, px + TILE_SIZE - 3, by);
        }
        MoveToEx(hDC, px + TILE_SIZE / 2, py + 3, NULL);
        LineTo(hDC, px + TILE_SIZE / 2, py + TILE_SIZE - 3);
        MoveToEx(hDC, px + 7, py + 7, NULL);
        LineTo(hDC, px + TILE_SIZE - 7, py + TILE_SIZE - 7);

        if (hasHiddenPower)
        {
          HPEN hPlus = CreatePen(PS_SOLID, 3, RGB(235, 255, 150));
          SelectObject(hDC, hPlus);
          MoveToEx(hDC, px + TILE_SIZE / 2, py + 7, NULL);
          LineTo(hDC, px + TILE_SIZE / 2, py + TILE_SIZE - 7);
          MoveToEx(hDC, px + 7, py + TILE_SIZE / 2, NULL);
          LineTo(hDC, px + TILE_SIZE - 7, py + TILE_SIZE / 2);
          DeleteObject(hPlus);
        }
        if (isBonusTarget)
        {
          HPEN hStar = CreatePen(PS_SOLID, 3, RGB(255, 245, 160));
          SelectObject(hDC, hStar);
          MoveToEx(hDC, px + TILE_SIZE / 2, py + 8, NULL);
          LineTo(hDC, px + TILE_SIZE / 2, py + TILE_SIZE - 8);
          MoveToEx(hDC, px + 8, py + TILE_SIZE / 2, NULL);
          LineTo(hDC, px + TILE_SIZE - 8, py + TILE_SIZE / 2);
          MoveToEx(hDC, px + 10, py + 10, NULL);
          LineTo(hDC, px + TILE_SIZE - 10, py + TILE_SIZE - 10);
          MoveToEx(hDC, px + TILE_SIZE - 10, py + 10, NULL);
          LineTo(hDC, px + 10, py + TILE_SIZE - 10);
          DeleteObject(hStar);
        }
        if (isBonusFuse)
        {
          HPEN hFuse = CreatePen(PS_SOLID, 3, RGB(255, 230, 95));
          SelectObject(hDC, hFuse);
          MoveToEx(hDC, px + 8, py + TILE_SIZE / 2, NULL);
          LineTo(hDC, px + TILE_SIZE - 8, py + TILE_SIZE / 2);
          MoveToEx(hDC, px + TILE_SIZE / 2, py + 8, NULL);
          LineTo(hDC, px + TILE_SIZE / 2 + 5, py + TILE_SIZE - 8);
          DeleteObject(hFuse);
        }
        DeleteObject(hPen);
      }
    }
}

/*
Amaç: Gizli çıkış bulunduysa kapı görselini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawExitGate(HDC hDC)
{
  if (!_exitGate.revealed)
    return;

  int px = _exitGate.col * TILE_SIZE;
  int py = _exitGate.row * TILE_SIZE;
  RECT rc = { px + 4, py + 4, px + TILE_SIZE - 4, py + TILE_SIZE - 4 };
  BOOL open = (CountAliveEnemies() <= 0);

  HBRUSH hBr = CreateSolidBrush(open ? RGB(80, 255, 140) : RGB(255, 210, 70));
  FillRect(hDC, &rc, hBr);
  DeleteObject(hBr);

  HPEN hPen = CreatePen(PS_SOLID, 2, open ? RGB(210, 255, 220) : RGB(255, 160, 0));
  HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
  SelectObject(hDC, hPen);
  SelectObject(hDC, hNull);
  Rectangle(hDC, px + 4, py + 4, px + TILE_SIZE - 4, py + TILE_SIZE - 4);
  Arc(hDC, px + 8, py + 6, px + TILE_SIZE - 8, py + TILE_SIZE - 2,
      px + 8, py + TILE_SIZE / 2, px + TILE_SIZE - 8, py + TILE_SIZE / 2);
  DeleteObject(hPen);
}

/*
Amaç: Portal çiftlerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawPortals(HDC hDC)
{
  COLORREF portalColors[] = { RGB(0, 200, 255), RGB(255, 0, 200), RGB(0, 255, 100) };
  static int animFrame = 0;
  animFrame++;

  for (int i = 0; i < MAX_PORTAL_PAIRS; i++)
  {
    if (!_portals[i].active)
      continue;

    COLORREF c = portalColors[i % 3];
    int pulse = (animFrame / 2) % 7;
    int outerR = TILE_SIZE / 2 - 3;
    int innerR = max(outerR - 4 - pulse / 2, 4);

    for (int side = 0; side < 2; side++)
    {
      int px = (side == 0 ? _portals[i].col1 : _portals[i].col2) * TILE_SIZE;
      int py = (side == 0 ? _portals[i].row1 : _portals[i].row2) * TILE_SIZE;
      int cx = px + TILE_SIZE / 2;
      int cy = py + TILE_SIZE / 2;

      HPEN hPen = CreatePen(PS_SOLID, 2, c);
      HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
      SelectObject(hDC, hPen);
      SelectObject(hDC, hNull);
      Ellipse(hDC, cx - outerR, cy - outerR, cx + outerR, cy + outerR);
      Ellipse(hDC, cx - innerR, cy - innerR, cx + innerR, cy + innerR);
      DeleteObject(hPen);
    }
  }
}

/*
Amaç: Açık power-up simgelerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawPowerUps(HDC hDC)
{
  for (int i = 0; i < MAX_POWERUPS; i++)
  {
    if (!_powerups[i].active)
      continue;

    int px = _powerups[i].col * TILE_SIZE;
    int py = _powerups[i].row * TILE_SIZE;
    int cx = px + TILE_SIZE / 2, cy = py + TILE_SIZE / 2;
    int r = TILE_SIZE / 3;

    COLORREF c;
    const char* label = "";
    switch (_powerups[i].type)
    {
    case POWERUP_SPEED:    c = RGB(0, 150, 255);   label = "SPD"; break;
    case POWERUP_RANGE:    c = RGB(255, 80, 60);   label = "F+";  break;
    case POWERUP_SHIELD:   c = RGB(220, 220, 255); label = "SH";  break;
    case POWERUP_BOMB:     c = RGB(255, 180, 40);  label = "B+";  break;
    case POWERUP_WALLPASS: c = RGB(60, 255, 150);  label = "W";   break;
    default:               c = RGB(200, 200, 200); label = "?";   break;
    }

    HBRUSH hBr = CreateSolidBrush(c);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    SelectObject(hDC, hBr);
    SelectObject(hDC, hPen);
    Ellipse(hDC, cx - r, cy - r, cx + r, cy + r);
    DeleteObject(hBr);
    DeleteObject(hPen);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(0, 0, 0));
    RECT rc = { px, py, px + TILE_SIZE, py + TILE_SIZE };
    DrawText(hDC, label, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  }
}

/*
Amaç: Oyuncunun grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar.
Girdi: int* px, int* py.
Çıktı: Yok.
*/
void GetPlayerDrawPosition(int* px, int* py)
{
  int currentX = _iPlayerCol * TILE_SIZE;
  int currentY = _iPlayerRow * TILE_SIZE;

  if (_iPlayerAnimTimer > 0 && _iPlayerAnimTotal > 0)
  {
    int oldX = _iPlayerPrevCol * TILE_SIZE;
    int oldY = _iPlayerPrevRow * TILE_SIZE;
    int elapsed = _iPlayerAnimTotal - _iPlayerAnimTimer;
    *px = oldX + (currentX - oldX) * elapsed / _iPlayerAnimTotal;
    *py = oldY + (currentY - oldY) * elapsed / _iPlayerAnimTotal;
    return;
  }

  *px = currentX;
  *py = currentY;
}

/*
Amaç: Düşmanın grid geçişini çizimde yumuşatmak için ara piksel konumunu hesaplar.
Girdi: int index, int* px, int* py.
Çıktı: Yok.
*/
void GetEnemyDrawPosition(int index, int* px, int* py)
{
  int currentX = _enemies[index].col * TILE_SIZE;
  int currentY = _enemies[index].row * TILE_SIZE;

  if (_enemies[index].animTimer > 0 && _enemies[index].animTotal > 0)
  {
    int oldX = _enemies[index].prevCol * TILE_SIZE;
    int oldY = _enemies[index].prevRow * TILE_SIZE;
    int elapsed = _enemies[index].animTotal - _enemies[index].animTimer;
    *px = oldX + (currentX - oldX) * elapsed / _enemies[index].animTotal;
    *py = oldY + (currentY - oldY) * elapsed / _enemies[index].animTotal;
    return;
  }

  *px = currentX;
  *py = currentY;
}

/*
Amaç: Oyuncu karakterini ve aktif aura efektlerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawPlayer(HDC hDC)
{
  if (_bPlayerInvincible && (_iInvincibleTimer % 4 < 2))
    return;

  int px, py;
  GetPlayerDrawPosition(&px, &py);
  int cx = px + TILE_SIZE / 2;

  

  if (_iWallPassTimer > 0 || _iSpeedTimer > 0 || _bHasShield)
  {
    COLORREF aura = _iWallPassTimer > 0 ? RGB(60, 210, 135) : RGB(65, 155, 255);
    if (_bHasShield) aura = RGB(170, 225, 255);
    HPEN hAuraPen = CreatePen(PS_SOLID, 2, RGB(220, 250, 255));
    HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(hDC, hAuraPen);
    SelectObject(hDC, hNull);
    Ellipse(hDC, px + 1, py + 1, px + TILE_SIZE - 1, py + TILE_SIZE - 1);
    HPEN hAuraPen2 = CreatePen(PS_SOLID, 1, aura);
    SelectObject(hDC, hAuraPen2);
    Ellipse(hDC, px + 3, py + 3, px + TILE_SIZE - 3, py + TILE_SIZE - 3);
    DeleteObject(hAuraPen);
    DeleteObject(hAuraPen2);
  }

  HBRUSH hShadow = CreateSolidBrush(RGB(4, 7, 15));
  SelectObject(hDC, hShadow);
  Ellipse(hDC, px + 5, py + TILE_SIZE - 8, px + TILE_SIZE - 5, py + TILE_SIZE - 2);
  DeleteObject(hShadow);

  HBRUSH hBoot = CreateSolidBrush(RGB(250, 250, 255));
  HPEN hBootPen = CreatePen(PS_SOLID, 1, RGB(82, 100, 142));
  SelectObject(hDC, hBoot);
  SelectObject(hDC, hBootPen);
  RoundRect(hDC, px + 6, py + 25, px + 15, py + 31, 4, 4);
  RoundRect(hDC, px + 17, py + 25, px + 26, py + 31, 4, 4);
  DeleteObject(hBoot);
  DeleteObject(hBootPen);

  HBRUSH hSuit = CreateSolidBrush(RGB(28, 96, 225));
  HPEN hSuitPen = CreatePen(PS_SOLID, 2, RGB(10, 35, 115));
  SelectObject(hDC, hSuit);
  SelectObject(hDC, hSuitPen);
  RoundRect(hDC, px + 7, py + 18, px + TILE_SIZE - 7, py + 29, 6, 6);
  DeleteObject(hSuit);
  DeleteObject(hSuitPen);

  HPEN hBelt = CreatePen(PS_SOLID, 2, RGB(255, 220, 80));
  SelectObject(hDC, hBelt);
  MoveToEx(hDC, px + 9, py + 22, NULL);
  LineTo(hDC, px + TILE_SIZE - 9, py + 22);
  DeleteObject(hBelt);

  HBRUSH hGlove = CreateSolidBrush(RGB(255, 255, 255));
  HPEN hGlovePen = CreatePen(PS_SOLID, 1, RGB(82, 100, 142));
  SelectObject(hDC, hGlove);
  SelectObject(hDC, hGlovePen);
  Ellipse(hDC, px + 1, py + 19, px + 11, py + 29);
  Ellipse(hDC, px + TILE_SIZE - 11, py + 19, px + TILE_SIZE - 1, py + 29);
  DeleteObject(hGlove);
  DeleteObject(hGlovePen);

  HBRUSH hHelmet = CreateSolidBrush(RGB(245, 248, 255));
  HPEN hHelmetPen = CreatePen(PS_SOLID, 2, RGB(45, 58, 95));
  SelectObject(hDC, hHelmet);
  SelectObject(hDC, hHelmetPen);
  RoundRect(hDC, px + 3, py + 1, px + TILE_SIZE - 3, py + 22, 14, 14);
  DeleteObject(hHelmet);
  DeleteObject(hHelmetPen);

  HBRUSH hCap = CreateSolidBrush(RGB(42, 116, 230));
  HPEN hCapPen = CreatePen(PS_SOLID, 1, RGB(10, 40, 120));
  SelectObject(hDC, hCap);
  SelectObject(hDC, hCapPen);
  Ellipse(hDC, px + 2, py + 9, px + 9, py + 20);
  Ellipse(hDC, px + TILE_SIZE - 9, py + 9, px + TILE_SIZE - 2, py + 20);
  DeleteObject(hCap);
  DeleteObject(hCapPen);

  HBRUSH hFace = CreateSolidBrush(RGB(255, 221, 181));
  HPEN hFacePen = CreatePen(PS_SOLID, 1, RGB(135, 83, 55));
  SelectObject(hDC, hFace);
  SelectObject(hDC, hFacePen);
  RoundRect(hDC, px + 8, py + 9, px + TILE_SIZE - 8, py + 24, 7, 7);
  DeleteObject(hFace);
  DeleteObject(hFacePen);

  HPEN hStripe = CreatePen(PS_SOLID, 2, RGB(42, 130, 255));
  SelectObject(hDC, hStripe);
  MoveToEx(hDC, cx, py + 3, NULL);
  LineTo(hDC, cx, py + 12);
  DeleteObject(hStripe);
  HPEN hShine = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
  SelectObject(hDC, hShine);
  Arc(hDC, px + 7, py + 3, px + TILE_SIZE - 7, py + 15, px + 8, py + 7, px + TILE_SIZE - 9, py + 4);
  DeleteObject(hShine);

  HBRUSH hEye = CreateSolidBrush(RGB(255, 255, 255));
  HBRUSH hPupil = CreateSolidBrush(RGB(10, 18, 45));
  SelectObject(hDC, hEye);
  Ellipse(hDC, px + 10, py + 13, px + 16, py + 19);
  Ellipse(hDC, px + TILE_SIZE - 16, py + 13, px + TILE_SIZE - 10, py + 19);
  SelectObject(hDC, hPupil);
  Ellipse(hDC, px + 12, py + 15, px + 15, py + 18);
  Ellipse(hDC, px + TILE_SIZE - 15, py + 15, px + TILE_SIZE - 12, py + 18);
  DeleteObject(hEye);
  DeleteObject(hPupil);

  HPEN hDir = CreatePen(PS_SOLID, 2, RGB(255, 220, 80));
  SelectObject(hDC, hDir);
  MoveToEx(hDC, cx, py + 5, NULL);
  LineTo(hDC, cx + _iFacingDX * 6, py + 5 + _iFacingDY * 6);
  DeleteObject(hDir);
}

/*
Amaç: Normal bombaları çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawBombs(HDC hDC)
{
  for (int i = 0; i < MAX_BOMBS; i++)
  {
    if (!_bombs[i].active)
      continue;

    int px = _bombs[i].col * TILE_SIZE;
    int py = _bombs[i].row * TILE_SIZE;
    BOOL blink = (_bombs[i].timer < 24) && (_bombs[i].timer % 4 < 2);

    HBRUSH hShadow = CreateSolidBrush(RGB(24, 24, 24));
    SelectObject(hDC, hShadow);
    Ellipse(hDC, px + 8, py + TILE_SIZE - 12, px + TILE_SIZE - 8, py + TILE_SIZE - 6);
    DeleteObject(hShadow);

    HBRUSH hBr = CreateSolidBrush(blink ? RGB(255, 90, 30) : RGB(42, 42, 42));
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
    SelectObject(hDC, hBr);
    SelectObject(hDC, hPen);
    Ellipse(hDC, px + 6, py + 6, px + TILE_SIZE - 6, py + TILE_SIZE - 6);
    DeleteObject(hBr);
    DeleteObject(hPen);

    HPEN hFuse = CreatePen(PS_SOLID, 2, RGB(255, 210, 0));
    SelectObject(hDC, hFuse);
    MoveToEx(hDC, px + TILE_SIZE / 2, py + 8, NULL);
    LineTo(hDC, px + TILE_SIZE / 2 + 5, py + 3);
    DeleteObject(hFuse);
  }
}

/*
Amaç: C4 bombalarını ve hareket durumlarını çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawStickyBombs(HDC hDC)
{
  for (int i = 0; i < MAX_STICKY_BOMBS; i++)
  {
    if (!_stickyBombs[i].active)
      continue;

    int px = _stickyBombs[i].col * TILE_SIZE;
    int py = _stickyBombs[i].row * TILE_SIZE;
    BOOL blink = (!_stickyBombs[i].moving && _stickyBombs[i].timer < 25) && (_stickyBombs[i].timer % 4 < 2);

    HBRUSH hBr = CreateSolidBrush(_stickyBombs[i].moving ? RGB(255, 150, 0) : (blink ? RGB(255, 230, 40) : RGB(255, 90, 20)));
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(120, 30, 0));
    SelectObject(hDC, hBr);
    SelectObject(hDC, hPen);
    RoundRect(hDC, px + 6, py + 9, px + TILE_SIZE - 6, py + TILE_SIZE - 9, 6, 6);
    DeleteObject(hBr);
    DeleteObject(hPen);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(0, 0, 0));
    RECT rc = { px, py, px + TILE_SIZE, py + TILE_SIZE };
    DrawText(hDC, "C4", 2, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  }
}

/*
Amaç: Ghost puller izlerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawGhostTrails(HDC hDC)
{
  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive || _enemies[i].type != ENEMY_GHOST)
      continue;

    for (int t = _enemies[i].trailCount - 1; t >= 0; t--)
    {
      if (t >= GHOST_TRAIL_LENGTH)
        continue;
      int c = _enemies[i].ghostTrailCol[t];
      int r = _enemies[i].ghostTrailRow[t];
      int px = c * TILE_SIZE;
      int py = r * TILE_SIZE;
      int margin = 5 + t * 2;
      if (margin > 14) margin = 14;

      HBRUSH hBr = CreateSolidBrush(RGB(max(45, 130 - t * 12), 50, max(90, 205 - t * 18)));
      HPEN hPen = CreatePen(PS_DOT, 1, RGB(220, 180, 255));
      SelectObject(hDC, hBr);
      SelectObject(hDC, hPen);
      RoundRect(hDC, px + margin, py + margin, px + TILE_SIZE - margin,
        py + TILE_SIZE - margin, 8, 8);
      DeleteObject(hBr);
      DeleteObject(hPen);
    }
  }
}

/*
Amaç: Tripwire anchor ve hatlarını çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawTripwires(HDC hDC)
{
  for (int i = 0; i < MAX_TRIPWIRES; i++)
  {
    if (!_tripwires[i].active)
      continue;

    int x1 = _tripwires[i].col1 * TILE_SIZE + TILE_SIZE / 2;
    int y1 = _tripwires[i].row1 * TILE_SIZE + TILE_SIZE / 2;
    int x2 = _tripwires[i].col2 * TILE_SIZE + TILE_SIZE / 2;
    int y2 = _tripwires[i].row2 * TILE_SIZE + TILE_SIZE / 2;

    HBRUSH hAnchor = CreateSolidBrush(RGB(180, 60, 255));
    HPEN hAnchorPen = CreatePen(PS_SOLID, 1, RGB(250, 210, 255));
    SelectObject(hDC, hAnchor);
    SelectObject(hDC, hAnchorPen);
    Ellipse(hDC, x1 - 5, y1 - 5, x1 + 5, y1 + 5);
    if (_tripwires[i].armed >= 2)
      Ellipse(hDC, x2 - 5, y2 - 5, x2 + 5, y2 + 5);
    DeleteObject(hAnchor);
    DeleteObject(hAnchorPen);

    if (_tripwires[i].armed >= 2)
    {
      HPEN hLine = CreatePen(PS_SOLID, 3, RGB(255, 80, 255));
      SelectObject(hDC, hLine);
      MoveToEx(hDC, x1, y1, NULL);
      LineTo(hDC, x2, y2);
      DeleteObject(hLine);
    }
  }
}

/*
Amaç: Aktif patlama hücrelerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawExplosions(HDC hDC)
{
  for (int i = 0; i < MAX_EXPLOSIONS; i++)
  {
    if (!_explosions[i].active)
      continue;

    int px = _explosions[i].col * TILE_SIZE;
    int py = _explosions[i].row * TILE_SIZE;
    int bright = 255 * _explosions[i].timer / EXPLOSION_TIMER;

    HBRUSH hOuter = CreateSolidBrush(RGB(255, bright, 0));
    HBRUSH hInner = CreateSolidBrush(RGB(255, 255, 180));
    RECT rcOuter = { px + 2, py + 2, px + TILE_SIZE - 2, py + TILE_SIZE - 2 };
    RECT rcInner = { px + 8, py + 8, px + TILE_SIZE - 8, py + TILE_SIZE - 8 };
    FillRect(hDC, &rcOuter, hOuter);
    FillRect(hDC, &rcInner, hInner);
    DeleteObject(hOuter);
    DeleteObject(hInner);
  }
}

/*
Amaç: Düşman tiplerini kendi siluet ve renkleriyle çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawEnemies(HDC hDC)
{
  for (int i = 0; i < _iNumEnemies; i++)
  {
    if (!_enemies[i].alive)
      continue;

    int px, py;
    GetEnemyDrawPosition(i, &px, &py);
    int cx = px + TILE_SIZE / 2;
    int cy = py + TILE_SIZE / 2;

    HBRUSH hShadow = CreateSolidBrush(RGB(5, 8, 16));
    SelectObject(hDC, hShadow);
    Ellipse(hDC, px + 6, py + TILE_SIZE - 8, px + TILE_SIZE - 6, py + TILE_SIZE - 2);
    DeleteObject(hShadow);

    if (_enemies[i].type == ENEMY_NORMAL)
    {

      HBRUSH hBody = CreateSolidBrush(RGB(235, 72, 88));
      HPEN hOut = CreatePen(PS_SOLID, 2, RGB(116, 24, 36));
      SelectObject(hDC, hBody);
      SelectObject(hDC, hOut);
      Ellipse(hDC, px + 5, py + 6, px + TILE_SIZE - 5, py + TILE_SIZE - 4);
      RoundRect(hDC, px + 7, py + 18, px + TILE_SIZE - 7, py + 29, 8, 8);
      DeleteObject(hBody);
      DeleteObject(hOut);
      HPEN hHi = CreatePen(PS_SOLID, 1, RGB(255, 210, 215));
      SelectObject(hDC, hHi);
      Arc(hDC, px + 8, py + 8, px + TILE_SIZE - 8, py + 21, px + 10, py + 12, px + 20, py + 8);
      DeleteObject(hHi);
    }
    else if (_enemies[i].type == ENEMY_RUNNER)
    {

      HBRUSH hBody = CreateSolidBrush(RGB(68, 218, 128));
      HPEN hOut = CreatePen(PS_SOLID, 2, RGB(20, 105, 55));
      SelectObject(hDC, hBody);
      SelectObject(hDC, hOut);
      RoundRect(hDC, px + 4, py + 7, px + TILE_SIZE - 4, py + 25, 16, 16);
      POINT beak[3] = { {px + TILE_SIZE - 5, py + 15}, {px + TILE_SIZE - 1, py + 18}, {px + TILE_SIZE - 5, py + 21} };
      Polygon(hDC, beak, 3);
      MoveToEx(hDC, px + 9, py + 25, NULL); LineTo(hDC, px + 4, py + 31);
      MoveToEx(hDC, px + 22, py + 25, NULL); LineTo(hDC, px + 28, py + 31);
      DeleteObject(hBody);
      DeleteObject(hOut);
    }
    else if (_enemies[i].type == ENEMY_HUNTER)
    {

      HBRUSH hBody = CreateSolidBrush(RGB(255, 150, 48));
      HPEN hOut = CreatePen(PS_SOLID, 2, RGB(128, 55, 8));
      SelectObject(hDC, hBody);
      SelectObject(hDC, hOut);
      RoundRect(hDC, px + 5, py + 6, px + TILE_SIZE - 5, py + 28, 8, 8);
      POINT hornL[3] = { {px + 8, py + 7}, {px + 12, py + 1}, {px + 15, py + 8} };
      POINT hornR[3] = { {px + TILE_SIZE - 8, py + 7}, {px + TILE_SIZE - 12, py + 1}, {px + TILE_SIZE - 15, py + 8} };
      Polygon(hDC, hornL, 3);
      Polygon(hDC, hornR, 3);
      DeleteObject(hBody);
      DeleteObject(hOut);
    }
    else if (_enemies[i].type == ENEMY_SPOOKY)
    {

      HBRUSH hBody = CreateSolidBrush(RGB(215, 224, 255));
      HPEN hOut = CreatePen(PS_SOLID, 2, RGB(75, 86, 165));
      SelectObject(hDC, hBody);
      SelectObject(hDC, hOut);
      Ellipse(hDC, px + 4, py + 4, px + TILE_SIZE - 4, py + 24);
      Rectangle(hDC, px + 5, py + 15, px + TILE_SIZE - 5, py + 27);
      POINT skirt[6] = {
        {px + 5, py + 27}, {px + 10, py + 31}, {px + 16, py + 27},
        {px + 22, py + 31}, {px + TILE_SIZE - 5, py + 27}, {px + 5, py + 27}
      };
      Polygon(hDC, skirt, 6);
      DeleteObject(hBody);
      DeleteObject(hOut);
    }
    else if (_enemies[i].type == ENEMY_GHOST)
    {

      HBRUSH hBody = CreateSolidBrush(RGB(150, 98, 230));
      HPEN hOut = CreatePen(PS_SOLID, 2, RGB(58, 31, 130));
      SelectObject(hDC, hBody);
      SelectObject(hDC, hOut);
      Ellipse(hDC, px + 3, py + 3, px + TILE_SIZE - 3, py + 26);
      Rectangle(hDC, px + 5, py + 15, px + TILE_SIZE - 5, py + 26);
      POINT tail[7] = {
        {px + 5, py + 26}, {px + 9, py + 31}, {px + 14, py + 26},
        {px + 19, py + 31}, {px + 24, py + 26}, {px + TILE_SIZE - 5, py + 31},
        {px + TILE_SIZE - 5, py + 26}
      };
      Polygon(hDC, tail, 7);
      DeleteObject(hBody);
      DeleteObject(hOut);

      HPEN hAura = CreatePen(PS_DOT, 1, RGB(230, 205, 255));
      HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
      SelectObject(hDC, hAura);
      SelectObject(hDC, hNull);
      Ellipse(hDC, px + 1, py + 1, px + TILE_SIZE - 1, py + TILE_SIZE - 1);
      DeleteObject(hAura);
    }

    
    HBRUSH hEye = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH hPupil = CreateSolidBrush(RGB(5, 8, 28));
    SelectObject(hDC, hEye);
    Ellipse(hDC, cx - 8, py + 11, cx - 2, py + 18);
    Ellipse(hDC, cx + 2, py + 11, cx + 8, py + 18);
    SelectObject(hDC, hPupil);
    Ellipse(hDC, cx - 6, py + 13, cx - 3, py + 16);
    Ellipse(hDC, cx + 3, py + 13, cx + 6, py + 16);
    DeleteObject(hEye);
    DeleteObject(hPupil);

    const char* label = "W";
    COLORREF textColor = RGB(255, 255, 255);
    if (_enemies[i].type == ENEMY_RUNNER) { label = "R"; textColor = RGB(10, 80, 40); }
    else if (_enemies[i].type == ENEMY_HUNTER) { label = "H"; textColor = RGB(80, 30, 0); }
    else if (_enemies[i].type == ENEMY_SPOOKY) { label = "S"; textColor = RGB(50, 60, 140); }
    else if (_enemies[i].type == ENEMY_GHOST) { label = "G"; textColor = RGB(235, 220, 255); }
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, textColor);
    RECT rc = { px + 1, py + 20, px + TILE_SIZE - 1, py + TILE_SIZE };
    DrawText(hDC, label, 1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  }
}

/*
Amaç: Skor, süre, level, can ve özellik bilgilerini gösterir.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawHUD(HDC hDC)
{
  int secLeft = max(0, _iLevelTimeLeft / 30);

  RECT top = { 0, 0, _pGame->GetWidth(), HUD_TOP };
  RECT bottom = { 0, _pGame->GetHeight() - HUD_BOTTOM, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH hTop = CreateSolidBrush(RGB(12, 18, 30));
  FillRect(hDC, &top, hTop);
  FillRect(hDC, &bottom, hTop);
  DeleteObject(hTop);

  SetBkMode(hDC, TRANSPARENT);

  HFONT hBigFont = CreateFont(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
  HFONT hSmallFont = CreateFont(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));

  HFONT hOldFont = (HFONT)SelectObject(hDC, hBigFont);
  TCHAR szTimer[64];
  wsprintf(szTimer, "TIME %02d", secLeft);
  SetTextColor(hDC, secLeft <= 20 ? RGB(255, 80, 80) : RGB(255, 235, 90));
  RECT rcTimer = { _pGame->GetWidth() / 2 - 85, 6, _pGame->GetWidth() / 2 + 85, 36 };
  DrawText(hDC, szTimer, -1, &rcTimer, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

  SelectObject(hDC, hSmallFont);
  TCHAR szLeft[180];
  TCHAR szRight[180];
  if (_bMineBonusLevel)
  {
    wsprintf(szLeft, "Lv:%d  BONUS HAMMER RUSH  Can:%d", _iLevel, _iNumLives);
    wsprintf(szRight, "Gold Crates:%d/%d", _iSafeCellsLeft, _iMineCount);
  }
  else
  {
    wsprintf(szLeft, "Lv:%d  Stage:%d/6  Score:%d  Can:%d",
      _iLevel, GetStageInCycle(), _iScore, _iNumLives);
    wsprintf(szRight, "Dusman:%d  Bomba:%d  Alev:%d",
      CountAliveEnemies(), _iMaxBombs, _iExplosionRange);
  }

  SetTextColor(hDC, RGB(210, 235, 255));
  RECT rcLeft = { 8, 38, _pGame->GetWidth() / 2 - 90, 60 };
  RECT rcRight = { _pGame->GetWidth() / 2 + 90, 38, _pGame->GetWidth() - 8, 60 };
  DrawText(hDC, szLeft, -1, &rcLeft, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
  DrawText(hDC, szRight, -1, &rcRight, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

  TCHAR szBuff[300] = "";
  if (_bMineBonusLevel)
  {
    wsprintf(szBuff, "SPACE:Cekic  Altin kasalari kir, bonus dusmanlari vur  Hedef:%d  ESC:Menu", _iSafeCellsLeft);
  }
  else
  {
    wsprintf(szBuff, "SPACE:Bomba  %s  %s  %s  T:Tron %s  Cikis:%s",
      _bRemoteUnlocked ? "R:Remote" : "R:L3",
      _bStickyUnlocked ? "C:C4" : "C:L4",
      _bTripwireUnlocked ? "V:Tripwire" : "V:L5",
      _bTrailEnabled ? "ON" : "OFF",
      !_exitGate.revealed ? "Gizli" : (CountAliveEnemies() <= 0 ? "Acik" : "Kilitli"));

    if (_iSpeedTimer > 0)
      wsprintf(szBuff + lstrlen(szBuff), "  SPD:%d", _iSpeedTimer / 30);
    if (_iRangeTimer > 0)
      wsprintf(szBuff + lstrlen(szBuff), "  FIRE:%d", _iRangeTimer / 30);
    if (_iWallPassTimer > 0)
      wsprintf(szBuff + lstrlen(szBuff), "  WALL:%d", _iWallPassTimer / 30);
    if (_bHasShield)
      wsprintf(szBuff + lstrlen(szBuff), "  SHIELD");
    if (_iLevel >= 5)
      wsprintf(szBuff + lstrlen(szBuff), "  Ghost aktif");
  }

  SetTextColor(hDC, RGB(150, 255, 180));
  RECT rcBottom = { 8, _pGame->GetHeight() - HUD_BOTTOM + 8, _pGame->GetWidth() - 8, _pGame->GetHeight() - 8 };
  DrawText(hDC, szBuff, -1, &rcBottom, DT_WORDBREAK | DT_LEFT | DT_VCENTER);

  SelectObject(hDC, hOldFont);
  DeleteObject(hBigFont);
  DeleteObject(hSmallFont);
}

/*
Amaç: Tek satır metni yatay merkezli olarak çizer.
Girdi: HDC hDC, int y, const char* text, int height, COLORREF color.
Çıktı: Yok.
*/
static void DrawCenteredTextLine(HDC hDC, int y, const char* text, int height, COLORREF color)
{
  RECT rc = { 0, y, _pGame->GetWidth(), y + height };
  SetBkMode(hDC, TRANSPARENT);
  SetTextColor(hDC, color);
  DrawTextA(hDC, text, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

/*
Amaç: Ana menüyü çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawMainMenu(HDC hDC)
{
  RECT rcAll = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH bg = CreateSolidBrush(RGB(8, 14, 28));
  FillRect(hDC, &rcAll, bg);
  DeleteObject(bg);

  HFONT hTitle = CreateFont(42, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
  HFONT hOld = (HFONT)SelectObject(hDC, hTitle);
  DrawCenteredTextLine(hDC, 48, "BOMBERMAN ULTIMATE", 54, RGB(255, 238, 112));
  SelectObject(hDC, hOld);
  DeleteObject(hTitle);
  DrawCenteredTextLine(hDC, 112, "Branch maze + A* enemies + tactical bombs", 28, RGB(160, 230, 255));

  if (_bLastRunGameOver)
    DrawCenteredTextLine(hDC, 158, "GAME OVER - Press ENTER to Start Again", 28, RGB(255, 110, 90));
  else
    DrawCenteredTextLine(hDC, 158, "Press ENTER to Start", 28, RGB(110, 255, 150));

  DrawCenteredTextLine(hDC, 205, "ENTER: Start     T: Test Mode     S: Scoreboard     C: Credits     ESC: Exit", 24, RGB(230, 230, 230));

  DrawCenteredTextLine(hDC, 270, "Controls", 26, RGB(255, 220, 120));
  DrawCenteredTextLine(hDC, 302, "Arrow Keys: Move     SPACE: Bomb     R: Remote Detonate", 24, RGB(210, 225, 245));
  DrawCenteredTextLine(hDC, 330, "C: Sticky C4     V: Tripwire     T: Tron Trail", 24, RGB(210, 225, 245));
  DrawCenteredTextLine(hDC, 390, "Goal: reveal the hidden door, defeat enemies, finish 5+1 levels.", 24, RGB(160, 255, 190));
}

/*
Amaç: Test level seçim ekranını çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawTestMenu(HDC hDC)
{
  RECT rcAll = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH bg = CreateSolidBrush(RGB(8, 16, 30));
  FillRect(hDC, &rcAll, bg);
  DeleteObject(bg);

  char line[160];
  DrawCenteredTextLine(hDC, 50, "TEST MODE", 46, RGB(255, 235, 120));
  DrawCenteredTextLine(hDC, 105, "Use this screen only for mechanic testing and demonstration.", 24, RGB(190, 225, 255));

  if (_iTestStartLevel == 6)
    wsprintfA(line, "Starting Stage: BONUS HAMMER RUSH");
  else
    wsprintfA(line, "Starting Level: %d  (Stage in 5+1 cycle: %d)", _iTestStartLevel, ((_iTestStartLevel - 1) % 6) + 1);
  DrawCenteredTextLine(hDC, 165, line, 32, RGB(130, 255, 170));

  DrawCenteredTextLine(hDC, 220, "LEFT / RIGHT: choose Level 1-5 or BONUS", 26, RGB(230, 230, 230));
  DrawCenteredTextLine(hDC, 252, "ENTER: start with all test abilities unlocked", 26, RGB(230, 230, 230));
  DrawCenteredTextLine(hDC, 284, "ESC: return to main menu", 26, RGB(230, 230, 230));

  DrawCenteredTextLine(hDC, 350, "Test abilities enabled at start:", 26, RGB(255, 220, 120));
  DrawCenteredTextLine(hDC, 382, "Remote detonation + Sticky C4 + Tripwire + Shield + Speed + Wall-pass", 23, RGB(210, 225, 245));
  DrawCenteredTextLine(hDC, 414, "Lives = 9, bombs = 3, fire range = 2. Normal gameplay mode is unchanged.", 23, RGB(210, 225, 245));
}

/*
Amaç: Proje ve ekip bilgilerini çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawCredits(HDC hDC)
{
  RECT rcAll = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH bg = CreateSolidBrush(RGB(8, 10, 22));
  FillRect(hDC, &rcAll, bg);
  DeleteObject(bg);

  DrawCenteredTextLine(hDC, 42, "CREDITS", 44, RGB(255, 235, 120));
  DrawCenteredTextLine(hDC, 96, "Dokuz Eylul University Faculty of Engineering", 24, RGB(210, 230, 255));
  DrawCenteredTextLine(hDC, 124, "Computer Engineering", 24, RGB(210, 230, 255));
  DrawCenteredTextLine(hDC, 152, "CME4407 Game Design", 24, RGB(210, 230, 255));
  DrawCenteredTextLine(hDC, 194, "Prepared By", 26, RGB(255, 210, 120));
  DrawCenteredTextLine(hDC, 232, "Yusuf Mutlu 2021510051", 24, RGB(235, 235, 235));
  DrawCenteredTextLine(hDC, 260, "Nazife Nur Temiz 2022510150", 24, RGB(235, 235, 235));
  DrawCenteredTextLine(hDC, 288, "Onur Vatansever 2022510157", 24, RGB(235, 235, 235));
  DrawCenteredTextLine(hDC, 316, "Abdulkadir Can 2021510020", 24, RGB(235, 235, 235));
  DrawCenteredTextLine(hDC, 344, "Oguzhan Karaman 2011510115", 24, RGB(235, 235, 235));
  DrawCenteredTextLine(hDC, _pGame->GetHeight() - 72, "Press ESC or ENTER to return to menu", 24, RGB(140, 255, 180));
}

/*
Amaç: Skor kaydı için isim giriş ekranını çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawNameEntry(HDC hDC)
{
  RECT rcAll = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH bg = CreateSolidBrush(RGB(8, 18, 26));
  FillRect(hDC, &rcAll, bg);
  DeleteObject(bg);

  DrawCenteredTextLine(hDC, 70, "RUN COMPLETE!", 48, RGB(255, 235, 120));
  char line[128];
  wsprintfA(line, "Final Score: %d    Completed Level: %d", _iScore, _iLevel);
  DrawCenteredTextLine(hDC, 135, line, 30, RGB(170, 240, 255));
  DrawCenteredTextLine(hDC, 205, "Enter your name for the scoreboard:", 28, RGB(235, 235, 235));

  RECT box = { _pGame->GetWidth() / 2 - 170, 250, _pGame->GetWidth() / 2 + 170, 300 };
  HBRUSH br = CreateSolidBrush(RGB(35, 45, 70));
  FillRect(hDC, &box, br);
  DeleteObject(br);
  FrameRect(hDC, &box, (HBRUSH)GetStockObject(WHITE_BRUSH));
  SetBkMode(hDC, TRANSPARENT);
  SetTextColor(hDC, RGB(255, 255, 255));
  DrawTextA(hDC, _szPlayerName, -1, &box, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

  DrawCenteredTextLine(hDC, 330, "A-Z / 0-9 to type, BACKSPACE to delete, ENTER to save", 24, RGB(180, 230, 180));
}

/*
Amaç: Scoreboard kayıtlarını çizer.
Girdi: HDC hDC.
Çıktı: Yok.
*/
void DrawScoreboard(HDC hDC)
{
  RECT rcAll = { 0, 0, _pGame->GetWidth(), _pGame->GetHeight() };
  HBRUSH bg = CreateSolidBrush(RGB(10, 12, 26));
  FillRect(hDC, &rcAll, bg);
  DeleteObject(bg);

  DrawCenteredTextLine(hDC, 50, "SCOREBOARD", 44, RGB(255, 235, 120));
  int y = 120;
  for (int i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    char line[180];
    if (_scoreValues[i] > 0)
      wsprintfA(line, "%d. %-16s  Score: %d  Level: %d", i + 1, _scoreNames[i], _scoreValues[i], _scoreLevels[i]);
    else
      wsprintfA(line, "%d. ---", i + 1);
    DrawCenteredTextLine(hDC, y, line, 30, RGB(225, 235, 245));
    y += 38;
  }
  DrawCenteredTextLine(hDC, _pGame->GetHeight() - 86, "ENTER: Start New Game     ESC: Menu", 24, RGB(140, 255, 180));
}

/*
Amaç: Menü, test, credits ve scoreboard tuşlarını işler.
Girdi: Yok.
Çıktı: Yok.
*/
// ============================================================
// BÖLÜM 11: Menü, İsim Girişi ve Scoreboard Kontrolleri
// Menü tuşları, isim yazma, skor yükleme/kaydetme işlemleri
// bu final bölümünde toplanmıştır.
// ============================================================
void HandleMenuKeys()
{
  if (_iActionCooldown > 0)
    _iActionCooldown--;

  if (GetAsyncKeyState(VK_RETURN) < 0)
  {
    if (_iGameState == STATE_CREDITS)
      GoToMainMenu();
    else if (_iGameState == STATE_TEST_MENU)
      NewTestGame();
    else
      NewGame();
    _iActionCooldown = 8;
    return;
  }

  if (_iActionCooldown > 0)
    return;

  if (_iGameState == STATE_TEST_MENU)
  {
    if (GetAsyncKeyState(VK_LEFT) < 0)
    {
      _iTestStartLevel--;
      if (_iTestStartLevel < 1) _iTestStartLevel = 6;
      _iActionCooldown = 8;
      return;
    }
    if (GetAsyncKeyState(VK_RIGHT) < 0)
    {
      _iTestStartLevel++;
      if (_iTestStartLevel > 6) _iTestStartLevel = 1;
      _iActionCooldown = 8;
      return;
    }
    if (GetAsyncKeyState(VK_RETURN) < 0)
    {
      NewTestGame();
      return;
    }
  }

  if (GetAsyncKeyState(VK_ESCAPE) < 0)
  {
    if (_iGameState == STATE_MENU)
      PostQuitMessage(0);
    else
      GoToMainMenu();
    _iActionCooldown = 10;
    return;
  }
  if (_iGameState == STATE_MENU && GetAsyncKeyState('T') < 0)
  {
    if (_iTestStartLevel < 1 || _iTestStartLevel > 6)
      _iTestStartLevel = 1;
    _iGameState = STATE_TEST_MENU;
    _iActionCooldown = 10;
    return;
  }

  if (_iGameState == STATE_MENU && GetAsyncKeyState('S') < 0)
  {
    _iGameState = STATE_SCOREBOARD;
    _iActionCooldown = 10;
    return;
  }
  if (_iGameState == STATE_MENU && GetAsyncKeyState('C') < 0)
  {
    _iGameState = STATE_CREDITS;
    _iActionCooldown = 10;
    return;
  }
}

/*
Amaç: Oyuncu adını alır ve skor kaydını tamamlar.
Girdi: Yok.
Çıktı: Yok.
*/
void HandleNameEntryKeys()
{
  if (_iActionCooldown > 0)
  {
    _iActionCooldown--;
    return;
  }

  if (GetAsyncKeyState(VK_RETURN) < 0)
  {
    if (_iNameLength <= 0)
    {
      lstrcpyA(_szPlayerName, "PLAYER");
      _iNameLength = 6;
    }
    AddScoreEntry(_szPlayerName, _iScore, _iLevel);
    SaveScoreboard();
    _iGameState = STATE_SCOREBOARD;
    _iActionCooldown = 12;
    return;
  }

  if (GetAsyncKeyState(VK_BACK) < 0)
  {
    if (_iNameLength > 0)
    {
      _iNameLength--;
      _szPlayerName[_iNameLength] = 0;
    }
    _iActionCooldown = 5;
    return;
  }

  for (int k = 'A'; k <= 'Z'; k++)
  {
    if (GetAsyncKeyState(k) < 0 && _iNameLength < MAX_PLAYER_NAME)
    {
      _szPlayerName[_iNameLength++] = (char)k;
      _szPlayerName[_iNameLength] = 0;
      _iActionCooldown = 5;
      return;
    }
  }
  for (int k = '0'; k <= '9'; k++)
  {
    if (GetAsyncKeyState(k) < 0 && _iNameLength < MAX_PLAYER_NAME)
    {
      _szPlayerName[_iNameLength++] = (char)k;
      _szPlayerName[_iNameLength] = 0;
      _iActionCooldown = 5;
      return;
    }
  }
}

/*
Amaç: Skor tablosu için oyuncu adını güvenli şekilde kopyalar.
Girdi: char* dest, int destSize, const char* src.
Çıktı: Yok.
*/
void SafeCopyPlayerName(char* dest, int destSize, const char* src)
{
  if (dest == NULL || destSize <= 0)
    return;

  if (src == NULL)
    src = "---";

  strncpy_s(dest, destSize, src, _TRUNCATE);
  dest[destSize - 1] = '\0';
}

/*
Amaç: Scoreboard dosyasını okur.
Girdi: Yok.
Çıktı: Yok.
*/
void LoadScoreboard()
{
  for (int i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    lstrcpyA(_scoreNames[i], "---");
    _scoreValues[i] = 0;
    _scoreLevels[i] = 0;
  }

  FILE* fp = fopen("scoreboard.txt", "r");
  if (!fp)
    return;

  for (int i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    char name[64];
    int score, level;
    if (fscanf(fp, "%16s %d %d", name, &score, &level) == 3)
    {
      SafeCopyPlayerName(_scoreNames[i], MAX_PLAYER_NAME + 1, name);
      _scoreValues[i] = score;
      _scoreLevels[i] = level;
    }
  }
  fclose(fp);
}

/*
Amaç: Scoreboard dosyasını yazar.
Girdi: Yok.
Çıktı: Yok.
*/
void SaveScoreboard()
{
  FILE* fp = fopen("scoreboard.txt", "w");
  if (!fp)
    return;

  for (int i = 0; i < MAX_SCORE_ENTRIES; i++)
    if (_scoreValues[i] > 0)
      fprintf(fp, "%s %d %d\n", _scoreNames[i], _scoreValues[i], _scoreLevels[i]);

  fclose(fp);
}

/*
Amaç: Yeni skor kaydını sıralı top listeye ekler.
Girdi: const char* name, int score, int level.
Çıktı: Yok.
*/
void AddScoreEntry(const char* name, int score, int level)
{
  int pos = MAX_SCORE_ENTRIES;
  for (int i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    if (score > _scoreValues[i])
    {
      pos = i;
      break;
    }
  }
  if (pos >= MAX_SCORE_ENTRIES)
    return;

  for (int i = MAX_SCORE_ENTRIES - 1; i > pos; i--)
  {
    lstrcpyA(_scoreNames[i], _scoreNames[i - 1]);
    _scoreValues[i] = _scoreValues[i - 1];
    _scoreLevels[i] = _scoreLevels[i - 1];
  }

  SafeCopyPlayerName(_scoreNames[pos], MAX_PLAYER_NAME + 1, name);
  _scoreValues[pos] = score;
  _scoreLevels[pos] = level;
}
