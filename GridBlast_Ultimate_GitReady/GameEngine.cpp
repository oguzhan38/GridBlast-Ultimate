#include "GameEngine.h"

// ============================================================
// GAMEENGINE KATMANI
// Bu dosya dersteki Win32 oyun motorunun çekirdeğidir.
// grid-based bomb-maze mantığı burada yazılmaz; sadece pencere, mesaj döngüsü,
// frame zamanlaması ve input callback bağlantısı sağlanır.
// ============================================================

GameEngine* GameEngine::m_pGameEngine = NULL;

/*
Amaç: Win32 uygulamasını başlatır, motoru hazırlar ve ana frame döngüsünü çalıştırır.
Girdi: Windows instance bilgileri ve pencere gösterim komutu.
Çıktı: Program çıkış kodu.
*/
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
  MSG msg;
  static ULONGLONG ullTickTrigger = 0;
  ULONGLONG ullTickCount;

  if (GameInitialize(hInstance))
  {
    if (!GameEngine::GetEngine()->Initialize(iCmdShow))
      return FALSE;

    while (TRUE)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT)
          break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
        if (!GameEngine::GetEngine()->GetSleep())
        {
          ullTickCount = GetTickCount64();
          if (ullTickCount > ullTickTrigger)
          {
            ullTickTrigger = ullTickCount + (ULONGLONG)GameEngine::GetEngine()->GetFrameDelay();
            HandleKeys();
            GameEngine::GetEngine()->CheckJoystick();
            GameCycle();
          }
        }
      }
    }
    return (int)msg.wParam;
  }

  GameEnd();
  return TRUE;
}

/*
Amaç: Windows mesajlarını aktif GameEngine nesnesine yönlendirir.
Girdi: Pencere handle değeri, mesaj kodu ve mesaj parametreleri.
Çıktı: Mesaj işleme sonucu.
*/
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return GameEngine::GetEngine()->HandleEvent(hWindow, msg, wParam, lParam);
}

/*
Amaç: Oyun motoru nesnesini kurar ve pencere/çalışma zamanı ayarlarını saklar.
Girdi: Instance handle, pencere sınıfı, başlık, ikonlar ve mantıksal oyun boyutu.
Çıktı: Oluşturulmuş GameEngine nesnesi.
*/
GameEngine::GameEngine(HINSTANCE hInstance, LPTSTR szWindowClass, LPTSTR szTitle,
  WORD wIcon, WORD wSmallIcon, int iWidth, int iHeight)
{
  m_pGameEngine = this;
  m_hInstance = hInstance;
  m_hWindow = NULL;
  if (lstrlen(szWindowClass) > 0)
    lstrcpy(m_szWindowClass, (LPTSTR)szWindowClass);
  if (lstrlen(szTitle) > 0)
    lstrcpy(m_szTitle, (LPTSTR)szTitle);
  m_wIcon = wIcon;
  m_wSmallIcon = wSmallIcon;
  m_iWidth = iWidth;
  m_iHeight = iHeight;
  m_iFrameDelay = 50;
  m_bSleep = TRUE;
  m_uiJoystickID = 0;
}

/*
Amaç: GameEngine tarafından tutulan kaynakları serbest bırakır.
Girdi: Yok.
Çıktı: Yok.
*/
GameEngine::~GameEngine()
{
}

/*
Amaç: Pencere sınıfını kaydeder ve oyun penceresini oluşturur.
Girdi: WinMain üzerinden gelen pencere gösterim komutu.
Çıktı: Pencere başarıyla oluşturulduysa TRUE, aksi halde FALSE.
*/
BOOL GameEngine::Initialize(int iCmdShow)
{
  WNDCLASSEX wndclass;

  wndclass.cbSize = sizeof(wndclass);
  wndclass.style = 0;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = m_hInstance;
  wndclass.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetIcon()));
  wndclass.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(GetSmallIcon()));
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = m_szWindowClass;

  if (!RegisterClassEx(&wndclass))
    return FALSE;

  int iWindowWidth = m_iWidth + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
  int iWindowHeight = m_iHeight + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
    GetSystemMetrics(SM_CYCAPTION);
  int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2;
  int iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

  m_hWindow = CreateWindow(m_szWindowClass, m_szTitle,
    WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
    iXWindowPos, iYWindowPos, iWindowWidth, iWindowHeight,
    NULL, NULL, m_hInstance, NULL);
  if (!m_hWindow)
    return FALSE;

  ShowWindow(m_hWindow, iCmdShow);
  SetFocus(m_hWindow);
  SetSleep(FALSE);
  UpdateWindow(m_hWindow);
  return TRUE;
}

/*
Amaç: Oyun çalışma zamanı için gerekli Windows mesajlarını işler.
Girdi: Pencere handle değeri, mesaj kodu ve parametreleri.
Çıktı: Mesaj işleme sonucu.
*/
LRESULT GameEngine::HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_CREATE:
      SetWindow(hWindow);
      GameStart(hWindow);
      SetSleep(FALSE);
      return 0;

    case WM_SETFOCUS:
      SetSleep(FALSE);
      return 0;

    case WM_ACTIVATE:
      if (wParam != WA_INACTIVE)
      {
        GameActivate(hWindow);
        SetSleep(FALSE);
      }
      else
      {
        GameDeactivate(hWindow);
        SetSleep(TRUE);
      }
      return 0;

    case WM_KEYDOWN:
      SetSleep(FALSE);
      HandleKeys();
      return 0;

    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      BeginPaint(hWindow, &ps);
      GameRenderFrame(hWindow);
      EndPaint(hWindow, &ps);
      return 0;
    }

    case WM_SIZE:
      InvalidateRect(hWindow, NULL, FALSE);
      return 0;

    case WM_LBUTTONDOWN:
      MouseButtonDown(LOWORD(lParam), HIWORD(lParam), TRUE);
      return 0;

    case WM_LBUTTONUP:
      MouseButtonUp(LOWORD(lParam), HIWORD(lParam), TRUE);
      return 0;

    case WM_RBUTTONDOWN:
      MouseButtonDown(LOWORD(lParam), HIWORD(lParam), FALSE);
      return 0;

    case WM_RBUTTONUP:
      MouseButtonUp(LOWORD(lParam), HIWORD(lParam), FALSE);
      return 0;

    case WM_MOUSEMOVE:
      MouseMove(LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_DESTROY:
      GameEnd();
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWindow, msg, wParam, lParam);
}

/*
Purpose: Displays a critical error and requests application shutdown.
Input: Error message text.
Output: None.
*/
void GameEngine::ErrorQuit(LPTSTR szErrorMsg)
{
  MessageBox(GetWindow(), szErrorMsg, TEXT("Critical Error"), MB_OK | MB_ICONERROR);
  PostQuitMessage(0);
}

/*
Purpose: Initializes joystick bounds if a joystick is attached.
Input: None.
Output: TRUE if joystick capture data was prepared; otherwise FALSE.
*/
BOOL GameEngine::InitJoystick()
{
  UINT uiNumJoysticks;
  if ((uiNumJoysticks = joyGetNumDevs()) == 0)
    return FALSE;

  JOYINFO jiInfo;
  if (joyGetPos(JOYSTICKID1, &jiInfo) != JOYERR_UNPLUGGED)
    m_uiJoystickID = JOYSTICKID1;
  else
    return FALSE;

  JOYCAPS jcCaps;
  joyGetDevCaps(m_uiJoystickID, &jcCaps, sizeof(JOYCAPS));
  DWORD dwXCenter = ((DWORD)jcCaps.wXmin + jcCaps.wXmax) / 2;
  DWORD dwYCenter = ((DWORD)jcCaps.wYmin + jcCaps.wYmax) / 2;
  m_rcJoystickTrip.left = (jcCaps.wXmin + (WORD)dwXCenter) / 2;
  m_rcJoystickTrip.right = (jcCaps.wXmax + (WORD)dwXCenter) / 2;
  m_rcJoystickTrip.top = (jcCaps.wYmin + (WORD)dwYCenter) / 2;
  m_rcJoystickTrip.bottom = (jcCaps.wYmax + (WORD)dwYCenter) / 2;

  return TRUE;
}

/*
Purpose: Captures the initialized joystick for the game window.
Input: None.
Output: None.
*/
void GameEngine::CaptureJoystick()
{
  if (m_uiJoystickID == JOYSTICKID1)
    joySetCapture(m_hWindow, m_uiJoystickID, NULL, TRUE);
}

/*
Purpose: Releases joystick capture.
Input: None.
Output: None.
*/
void GameEngine::ReleaseJoystick()
{
  if (m_uiJoystickID == JOYSTICKID1)
    joyReleaseCapture(m_uiJoystickID);
}

/*
Purpose: Polls the joystick and forwards the compact joystick state to the game.
Input: None.
Output: None.
*/
void GameEngine::CheckJoystick()
{
  if (m_uiJoystickID == JOYSTICKID1)
  {
    JOYINFO jiInfo;
    JOYSTATE jsJoystickState = 0;
    if (joyGetPos(m_uiJoystickID, &jiInfo) == JOYERR_NOERROR)
    {
      if (jiInfo.wXpos < (WORD)m_rcJoystickTrip.left)
        jsJoystickState |= JOY_LEFT;
      else if (jiInfo.wXpos > (WORD)m_rcJoystickTrip.right)
        jsJoystickState |= JOY_RIGHT;

      if (jiInfo.wYpos < (WORD)m_rcJoystickTrip.top)
        jsJoystickState |= JOY_UP;
      else if (jiInfo.wYpos > (WORD)m_rcJoystickTrip.bottom)
        jsJoystickState |= JOY_DOWN;

      if (jiInfo.wButtons & JOY_BUTTON1)
        jsJoystickState |= JOY_FIRE1;
      if (jiInfo.wButtons & JOY_BUTTON2)
        jsJoystickState |= JOY_FIRE2;
    }

    HandleJoystick(jsJoystickState);
  }
}
