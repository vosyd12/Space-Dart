#include "CButton.h"

int CButton::m_baseWidth = 900;
int CButton::m_baseHeight = 1000;
int CButton::m_screenWidth = 900;
int CButton::m_screenHeight = 1000;

CButton::CButton()
{
	// RECT 구조체를 0으로 초기화
	ZeroMemory(&m_rect, sizeof(RECT));

	m_hover = false;				// 마우스가 버튼 위에 올라와 있는지 여부
	m_pressed = false;				// 버튼을 누르고 있는 상태인지 여부
	m_prevMouseDown = false;		// 이전 프레임에서 마우스가 눌려 있었는지 저장
	m_clicked = false;				// 이번 프레임에 클릭이 발생했는지 여부

	m_normalColor = D3DCOLOR_ARGB(150, 25, 25, 40);
	m_hoverColor = D3DCOLOR_ARGB(210, 60, 70, 110);
	m_pressColor = D3DCOLOR_ARGB(240, 120, 140, 220);

	m_texture = nullptr;
}

void CButton::Init(int x, int y, int width, int height, const wstring& text)
{
	// 버튼의 화면 좌표 설정
	m_rect.left = x;
	m_rect.top = y;
	m_rect.right = x + width;
	m_rect.bottom = y + height;

	// 버튼에 출력할 글자
	m_text = text;

	m_hover = false;
	m_pressed = false;
	m_prevMouseDown = false;
	m_clicked = false;
}

// 버튼 상태 갱신
void CButton::Update(POINT mousePos, bool mouseDown)
{
	// 현재 마우스가 버튼 사각형 안에 있는지 확인
	m_hover = PtInRect(&m_rect, mousePos);
	// 마우스가 버튼위에 있고, 마우스 버튼이 눌려있으면 pressed 상태
	m_pressed = m_hover && mouseDown;

	// 클릭 상태는 매 프레임 초기화한다.
	m_clicked = false;

	if (m_hover && mouseDown && !m_prevMouseDown) { m_clicked = true; }
	
	// 다음 프레임에서 비교하기 위해 현재 마우스 상태 저장
	m_prevMouseDown = mouseDown;
}

void CButton::Render(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	if (!device) return;

	D3DCOLOR color = m_normalColor;

	if (m_pressed)		color = m_pressColor;
	else if (m_hover)	color = m_hoverColor;

	RECT scaledRect = ScaleRect(m_rect);

	if (m_texture)		{ DrawTexture(device, scaledRect, m_texture, color); }
	else				{ DrawRect(device, scaledRect, color); }

	DrawTextUI(font, m_text, scaledRect, D3DCOLOR_XRGB(255, 255, 255));

	// UI 렌더 최종 상태 복구
	device->SetTexture(0, nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CButton::SetScreenSize(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}

RECT CButton::ScaleRect(const RECT& rc)
{
	float sx = (float)m_screenWidth / (float)m_baseWidth;
	float sy = (float)m_screenHeight / (float)m_baseHeight;

	RECT out;
	out.left = (LONG)(rc.left * sx);
	out.top = (LONG)(rc.top * sy);
	out.right = (LONG)(rc.right * sx);
	out.bottom = (LONG)(rc.bottom * sy);

	return out;
}

POINT CButton::ToVirtualPoint(POINT pt)
{
	float sx = (float)m_baseWidth / (float)m_screenWidth;
	float sy = (float)m_baseHeight / (float)m_screenHeight;

	POINT out;
	out.x = (LONG)(pt.x * sx);
	out.y = (LONG)(pt.y * sy);

	return out;
}

// D3D 사각형 출력 함수
// GDI의 Rectangle 역할을 Direct3D로 처리하는 함수
void CButton::DrawRect(LPDIRECT3DDEVICE9 device, const RECT& rc, D3DCOLOR color)
{
	if (!device) return;

	// 2D 화면 좌표용 저정 4개
	// rhw를 사용하는 이유:
	// 이미 화면 좌표계로 직접 그릴 것이기 때문에 월드 / 뷰 / 프로젝션 변환을 거치지 않기 위함
	UIVERTEX quad[4];

	quad[0] = { (float)rc.left,  (float)rc.top,    0.0f, 1.0f, color, 0.0f, 0.0f };
	quad[1] = { (float)rc.right, (float)rc.top,    0.0f, 1.0f, color, 0.0f, 0.0f };
	quad[2] = { (float)rc.left,  (float)rc.bottom, 0.0f, 1.0f, color, 0.0f, 0.0f };
	quad[3] = { (float)rc.right, (float)rc.bottom, 0.0f, 1.0f, color, 0.0f, 0.0f };

	// 텍스처 없이 색상만으로 그린다.
	device->SetTexture(0, nullptr);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	// 알파 블랜딩 활성화
	// 버튼을 반투명하게 그리기 위해서 필요
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	// 소스 색상의 알파값을 사용
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	// 배경색과 자연스럽게 섞이게 함
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// 사각형 출력
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(UIVERTEX));
}

void CButton::DrawTexture(LPDIRECT3DDEVICE9 device, const RECT& rc, LPDIRECT3DTEXTURE9 texture, D3DCOLOR color)
{
	if (!device) return;
	if (!texture) return;

	UIVERTEX quad[4];

	quad[0] = { (float)rc.left, (float)rc.top, 0.0f, 1.0f, color, 0.0f, 0.0f };
	quad[1] = { (float)rc.right, (float)rc.top, 0.0f, 1.0f, color, 1.0f, 0.0f };
	quad[2] = { (float)rc.left, (float)rc.bottom, 0.0f, 1.0f, color, 0.0f, 1.0f };
	quad[3] = { (float)rc.right, (float)rc.bottom, 0.0f, 1.0f, color, 1.0f, 1.0f };

	device->SetTexture(0, texture);

	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// 텍스처 색상 * 정점 색상.
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// 텍스처 알파 * 정점 알파.
	// 이 설정이 있어야 D3DCOLOR_ARGB(alpha, ...)의 alpha가 실제로 적용된다.
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(UIVERTEX));

	device->SetTexture(0, nullptr);
}

// D3D 텍스트 출력 함수
// ID3DXFont를 이용해서 문자열을 화면에 출력한다.
void CButton::DrawTextUI(ID3DXFont* font, const wstring& text, const RECT& rc, D3DCOLOR color)
{
	if (!font) return;
	if (text.empty()) return;

	RECT drawRc = rc;

	// DrawTexW는 RECT 영역 안에 문자열을 출력한다.
	// DT_CENTER : 가로 중앙 정렬
	// DT_VCENTER : 세로 중앙 정렬
	// DT_SINGLELINE : 한 줄 출력
	font->DrawTextW(
		nullptr,
		text.c_str(),
		-1,
		&drawRc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		color);
}

// 슬라이더 출력 함수
// bar 영역에 배경 바, 채워진 바, 손잡이(knob)를 출력한다.
void CButton::DrawSlider(LPDIRECT3DDEVICE9 device, const RECT& bar, float ratio)
{
	if (!device) return;

	// ratio는 0.0f ~ 1.0f 사이 값이어야한다
	// 그래서 안전용으로 강제로 보정한다.
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	RECT scaledBar = ScaleRect(bar);

	// 전체 슬라이더 배경 영역
	RECT back = scaledBar;
	// 슬라이더 배경 출력
	DrawRect(device, back, D3DCOLOR_ARGB(120, 25, 25, 35));

	// 실제 값만큼 채워지는 영역
	RECT fill = scaledBar;

	// ratio에 따라 fill의 오른쪽 끝 위치를 계산
	fill.right = fill.left + (int)((scaledBar.right - scaledBar.left) * ratio);

	// 슬라이더 채워진 부분 출력
	DrawRect(device, fill, D3DCOLOR_ARGB(220, 100, 180, 255));

	// 슬라이더 손잡이 위치 계산
	RECT knob;
	int knobX = fill.right;

	// 손잡이 현재 값 위치를 중심으로 작은 사각형으로 표시
	knob.left = knobX - 10;
	knob.right = knobX + 10;
	knob.top = scaledBar.top - 8;
	knob.bottom = scaledBar.bottom + 8;

	// 손잡이 출력
	DrawRect(device, knob, D3DCOLOR_ARGB(255, 230, 240, 255));
}