#include "CSettingScene.h"
#include "GameSetting.h"
#include "CButton.h"
#include "KeyProc.h"

extern POINT g_mousePos;
extern GameScene g_SettingReturnScene;

void CSattingScene::Init(HWND hwnd, CD3D* d3d)
{
	CScene::Init(hwnd, d3d);

	m_currentTab = SettingTab::E_GAME;
	m_dragType = DragType::E_NONE;
	m_waitingKey = KeyBindTarget::E_NONE;

	m_waitInputReady = false;

	for (int i = 0; i < 256; i++)
	{
		m_bindPrevKey[i] = false;
	}

	// 전역 게임 설정값을 설정창 내부 값에 그대로 복사한다.
	// 설정창에서는 m_value를 수정하고, 실제 적용 시 g_GameSetting에도 동기화한다.
	m_value = g_GameSetting;

	m_slider.mouse = { 250, 400, 850, 430 };
	m_slider.lumi = { 250, 450, 850, 480 };
	m_slider.frame = { 250, 500, 850, 530 };
	m_slider.fov = { 250, 550, 850, 580 };
	m_slider.bgm = { 250, 600, 850, 630 };
	m_slider.sfx = { 250, 650, 850, 680 };

	m_button.gameTab.Init(80, 80, 170, 55, L"GAME");
	m_button.systemTab.Init(260, 80, 170, 55, L"SYSTEM");
	m_button.soundTab.Init(440, 80, 170, 55, L"SOUND");
	m_button.keyTab.Init(620, 80, 170, 55, L"KEY");

	m_button.fullScreen.Init(80, 700, 230, 80, L"FULLSCREEN : OFF");
	m_button.invertMouse.Init(335, 700, 230, 80, L"KEY INVERT : OFF");
	m_button.isEnglish.Init(590, 700, 230, 80, L"LANGUAGE : KOR");
	m_button.back.Init(350, 900, 200, 70, L"BACK");

	m_button.fullScreen.SetText(m_value.fullScreen ? L"FULLSCREEN : ON" : L"FULLSCREEN : OFF");
	m_button.invertMouse.SetText(m_value.invertMouse ? L"KEY INVERT : ON" : L"KEY INVERT : OFF");
	m_button.isEnglish.SetText(m_value.isEnglish ? L"LANGUAGE : ENG" : L"LANGUAGE : KOR");

	// 키 설정 2열 배치
	m_keyButton.up.Init(300, 180, 140, 45, L"");
	m_keyButton.down.Init(300, 235, 140, 45, L"");
	m_keyButton.left.Init(300, 290, 140, 45, L"");
	m_keyButton.right.Init(300, 345, 140, 45, L"");
	m_keyButton.useitem.Init(300, 400, 140, 45, L"");
	m_keyButton.pause.Init(300, 455, 140, 45, L"");
	m_keyButton.fly.Init(300, 510, 140, 45, L"");

	m_keyButton.usingitemkey[0].Init(700, 180, 140, 45, L"");
	m_keyButton.usingitemkey[1].Init(700, 235, 140, 45, L"");
	m_keyButton.usingitemkey[2].Init(700, 290, 140, 45, L"");
	m_keyButton.usingitemkey[3].Init(700, 345, 140, 45, L"");
	m_keyButton.usingitemkey[4].Init(700, 400, 140, 45, L"");
	m_keyButton.view.Init(700, 455, 140, 45, L"");
	m_keyButton.controlChange.Init(700, 510, 140, 45, L"");

	RefreshKeyButtonText();
}

void CSattingScene::Update(float dt)
{
	POINT pt = g_mousePos;

	bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	UpdateTabButtons(pt, mouseDown);

	// 키 입력 대기 중이면 다른 UI 입력은 전부 막는다.
	if (m_waitingKey != KeyBindTarget::E_NONE)
	{
		// 키 설정 버튼을 누른 그 마우스 클릭이 아직 유지 중이면
		// 이 클릭은 "설정할 키"로 받으면 안 된다.
		if (!m_waitInputReady)
		{
			bool leftHold = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			bool rightHold = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			bool middleHold = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

			// 마우스 버튼이 모두 떨어진 뒤부터 실제 키 입력 대기 시작
			if (!leftHold && !rightHold && !middleHold) { m_waitInputReady = true; }

			return;
		}

		int vk = GetPressedKey();

		if (vk != 0)
		{
			ApplyKeyBind(m_waitingKey, vk);
			RefreshKeyButtonText();

			m_waitingKey = KeyBindTarget::E_NONE;
			m_waitInputReady = false;
		}

		return;
	}

	switch (m_currentTab)
	{
		case SettingTab::E_GAME:
		case SettingTab::E_SYSTEM:
		{
			UpdateGameTab(pt, mouseDown);
			break;
		}

		case SettingTab::E_SOUND:
		{
			UpdateSoundTab(pt, mouseDown);
			break;
		}

		case SettingTab::E_KEY:
		{
			UpdateKeyTab(pt, mouseDown);
			break;
		}
	}
}

void CSattingScene::Render(HDC hdc)
{
	if (!m_d3d) return;

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();
	ID3DXFont* font = m_d3d->GetUIFont();

	if (!device || !font) return;

	RECT title = { 0, 20, 900, 70 };
	title = CButton::ScaleRect(title);
	CButton::DrawTextUI(font, L"SETTING", title, D3DCOLOR_XRGB(255, 255, 255));

	RenderTabs(device, font);

	switch (m_currentTab)
	{
		case SettingTab::E_GAME:
		case SettingTab::E_SYSTEM:
		{
			RenderGameTab(device, font);
			break;
		}

		case SettingTab::E_SOUND:
		{
			RenderSoundTab(device, font);
			break;
		}

		case SettingTab::E_KEY:
		{
			RenderKeyTab(device, font);
			break;
		}
	}

	m_button.back.Render(device, font);
}

void CSattingScene::UpdateTabButtons(POINT pt, bool mouseDown)
{
	m_button.gameTab.Update(pt, mouseDown);
	m_button.systemTab.Update(pt, mouseDown);
	m_button.soundTab.Update(pt, mouseDown);
	m_button.keyTab.Update(pt, mouseDown);
	m_button.back.Update(pt, mouseDown);

	if (m_button.gameTab.IsClicked())		m_currentTab = SettingTab::E_GAME;
	if (m_button.systemTab.IsClicked())		m_currentTab = SettingTab::E_SYSTEM;
	if (m_button.soundTab.IsClicked())		m_currentTab = SettingTab::E_SOUND;
	if (m_button.keyTab.IsClicked())		m_currentTab = SettingTab::E_KEY;

	if (m_button.back.IsClicked())
	{
		m_nextScene = g_SettingReturnScene;
	}
}

void CSattingScene::UpdateGameTab(POINT pt, bool mouseDown)
{
	m_button.fullScreen.Update(pt, mouseDown);
	m_button.invertMouse.Update(pt, mouseDown);
	m_button.isEnglish.Update(pt, mouseDown);

	if (m_button.fullScreen.IsClicked())
	{
		m_value.fullScreen = !m_value.fullScreen;
		g_GameSetting.fullScreen = m_value.fullScreen;

		m_button.fullScreen.SetText(m_value.fullScreen ? L"FULLSCREEN : ON" : L"FULLSCREEN : OFF");

		if (m_d3d) { m_d3d->SetFullscreen(g_GameSetting.fullScreen); }
	}

	if (m_button.invertMouse.IsClicked())
	{
		m_value.invertMouse = !m_value.invertMouse;
		g_GameSetting.invertMouse = m_value.invertMouse;

		m_button.invertMouse.SetText(m_value.invertMouse ? L"KEY INVERT : ON" : L"KEY INVERT : OFF");
	}

	if (m_button.isEnglish.IsClicked())
	{
		m_value.isEnglish = !m_value.isEnglish;
		g_GameSetting.isEnglish = m_value.isEnglish;

		m_button.isEnglish.SetText(m_value.isEnglish ? L"LANGUAGE : ENG" : L"LANGUAGE : KOR");
	}

	if (mouseDown && m_dragType == DragType::E_NONE)
	{
		if (IsMouseOver(m_slider.mouse, pt))		m_dragType = DragType::E_MOUSE;
		else if (IsMouseOver(m_slider.lumi, pt))	m_dragType = DragType::E_LUMI;
		else if (IsMouseOver(m_slider.frame, pt))	m_dragType = DragType::E_FRAME;
		else if (IsMouseOver(m_slider.fov, pt))		m_dragType = DragType::E_FOV;
	}

	if (mouseDown)
	{
		float ratio = 0.0f;

		switch (m_dragType)
		{
			case DragType::E_MOUSE:
			{
				m_value.mouseSensitivity = CalcRatio(m_slider.mouse, pt);
				g_GameSetting.mouseSensitivity = m_value.mouseSensitivity;
				break;
			}

			case DragType::E_LUMI:
			{
				m_value.luminosity = CalcRatio(m_slider.lumi, pt);
				g_GameSetting.luminosity = m_value.luminosity;
				break;
			}

			case DragType::E_FRAME:
			{
				m_value.frameIndex = CalcStepIndex(m_slider.frame, pt, 4);
				m_value.frameLimit = m_value.frameValues[m_value.frameIndex];

				g_GameSetting.frameIndex = m_value.frameIndex;
				g_GameSetting.frameLimit = m_value.frameLimit;
				break;
			}

			case DragType::E_FOV:
			{
				ratio = CalcRatio(m_slider.fov, pt);

				m_value.fov = 30.0f + (90.0f * ratio);
				g_GameSetting.fov = m_value.fov;

				if (m_d3d) { m_d3d->SetFOV(g_GameSetting.fov); }

				break;
			}
		}
	}
	else
	{
		m_dragType = DragType::E_NONE;
	}
}

void CSattingScene::UpdateSoundTab(POINT pt, bool mouseDown)
{
	if (mouseDown && m_dragType == DragType::E_NONE)
	{
		if (IsMouseOver(m_slider.bgm, pt))		{ m_dragType = DragType::E_BGM; }
		else if (IsMouseOver(m_slider.sfx, pt)) { m_dragType = DragType::E_SFX; }
	}

	if (mouseDown)
	{
		switch (m_dragType)
		{
			case DragType::E_BGM:
			{
				m_value.BGM = (int)(CalcRatio(m_slider.bgm, pt) * 100.0f);
				g_GameSetting.BGM = m_value.BGM;
				break;
			}

			case DragType::E_SFX:
			{
				m_value.SFX = (int)(CalcRatio(m_slider.sfx, pt) * 100.0f);
				g_GameSetting.SFX = m_value.SFX;
				break;
			}
		}
	}
	else { m_dragType = DragType::E_NONE; }
}

void CSattingScene::UpdateKeyTab(POINT pt, bool mouseDown)
{
	m_keyButton.up.Update(pt, mouseDown);
	m_keyButton.down.Update(pt, mouseDown);
	m_keyButton.left.Update(pt, mouseDown);
	m_keyButton.right.Update(pt, mouseDown);
	m_keyButton.useitem.Update(pt, mouseDown);

	for (int i = 0; i < 5; i++)
	{
		m_keyButton.usingitemkey[i].Update(pt, mouseDown);
	}

	m_keyButton.pause.Update(pt, mouseDown);
	m_keyButton.fly.Update(pt, mouseDown);
	m_keyButton.view.Update(pt, mouseDown);
	m_keyButton.controlChange.Update(pt, mouseDown);

	if (m_keyButton.up.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_UP;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.down.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_DOWN;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.left.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_LEFT;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.right.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_RIGHT;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.useitem.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USEITEM;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.usingitemkey[0].IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USINGITEMKEY0;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.usingitemkey[1].IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USINGITEMKEY1;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.usingitemkey[2].IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USINGITEMKEY2;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.usingitemkey[3].IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USINGITEMKEY3;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.usingitemkey[4].IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_USINGITEMKEY4;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.pause.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_PAUSE;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.fly.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_FLY;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.view.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_VIEW;
		m_waitInputReady = false;
		ClearBindInputState();
	}

	else if (m_keyButton.controlChange.IsClicked())
	{
		m_waitingKey = KeyBindTarget::E_CONTROLCHANGE;
		m_waitInputReady = false;
		ClearBindInputState();
	}
}

void CSattingScene::RenderTabs(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	m_button.gameTab.Render(device, font);
	m_button.systemTab.Render(device, font);
	m_button.soundTab.Render(device, font);
	m_button.keyTab.Render(device, font);
}

void CSattingScene::RenderGameTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	DrawLabel(font, L"KEY AIM SPEED", 60, 390, 170, 50);
	DrawLabel(font, L"LUMINOSITY", 60, 440, 170, 50);
	DrawLabel(font, L"FRAME", 60, 490, 170, 50);
	DrawLabel(font, L"FOV", 60, 540, 170, 50);

	CButton::DrawSlider(device, m_slider.mouse, m_value.mouseSensitivity);
	CButton::DrawSlider(device, m_slider.lumi, m_value.luminosity);
	CButton::DrawSlider(device, m_slider.frame, m_value.frameIndex / 3.0f);
	CButton::DrawSlider(device, m_slider.fov, (m_value.fov - 30.0f) / 90.0f);

	RECT frameText = { 720, 490, 890, 540 };
	frameText = CButton::ScaleRect(frameText);

	wchar_t frameBuf[64];
	wsprintfW(frameBuf, L"%d FPS", m_value.frameLimit);

	CButton::DrawTextUI(font, frameBuf, frameText, D3DCOLOR_XRGB(255, 255, 255));

	m_button.fullScreen.Render(device, font);
	m_button.invertMouse.Render(device, font);
	m_button.isEnglish.Render(device, font);
}

void CSattingScene::RenderSoundTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	DrawLabel(font, L"BGM", 60, 590, 170, 50);
	DrawLabel(font, L"SFX", 60, 640, 170, 50);

	CButton::DrawSlider(device, m_slider.bgm, m_value.BGM / 100.0f);
	CButton::DrawSlider(device, m_slider.sfx, m_value.SFX / 100.0f);
}

void CSattingScene::RenderKeyTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	// 왼쪽 열: 기본 조작 / 사용 / 정지 / 발사
	DrawKeyRow(font, L"UP", 90, 180, m_keyButton.up);
	DrawKeyRow(font, L"DOWN", 90, 235, m_keyButton.down);
	DrawKeyRow(font, L"LEFT", 90, 290, m_keyButton.left);
	DrawKeyRow(font, L"RIGHT", 90, 345, m_keyButton.right);
	DrawKeyRow(font, L"USE ITEM", 90, 400, m_keyButton.useitem);
	DrawKeyRow(font, L"PAUSE", 90, 455, m_keyButton.pause);
	DrawKeyRow(font, L"FLY", 90, 510, m_keyButton.fly);

	// 오른쪽 열: 아이템 슬롯 / 시점 / 조작 변경
	DrawKeyRow(font, L"ITEM SLOT 1", 480, 180, m_keyButton.usingitemkey[0]);
	DrawKeyRow(font, L"ITEM SLOT 2", 480, 235, m_keyButton.usingitemkey[1]);
	DrawKeyRow(font, L"ITEM SLOT 3", 480, 290, m_keyButton.usingitemkey[2]);
	DrawKeyRow(font, L"ITEM SLOT 4", 480, 345, m_keyButton.usingitemkey[3]);
	DrawKeyRow(font, L"ITEM SLOT 5", 480, 400, m_keyButton.usingitemkey[4]);
	DrawKeyRow(font, L"VIEW", 480, 455, m_keyButton.view);
	DrawKeyRow(font, L"CONTROLCHANGE", 480, 510, m_keyButton.controlChange);

	if (m_waitingKey != KeyBindTarget::E_NONE)
	{
		RECT wait = { 0, 820, 900, 870 };
		wait = CButton::ScaleRect(wait);

		CButton::DrawTextUI(font, L"PRESS ANY KEY...", wait, D3DCOLOR_XRGB(255, 230, 120));
	}
}

float CSattingScene::CalcRatio(const RECT& bar, POINT pt)
{
	float ratio = (pt.x - bar.left) / (float)(bar.right - bar.left);

	if (ratio < 0.0f) { ratio = 0.0f; }
	if (ratio > 1.0f) { ratio = 1.0f; }

	return ratio;
}

int CSattingScene::CalcStepIndex(const RECT& bar, POINT pt, int stepCount)
{
	float ratio = CalcRatio(bar, pt);

	int index = (int)(ratio * (stepCount - 1) + 0.5f);

	if (index < 0)			{ index = 0; }
	if (index >= stepCount) { index = stepCount - 1; }

	return index;
}

bool CSattingScene::IsMouseOver(const RECT& rc, POINT pt)
{ 
	return PtInRect(&rc, pt);
}

// 키 검사
int CSattingScene::GetPressedKey()
{
	for (int vk = 0; vk < 256; vk++)
	{
		bool current = (GetAsyncKeyState(vk) & 0x8000) != 0;

		if (current && !m_bindPrevKey[vk])
		{
			m_bindPrevKey[vk] = current;
			return vk;
		}

		m_bindPrevKey[vk] = current;
	}

	return 0;
}

wstring CSattingScene::KeyToString(int vk)
{
	if (vk == 0) { return L"NONE"; }

	if (vk >= 'A' && vk <= 'Z')
	{
		wchar_t buf[2] = { (wchar_t)vk, 0 };
		return buf;
	}

	if (vk >= '0' && vk <= '9')
	{
		wchar_t buf[2] = { (wchar_t)vk, 0 };
		return buf;
	}

	switch (vk)
	{
		case VK_SPACE:		return L"SPACE";
		case VK_LBUTTON:	return L"L-MOUSE";
		case VK_RBUTTON:	return L"R-MOUSE";
		case VK_MBUTTON:	return L"M-MOUSE";
		case VK_SHIFT:		return L"SHIFT";
		case VK_CONTROL:	return L"CTRL";
		case VK_MENU:		return L"ALT";
		case VK_ESCAPE:		return L"ESC";
	}

	return L"KEY";
}

void CSattingScene::ApplyKeyBind(KeyBindTarget target, int vk)
{
	for (int i = 0; i < (int)InputAction::E_END; i++)
	{
		if (GetKeyBinding((InputAction)i) == vk)
		{
			SetKeyBinding((InputAction)i, 0);
		}
	}

	switch (target)
	{
		case KeyBindTarget::E_UP:
		{
			SetKeyBinding(InputAction::E_MOVE_UP, vk);
			break;
		}

		case KeyBindTarget::E_DOWN:
		{
			SetKeyBinding(InputAction::E_MOVE_DOWN, vk);
			break;
		}

		case KeyBindTarget::E_LEFT:
		{
			SetKeyBinding(InputAction::E_MOVE_LEFT, vk);
			break;
		}

		case KeyBindTarget::E_RIGHT:
		{
			SetKeyBinding(InputAction::E_MOVE_RIGHT, vk);
			break;
		}

		case KeyBindTarget::E_USEITEM:
		{
			SetKeyBinding(InputAction::E_USEITEM, vk);
			break;
		}

		case KeyBindTarget::E_USINGITEMKEY0:
		{
			SetKeyBinding(InputAction::E_ITEM0, vk);
			break;
		}

		case KeyBindTarget::E_USINGITEMKEY1:
		{
			SetKeyBinding(InputAction::E_ITEM1, vk);
			break;
		}

		case KeyBindTarget::E_USINGITEMKEY2:
		{
			SetKeyBinding(InputAction::E_ITEM2, vk);
			break;
		}

		case KeyBindTarget::E_USINGITEMKEY3:
		{
			SetKeyBinding(InputAction::E_ITEM3, vk);
			break;
		}

		case KeyBindTarget::E_USINGITEMKEY4:
		{
			SetKeyBinding(InputAction::E_ITEM4, vk);
			break;
		}

		case KeyBindTarget::E_PAUSE:
		{
			SetKeyBinding(InputAction::E_PAUSE, vk);
			break;
		}

		case KeyBindTarget::E_FLY:
		{
			SetKeyBinding(InputAction::E_FLY, vk);
			break;
		}

		case KeyBindTarget::E_VIEW:
		{
			SetKeyBinding(InputAction::E_VIEWPOINTCONVERSION, vk);
			break;
		}

		case KeyBindTarget::E_CONTROLCHANGE:
		{
			SetKeyBinding(InputAction::E_CONTROL_CHANGE, vk);
			break;
		}
	}
}

void CSattingScene::RefreshKeyButtonText()
{
	m_keyButton.up.SetText(KeyToString(GetKeyBinding(InputAction::E_MOVE_UP)));
	m_keyButton.down.SetText(KeyToString(GetKeyBinding(InputAction::E_MOVE_DOWN)));
	m_keyButton.left.SetText(KeyToString(GetKeyBinding(InputAction::E_MOVE_LEFT)));
	m_keyButton.right.SetText(KeyToString(GetKeyBinding(InputAction::E_MOVE_RIGHT)));

	m_keyButton.useitem.SetText(KeyToString(GetKeyBinding(InputAction::E_USEITEM)));

	m_keyButton.usingitemkey[0].SetText(KeyToString(GetKeyBinding(InputAction::E_ITEM0)));
	m_keyButton.usingitemkey[1].SetText(KeyToString(GetKeyBinding(InputAction::E_ITEM1)));
	m_keyButton.usingitemkey[2].SetText(KeyToString(GetKeyBinding(InputAction::E_ITEM2)));
	m_keyButton.usingitemkey[3].SetText(KeyToString(GetKeyBinding(InputAction::E_ITEM3)));
	m_keyButton.usingitemkey[4].SetText(KeyToString(GetKeyBinding(InputAction::E_ITEM4)));

	m_keyButton.pause.SetText(KeyToString(GetKeyBinding(InputAction::E_PAUSE)));
	m_keyButton.fly.SetText(KeyToString(GetKeyBinding(InputAction::E_FLY)));
	m_keyButton.view.SetText(KeyToString(GetKeyBinding(InputAction::E_VIEWPOINTCONVERSION)));
	m_keyButton.controlChange.SetText(KeyToString(GetKeyBinding(InputAction::E_CONTROL_CHANGE)));
}

void CSattingScene::DrawLabel(ID3DXFont* font, const wstring& text, int x, int y, int width, int height)
{
	RECT rc = { x, y, x + width, y + height };
	rc = CButton::ScaleRect(rc);

	CButton::DrawTextUI(font, text, rc, D3DCOLOR_XRGB(255, 255, 255));
}

void CSattingScene::DrawKeyRow(ID3DXFont* font, const wstring& label, int x, int y, CButton& keyButton)
{
	// 라벨 좌표만 왼쪽에 추가.
	// 키 버튼 좌표는 Init에서 설정한 기존 좌표 그대로 사용한다.
	// x를 받아서 왼쪽/오른쪽 열 배치에 모두 사용할 수 있게 한다.
	// 긴 라벨이 잘리지 않도록 라벨 영역을 조금 넓게 잡는다.
	RECT labelRect = { x, y, x + 210, y + 45 };
	labelRect = CButton::ScaleRect(labelRect);

	CButton::DrawTextUI(font, label, labelRect, D3DCOLOR_XRGB(255, 255, 255));

	if (m_d3d) { keyButton.Render(m_d3d->GetDevice(), font); }
}

void CSattingScene::ClearBindInputState()
{
	for (int vk = 0; vk < 256; vk++)
	{
		m_bindPrevKey[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
	}
}

// 지금의 키 설정 흐름은
// 키 설정 버튼 클릭한 클릭은 무시
// 마우스 버튼을 뗌과 동시에 입력 상태 초기화
// 그 다음 새로 누른 키 / 마우스만 등록