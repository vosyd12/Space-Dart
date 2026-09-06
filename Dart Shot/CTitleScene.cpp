#include "CTitleScene.h"
#include "KeyProc.h"
#include <windows.h>

extern POINT g_mousePos;

CTitleScene::CTitleScene()
{
	m_nextScene = GameScene::E_NONE;

	m_titleTimer = 0.0f;
	m_startFlyTimer = 0.0f;
	m_startFly = false;
}

CTitleScene::~CTitleScene() {}

void CTitleScene::Init(HWND hwnd, CD3D* d3d)
{
	CScene::Init(hwnd, d3d);

	m_startBtn.Init(300, 500, 300, 60, L"START");
	m_settingBtn.Init(300, 600, 300, 60, L"SETTING");
	m_exitBtn.Init(300, 700, 300, 60, L"EXIT");

	m_titleTimer = 0.0f;
	m_startFlyTimer = 0.0f;
	m_startFly = false;
}

void CTitleScene::Update(float dt)
{
	m_titleTimer += dt;

	POINT pt = g_mousePos;

	bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

	if (m_startFly)
	{
		// START를 누르면 로켓이 앞으로 날아가는 연출을 진행한다.
		m_startFlyTimer += dt;

		// 연출이 끝난 뒤 모드 선택 화면으로 이동한다.
		if (m_startFlyTimer >= 0.85f)
		{
			m_nextScene = GameScene::E_MODE_SELECT;
		}

		return;
	}

	m_startBtn.Update(pt, mouseDown);
	m_settingBtn.Update(pt, mouseDown);
	m_exitBtn.Update(pt, mouseDown);

	if (m_startBtn.IsClicked())
	{
		// 바로 화면 전환하지 않고 로켓 전진 애니메이션을 먼저 재생한다.
		m_startFly = true;
		m_startFlyTimer = 0.0f;
	}

	else if (m_settingBtn.IsClicked())
	{
		m_nextScene = GameScene::E_SETTING;
	}

	else if (m_exitBtn.IsClicked())
	{
		PostQuitMessage(0);
	}
}

void CTitleScene::Render(HDC hdc)
{
	if (!m_d3d) { return; }

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();
	ID3DXFont* font = m_d3d->GetUIFont();

	if (!device || !font) { return; }

	// 3D 타이틀 로고를 출력한다.
	m_d3d->RenderTitleLogo(m_titleTimer, m_startFly, m_startFlyTimer);

	if (!m_startFly)
	{
		m_startBtn.Render(device, font);
		m_settingBtn.Render(device, font);
		m_exitBtn.Render(device, font);
	}
}