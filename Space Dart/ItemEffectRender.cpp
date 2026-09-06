#include "CEffect.h"
// EXCHANGE 아이템 전용 빛 폭발 이펙트.
void CEffect::RenderExchangeEffect(LPDIRECT3DDEVICE9 device)
{
	if (!device) { return; }

	float rate = GetEffectRate();

	// moveRate:
	// 이펙트의 이동 진행률.

	// rate를 그대로 쓰면 처음부터 끝까지 같은 속도로 퍼진다.
	// 그러면 움직임이 기계적으로 보인다.

	// 1 - (1 - rate)^3 형태를 쓰면
	// 초반에는 빠르게 터지고, 후반에는 서서히 멈추는 느낌이 난다.
	float moveRate = 1.0f - ((1.0f - rate) * (1.0f - rate) * (1.0f - rate));

	// alphaRate:
	// 시간이 지날수록 이펙트가 사라지게 만드는 값.
	// rate가 0이면 alphaRate는 1이라 가장 선명하고,
	// rate가 1이면 alphaRate는 0이라 완전히 사라진다.
	float alphaRate = 1.0f - rate;
	if (alphaRate < 0.0f) { alphaRate = 0.0f; }

	// DrawScoreUI의 점수판 위치 기준.
	// EXCHANGE는 점수 교환 아이템이므로 점수판 중앙에서 터지는 게 자연스럽다.
	float centerX = 142.0f;
	float centerY = 128.0f;

	// 기존 렌더 상태 저장.
	// DrawGlowCircle은 Additive Blend를 사용하므로,
	// 함수 종료 후 다른 UI 렌더링에 영향이 가지 않게 복구한다.
	DWORD oldFVF = 0;
	DWORD oldAlphaBlend = 0;
	DWORD oldSrcBlend = 0;
	DWORD oldDestBlend = 0;

	device->GetFVF(&oldFVF);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);

	// 중앙 플래시:
	// 아이템 사용 순간 점수판에서 빛이 번쩍 터지는 느낌.
	int flashAlpha = (int)(210.0f * alphaRate);

	if (flashAlpha > 0)
	{
		DrawGlowCircle(
			device,
			centerX,
			centerY,
			42.0f + rate * 18.0f,
			D3DCOLOR_ARGB(flashAlpha, 255, 255, 255),
			D3DCOLOR_ARGB(0, 80, 220, 255));

		DrawGlowCircle(
			device,
			centerX,
			centerY,
			20.0f + rate * 10.0f,
			D3DCOLOR_ARGB(flashAlpha, 255, 245, 180),
			D3DCOLOR_ARGB(0, 255, 180, 80));
	}

	// 사방으로 퍼지는 빛덩어리.
	for (int i = 0; i < 40; i++)
	{
		float indexRate = (float)i / 22.0f;

		// angle:
		// 빛덩어리가 퍼지는 방향.
		// 원형으로 고르게 배치하되, sin 흔들림을 조금 더해서
		// 너무 규칙적인 시계 모양이 되지 않게 한다.
		float angle = indexRate * D3DX_PI * 2.0f;
		angle += sinf((float)i * 1.73f + rate * 5.0f) * 0.22f;

		// distance:
		// 중심에서 얼마나 멀리 퍼질지.
		// moveRate가 커질수록 중심에서 바깥쪽으로 이동한다.
		float distance = 12.0f + moveRate * (68.0f + (float)((i % 5) * 9));

		float x = centerX + cosf(angle) * distance;
		float y = centerY + sinf(angle) * distance;

		// orbSize:
		// 빛덩어리 크기.
		// 바깥 Glow와 안쪽 Core를 따로 그려서
		// 부드러운 빛덩어리처럼 보이게 한다.
		float orbSize = 7.0f + (float)(i % 4) * 2.0f;
		orbSize += rate * 8.0f;

		int alpha = (int)(190.0f * alphaRate);
		alpha -= (i % 3) * 18;

		if (alpha < 0) { alpha = 0; }
		if (alpha > 190) { alpha = 190; }

		D3DCOLOR coreColor;
		D3DCOLOR edgeColor;

		if (i % 3 == 0)
		{
			coreColor = D3DCOLOR_ARGB(alpha, 255, 255, 255);
			edgeColor = D3DCOLOR_ARGB(0, 80, 220, 255);
		}

		else if (i % 3 == 1)
		{
			coreColor = D3DCOLOR_ARGB(alpha, 170, 240, 255);
			edgeColor = D3DCOLOR_ARGB(0, 40, 120, 255);
		}

		else
		{
			coreColor = D3DCOLOR_ARGB(alpha, 255, 235, 150);
			edgeColor = D3DCOLOR_ARGB(0, 255, 130, 40);
		}

		// 바깥쪽 흐린 빛.
		DrawGlowCircle(
			device,
			x,
			y,
			orbSize * 1.9f,
			D3DCOLOR_ARGB(alpha / 3, 180, 240, 255),
			edgeColor);

		// 안쪽 밝은 핵.
		DrawGlowCircle(
			device,
			x,
			y,
			orbSize * 0.75f,
			coreColor,
			D3DCOLOR_ARGB(0, 255, 255, 255));
	}

	// 짧은 빛 꼬리.
	// 빛덩어리 뒤쪽에 작은 Glow를 찍어 이동감을 만든다.
	for (int i = 0; i < 20; i++)
	{
		float indexRate = (float)i / 14.0f;

		float angle = indexRate * D3DX_PI * 2.0f;
		angle += cosf((float)i * 2.11f + rate * 4.0f) * 0.18f;

		float distance = 25.0f + moveRate * (80.0f + (float)((i % 4) * 7));

		float x = centerX + cosf(angle) * distance;
		float y = centerY + sinf(angle) * distance;

		float tailX = centerX + cosf(angle) * (distance - 14.0f);
		float tailY = centerY + sinf(angle) * (distance - 14.0f);

		int alpha = (int)(120.0f * alphaRate);

		if (alpha < 0) { alpha = 0; }
		if (alpha > 120) { alpha = 120; }

		DrawGlowCircle(
			device,
			tailX,
			tailY,
			5.0f + rate * 4.0f,
			D3DCOLOR_ARGB(alpha, 100, 220, 255),
			D3DCOLOR_ARGB(0, 50, 120, 255));

		DrawGlowCircle(
			device,
			x,
			y,
			4.0f + rate * 3.0f,
			D3DCOLOR_ARGB(alpha, 255, 245, 180),
			D3DCOLOR_ARGB(0, 255, 120, 40));
	}

	// 저장해둔 렌더 상태 복구.
	device->SetFVF(oldFVF);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetTexture(0, nullptr);
}

void CEffect::RenderPointSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	if (!device) { return; }
	if (!font) { return; }

	float rate = GetEffectRate();

	// isStopped:
	// 숫자가 최종값에 멈춘 상태인지 확인하는 값.
	// 이 값이 true가 되면 숫자를 노란색으로 고정해서 보여준다.
	bool isStopped = rate >= 0.70f;

	// stopRate:
	// 멈춘 뒤 결과를 강조하는 구간.
	// 0.86부터 바로 강조가 시작되어야 "멈췄다!"는 느낌이 강하다.
	float stopRate = 0.0f;

	if (rate > 0.66f)
	{
		stopRate = (rate - 0.70f) / 0.30f;
		stopRate = Clamp(stopRate, 0.0f, 1.0f);
	}

	// pop:
	// 멈추는 순간 숫자가 살짝 커지는 값.
	float pop = sinf(stopRate * D3DX_PI);

	// numberScale:
	// 숫자 확대 비율.
	// 기존보다 더 크게 보이게 하기 위해 기본 영역 자체도 아래에서 키우고,
	// 멈추는 순간 확대도 조금 더 강하게 준다.
	float numberScale = 1.0f + (pop * 0.38f);

	if (m_NumberSlot.jackpot)
	{
		numberScale = 1.0f + (pop * 0.60f);
	}

	float centerX = 465.0f;
	float centerY = 465.0f;

	D3DCOLOR titleColor;
	D3DCOLOR numberColor;
	D3DCOLOR targetColor;
	D3DCOLOR glowEdgeColor;

	if (m_NumberSlot.jackpot)
	{
		titleColor = D3DCOLOR_XRGB(255, 245, 160);
		numberColor = D3DCOLOR_XRGB(255, 255, 255);
		targetColor = D3DCOLOR_XRGB(120, 240, 255);
		glowEdgeColor = D3DCOLOR_ARGB(0, 255, 190, 60);
	}

	else if (m_NumberSlot.isPositive)
	{
		titleColor = D3DCOLOR_XRGB(120, 240, 255);
		numberColor = D3DCOLOR_XRGB(255, 230, 120);
		targetColor = D3DCOLOR_XRGB(180, 240, 255);
		glowEdgeColor = D3DCOLOR_ARGB(0, 80, 220, 255);
	}

	else
	{
		titleColor = D3DCOLOR_XRGB(255, 90, 140);
		numberColor = D3DCOLOR_XRGB(220, 130, 255);
		targetColor = D3DCOLOR_XRGB(255, 180, 220);
		glowEdgeColor = D3DCOLOR_ARGB(0, 255, 60, 130);
	}

	DWORD oldFVF = 0;
	DWORD oldAlphaBlend = 0;
	DWORD oldSrcBlend = 0;
	DWORD oldDestBlend = 0;

	device->GetFVF(&oldFVF);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);

	// 중앙 빛 플래시
	// 알파 페이드 없이 항상 선명하게 보이게 한다.
	int flashAlpha = 210 + (int)(stopRate * 45.0f);

	if (m_NumberSlot.jackpot) { flashAlpha = 255; }
	if (flashAlpha > 255) { flashAlpha = 255; }

	// 바깥 큰 Glow.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		145.0f + stopRate * 75.0f,
		D3DCOLOR_ARGB(flashAlpha / 2, 255, 255, 255),
		glowEdgeColor);

	// 중앙 강한 Glow.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		82.0f + stopRate * 50.0f,
		D3DCOLOR_ARGB(flashAlpha, 255, 245, 180),
		D3DCOLOR_ARGB(0, 255, 220, 80));

	// 숫자 바로 뒤 하이라이트.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		48.0f + stopRate * 30.0f,
		D3DCOLOR_ARGB(255, 255, 255, 255),
		D3DCOLOR_ARGB(0, 255, 245, 120));

	// 멈춘 순간 방사형 빛덩어리.
	if (isStopped)
	{
		for (int i = 0; i < 30; i++)
		{
			float angle = ((float)i / 16.0f) * D3DX_PI * 2.0f;
			float dist = 85.0f + stopRate * 65.0f;

			float x = centerX + cosf(angle) * dist;
			float y = centerY + sinf(angle) * dist;

			DrawGlowCircle(
				device,
				x,
				y,
				9.0f + stopRate * 8.0f,
				D3DCOLOR_ARGB(220, 255, 240, 120),
				D3DCOLOR_ARGB(0, 255, 120, 40));
		}
	}

	if (m_NumberSlot.jackpot)
	{
		DrawGlowCircle(
			device,
			centerX,
			centerY,
			280.0f + stopRate * 150.0f,
			D3DCOLOR_ARGB(220, 120, 240, 255),
			D3DCOLOR_ARGB(0, 255, 190, 40));
	}

	device->SetFVF(oldFVF);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetTexture(0, nullptr);

	wchar_t titleText[64] = L"";
	wchar_t targetText[64] = L"";

	if (m_NumberSlot.jackpot)					{ swprintf_s(titleText, L"PERFECT ZERO!"); }
	else if (m_Type == EffectType::E_POINT_UP)	{ swprintf_s(titleText, L"POINT UP"); }
	else										{ swprintf_s(titleText, L"POINT DOWN"); }

	if (m_NumberSlot.targetIsPlayer)	{ swprintf_s(targetText, L"TO YOU"); }
	else								{ swprintf_s(targetText, L"TO COMPUTER"); }

	RECT titleRc = CButton::ScaleRect({ 0, 205, 900, 310 });
	RECT targetRc = CButton::ScaleRect({ 0, 650, 900, 730 });

	RECT titleShadow = titleRc;
	OffsetRect(&titleShadow, 6, 6);

	font->DrawTextW(
		nullptr,
		titleText,
		-1,
		&titleShadow,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		D3DCOLOR_ARGB(255, 0, 0, 0));

	font->DrawTextW(
		nullptr,
		titleText,
		-1,
		&titleRc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		D3DCOLOR_ARGB(255, (titleColor >> 16) & 0xFF, (titleColor >> 8) & 0xFF, titleColor & 0xFF));

	// 숫자 릴 출력부 
	int baseY = 375;
	int digitHeight = (int)(155.0f * numberScale);

	int signLeft = 392;
	int tensLeft = 412;
	int onesLeft = 437;

	int signWidth = 70;
	int digitWidth = 76;

	RECT signRc =
	{
		signLeft,
		baseY,
		signLeft + signWidth,
		baseY + digitHeight
	};

	RECT tensRc =
	{
		tensLeft,
		baseY,
		tensLeft + digitWidth,
		baseY + digitHeight
	};

	RECT onesRc =
	{
		onesLeft,
		baseY,
		onesLeft + digitWidth,
		baseY + digitHeight
	};

	auto DrawOneText = [&](const wchar_t* text, RECT rc, D3DCOLOR color)
		{
			rc = CButton::ScaleRect(rc);

			RECT shadow = rc;
			OffsetRect(&shadow, 10, 10);

			font->DrawTextW(
				nullptr,
				text,
				-1,
				&shadow,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				D3DCOLOR_ARGB(255, 0, 0, 0));

			font->DrawTextW(
				nullptr,
				text,
				-1,
				&rc,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE,
				color);
		};

	auto DrawDigitReel = [&](int currentDigit, int nextDigit, float scrollRate, RECT rc, D3DCOLOR color)
		{
			wchar_t currentText[8];
			wchar_t nextText[8];

			swprintf_s(currentText, L"%d", currentDigit);
			swprintf_s(nextText, L"%d", nextDigit);

			int moveY = (int)((float)digitHeight * scrollRate);

			// currentRc:
			// 현재 중앙에 있던 숫자.
			// scrollRate가 증가할수록 아래로 내려간다.
			RECT currentRc = rc;
			OffsetRect(&currentRc, 0, moveY);

			// nextRc:
			// 다음 숫자.
			// 처음에는 위쪽에 있다가 scrollRate가 증가하면 중앙으로 내려온다.
			RECT nextRc = rc;
			OffsetRect(&nextRc, 0, -digitHeight + moveY);

			DrawOneText(currentText, currentRc, color);

			// 멈춘 상태에서는 next 숫자를 굳이 출력하지 않는다.
			// 멈춘 뒤에는 최종 숫자 하나가 또렷하게 보여야 하기 때문이다.
			if (scrollRate > 0.0f && scrollRate < 1.0f)
			{
				DrawOneText(nextText, nextRc, color);
			}
		};

	D3DCOLOR drawNumberColor = D3DCOLOR_ARGB(255, (numberColor >> 16) & 0xFF, (numberColor >> 8) & 0xFF, numberColor & 0xFF);

	wchar_t signText[4];

	if (m_NumberSlot.isPositive)	{ swprintf_s(signText, L"+"); }
	else							{ swprintf_s(signText, L"-"); }

	DrawOneText(signText, signRc, drawNumberColor);

	DrawDigitReel(
		m_NumberSlot.tensDisplay,
		m_NumberSlot.tensNext,
		m_NumberSlot.tensScrollRate,
		tensRc,
		drawNumberColor);

	DrawDigitReel(
		m_NumberSlot.onesDisplay,
		m_NumberSlot.onesNext,
		m_NumberSlot.onesScrollRate,
		onesRc,
		drawNumberColor);

	font->DrawTextW(
		nullptr,
		targetText,
		-1,
		&targetRc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		D3DCOLOR_ARGB(255, (targetColor >> 16) & 0xFF, (targetColor >> 8) & 0xFF, targetColor & 0xFF));
}

bool CEffect::IsItemChargeSlotActive(int index) const
{
	if (index < 0 || index >= ITEM_EFFECT_SLOT_MAX) { return false; }
	return m_ItemChargeSlot.active[index];
}

bool CEffect::IsItemChargeJackpotSlot(int index) const
{
	if (!m_ItemChargeSlot.jackpot) { return false; }

	return index >= m_ItemChargeSlot.jackpotStartIndex &&
		index < m_ItemChargeSlot.jackpotStartIndex + m_ItemChargeSlot.jackpotCount;
}

ItemType CEffect::GetItemChargeDisplayItem(int index) const
{
	if (index < 0 || index >= ITEM_EFFECT_SLOT_MAX) { return ItemType::E_NONE; }
	return m_ItemChargeSlot.displayItems[index];
}

void CEffect::RenderItemChargeSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font, int slotIndex, const RECT& slotRc)
{
	if (!device) { return; }
	if (slotIndex < 0 || slotIndex >= ITEM_EFFECT_SLOT_MAX) { return; }
	if (!m_ItemChargeSlot.active[slotIndex]) { return; }

	float rate = GetEffectRate();

	bool stopped = rate >= 0.72f;

	float stopRate = 0.0f;

	if (rate > 0.72f)
	{
		stopRate = (rate - 0.72f) / 0.28f;
		stopRate = Clamp(stopRate, 0.0f, 1.0f);
	}

	bool jackpotSlot = IsItemChargeJackpotSlot(slotIndex);

	RECT scaled = CButton::ScaleRect(slotRc);

	float cx = (float)(scaled.left + scaled.right) * 0.5f;
	float cy = (float)(scaled.top + scaled.bottom) * 0.5f;

	float pop = sinf(stopRate * D3DX_PI);

	float glowRadius = 28.0f + pop * 14.0f;

	if (jackpotSlot)
	{
		glowRadius = 42.0f + pop * 22.0f;
	}

	DWORD oldFVF = 0;
	DWORD oldAlphaBlend = 0;
	DWORD oldSrcBlend = 0;
	DWORD oldDestBlend = 0;

	device->GetFVF(&oldFVF);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);

	if (stopped)
	{
		DrawGlowCircle(
			device,
			cx,
			cy,
			glowRadius,
			D3DCOLOR_ARGB(170, 255, 245, 160),
			D3DCOLOR_ARGB(0, 255, 180, 40));
	}

	if (jackpotSlot)
	{
		DrawGlowCircle(
			device,
			cx,
			cy,
			glowRadius + 22.0f,
			D3DCOLOR_ARGB(210, 255, 255, 255),
			D3DCOLOR_ARGB(0, 255, 200, 40));

		// 슬롯에서 위로 튀는 빛 입자.
		for (int i = 0; i < 20; i++)
		{
			float angle = -D3DX_PI * 0.5f + ((float)i - 2.5f) * 0.20f;
			float dist = 28.0f + stopRate * (35.0f + i * 4.0f);

			float px = cx + cosf(angle) * dist;
			float py = cy + sinf(angle) * dist;

			DrawGlowCircle(
				device,
				px,
				py,
				7.0f + stopRate * 4.0f,
				D3DCOLOR_ARGB(230, 255, 240, 120),
				D3DCOLOR_ARGB(0, 255, 160, 40));
		}
	}

	device->SetFVF(oldFVF);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetTexture(0, nullptr);

	if (font && jackpotSlot)
	{
		RECT textRc =
		{
			slotRc.left - 20,
			slotRc.top - 38,
			slotRc.right + 20,
			slotRc.top - 5
		};

		textRc = CButton::ScaleRect(textRc);

		font->DrawTextW(
			nullptr,
			L"TRIPLE!",
			-1,
			&textRc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			D3DCOLOR_ARGB(255, 255, 220, 80));
	}
}

void CEffect::RenderItemChargeCenterEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	if (!device) { return; }
	if (!font) { return; }

	if (!m_ItemChargeSlot.jackpot) { return; }

	float rate = GetEffectRate();

	if (rate < 0.72f) { return; }

	RECT rc = { 0, 620, 900, 700 };
	rc = CButton::ScaleRect(rc);

	font->DrawTextW(
		nullptr,
		L"ITEM JACKPOT!",
		-1,
		&rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		D3DCOLOR_ARGB(255, 255, 230, 80));
}

void CEffect::RenderStealSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font, const RECT& slotRc)
{
	if (!device) { return; }

	float rate = GetEffectRate();

	// alphaRate:
	// STEAL 슬롯 플래시는 짧게 터졌다가 사라지는 연출이다.
	// rate가 0이면 가장 강하고, rate가 1이면 완전히 사라진다.
	float alphaRate = 1.0f - rate;

	if (alphaRate < 0.0f) { alphaRate = 0.0f; }
	if (alphaRate > 1.0f) { alphaRate = 1.0f; }

	// pop:
	// 처음에 확 커졌다가 줄어드는 느낌.
	// 슬롯에서 마법이 터지는 것처럼 보이게 한다.
	float pop = sinf(rate * D3DX_PI);

	float cx = (float)(slotRc.left + slotRc.right) * 0.5f;
	float cy = (float)(slotRc.top + slotRc.bottom) * 0.5f;

	DWORD oldFVF = 0;
	DWORD oldAlphaBlend = 0;
	DWORD oldSrcBlend = 0;
	DWORD oldDestBlend = 0;

	device->GetFVF(&oldFVF);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);

	int alpha = (int)(230.0f * alphaRate);
	if (alpha < 0) { alpha = 0; }
	if (alpha > 255) { alpha = 255; }

	// 슬롯 중심 보라색 플래시.
	DrawGlowCircle(
		device,
		cx,
		cy,
		34.0f + pop * 24.0f,
		D3DCOLOR_ARGB(alpha, 230, 120, 255),
		D3DCOLOR_ARGB(0, 90, 20, 160));

	// 안쪽 밝은 핵.
	DrawGlowCircle(
		device,
		cx,
		cy,
		16.0f + pop * 12.0f,
		D3DCOLOR_ARGB(alpha, 255, 255, 255),
		D3DCOLOR_ARGB(0, 210, 80, 255));

	// 작은 보라 입자.
	for (int i = 0; i < 12; i++)
	{
		float angle = ((float)i / 12.0f) * D3DX_PI * 2.0f;
		float dist = 16.0f + rate * (35.0f + (i % 3) * 8.0f);

		float x = cx + cosf(angle) * dist;
		float y = cy + sinf(angle) * dist;

		DrawGlowCircle(
			device,
			x,
			y,
			4.0f + pop * 3.0f,
			D3DCOLOR_ARGB(alpha, 210, 100, 255),
			D3DCOLOR_ARGB(0, 80, 20, 160));
	}

	device->SetFVF(oldFVF);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetTexture(0, nullptr);
}

void CEffect::RenderCenterTextureEffect(LPDIRECT3DDEVICE9 device, LPDIRECT3DTEXTURE9 texture)
{
	if (!device) { return; }
	if (!texture) { return; }

	float rate = GetEffectRate();

	// alphaRate:
	// 0.0 ~ 1.0 사이의 투명도 비율.
	//
	// rate 0.0 ~ 0.25 : 서서히 나타남
	// rate 0.25 ~ 0.75 : 선명하게 유지
	// rate 0.75 ~ 1.0 : 서서히 사라짐
	float alphaRate = 1.0f;

	if (rate < 0.25f)
	{
		alphaRate = rate / 0.25f;
	}

	else if (rate > 0.75f)
	{
		alphaRate = 1.0f - ((rate - 0.75f) / 0.25f);
	}

	alphaRate = Clamp(alphaRate, 0.0f, 1.0f);

	int alpha = (int)(255.0f * alphaRate);

	if (alpha < 0) { alpha = 0; }
	if (alpha > 255) { alpha = 255; }

	// 화면 중앙 출력 영역.
	// 크기를 바꾸고 싶으면 이 RECT만 조절하면 된다.
	RECT rc =
	{
		250,
		300,
		650,
		700
	};

	rc = CButton::ScaleRect(rc);

	CButton::DrawTexture(
		device,
		rc,
		texture,
		D3DCOLOR_ARGB(alpha, 255, 255, 255));
}

void CEffect::RenderSneakPeekEffect(LPDIRECT3DDEVICE9 device)
{
	if (!device) { return; }

	float rate = GetEffectRate();

	// alphaRate:
	// 이펙트가 시작할 때는 서서히 나타나고,
	// 중간에는 가장 선명하고,
	// 끝날 때는 다시 사라지게 만드는 값.

	// sin(0) = 0
	// sin(PI / 2) = 1
	// sin(PI) = 0

	// 그래서 rate가 0 -> 1로 갈 때
	// alphaRate는 0 -> 1 -> 0 형태가 된다.
	float alphaRate = sinf(rate * D3DX_PI);
	alphaRate = Clamp(alphaRate, 0.0f, 1.0f);

	int alpha = (int)(255.0f * alphaRate);
	if (alpha < 170 && alphaRate > 0.05f) { alpha = 170; }
	alpha = ClampInt(alpha, 0, 255);

	// pulse:
	// 화살표가 가만히 있으면 딱딱한 UI처럼 보인다.
	// sin을 이용해서 크기를 살짝 커졌다 작아졌다 하게 만들면
	// 살아있는 이펙트처럼 보인다.
	float pulse = sinf(rate * D3DX_PI * 6.0f);
	float scale = 1.0f + (pulse * 0.06f);

	float time = GetTickCount() * 0.001f;

	// shake:
	// 아주 약한 흔들림.
	// 너무 강하면 방향을 읽기 어려우므로 2~3픽셀 정도만 준다.
	float shakeX = 450.0f + sinf(time * 18.0f) * 2.0f;
	float shakeY = 455.0f + cosf(time * 15.0f) * 2.0f;

	// 화살표 기본 길이 / 두께.
	float halfLength = 170.0f * scale;
	float shaftHalfWidth = 15.0f * scale;
	float headLength = 72.0f * scale;
	float headHalfWidth = 62.0f * scale;

	// 방향 벡터.
	// 여기서 방향을 새로 추측하지 않는다.
	// PlaySneakPeek()에서 저장한 m_DirectionIndex만 사용한다.

	// m_DirectionIndex는 CTarget::GetDirectionIndex()에서 온 값이므로
	// 실제 과녁이 움직일 방향과 같은 번호다.
	float dx = 1.0f;
	float dy = 0.0f;

	switch (m_DirectionIndex)
	{
		case 0: { dx = 1.0f;  dy = 0.0f;  break; } // 좌우
		case 1: { dx = 0.0f;  dy = 1.0f;  break; } // 상하
		case 2: { dx = 1.0f;  dy = -1.0f; break; } // ↙↗, 화면 좌표에서는 위가 -Y
		case 3: { dx = -1.0f; dy = -1.0f; break; } // ↖↘
		default: { dx = 1.0f;  dy = 0.0f;  break; }
	}

	float len = sqrtf((dx * dx) + (dy * dy));

	if (len < 0.0001f)
	{
		dx = 1.0f;
		dy = 0.0f;
	}

	else
	{
		dx /= len;
		dy /= len;
	}

	// right vector:
	// 화살표 진행 방향에 수직인 방향.
	// 화살표 몸통 두께와 머리 폭을 만들 때 사용한다.
	float px = -dy;
	float py = dx;

	struct ARROW_VERTEX
	{
		float x, y, z, rhw;
		D3DCOLOR color;
	};

	auto MakeVirtualPoint = [&](float forward, float side, float& outX, float& outY)
	{
		outX = shakeX + (dx * forward) + (px * side);
		outY = shakeY + (dy * forward) + (py * side);
	};

	auto MakeVertex = [](float x, float y, D3DCOLOR color) -> ARROW_VERTEX
	{
		// x, y는 900x1000 가상 좌표 기준이다.
		// 실제 화면 크기에 맞게 ScaleRect로 변환한다.
		RECT rc = { (int)x, (int)y, (int)x + 1, (int)y + 1 };
		RECT scaled = CButton::ScaleRect(rc); 

		ARROW_VERTEX v;
		v.x = (float)scaled.left;
		v.y = (float)scaled.top;
		v.z = 0.0f;
		v.rhw = 1.0f;
		v.color = color;
		return v;
	};

	// 기존 렌더 상태 저장.
	DWORD oldFVF = 0;
	DWORD oldAlphaBlend = 0;
	DWORD oldSrcBlend = 0;
	DWORD oldDestBlend = 0;

	device->GetFVF(&oldFVF);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
	device->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	device->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);

	// Smoke / Mist
	// 화살표 주변에 낮은 알파의 원형 Glow를 여러 개 찍는다.
	// 이것은 실제 연기 텍스처가 아니라,
	// DrawGlowCircle()을 여러 번 겹쳐서 연기처럼 보이게 만드는 방식이다.
	for (int i = 0; i < 17; i++)
	{
		float t = (float)i / 17.0f;

		// 화살표 선을 따라 배치.
		float along = -halfLength + (halfLength * 2.0f * t);

		// sin/cos를 섞어서 매 프레임 조금씩 흔들리는 스모크 위치를 만든다.
		float drift = sinf(time * 2.0f + (float)i * 1.7f) * (20.0f + (float)(i % 3) * 5.0f);

		float sx, sy;
		MakeVirtualPoint(along, drift, sx, sy);

		float smokeRadius = 33.0f + (float)(i % 4) * 8.0f;
		int smokeAlpha = (int)(22.0f * alphaRate);

		DrawGlowCircle(
			device,
			sx,
			sy,
			smokeRadius,
			D3DCOLOR_ARGB(smokeAlpha, 120, 230, 255),
			D3DCOLOR_ARGB(0, 60, 160, 190));
	}

	// Arrow Glow
	// 화살표 뒤에 큰 청록색 빛을 깔아서
	// 실제 화살표 정점이 더 강하게 보이도록 만든다.
	DrawGlowCircle(
		device,
		shakeX,
		shakeY,
		150.0f * scale,
		D3DCOLOR_ARGB(alpha / 6, 80, 180, 220),
		D3DCOLOR_ARGB(0, 40, 120, 180));

	device->SetTexture(0, nullptr);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	D3DCOLOR edgeColor = D3DCOLOR_ARGB(alpha, 80, 220, 255);
	D3DCOLOR coreColor = D3DCOLOR_ARGB(alpha, 220, 255, 255);

	float ax, ay, bx, by, cx, cy, dx2, dy2;
	float ex, ey, fx, fy;

	// 왼쪽 화살촉.
	// 한쪽 끝이 단순한 작은 삼각형이 아니라,
	// 몸통보다 넓게 퍼지는 큰 화살촉이 되도록 만든다.
	MakeVirtualPoint(-halfLength, 0.0f, ax, ay);
	MakeVirtualPoint(-halfLength + headLength, -headHalfWidth, bx, by);
	MakeVirtualPoint(-halfLength + headLength, headHalfWidth, cx, cy);

	ARROW_VERTEX leftHead[3] =
	{
		MakeVertex(ax, ay, coreColor),
		MakeVertex(bx, by, edgeColor),
		MakeVertex(cx, cy, edgeColor)
	};

	device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, leftHead, sizeof(ARROW_VERTEX));

	// 오른쪽 화살촉.
	MakeVirtualPoint(halfLength, 0.0f, ax, ay);
	MakeVirtualPoint(halfLength - headLength, headHalfWidth, bx, by);
	MakeVirtualPoint(halfLength - headLength, -headHalfWidth, cx, cy);

	ARROW_VERTEX rightHead[3] =
	{
		MakeVertex(ax, ay, coreColor),
		MakeVertex(bx, by, edgeColor),
		MakeVertex(cx, cy, edgeColor)
	};

	device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, rightHead, sizeof(ARROW_VERTEX));

	// 몸통.
	// 화살촉과 자연스럽게 이어지도록
	// 양쪽 화살촉 안쪽 지점 사이를 사각형으로 연결한다.
	MakeVirtualPoint(-halfLength + headLength * 0.55f, -shaftHalfWidth, ax, ay);
	MakeVirtualPoint(halfLength - headLength * 0.55f, -shaftHalfWidth, bx, by);
	MakeVirtualPoint(-halfLength + headLength * 0.55f, shaftHalfWidth, cx, cy);
	MakeVirtualPoint(halfLength - headLength * 0.55f, shaftHalfWidth, dx2, dy2);

	ARROW_VERTEX shaft[4] =
	{
		MakeVertex(ax, ay, edgeColor),
		MakeVertex(bx, by, coreColor),
		MakeVertex(cx, cy, edgeColor),
		MakeVertex(dx2, dy2, coreColor)
	};

	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shaft, sizeof(ARROW_VERTEX));

	// 내부 하이라이트.
	// 큰 화살표 안쪽에 얇고 밝은 선을 한 번 더 그려서
	// 단순 막대가 아니라 에너지 화살표처럼 보이게 한다.
	float innerWidth = shaftHalfWidth * 0.35f;

	MakeVirtualPoint(-halfLength + headLength * 0.75f, -innerWidth, ax, ay);
	MakeVirtualPoint(halfLength - headLength * 0.75f, -innerWidth, bx, by);
	MakeVirtualPoint(-halfLength + headLength * 0.75f, innerWidth, cx, cy);
	MakeVirtualPoint(halfLength - headLength * 0.75f, innerWidth, dx2, dy2);

	ARROW_VERTEX inner[4] =
	{
		MakeVertex(ax, ay, D3DCOLOR_ARGB(alpha, 230, 255, 255)),
		MakeVertex(bx, by, D3DCOLOR_ARGB(alpha, 180, 255, 255)),
		MakeVertex(cx, cy, D3DCOLOR_ARGB(alpha, 230, 255, 255)),
		MakeVertex(dx2, dy2, D3DCOLOR_ARGB(alpha, 180, 255, 255))
	};

	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, inner, sizeof(ARROW_VERTEX));

	// 양쪽 끝에 작은 Glow.
	// 화살촉 끝을 강조해서 방향성이 더 잘 보이게 한다.
	MakeVirtualPoint(-halfLength, 0.0f, ex, ey);
	MakeVirtualPoint(halfLength, 0.0f, fx, fy);

	DrawGlowCircle(
		device,
		ex,
		ey,
		50.0f * scale,
		D3DCOLOR_ARGB(alpha / 5, 120, 220, 255),
		D3DCOLOR_ARGB(0, 80, 220, 255));

	DrawGlowCircle(
		device,
		fx,
		fy,
		50.0f * scale,
		D3DCOLOR_ARGB(alpha / 5, 120, 220, 255),
		D3DCOLOR_ARGB(0, 80, 220, 255));

	// 중앙 Core.
	DrawGlowCircle(
		device,
		shakeX,
		shakeY,
		42.0f * scale,
		D3DCOLOR_ARGB(alpha / 3, 180, 255, 255),
		D3DCOLOR_ARGB(0, 100, 240, 255));

	device->SetFVF(oldFVF);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
	device->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	device->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
	device->SetTexture(0, nullptr);
}

void CEffect::RenderExtraChanceEffect(LPDIRECT3DDEVICE9 device)
{
	if (!device) { return; }

	float rate = GetEffectRate();

	// alphaRate:
	// 이펙트가 시작할 때는 서서히 나타나고,
	// 중간에는 가장 선명하고,
	// 끝날 때는 다시 사라지게 만든다.

	// sin(0)      = 0
	// sin(PI / 2) = 1
	// sin(PI)     = 0

	// 그래서 rate가 0 -> 1로 갈 때
	// alphaRate는 0 -> 1 -> 0 형태가 된다.
	float alphaRate = sinf(rate * D3DX_PI);
	alphaRate = Clamp(alphaRate, 0.0f, 1.0f);

	// EXTRA_CHANCE는 의미가 확실히 보여야 하므로
	// 너무 흐릿하게 사라지지 않게 최소 알파를 조금 보정한다.
	int alpha = (int)(255.0f * alphaRate);
	if (alphaRate > 0.08f && alpha < 150) { alpha = 150; }
	alpha = ClampInt(alpha, 0, 255);

	float time = GetTickCount() * 0.001f;

	float centerX = 450.0f;
	float centerY = 455.0f;

	// expandRate:
	// 이펙트가 진행될수록 글로우 궤도가 살짝 넓어지게 만든다.
	// 처음부터 너무 넓으면 산만하고,
	// 끝까지 고정이면 정적인 느낌이 강하다.
	float expandRate = SmoothStep(0.0f, 1.0f, rate);

	// orbitAngle:
	// 페어리 글로우들이 한 바퀴 이상 회전하는 각도.
	// rate 기반이라 이펙트 시간과 정확히 같이 진행된다.
	float orbitAngle = rate * D3DX_PI * 2.6f;

	// orbitRadius:
	// 글로우 덩어리들이 도는 기본 반지름.
	// 진행되면서 약간 커져서 "찬스 보호막이 펼쳐지는 느낌"을 준다.
	float orbitRadius = 200.0f + (expandRate * 55.0f);

	// background glow:
	// 중앙에 아주 약한 보호빛만 깐다.
	// 기존 노란 링처럼 강한 도형이 보이지 않도록 알파를 낮게 둔다.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		128.0f + expandRate * 25.0f,
		D3DCOLOR_ARGB(alpha / 8, 90, 210, 255),
		D3DCOLOR_ARGB(0, 30, 90, 170));

	// soft gold core:
	// EXTRA_CHANCE는 보상/기회 느낌이 있으므로
	// 중앙에 아주 약한 금빛 코어를 섞는다.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		54.0f,
		D3DCOLOR_ARGB(alpha / 4, 255, 245, 170),
		D3DCOLOR_ARGB(0, 255, 200, 80));

	// fairy ring:
	// 작은 글로우 덩어리들이 원 궤도를 따라 회전한다.
	// 링을 직접 그리는 것이 아니라 점 형태의 빛들이 돌기 때문에
	// 더 가볍고 예쁜 느낌이 난다.
	const int FAIRY_COUNT = 18;

	for (int i = 0; i < FAIRY_COUNT; i++)
	{
		float baseAngle = (D3DX_PI * 2.0f * i) / (float)FAIRY_COUNT;

		// 각 글로우가 같은 속도로만 돌면 너무 기계적이다.
		// 전체 회전 orbitAngle에 각 글로우마다 약간 다른 흔들림을 섞는다.
		float wobble = sinf(time * 3.5f + (float)i * 0.9f) * 0.10f;
		float angle = baseAngle + orbitAngle + wobble;

		// radiusWave:
		// 글로우들이 완전히 같은 원 위에 있으면 딱딱해 보인다.
		// 반지름을 조금씩 다르게 해서 반짝이는 요정가루처럼 보이게 한다.
		float radiusWave = sinf(time * 4.0f + (float)i * 1.4f) * 9.0f;
		float r = orbitRadius + radiusWave;

		float x = centerX + cosf(angle) * r;
		float y = centerY + sinf(angle) * r;

		// twinkle:
		// 각 글로우의 밝기가 시간에 따라 달라지는 값.
		// 0~1 사이로 움직이며, 이 값으로 크기와 알파를 같이 조절한다.
		float twinkle = (sinf(time * 9.0f + (float)i * 1.7f) + 1.0f) * 0.5f;

		int fairyAlpha = (int)((90.0f + twinkle * 130.0f) * alphaRate);
		fairyAlpha = ClampInt(fairyAlpha, 0, 220);

		float fairySize = 10.0f + (twinkle * 12.0f);

		// outer aura:
		// 글로우 바깥쪽의 부드러운 청록빛.
		DrawGlowCircle(
			device,
			x,
			y,
			fairySize * 2.3f,
			D3DCOLOR_ARGB(fairyAlpha / 3, 100, 230, 255),
			D3DCOLOR_ARGB(0, 60, 150, 220));

		// bright core:
		// 글로우 중심의 작은 금빛 핵.
		// 바깥은 청록, 안쪽은 금빛이라 보상/기회 느낌이 난다.
		DrawGlowCircle(
			device,
			x,
			y,
			fairySize,
			D3DCOLOR_ARGB(fairyAlpha, 255, 245, 150),
			D3DCOLOR_ARGB(0, 120, 230, 255));
	}

	// second orbit:
	// 첫 번째 페어리링만 있으면 단순한 원처럼 보일 수 있어서
	// 반대 방향으로 도는 작은 글로우들을 추가한다.
	// 개수와 크기를 줄여서 복잡하지 않게 보조 효과로만 사용한다.
	const int SMALL_COUNT = 10;

	for (int i = 0; i < SMALL_COUNT; i++)
	{
		float baseAngle = (D3DX_PI * 2.0f * i) / (float)SMALL_COUNT;

		// 첫 번째 링과 반대 방향으로 회전.
		float angle = baseAngle - orbitAngle * 0.75f;

		float r = orbitRadius * 0.78f;

		float x = centerX + cosf(angle) * r;
		float y = centerY + sinf(angle) * r;

		float twinkle = (cosf(time * 10.0f + (float)i * 1.2f) + 1.0f) * 0.5f;

		int smallAlpha = (int)((55.0f + twinkle * 90.0f) * alphaRate);
		smallAlpha = ClampInt(smallAlpha, 0, 150);

		float size = 6.0f + twinkle * 7.0f;

		DrawGlowCircle(
			device,
			x,
			y,
			size * 2.0f,
			D3DCOLOR_ARGB(smallAlpha / 3, 150, 240, 255),
			D3DCOLOR_ARGB(0, 60, 120, 200));

		DrawGlowCircle(
			device,
			x,
			y,
			size,
			D3DCOLOR_ARGB(smallAlpha, 255, 255, 210),
			D3DCOLOR_ARGB(0, 120, 220, 255));
	}

	// sparkle burst:
	// 중간 시점 근처에서 살짝 더 반짝이게 한다.
	// EXTRA_CHANCE 발동이 "확정"되는 느낌을 주는 포인트 효과.
	float burstRate = 1.0f - fabsf((rate - 0.5f) * 2.0f);
	burstRate = Clamp(burstRate, 0.0f, 1.0f);

	for (int i = 0; i < 8; i++)
	{
		float angle = (D3DX_PI * 2.0f * i) / 8.0f;
		float r = 38.0f + burstRate * 42.0f;

		float x = centerX + cosf(angle) * r;
		float y = centerY + sinf(angle) * r;

		int burstAlpha = (int)(180.0f * burstRate * alphaRate);
		burstAlpha = ClampInt(burstAlpha, 0, 180);

		DrawGlowCircle(
			device,
			x,
			y,
			9.0f + burstRate * 7.0f,
			D3DCOLOR_ARGB(burstAlpha, 255, 255, 180),
			D3DCOLOR_ARGB(0, 120, 230, 255));
	}

	// final center shine:
	// 가장 중앙에 작게 빛을 남겨서
	// 텍스트와 페어리링 사이의 중심을 잡아준다.
	DrawGlowCircle(
		device,
		centerX,
		centerY,
		30.0f,
		D3DCOLOR_ARGB(alpha / 2, 255, 255, 220),
		D3DCOLOR_ARGB(0, 100, 220, 255));
}

// 참고 사이트 : https://codingfarm.tistory.com/389 (다중 샘플링), 

// 상수버퍼 
// 장면의 물체마다 달라지는 상수 데이터를 담기 위한 저장공간
// Vertex Buffer나 Index Buffer와 달리 Constant Buffer는 CPU가 프레임당 한번 갱신하는 것이 일반적이다.
// 크가는 반드시 최소 하드웨어 할당 크기(256byte)의 배수여야한다.
// default heap? upload heap?

// 랜더링 파이프라인?
// WVP변환
// 변환에는 총 5가지의 단계가 존재한다.
// 1. Input Assmbler : 정점(Vertex) 데이터, WVP 행렬 준비
// 2. Vertex Shader : 정점 데이터와 WVP 연산을 통해 삼각형(폴리곤) 생성
// 3. Rasterizer : 그릴 픽셀을 결정
// 4. Pixel Shader : 픽셀 색상을 결정
// 5. OupputMerge : 화면 출력

// WVP변환은 컴퓨터그래픽스에서 최종적으로 2D모니터에 출력하기 위한 위치 변환과정중 하나이고, 그중에서 Clip Space로 변환하기 위한 과정이다.
// World, View, Projection

// 멀티 샘플링, 버텍스 프로세싱, 안티 앨리어싱

// 멀티 샘플링 : 3D 렌더링 시 폴리곤의 가장자리가 계단처럼 보이는 계단 현상을 부드럽게 완화하는 기법
// 이것을 이용해서 지금의 CButton UI호출시 생기는 가장자리의 픽셀로 보이는 부분을 완화할수 있는가?

// D3DPRESENT_PARAMETERS d3dd;
// d3dd.MultiSampleType = D3DMULTISAMPLE_4_SAMPLE;
// d3dd.MultiSampleQuality = 0; (품질 레벨)

// 모니터의 픽셀들이 무한히 작지는 않기 때문에 모니터의 화면에 임의의 선을 완벽하게 나타내는것은 불가능하다
// 선을 픽셀들의 배열로 근사할 때 생기는, 계단현상을 엘리어싱(aliasing)이라고 지칭한다.
// 그러면 엘리어싱을 제거하고 좀더 좋은 품질의 개선되게 하려면 대표적으로 2가지의 방법이 있다.
// 1. 초과 표본화(supersampling)
// 후면 버퍼와 깊이 버퍼를 화면 해상도보다 4배(가로, 세로 2배씩) 크게 잡는다.
// 후면 버퍼에 렌더링하고, 이미지를 화면에 제시할 때 후면 버퍼를 원래 크기의 환원(resolving)한다.
// 하향표준화(downsampling)이라고도 하는 이 공정은 4픽셀 블록의 네개색상의 평균을 최종 생삭으로 사용한다.
// 단 이 방법은 메모리 소비량이 4배라서 비용이 크다

// 2. 다중표본화(multisampling)