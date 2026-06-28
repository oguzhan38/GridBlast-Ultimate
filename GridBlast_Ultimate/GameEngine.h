#pragma once

#include <windows.h>
#include <mmsystem.h>

typedef WORD JOYSTATE;
const JOYSTATE JOY_NONE  = 0x0000L;
const JOYSTATE JOY_LEFT  = 0x0001L;
const JOYSTATE JOY_RIGHT = 0x0002L;
const JOYSTATE JOY_UP    = 0x0004L;
const JOYSTATE JOY_DOWN  = 0x0008L;
const JOYSTATE JOY_FIRE1 = 0x0010L;
const JOYSTATE JOY_FIRE2 = 0x0020L;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL GameInitialize(HINSTANCE hInstance);
void GameStart(HWND hWindow);
void GameEnd();
void GameActivate(HWND hWindow);
void GameDeactivate(HWND hWindow);
void GamePaint(HDC hDC);
void GameRenderFrame(HWND hWindow);
void GameCycle();
void HandleKeys();
void MouseButtonDown(int x, int y, BOOL bLeft);
void MouseButtonUp(int x, int y, BOOL bLeft);
void MouseMove(int x, int y);
void HandleJoystick(JOYSTATE jsJoystickState);

class GameEngine
{
protected:
  static GameEngine* m_pGameEngine;
  HINSTANCE m_hInstance;
  HWND m_hWindow;
  TCHAR m_szWindowClass[32];
  TCHAR m_szTitle[32];
  WORD m_wIcon;
  WORD m_wSmallIcon;
  int m_iWidth;
  int m_iHeight;
  int m_iFrameDelay;
  BOOL m_bSleep;
  UINT m_uiJoystickID;
  RECT m_rcJoystickTrip;

public:
  GameEngine(HINSTANCE hInstance, LPTSTR szWindowClass, LPTSTR szTitle,
    WORD wIcon, WORD wSmallIcon, int iWidth = 640, int iHeight = 480);
  virtual ~GameEngine();

  static GameEngine* GetEngine()
  {
    return m_pGameEngine;
  }

  BOOL Initialize(int iCmdShow);
  LRESULT HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
  void ErrorQuit(LPTSTR szErrorMsg);
  BOOL InitJoystick();
  void CaptureJoystick();
  void ReleaseJoystick();
  void CheckJoystick();

  HINSTANCE GetInstance()
  {
    return m_hInstance;
  }

  HWND GetWindow()
  {
    return m_hWindow;
  }

  void SetWindow(HWND hWindow)
  {
    m_hWindow = hWindow;
  }

  LPTSTR GetTitle()
  {
    return m_szTitle;
  }

  WORD GetIcon()
  {
    return m_wIcon;
  }

  WORD GetSmallIcon()
  {
    return m_wSmallIcon;
  }

  int GetWidth()
  {
    return m_iWidth;
  }

  int GetHeight()
  {
    return m_iHeight;
  }

  int GetFrameDelay()
  {
    return m_iFrameDelay;
  }

  void SetFrameRate(int iFrameRate)
  {
    m_iFrameDelay = 1000 / iFrameRate;
  }

  BOOL GetSleep()
  {
    return m_bSleep;
  }

  void SetSleep(BOOL bSleep)
  {
    m_bSleep = bSleep;
  }
};
