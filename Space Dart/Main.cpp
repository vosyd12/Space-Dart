#include <time.h>
#include "WinProc.h"
#include "KeyProc.h"
#include "CTarget.h"
#include "CDart.h"
#include "CCamera.h"
#include "CGameManager.h"
#include "GameSetting.h"

#define IDI_APP_ICON 101

CGameManager g_gameManager;

void GameLoop(HWND hwnd);
void ApplyWindowIconFromTextureManager(HWND hwnd);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{

	srand((unsigned int)time(NULL));

	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		WinProc,
		0L,
		0L,
		hInst,
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON)),
		NULL,
		NULL,
		NULL,
		L"Space Dart",
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON))
	};

	RegisterClassEx(&wc);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int windowX = (screenWidth - 900) / 2;
	int windowY = (screenHeight - 1000) / 2;


	HWND hwnd = CreateWindow(
		L"Space Dart",
		L"Space Dart",
		WS_OVERLAPPEDWINDOW,
		windowX,
		windowY,
		900,
		1000,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	ApplyWindowIconFromTextureManager(hwnd);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	MSG msg;

	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return 0;
			}
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		GameLoop(hwnd);
	}

	UnregisterClass(L"Space Dart", wc.hInstance);

	return 0;
}

void GameLoop(HWND hwnd)
{
	DWORD startTime = GetTickCount();

	KeyProc(hwnd);

	g_gameManager.Update();

	g_gameManager.Render(nullptr);

	// 프레임설정
	int frameLimit = g_GameSetting.frameLimit;

	if (frameLimit <= 0) { frameLimit = 60; }

	DWORD targetFrameTime = 1000 / frameLimit;
	DWORD elapsedTime = GetTickCount() - startTime;

	if (elapsedTime < targetFrameTime) { Sleep(targetFrameTime - elapsedTime); }
}

void ApplyWindowIconFromTextureManager(HWND hwnd)
{
	// GameManager/CD3D 초기화가 끝난 뒤에만 정상 동작한다.
	wstring iconPath = g_gameManager.GetD3D().GetTextureManager().GetFullPath(L"title");

	if (iconPath.empty()) { return; }

	HICON hIcon = (HICON)LoadImageW(
		NULL,
		iconPath.c_str(),
		IMAGE_ICON,
		0,
		0,
		LR_LOADFROMFILE | LR_DEFAULTSIZE);

	if (!hIcon) { return; }

	// 창 좌상단, Alt+Tab에 사용되는 큰 아이콘
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	// 작업표시줄, 작은 창 아이콘
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	// 같은 클래스의 기본 아이콘도 변경
	SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)hIcon);
	SetClassLongPtr(hwnd, GCLP_HICONSM, (LONG_PTR)hIcon);
}

// 정규화란 데이베이스상 이상현상(삽입|삭제|갱신)을 줄이고 데이터 무결성을 유지하기 위해서 테이블간의 중복된 데이터를 분해하여 저장공간을 효율화하는 설계 기법중에 하나이다.
// 장점으로는 데이터를 일관성이 강화되지만
// 단점으로는 테이블 분해로 인해 조회할때마다 연산처리가 많아져서 성능이 저하될수도있다.

// 간단한 스토리는 우주인에게 납치된 지구인이 지구의 운명을 건 대결인 쪽으로 
// 그러면 뒷 배경을 우주인것처럼 별표현하고
// 과녁뒤면을 행성처럼 그려서 행성을 과녁으로 표현하는 방법
// 그리고 다트라고 생각했지만 그게 우주선이였다는 설정까지 사용이 가능

// 기본적으로 폰트들이 너무 딱딱한 느낌이 들므로 방법을 강구해야함

// 참고사이트 : https://blog.naver.com/jsjhahi/206408464 (별 파티클 시스템)

// D3DCOLOR는 CPU → GPU로 색상 데이터를 넘길 때
// 추가 변환이 거의 필요 없는 Direct3D 전용 색상 포맷이다.

/* 화면 비율또는 마우스 커서 관련 함수들이 사용된 장소
GetClientRect = KeyProc(), CD3D::Init, CD3D::ResetDevice(), CD3D::UpdateScreenSize()
GetWindowRect = CD3D::Init, CD3D::SetFullscreen
SetWindowPos = CD3D::SetFullscreen
SetWindowLong = CD3D::SetFullscreen
MoveWindow = 해당없음
ShowWindow = WinMain
UpdateWindow = WinMain
ScreenToClient = KeyProc, 각 Scene::Update마다 선언
ClientToScreen = 해당없음
GetCursorPos = KeyProc, 각 Scene::Update마다 선언
GetSystemMetrics = WinMain, CD3D::SetFullscreen
D3DPRESENT_PARAMETERS = CD3D 멤버변수
BackBufferWidth = CD3D::ResetDevice(), CD3D::Init
BackBufferHeight = CD3D::ResetDevice(), CD3D::Init
SetViewport = CD3D::ResetDevice()
D3DXMatrixPerspectiveFovLH = CD3D::SetFOV
SetTransform = CCamera::Apply, 각 그리기 함수안에 존재
Reset = CD3D::ResetDevice()
OnLostDevice = CD3D::ResetDevice()
OnResetDevice = CD3D::ResetDevice()
*/


// 아이템에 따른 사용효과 및 각종이펙트
// 사운드 관련 
// 전용 커서 추가
// 아이콘을 로드 이미지로 출력해서 사용하는 방법

// DWORD :
// UINT : 

// 만약 AI로 이미지파일을 생성할경우 배경색이 단색으로 설정이 안된다면
// 그냥 배경색을 제거한 버전으로 설정해서 나온 오브젝트에서 손수 배경을 채워넣는 방법이 좀더 합리적임

// 게임화면을 사용하지 않을때도 다른 화면에서 조작한 키가 그대로 입력되는 버그