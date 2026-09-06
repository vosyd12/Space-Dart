#ifndef __KEYPROC_H__
#define __KEYPROC_H__

#include <windows.h>
#include <string>

using namespace std;

enum class InputAction
{
	E_MOVE_UP,				// 다트 위로움직임
	E_MOVE_DOWN,			// 다트 아래로움직임
	E_MOVE_LEFT,			// 다트 왼쪽으로 움직임
	E_MOVE_RIGHT,			// 다트 오른쪽으로 움직임

	E_FLY,					// 다트를 날린다.

	E_USEITEM,				// 아이템사용키
	E_ITEM0,				// 아이템 1번칸
	E_ITEM1,				// 아이템 2번칸
	E_ITEM2,				// 아이템 3번칸
	E_ITEM3,				// 아이템 4번칸
	E_ITEM4,				// 아이템 5번칸

	E_VIEWPOINTCONVERSION,	//시점변환 숄더뷰 -> 1인칭, 1인칭 -> 숄더뷰 - z키

	E_MOUSE_LEFT,			// 마우스 왼클릭 감지
	E_MOUSE_RIGHT,			// 마우스 오른클릭 감지

	E_PAUSE,				// 일시정지(ESC)
	E_CONTROL_CHANGE,		// Q키 - 다트 조작키 변경키

	E_END
};

extern POINT g_mousePos;

extern float g_mouseDeltaX;
extern float g_mouseDeltaY;

extern float g_mouseWheelDelta;

void KeyProc(HWND hwnd);

// 특정 행동이 눌려있는 상태
bool IsActionPressed(InputAction action);

// 특정 행동이 "이번 프레임에 막 눌림"
bool IsActionTriggered(InputAction action);

// 키 변경
void SetKeyBinding(InputAction action, int vk);

// 현재 키 가져오기
int GetKeyBinding(InputAction action);

// 기본 키 세팅
void InitKeyBinding();

void ResetMouseWheel();

void ResetInputState(HWND hwnd);

// 현재 액션에 연결된 키를 사람이 읽기 좋은 문자열로 반환
wstring GetKeyBindingName(InputAction action);

#endif