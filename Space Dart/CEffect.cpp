#include "CEffect.h"
#include "CButton.h"

CEffect::CEffect()
{
	m_timer = 0.0f;
	m_limitTime = 30.0f;

	m_Type = EffectType::E_NONE;
	m_isPlaying = false;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 0.0f;

	m_DisplayScore = 0;
	m_CurrentPoint = 0;
	m_FinalPoint = 0;

	for (int i = 0; i < ITEM_EFFECT_SLOT_MAX; i++)
	{
		m_CurrentItems[i] = ItemType::E_NONE;
		m_FinalItems[i] = ItemType::E_NONE;
	}

	m_DirectionIndex = 0;

	m_NumberSlot.Reset();
	m_ItemChargeSlot.Reset();

	m_StealItem = ItemType::E_NONE;
	m_StealProgress = 0.0f;
}

CEffect::~CEffect() {}

void CEffect::Init(float limitTime)
{
	m_limitTime = limitTime;
	m_timer = limitTime;

	m_Type = EffectType::E_NONE;
	m_isPlaying = false;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 0.0f;

	m_DisplayScore = 0;
	m_CurrentPoint = 0;
	m_FinalPoint = 0;

	for (int i = 0; i < ITEM_EFFECT_SLOT_MAX; i++)
	{
		m_CurrentItems[i] = ItemType::E_NONE;
		m_FinalItems[i] = ItemType::E_NONE;
	}

	m_DirectionIndex = 0;

	m_NumberSlot.Reset();
	m_ItemChargeSlot.Reset();

	m_StealItem = ItemType::E_NONE;
	m_StealProgress = 0.0f;

	m_StealSuccess = false;
	m_StealSlotIndex = -1;
}

void CEffect::SetLimitTime(float time)
{
	m_limitTime = time;

	if (m_limitTime < 0.0f) { m_limitTime = 0.0f; }

	m_timer = m_limitTime;
}

void CEffect::UpdateTimer(float dt)
{
	if (m_timer > 0.0f)
	{
		m_timer -= dt;

		if (m_timer < 0.0f){ m_timer = 0.0f; }
	}
}

void CEffect::UpdateEffect(float dt)
{
	if (!m_isPlaying) { return; }

	m_EffectTimer += dt;

	float rate = SafeDivide(m_EffectTimer, m_EffectDuration, 1.0f);
	rate = Clamp(rate, 0.0f, 1.0f);

	if (m_Type == EffectType::E_ITEMCHARGE)
	{
		UpdateItemChargeSlotEffect(dt, rate);
	}

	else if (m_Type == EffectType::E_POINT_UP || m_Type == EffectType::E_POINT_DOWN)
	{
		// POINT_UP / POINT_DOWN은 공용 숫자 슬롯머신 이펙트를 사용한다.
		UpdateNumberSlotEffect(dt, rate);
	}

	else if (m_Type == EffectType::E_STEAL)
	{
		m_StealProgress = SmoothStep(0.0f, 1.0f, rate);
	}

	if (m_EffectTimer >= m_EffectDuration)
	{
		m_isPlaying = false;
		m_EffectTimer = m_EffectDuration;

		if (m_Type == EffectType::E_STEAL)
		{
			m_StealProgress = 1.0f;
		}

		if (m_Type == EffectType::E_POINT_UP || m_Type == EffectType::E_POINT_DOWN)
		{
			m_NumberSlot.displayValue = m_NumberSlot.finalValue;
			m_CurrentPoint = m_NumberSlot.finalValue;
		}
	}
}

void CEffect::Update(float dt)
{
	UpdateTimer(dt);
	UpdateEffect(dt);
}

void CEffect::PlayHit(int score)
{
	m_Type = EffectType::E_HIT;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	// 과녁에 맞은 뒤 다트가 잠깐 멈춰 보이도록
	// 턴 종료 대기 시간을 기존 1.0초에서 1.3초로 약간 늘린다.
	m_EffectDuration = 2.0f;

	m_DisplayScore = score;
}

void CEffect::PlayBull()
{
	m_Type = EffectType::E_BULL;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 1.0f;

	m_DisplayScore = 50;
}

void CEffect::PlayCube()
{
	m_Type = EffectType::E_CUBE;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 0.8f;
}

void CEffect::PlayMiss()
{
	// MISS 이펙트 시작
	m_Type = EffectType::E_MISS;
	m_isPlaying = true;

	// 이펙트 시간 초기화
	m_EffectTimer = 0.0f;

	// MISS는 눈에 잘 보여야 하므로 HIT보다 조금 길게 표시
	m_EffectDuration = 1.0f;

	// MISS는 점수 표시가 없으므로 0으로 설정
	m_DisplayScore = 0;
}

void CEffect::PlayCenterTextureEffect(EffectType type)
{
	m_Type = type;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 1.5f;
}

void CEffect::RenderTimer(ID3DXFont* font)
{
	if (!font) return;

	// 표시용 남은 시간
	int remain = (int)ceilf(m_timer);
	if (remain < 0){ remain = 0; }

	wchar_t buf[64];
	swprintf_s(buf, L"%02d", remain);

	// 타이머 출력 위치
	RECT rc = { 390, 35, 510, 95 };
	rc = CButton::ScaleRect(rc);

	D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255);

	// 5초 이하일 때 깜빡이는 경고 색
	if (remain <= 5)
	{
		float blink = (sinf(GetTickCount() * 0.02f) + 1.0f) * 0.5f;

		int g = (int)Lerp(40.0f, 140.0f, blink);
		int b = (int)Lerp(40.0f, 140.0f, blink);

		color = D3DCOLOR_XRGB(255, g, b);
	}

	// 10초 이하일 때 노란색 경고
	else if (remain <= 10)
	{
		color = D3DCOLOR_XRGB(255, 200, 80);
	}

	font->DrawTextW(
		nullptr,
		buf,
		-1,
		&rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		color);
}

void CEffect::DrawGlowCircle(LPDIRECT3DDEVICE9 device, float x, float y, float radius, D3DCOLOR centerColor, D3DCOLOR edgeColor)
{
	if (!device) { return; }
	if (radius <= 0.0f) { return; }

	struct GLOW_VERTEX
	{
		float x, y, z, rhw;
		D3DCOLOR color;
	};

	// centerRc:
	// 현재 게임 UI는 900x1000 가상 좌표를 기준으로 관리된다.
	// 그래서 x, y도 바로 화면 좌표로 쓰면 해상도 변경 시 위치가 틀어질 수 있다.
	// CButton::ScaleRect()를 사용하기 위해 1x1 RECT로 변환한다.
	RECT centerRc =
	{
		(int)x,
		(int)y,
		(int)x + 1,
		(int)y + 1
	};

	// radiusRc:
	// radius도 가상 좌표 기준 값이므로 실제 해상도 크기에 맞게 스케일링한다.
	RECT radiusRc =
	{
		0,
		0,
		(int)radius,
		(int)radius
	};

	RECT scaledCenter = CButton::ScaleRect(centerRc);
	RECT scaledRadius = CButton::ScaleRect(radiusRc);

	float sx = (float)scaledCenter.left;
	float sy = (float)scaledCenter.top;

	float rx = (float)(scaledRadius.right - scaledRadius.left);
	float ry = (float)(scaledRadius.bottom - scaledRadius.top);

	if (rx < 1.0f) { rx = 1.0f; }
	if (ry < 1.0f) { ry = 1.0f; }

	// SEG:
	// 원을 몇 조각의 삼각형으로 나눌지 정하는 값.
	// 값이 높을수록 원이 부드러워지지만 정점 수가 늘어난다.
	// UI 이펙트용 작은 빛덩어리라 24면 충분히 둥글게 보인다.
	const int SEG = 24;

	// TRIANGLEFAN 구조:
	// vertex[0]은 원의 중심.
	// vertex[1]부터는 원 둘레.
	// 중심 하나에서 둘레 정점들을 부채꼴처럼 연결해서 원형 면을 만든다.
	GLOW_VERTEX vertex[SEG + 2];

	// 중심 정점:
	// 가장 밝은 색을 넣는다.
	vertex[0].x = sx;
	vertex[0].y = sy;
	vertex[0].z = 0.0f;
	vertex[0].rhw = 1.0f;
	vertex[0].color = centerColor;

	for (int i = 0; i <= SEG; i++)
	{
		float angle = (D3DX_PI * 2.0f * i) / SEG;

		vertex[i + 1].x = sx + cosf(angle) * rx;
		vertex[i + 1].y = sy + sinf(angle) * ry;
		vertex[i + 1].z = 0.0f;
		vertex[i + 1].rhw = 1.0f;

		// 외곽 정점:
		// 알파가 낮거나 0인 색을 넣으면
		// 중심에서 바깥쪽으로 자연스럽게 사라지는 빛덩어리가 된다.
		vertex[i + 1].color = edgeColor;
	}

	device->SetTexture(0, nullptr);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	// Additive Blend:
	// 기존 색 위에 현재 색을 더하는 방식.
	// 일반 알파 블렌딩은 반투명 종이를 얹는 느낌이고,
	// Additive Blend는 빛을 더하는 느낌이다.
	// 그래서 Glow, 폭발, 마법, 플라즈마, 빛덩어리 이펙트에 잘 어울린다.
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	device->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		SEG,
		vertex,
		sizeof(GLOW_VERTEX));

	device->SetTexture(0, nullptr);
}