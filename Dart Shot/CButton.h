#ifndef CBUTTON_H
#define CBUTTON_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>

using namespace std;

// D3D UI

struct UIVERTEX
{
	FLOAT x, y, z, rhw;
	D3DCOLOR color;
	FLOAT u, v;
};

class CButton
{
	private:
		RECT m_rect;
		wstring m_text;		// 버튼 내부 문자열

		bool m_hover;				// 마우스가 버튼 위에 있는가
		bool m_pressed;				// 현재 누르고 있는가
		bool m_prevMouseDown;		// 이전 프레임 마우스 입력 상태
		bool m_clicked;				// 클릭 순간 감지

		D3DCOLOR m_normalColor;
		D3DCOLOR m_hoverColor;
		D3DCOLOR m_pressColor;

		LPDIRECT3DTEXTURE9 m_texture;

		static int m_baseWidth;
		static int m_baseHeight;
		static int m_screenWidth;
		static int m_screenHeight;

	public:
		CButton();

		void Init(int x, int y, int width, int height, const wstring& text);
		void Update(POINT mousePos, bool mouseDown);
		void Render(LPDIRECT3DDEVICE9 device, ID3DXFont* font);

		static RECT ScaleRect(const RECT& rc);

		static void SetScreenSize(int width, int height);
		static POINT ToVirtualPoint(POINT pt);

		bool IsClicked() const		{ return m_clicked; }
		const RECT& GetRect() const { return m_rect; }

		void SetText(const wstring& text)		{ m_text = text; }
		void SetTexture(LPDIRECT3DTEXTURE9 tex) { m_texture = tex; }

		static void DrawRect(LPDIRECT3DDEVICE9 device, const RECT& rc, D3DCOLOR color);
		static void DrawTexture(LPDIRECT3DDEVICE9 device, const RECT& rc, LPDIRECT3DTEXTURE9 texture, D3DCOLOR color = D3DCOLOR_ARGB(255,255,255,255));
		static void DrawTextUI(ID3DXFont* font, const wstring& text, const RECT& rc, D3DCOLOR color);
		static void DrawSlider(LPDIRECT3DDEVICE9 device, const RECT& bar, float ratio);
};

#endif

// 참고 사이트 : https://m.blog.naver.com/whdgkks12347/221453149409 , https://mawile.tistory.com/347 