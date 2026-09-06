#include "WInProc.h"
#include "KeyProc.h"
#include "CGameManager.h"

extern CGameManager g_gameManager;

LRESULT WINAPI WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
// lParam안에는 여러정보가 한번에 압축되어서 들어있다.
{
	switch(msg)
	{
		case WM_CREATE:
		{
			g_gameManager.Init(hwnd);

			InitKeyBinding();

			break;
		}

		case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);

			g_mouseWheelDelta = (float)delta / 120.0f;

			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}

		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				g_gameManager.GetD3D().UpdateScreenSize();
			}
			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}