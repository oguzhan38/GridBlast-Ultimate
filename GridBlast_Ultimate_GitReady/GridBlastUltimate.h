

#pragma once

#include <windows.h>
#include "Resource.h"
#include "GameEngine.h"

// ============================================================
// SABİT DEĞERLER
// Harita ölçüsü, HUD yüksekliği, maksimum obje sayıları ve
// oyun durumları burada tanımlanır. Kod kontrolünde bu değerler
// oyunun ölçeklenebilirliğini ve zorluk sınırlarını açıklar.
// ============================================================

#define TILE_SIZE          32

#define MAZE_COLS          36
#define MAZE_ROWS          17
#define HUD_TOP            52
#define HUD_BOTTOM         52

#define MAX_ENEMIES        18
#define MAX_BOMBS          8
#define MAX_STICKY_BOMBS   3
#define MAX_TRIPWIRES      2
#define BOMB_TIMER         90
#define STICKY_TRAVEL_DELAY 7
#define EXPLOSION_TIMER    12
#define MAX_EXPLOSIONS     140
#define MAX_PORTAL_PAIRS   3
#define MAX_TRAIL_CELLS    10
#define GHOST_TRAIL_LENGTH  5
#define MAX_MINES          90
#define ACTION_COOLDOWN    8

#define STATE_MENU          0
#define STATE_PLAYING       1
#define STATE_NAME_ENTRY    2
#define STATE_SCOREBOARD    3
#define STATE_CREDITS       4
#define STATE_TEST_MENU     5
#define MAX_SCORE_ENTRIES   5
#define MAX_PLAYER_NAME     16

#define CELL_PATH           0
#define CELL_WALL           1
#define CELL_BREAKABLE      2
#define CELL_MINE_HIDDEN    3
#define CELL_MINE_REVEALED  4
#define CELL_PORTAL_A       10
#define CELL_PORTAL_B       11
#define CELL_PORTAL_C       12
#define CELL_EXIT           13

#define POWERUP_NONE        0
#define POWERUP_SPEED       1
#define POWERUP_RANGE       2
#define POWERUP_SHIELD      3
#define POWERUP_BOMB        4
#define POWERUP_WALLPASS    5
#define MAX_POWERUP_TYPES    5
#define MAX_POWERUPS        12

#define ENEMY_NORMAL        0
#define ENEMY_HUNTER        1
#define ENEMY_SPOOKY        2
#define ENEMY_GHOST         3
#define ENEMY_RUNNER        4

// Normal bomba verisi: grid konumu, patlama sayacı ve aktiflik durumu.
struct Bomb
{
  int col, row, timer;
  BOOL active;
};

// Sticky C4 verisi: hareket yönü, yolculuk sayacı ve yapıştıktan sonraki patlama süresi.
struct StickyBomb
{
  int col, row, timer;
  int dx, dy;
  int moveTimer;
  BOOL moving;
  BOOL active;
};

// Tripwire verisi: iki anchor nokta, kurulma gecikmesi ve aktiflik durumu.
struct Tripwire
{
  int col1, row1, col2, row2;
  int armed;
  int armDelay;
  BOOL active;
};

// Patlama hücresi: patlamanın hangi grid hücresinde kaç frame kalacağını tutar.
struct ExplosionCell
{
  int col, row, timer;
  BOOL active;
};

// Düşman verisi: konum, animasyon, tip, AI yönü ve Ghost için iz bilgisi.
struct Enemy
{
  int col, row, moveDelay, moveTimer;
  int prevCol, prevRow;
  int animTimer, animTotal;
  int type;
  int level;

  int dirX, dirY;

  int ghostTrailCol[GHOST_TRAIL_LENGTH];
  int ghostTrailRow[GHOST_TRAIL_LENGTH];
  int trailCount;
  BOOL alive;
};

// Portal çifti: aynı tipteki iki hücre arasında teleport geçişi sağlar.
struct Portal
{
  int col1, row1, col2, row2;
  int cellType;
  BOOL active;
};

// Power-up verisi: grid konumu, güç türü ve sahnede aktif olup olmadığı.
struct PowerUp
{
  int col, row, type;
  BOOL active;
};

// Tron izi hücresi: oyuncunun bıraktığı ve düşmanlara engel olan hücre.
struct TrailCell
{
  int col, row;
  BOOL active;
};

// Çıkış kapısı: kırılabilir duvar altında saklanan ve düşmanlar ölünce kullanılabilen hedef.
struct ExitGate
{
  int col, row;
  BOOL revealed;
};

// ============================================================
// GLOBAL OYUN DURUMU
// Ders motorunun callback yapısı global değişkenlerle çalıştığı için
// oyun state, maze, objeler, düşmanlar, skor ve bonus verileri burada
// tutulur.
// ============================================================

HINSTANCE         _hInstance;
GameEngine*       _pGame;
HDC               _hOffscreenDC;
HBITMAP           _hOffscreenBitmap;
int               _iNumLives, _iScore, _iLevel;
BOOL              _bGameOver;
int               _iPlayerCol, _iPlayerRow;
int               _iPlayerPrevCol, _iPlayerPrevRow;
int               _iPlayerAnimTimer, _iPlayerAnimTotal;
int               _iMoveDelay;
int               _iActionCooldown;
int               _iBreakableCount;
int               _iFacingDX, _iFacingDY;
int               _iLevelTime, _iLevelTimeLeft;
BOOL              _bRemoteUnlocked, _bStickyUnlocked, _bTripwireUnlocked;
BOOL              _bTestMode, _bTestUnlockAll;
int               _iTestStartLevel;
BOOL              _powerupSpawned[MAX_POWERUP_TYPES + 1];
int               _hiddenPowerupType[MAZE_COLS][MAZE_ROWS];

int               _maze[MAZE_COLS][MAZE_ROWS];
BOOL              _visited[MAZE_COLS][MAZE_ROWS];

Bomb              _bombs[MAX_BOMBS];
StickyBomb        _stickyBombs[MAX_STICKY_BOMBS];
Tripwire          _tripwires[MAX_TRIPWIRES];
int               _iMaxBombs;
int               _iExplosionRange;
ExplosionCell     _explosions[MAX_EXPLOSIONS];
Enemy             _enemies[MAX_ENEMIES];
int               _iNumEnemies;
Portal            _portals[MAX_PORTAL_PAIRS];
PowerUp           _powerups[MAX_POWERUPS];
ExitGate          _exitGate;

BOOL              _bPlayerInvincible;
int               _iInvincibleTimer;
BOOL              _bHasShield;
int               _iSpeedTimer;
int               _iRangeTimer;
int               _iWallPassTimer;
int               _iGhostPullCooldown;

BOOL              _bTrailEnabled;
TrailCell         _trail[MAX_TRAIL_CELLS];
int               _iTrailCount;

BOOL              _bMineBonusLevel;
BOOL              _mineGrid[MAZE_COLS][MAZE_ROWS];
BOOL              _mineRevealed[MAZE_COLS][MAZE_ROWS];
BOOL              _mineFlagged[MAZE_COLS][MAZE_ROWS];
int               _iMineCount;
int               _iSafeCellsLeft;
int               _iHammerSwingTimer;
int               _iHammerSwingCol, _iHammerSwingRow;
int               _iHammerSwingDX, _iHammerSwingDY;

int               _iGameState;
BOOL              _bLastRunGameOver;
char              _szPlayerName[MAX_PLAYER_NAME + 1];
int               _iNameLength;
char              _scoreNames[MAX_SCORE_ENTRIES][MAX_PLAYER_NAME + 1];
int               _scoreValues[MAX_SCORE_ENTRIES];
int               _scoreLevels[MAX_SCORE_ENTRIES];

// ============================================================
// FONKSİYON PROTOTİPLERİ
// Uygulama akışı, harita üretimi, AI, bomba sistemi, bonus, çizim
// ve skor işlemleri için kullanılan ana fonksiyonlar.
// Ayrıntılı amaç/girdi/çıktı açıklamaları .cpp dosyasında vardır.
// ============================================================

void NewGame();
void NewTestGame();
void GoToMainMenu();
void CompleteRun();
void AwardLevelClearBonus();
void NextLevel();
void GenerateMaze();
void GenerateBranchingMaze();
void CarveBranchMaze(int startCol, int startRow);
void OpenExtraBranches();
BOOL IsPathLikeCell(int col, int row);
void PlaceBreakableWalls();
void PlacePortals();
void PlaceEnemies();
void PlaceExitGate();
void PlaceHiddenPowerups();
void PlaceBomb();
void DetonateRemoteBombs();
void ThrowStickyBomb();
int CountActiveTripwires();
BOOL IsTripwireAnchorCellValid(int col, int row);
BOOL IsTripwirePathClear(int c1, int r1, int c2, int r2);
void PlaceTripwireAnchor();
void UpdateBombs();
void UpdateStickyBombs();
void UpdateTripwires();
void Explode(int col, int row);
void ExplodeTripwireCell(int col, int row);
BOOL AddExplosionCellSafe(int col, int row);
void ApplyExplosionDamageAtCell(int col, int row);
void TriggerBombChainAtCell(int col, int row);
void KillEnemyAtCell(int col, int row);
void CheckEnemyExplosionDamage();
void UpdateExplosions();
void UpdateEnemies();
BOOL FindNextStepAStar(int startCol, int startRow, int targetCol, int targetRow, int enemyType, int* nextCol, int* nextRow);
BOOL IsAStarWalkable(int col, int row, int enemyType);
void UpdateGhostPullEffects();
BOOL IsGhostTrailNearPlayer();
BOOL IsGhostTrailCell(int col, int row);
void DrawGhostTrails(HDC hDC);
void UpdatePowerUpTimers();
void CheckPlayerPickups();
void CheckPlayerDeath();
void DrawFloorBackground(HDC hDC);
void DrawMaze(HDC hDC);
void DrawExitGate(HDC hDC);
void DrawPortals(HDC hDC);
void DrawPowerUps(HDC hDC);
void DrawPlayer(HDC hDC);
void GetPlayerDrawPosition(int* px, int* py);
void GetEnemyDrawPosition(int index, int* px, int* py);
void DrawBombs(HDC hDC);
void DrawStickyBombs(HDC hDC);
void DrawTripwires(HDC hDC);
void DrawExplosions(HDC hDC);
void DrawEnemies(HDC hDC);
void DrawHUD(HDC hDC);
void DrawMainMenu(HDC hDC);
void DrawNameEntry(HDC hDC);
void DrawScoreboard(HDC hDC);
void DrawCredits(HDC hDC);
void DrawTestMenu(HDC hDC);
void HandleMenuKeys();
void HandleNameEntryKeys();
void SafeCopyPlayerName(char* dest, int destSize, const char* src);
void LoadScoreboard();
void SaveScoreboard();
void AddScoreEntry(const char* name, int score, int level);
void CheckBonusTargetHit(int col, int row);
void StartBackgroundMusic();
void StopBackgroundMusic();
void PlayEffect(int soundId);

void ToggleTrail();
void ClearTrail();
void AddTrailCell(int col, int row);
BOOL IsTrailCell(int col, int row);
void UpdateTrailEffects();
void DrawTrail(HDC hDC);

void StartBonusArenaLevel();
void GenerateBonusArena();
void HandleBonusArenaAction();
void SwingBonusHammer();
void PlaceBonusEnemies();
void ToggleMineFlag();
void RevealMineCell(int col, int row);
int CountAdjacentMines(int col, int row);
void DrawBonusArena(HDC hDC);
BOOL IsEnemyWalkable(int col, int row, int enemyType);
BOOL IsBombAt(int col, int row);
BOOL IsEnemyAtCell(int col, int row);
BOOL IsTripwireLineCell(int col, int row, int wireIndex);
int GetStageInCycle();
int CountAliveEnemies();
void KillPlayer();
