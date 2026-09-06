#include "CPlayScene.h"

void CPlayScene::DrawPlayHUD(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	DrawScoreUI(device, font);
	DrawItemUI(device, font);
	DrawChanceUI(device, font);
	DrawEffectUI(device, font);

	m_settingButton.Render(device, font);
	DrawSettingGearIcon(device);

	m_effect.RenderTimer(font);
}

void CPlayScene::DrawScoreUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	RECT panel = { 30, 40, 255, 200 };
	DrawHUDPanel(device, panel, D3DCOLOR_ARGB(220, 0, 180, 255));

	wstring modeText = (g_GameSetting.type == GameType::E_ZERO_ONE) ? L"ZERO ONE" : L"COUNT UP";

	RECT title = { 30, 55, 255, 90 };
	CButton::DrawTextUI(font, modeText, CButton::ScaleRect(title), D3DCOLOR_XRGB(80, 220, 255));

	wchar_t buf[64];

	if (g_GameSetting.type == GameType::E_ZERO_ONE) { swprintf_s(buf, L"LEFT : %d", GetCurrentStatus().GetPoint()); }
	else { swprintf_s(buf, L"SCORE : %d", GetCurrentStatus().GetPoint()); }

	RECT score = { 30, 105, 255, 150 };
	CButton::DrawTextUI(font, buf, CButton::ScaleRect(score), D3DCOLOR_XRGB(255, 255, 255));
}

void CPlayScene::DrawItemUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	// 난이도별 실제 표시 슬롯 수
	int slotCount;

	if (g_GameSetting.step == Step::E_EASY)			{ slotCount = 5; }
	else if (g_GameSetting.step == Step::E_NOMAL)	{ slotCount = 4; }
	else											{ slotCount = 3; }

	// ITEM_SLOT_MAX를 넘지 않도록 방어
	if (slotCount > ITEM_SLOT_MAX) { slotCount = ITEM_SLOT_MAX; }

	// 슬롯 수에 맞춰 패널 크기 확장
	RECT panel = { 35, 825, 75 + (slotCount * 80), 970 };
	DrawHUDPanel(device, panel, D3DCOLOR_ARGB(220, 0, 180, 255));

	RECT title = { 35, 830, 340, 860 };
	CButton::DrawTextUI(
		font,
		L"ITEM",
		CButton::ScaleRect(title),
		D3DCOLOR_XRGB(80, 220, 255));

	// 현재 턴의 아이템 슬롯 사용
	CItem& item = GetCurrentItem();

	for (int i = 0; i < slotCount; i++)
	{
		RECT slot = { 65 + i * 80, 875, 120 + i * 80, 930 };

		D3DCOLOR color = (i == m_selectedItem) ? D3DCOLOR_ARGB(210, 80, 180, 255) : D3DCOLOR_ARGB(120, 10, 20, 35);

		CButton::DrawRect(device, CButton::ScaleRect(slot), color);

		RECT num = { slot.left, 855, slot.right, 880 };

		wchar_t buf[16];
		swprintf_s(buf, L"%d", i + 1);

		CButton::DrawTextUI(
			font,
			buf,
			CButton::ScaleRect(num),
			D3DCOLOR_XRGB(255, 255, 255));

		RECT iconRc = { slot.left + 5, slot.top + 5, slot.right - 5, slot.bottom - 5 };

		// ITEMCHARGE 슬롯머신 이펙트
		if (m_effect.IsPlaying() && m_effect.GetType() == EffectType::E_ITEMCHARGE && m_effect.IsItemChargeSlotActive(i))
		{
			ItemType current = m_effect.GetItemChargeDisplayItem(i);
			ItemType next = m_effect.GetItemChargeNextItem(i);
			float scrollRate = m_effect.GetItemChargeScrollRate(i);

			int iconHeight = iconRc.bottom - iconRc.top;
			int moveY = (int)((float)iconHeight * scrollRate);

			// current는 아래로 내려가고,
			// next는 위에서 내려온다.
			RECT currentRc = iconRc;
			OffsetRect(&currentRc, 0, moveY);

			RECT nextRc = iconRc;
			OffsetRect(&nextRc, 0, -iconHeight + moveY);


			if (current != ItemType::E_NONE)
			{
				item.DrawItemIcon(
					device,
					current,
					currentRc,
					D3DCOLOR_ARGB(255, 255, 255, 255));
			}

			if (next != ItemType::E_NONE && scrollRate > 0.0f && scrollRate < 1.0f)
			{
				item.DrawItemIcon(
					device,
					next,
					nextRc,
					D3DCOLOR_ARGB(210, 255, 255, 255));
			}

			m_effect.RenderItemChargeSlotEffect(
				device,
				font,
				i,
				slot);
		}

		else
		{
			if (item.GetSlotType(i) != ItemType::E_NONE)
			{
				item.DrawSlotIcon(device, i, iconRc);
			}
		}

		// STEAL 사용 시 슬롯 보라색 플래시.
		// 성공/실패와 상관없이 사용한 슬롯에서 터진다.
		if (m_effect.IsPlaying() && m_effect.GetType() == EffectType::E_STEAL && m_effect.GetStealSlotIndex() == i)
		{
			m_effect.RenderStealSlotEffect(
				device,
				font,
				slot);
		}

	}

	// STEAL 아이템을 선택 중이면 실패 확률 말풍선을 보여준다.
	if (m_selectedItem >= 0 && m_selectedItem < slotCount && item.GetSlotType(m_selectedItem) == ItemType::E_ITEM_STEAL)
	{
		int failRate = GetStealFailRate();

		RECT selectedSlot =
		{
			65 + m_selectedItem * 80,
			875,
			120 + m_selectedItem * 80,
			930
		};

		// 말풍선 위치.
		// 선택한 아이템 슬롯 바로 위에 작게 출력한다.
		RECT bubble =
		{
			selectedSlot.left - 28,
			selectedSlot.top - 52,
			selectedSlot.right + 28,
			selectedSlot.top - 14
		};

		RECT bubbleScaled = CButton::ScaleRect(bubble);

		CButton::DrawRect(
			device,
			bubbleScaled,
			D3DCOLOR_ARGB(210, 35, 10, 55));

		RECT lineTop = { bubble.left, bubble.top, bubble.right, bubble.top + 2 };
		RECT lineBottom = { bubble.left, bubble.bottom - 2, bubble.right, bubble.bottom };
		RECT lineLeft = { bubble.left, bubble.top, bubble.left + 2, bubble.bottom };
		RECT lineRight = { bubble.right - 2, bubble.top, bubble.right, bubble.bottom };

		CButton::DrawRect(device, CButton::ScaleRect(lineTop), D3DCOLOR_ARGB(230, 220, 120, 255));
		CButton::DrawRect(device, CButton::ScaleRect(lineBottom), D3DCOLOR_ARGB(230, 220, 120, 255));
		CButton::DrawRect(device, CButton::ScaleRect(lineLeft), D3DCOLOR_ARGB(230, 220, 120, 255));
		CButton::DrawRect(device, CButton::ScaleRect(lineRight), D3DCOLOR_ARGB(230, 220, 120, 255));

		wchar_t chanceText[32];
		swprintf_s(chanceText, L"FAIL %d%%", failRate);

		DrawTextEx(
			device,
			chanceText,
			bubble,
			D3DCOLOR_XRGB(245, 220, 255),
			16,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			false);
	}

	if (m_effect.IsPlaying() && m_effect.GetType() == EffectType::E_ITEMCHARGE)
	{
		m_effect.RenderItemChargeCenterEffect(device, font);
	}

	RECT guide = { 35, 930, 340, 965 };

	CButton::DrawTextUI(
		font,
		L"1,2,3 SELECT / SPACE USE",
		CButton::ScaleRect(guide),
		D3DCOLOR_XRGB(220, 240, 255));
}

void CPlayScene::DrawChanceUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	RECT panel = { 565, 845, 870, 970 };
	DrawHUDPanel(device, panel, D3DCOLOR_ARGB(230, 255, 160, 40));

	// ZERO ONE에서는 찬스가 남은 기회가 아니라 투척 횟수 카운트다.
	const wchar_t* titleText = L"CHANCE";

	if (g_GameSetting.type == GameType::E_ZERO_ONE) { titleText = L"THROW COUNT"; }

	RECT title = { 565, 820, 870, 855 };
	CButton::DrawTextUI(font, titleText, CButton::ScaleRect(title), D3DCOLOR_XRGB(255, 180, 60));

	int chance = GetCurrentStatus().GetChance();

	LPDIRECT3DTEXTURE9 dartTex = nullptr;

	if (m_currentTurn == E_PLAYER) { dartTex = m_d3d->GetTextureManager().LoadTexture(L"dart_pin_blue"); }
	else { dartTex = m_d3d->GetTextureManager().LoadTexture(L"dart_pin_red"); }

	for (int i = 0; i < chance; i++)
	{
		RECT dartIcon =
		{
			585 + i * 27,
			865,
			610 + i * 27,
			920
		};

		if (dartTex)
		{
			CButton::DrawTexture(
				device,
				CButton::ScaleRect(dartIcon),
				dartTex,
				D3DCOLOR_ARGB(255, 255, 255, 255));
		}
	}

	RECT remain = { 565, 925, 870, 965 };

	wchar_t buf[64];

	if (g_GameSetting.type == GameType::E_ZERO_ONE) { swprintf_s(buf, L"THROW COUNT : %d", chance); }
	else											{ swprintf_s(buf, L"LEFT CHANCE : %d", chance); }

	CButton::DrawTextUI(font, buf, CButton::ScaleRect(remain), D3DCOLOR_XRGB(255, 190, 80));
}

void CPlayScene::DrawHUDPanel(LPDIRECT3DDEVICE9 device, const RECT& rc, D3DCOLOR lineColor)
{
	RECT bg = CButton::ScaleRect(rc);

	CButton::DrawRect(
		device,
		bg,
		D3DCOLOR_ARGB(125, 5, 12, 25));

	RECT top = { rc.left, rc.top, rc.right, rc.top + 3 };
	RECT bottom = { rc.left, rc.bottom - 3, rc.right, rc.bottom };
	RECT left = { rc.left, rc.top, rc.left + 3, rc.bottom };
	RECT right = { rc.right - 3, rc.top, rc.right, rc.bottom };

	CButton::DrawRect(device, CButton::ScaleRect(top), lineColor);
	CButton::DrawRect(device, CButton::ScaleRect(bottom), lineColor);
	CButton::DrawRect(device, CButton::ScaleRect(left), lineColor);
	CButton::DrawRect(device, CButton::ScaleRect(right), lineColor);
}

void CPlayScene::DrawEffectUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	// 재생 중인 이펙트가 없으면 출력하지 않음
	if (!m_effect.IsPlaying()) { return; }

	EffectType type = m_effect.GetType();

	// 화면 중앙 출력 위치
	RECT centerRc = { 0, 250, 900, 750 };

	if (type == EffectType::E_HIT)
	{
		// 일반 점수 이펙트
		wchar_t text[64];
		swprintf_s(text, L"+%d", m_effect.GetDisplayScore());

		DrawTextEx(
			device,
			text,
			centerRc,
			D3DCOLOR_XRGB(120, 220, 255),
			72,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		return;
	}

	else if (type == EffectType::E_BULL)
	{
		// 불스아이 이펙트
		DrawTextEx(
			device,
			L"BULL!",
			centerRc,
			D3DCOLOR_XRGB(255, 220, 80),
			96,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		return;
	}

	else if (type == EffectType::E_MISS)
	{
		// 과녁을 맞추지 못했을 때 이펙트
		// 눈에 잘 띄도록 붉은색으로 출력
		DrawTextEx(
			device,
			L"MISS",
			centerRc,
			D3DCOLOR_XRGB(255, 60, 60),
			110,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		return;
	}

	else if (type == EffectType::E_EXCHANGE)
	{
		// 점수 교환 이펙트
		m_effect.RenderExchangeEffect(device);

		DrawTextEx(
			device,
			L"EXCHANGE!",
			centerRc,
			D3DCOLOR_XRGB(180, 240, 255),
			86,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		return;
	}

	else if (type == EffectType::E_POINT_UP || type == EffectType::E_POINT_DOWN)
	{
		// POINT_UP / POINT_DOWN:
		// 화면 중앙 슬롯머신 숫자 연출.
		// 숫자 회전, 중앙 플래시, 확대, 적용 대상 표시,
		// ZERO ONE PERFECT ZERO 잭팟 연출까지 CEffect 쪽에서 처리한다.
		m_effect.RenderPointSlotEffect(device, font);
		return;
	}

	else if (type == EffectType::E_STEAL)
	{
		if (m_effect.IsStealSuccess())
		{
			DrawTextEx(
				device,
				L"ITEM STEAL!",
				centerRc,
				D3DCOLOR_XRGB(220, 120, 255),
				86,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				true);

			RECT itemRc = { 0, 535, 900, 620 };

			const wchar_t* itemName = GetItemDisplayName(m_effect.GetStealItem());

			DrawTextEx(
				device,
				itemName,
				itemRc,
				D3DCOLOR_XRGB(240, 210, 255),
				48,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				true);
		}

		else
		{
			DrawTextEx(
				device,
				L"STEAL FAILED!",
				centerRc,
				D3DCOLOR_XRGB(255, 90, 150),
				86,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				true);

			RECT failRc = { 0, 535, 900, 620 };

			DrawTextEx(
				device,
				L"NO ITEM STOLEN",
				failRc,
				D3DCOLOR_XRGB(210, 180, 230),
				48,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				true);
		}

		return;
	}

	else if (type == EffectType::E_TARGET_SLOW || type == EffectType::E_TARGET_STOP)
	{
		// TARGET_SLOW / TARGET_STOP 전용 중앙 텍스처 이펙트

		// TARGET_SLOW:
		// target_slow_effect 텍스처 사용

		// TARGET_STOP:
		// target_stop_effect 텍스처 사용

		// RenderCenterTextureEffect() 내부에서
		// 1.5초 동안
		// Alpha 0 → 255 → 0
		// 형태로 페이드 인/아웃 처리된다.

		const wchar_t* textureName =
			(type == EffectType::E_TARGET_SLOW) ? L"target_slow_effect" : L"target_stop_effect";

		LPDIRECT3DTEXTURE9 texture = m_d3d->GetTextureManager().LoadTexture(textureName);

		m_effect.RenderCenterTextureEffect(device, texture);

		return;
	}

	else if (type == EffectType::E_SNEAK_PEEK)
	{
		// SNEAK PEEK:
		// 과녁이 실제로 움직일 방향을 미리 알려주는 이펙트.

		// 제목은 텍스트로 출력하고,
		// 방향 표시는 CEffect::RenderSneakPeekEffect()에서
		// Direct3D 정점으로 직접 그린다.

		// 방향은 m_Target.GetDirectionIndex()가 PlaySneakPeek()로 전달된 값이다.
		// 따라서 여기서는 방향을 다시 계산하지 않는다.

		RECT titleRc = { 0, 210, 900, 310 };

		DrawTextEx(
			device,
			L"SNEAK PEEK!",
			titleRc,
			D3DCOLOR_XRGB(100, 240, 255),
			72,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		m_effect.RenderSneakPeekEffect(device);
		return;
	}

	else if (type == EffectType::E_EXTRA_CHANCE)
	{
		// EXTRA_CHANCE:
		// 이번 투척은 찬스를 소모하지 않는다는 것을 보여주는 이펙트.
		// CEffect에서는 보호막 링을 그리고,
		// CPlayScene에서는 텍스트를 출력한다.

		m_effect.RenderExtraChanceEffect(device);

		float rate = m_effect.GetEffectRate();

		// 텍스트도 링과 같은 방식으로
		// 시작/끝은 흐리고 중간은 선명하게 만든다.
		float alphaRate = sinf(rate * D3DX_PI);
		alphaRate = Clamp(alphaRate, 0.0f, 1.0f);

		int alpha = (int)(255.0f * alphaRate);

		if (alphaRate > 0.08f && alpha < 170) { alpha = 170; }

		alpha = ClampInt(alpha, 0, 255);

		RECT titleRc = { 0, 275, 900, 370 };

		DrawTextEx(
			device,
			L"EXTRA CHANCE!",
			titleRc,
			D3DCOLOR_ARGB(alpha, 255, 230, 90),
			76,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		RECT subRc = { 0, 520, 900, 600 };

		DrawTextEx(
			device,
			L"NO COUNT",
			subRc,
			D3DCOLOR_ARGB(alpha, 120, 230, 255),
			54,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			true);

		return;
		}

	else
	{
		// 다른 아이템 이펙트들은 이후 추가될 예정
		return;
	}
}

void CPlayScene::DrawTextEx(LPDIRECT3DDEVICE9 device, const wchar_t* text, const RECT& rc, D3DCOLOR color, int fontSize, DWORD format, bool shadow)
{
	if (!device || !text) { return; }

	ID3DXFont* drawFont = nullptr;

	// 원하는 크기의 폰트 생성
	D3DXCreateFont(
		device,
		fontSize,
		0,
		FW_HEAVY,
		1,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"UI",
		&drawFont);

	if (!drawFont) { return; }

	// 가상 좌표 기준 RECT를 실제 화면 크기에 맞게 변환
	RECT scaledRc = CButton::ScaleRect(rc);

	// 그림자 출력 여부
	if (shadow)
	{
		RECT shadowRc = scaledRc;
		OffsetRect(&shadowRc, 8, 8);

		drawFont->DrawTextW(
			nullptr,
			text,
			-1,
			&shadowRc,
			format,
			D3DCOLOR_XRGB(0, 0, 0));
	}

	// 본문 출력
	drawFont->DrawTextW(
		nullptr,
		text,
		-1,
		&scaledRc,
		format,
		color);

	drawFont->Release();
}

void CPlayScene::DrawSettingGearIcon(LPDIRECT3DDEVICE9 device)
{
	if (!device) { return; }

	D3DCOLOR gearColor = D3DCOLOR_ARGB(235, 220, 240, 255);
	D3DCOLOR holeColor = D3DCOLOR_ARGB(230, 10, 20, 35);

	float cx = 857.0f;
	float cy = 47.0f;

	// 톱니 이빨 8개
	for (int i = 0; i < 8; i++)
	{
		float angle = (D3DX_PI * 2.0f * i) / 8.0f;

		float x = cx + cosf(angle) * 13.0f;
		float y = cy + sinf(angle) * 13.0f;

		RECT tooth =
		{
			(int)x - 3,
			(int)y - 3,
			(int)x + 3,
			(int)y + 3
		};

		CButton::DrawRect(device, CButton::ScaleRect(tooth), gearColor);
	}

	// 바깥 몸체를 여러 사각형으로 겹쳐 원형 느낌을 낸다.
	for (int i = 0; i < 7; i++)
	{
		RECT body =
		{
			(int)(cx - 9 + i),
			(int)(cy - 10),
			(int)(cx + 9 - i),
			(int)(cy + 10)
		};

		CButton::DrawRect(device, CButton::ScaleRect(body), gearColor);
	}

	// 가운데 구멍
	for (int i = 0; i < 3; i++)
	{
		RECT hole =
		{
			(int)(cx - 4 + i),
			(int)(cy - 4),
			(int)(cx + 4 - i),
			(int)(cy + 4)
		};

		CButton::DrawRect(device, CButton::ScaleRect(hole), holeColor);
	}
}