#include "KeyProc.h"
#include "CButton.h"

POINT g_mousePos = { 0,0 };

float g_mouseDeltaX = 0.0f;
float g_mouseDeltaY = 0.0f;

float g_mouseWheelDelta = 0.0f;

// 현재 눌림 여부
bool g_ActionPressed[(int)InputAction::E_END];
// 이번 프레임에 눌렸는지 여부
bool g_ActionTriggered[(int)InputAction::E_END];
// 이전 프레임 상태 저장
bool g_PrevAction[(int)InputAction::E_END];
// 액션에 연결된 실제 키
int g_KeyBinding[(int)InputAction::E_END];

// 기본 키 세팅
void InitKeyBinding()
{
	// 입력 상태 배열 초기화
	for (int i = 0; i < (int)InputAction::E_END; i++)
	{
		g_ActionPressed[i] = false;
		g_ActionTriggered[i] = false;
		g_PrevAction[i] = false;
		g_KeyBinding[i] = 0;
	}

	// 이동
	g_KeyBinding[(int)InputAction::E_MOVE_UP] = 'W';
	g_KeyBinding[(int)InputAction::E_MOVE_DOWN] = 'S';
	g_KeyBinding[(int)InputAction::E_MOVE_LEFT] = 'A';
	g_KeyBinding[(int)InputAction::E_MOVE_RIGHT] = 'D';

	// 다트 발사
	g_KeyBinding[(int)InputAction::E_FLY] = VK_LBUTTON;

	// 아이템
	g_KeyBinding[(int)InputAction::E_USEITEM] = VK_SPACE;
	g_KeyBinding[(int)InputAction::E_ITEM0] = '1';
	g_KeyBinding[(int)InputAction::E_ITEM1] = '2';
	g_KeyBinding[(int)InputAction::E_ITEM2] = '3';
	g_KeyBinding[(int)InputAction::E_ITEM3] = '4';
	g_KeyBinding[(int)InputAction::E_ITEM4] = '5';

	// 시점 / 조작 변경
	g_KeyBinding[(int)InputAction::E_VIEWPOINTCONVERSION] = 'Z';
	g_KeyBinding[(int)InputAction::E_CONTROL_CHANGE] = 'Q';

	// 마우스
	g_KeyBinding[(int)InputAction::E_MOUSE_LEFT] = VK_LBUTTON;
	g_KeyBinding[(int)InputAction::E_MOUSE_RIGHT] = VK_RBUTTON;

	// 일시정지
	g_KeyBinding[(int)InputAction::E_PAUSE] = VK_ESCAPE;
}

// 키 변경
void SetKeyBinding(InputAction action, int vk)
{
	g_KeyBinding[(int)action] = vk;
}

// 현재 키 반환
int GetKeyBinding(InputAction action)
{
	return g_KeyBinding[(int)action];
}

// 현재 눌린 상태
bool IsActionPressed(InputAction action)
{
	return g_ActionPressed[(int)action];
}

// 이번 프레임에 막 눌렸는지 여부
bool IsActionTriggered(InputAction action)
{
	return g_ActionTriggered[(int)action];
}

// 입력 처리
void KeyProc(HWND hwnd)
{
	// 창이 활성화 아닐 때 입력 무시
	if (GetForegroundWindow() != hwnd)
	{
		for (int i = 0; i < (int)InputAction::E_END; i++)
		{
			g_ActionPressed[i] = false;
			g_ActionTriggered[i] = false;
			g_PrevAction[i] = false;
		}

		g_mouseDeltaX = 0.0f;
		g_mouseDeltaY = 0.0f;
		return;
	}

	RECT rc;
	GetClientRect(hwnd, &rc);

	int clientW = rc.right - rc.left;
	int clientH = rc.bottom - rc.top;

	if (clientW > 0 && clientH > 0) { CButton::SetScreenSize(clientW, clientH); }

	// 이전 프레임 마우스 위치를 저장하기 위한 변수
	// static으로 선언한 이유는 함수가 끝나도 값을 유지하기 위함
	static POINT mouse = { 0,0 };

	POINT cur;		// 현재 프레임 커서 위치
	GetCursorPos(&cur);	// 현재 커서 위치를 "스크린 좌표 기준"으로 가져옴

	// 스크린 좌표 -> 현재 윈도우 내부 좌표로 변환
	ScreenToClient(hwnd, &cur);

	// 마우스 이동량 계산
	g_mouseDeltaX = (float)(cur.x - mouse.x);
	g_mouseDeltaY = (float)(cur.y - mouse.y);

	g_mousePos = CButton::ToVirtualPoint(cur);

	for (int i = 0; i < (int)InputAction::E_END; i++)
	{
		bool current = (GetAsyncKeyState(g_KeyBinding[i]) & 0x8000) != 0;

		g_ActionPressed[i] = current;

		// 이번 프레임에 막 눌렸는지
		g_ActionTriggered[i] = (current && !g_PrevAction[i]);

		// 다음 프레임용 저장
		g_PrevAction[i] = current;
	}

	mouse = cur;
}

void ResetMouseWheel()
{
	g_mouseWheelDelta = 0.0f;
}

wstring GetKeyBindingName(InputAction action)
{
	int vk = GetKeyBinding(action);

	// 마우스 키
	if (vk == VK_LBUTTON)	return L"Mouse Left";
	if (vk == VK_RBUTTON)	return L"Mouse Right";
	if (vk == VK_MBUTTON)	return L"Mouse Middle";

	// 특수 키
	if (vk == VK_SPACE)		return L"SpaceBar";
	if (vk == VK_ESCAPE)	return L"ESC";
	if (vk == VK_RETURN)	return L"Enter";
	if (vk == VK_TAB)		return L"Tab";
	if (vk == VK_SHIFT)		return L"Shift";
	if (vk == VK_CONTROL)	return L"Ctrl";
	if (vk == VK_MENU)		return L"Alt";

	// 방향키
	if (vk == VK_UP)		return L"Up";
	if (vk == VK_DOWN)		return L"Down";
	if (vk == VK_LEFT)		return L"Left";
	if (vk == VK_RIGHT)		return L"Right";

	// 알파벳 / 숫자
	if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
	{
		wstring result;
		result += (wchar_t)vk;
		return result;
	}

	// 그 외 키
	return L"Key";
}

void ResetInputState(HWND hwnd)
{
	// 현재 실제로 눌려있는 키 상태를 기준으로 prev 상태를 맞춘다.
	// 이렇게 해야 씬 전환 직후 "이미 누르고 있던 클릭"이 토글에서 처리하지 않는다.
	for (int i = 0; i < (int)InputAction::E_END; i++)
	{
		bool current = (GetAsyncKeyState(g_KeyBinding[i]) & 0x8000) != 0;

		g_ActionPressed[i] = current;
		g_ActionTriggered[i] = false;
		g_PrevAction[i] = current;
	}

	g_mouseDeltaX = 0.0f;
	g_mouseDeltaY = 0.0f;
	g_mouseWheelDelta = 0.0f;

	POINT cur;
	GetCursorPos(&cur);
	ScreenToClient(hwnd, &cur);

	g_mousePos = CButton::ToVirtualPoint(cur);
}

// 할당 키
// wsad - 다트위 위치를 위아래좌우로 
// 스페이스바 - 아이템 사용
// 1,2,3 - 아이템칸 번호
// ESC - 일시정지
// Z - 시점변환
// Q - 조작키 변환
// 아이템칸의 번호로 지정을하고 스페이스바로 아이템을사용한다.

// 위에 마우스관련해서 프레임단위로 계산을 하는데
// 프레임은 60FPS 기준으로 약 0.016초이기에
// 사람이 잠깐 눌렀다고 해도 컴퓨터기준으로는 이미 1초이상이 지나간다.

// GetAsyncKeyState()는 "두 가지 상태"를 동시에 담을수 있는 함수이다.

// 원본은 SHORT state = GetAsyncKeyState(key) <- key는 가상 키 코드(VK_)

// 비트의 의미
// 0x8000 (상위비트) - 현재 키가 눌려있는 상태라는 뜻
// 0x0001 (하위비트) - 최근에 키가 눌린 적이 있느지(이벤트성)

// 그러면 여기서는 드는 의문 우리는 예제를 통해서 0x80으로 Key입력을 했을때는 잘되는데 왜 마우스는 안되는가?
// 그건 뜻이 완전히 다르기 때문에 그러하다.

// 0x80  = 0000 0000 1000 0000
// 0x8000 = 1000 0000 0000 0000
// 보다시피 각각의 이진법은 전혀 다른뜻으로 접근을 하고 있다.

// GetAsyncKeyState는 SHORT(16비트)를 변환하여야하는데, 눌림 상태는 최상위 비트인(0x8000)에 들어있다.
// 따라서 0x80으로 체크하면 : 잘못된 비트를 검사하는 것과 같으며, 환경에 따라서는 동작이 불안정 또는 안 먹을수도 있다.

// 그러면 여기서 드는 의문
// LOWORD / HIWORD는 내가 말하는 상위비트 / 하위비트와 같은가?

// 완전히 다른 개념

// LOWORD / HIWORD는 "비트" 개념이 아니라
// 32비트 값을 16비트 단위(WORD)로 나누는 매크로이다.

// g_startMouse.x = LOWORD(lParam); 만약 이렇게 만든 코드에 대한 설명으로는
// lParam(32비트) 안에 LOWORD(하위 16비트)를 불러와서 

// 그걸 g_startMouse에 x값에 집어 넣는다하고 같은 설명이다.
// 이때 하위 16비트는 현재 마우스의 x좌표를 의미하지만 무조건은 아니다
// 메시지 성격에 따라서는 x좌표, 키보드 메시지에서는 반복 횟수 그리고 다른 메시지들에서도 전혀 다르게 쓰인다.
// 그점은 꼭 유의하면서 사용할것!!!