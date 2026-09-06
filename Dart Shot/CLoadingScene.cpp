#include "CLoadingScene.h"

CLoadingScene::CLoadingScene()
{
	m_state = LoadingState::E_ROCKET_IN;

	m_timer = 0.0f;
	m_rocketX = -50.0f;
	m_rocketY = 9.5f;
	m_rocketSpeed = 0.0f;

	m_bodyWidthScale = 1.0f;
	m_bodyLengthScale = 1.0f;
	m_flamePower = 1.0f;
}

CLoadingScene::~CLoadingScene() {}

void CLoadingScene::Init(HWND hwnd, CD3D* d3d)
{
	CScene::Init(hwnd, d3d);

	m_state = LoadingState::E_ROCKET_IN;
	m_timer = 0.0f;

	m_rocketX = -50.0f;
	m_rocketY = 9.5f;
	m_rocketSpeed = 0.0f;

	m_bodyWidthScale = 1.0f;
	m_bodyLengthScale = 1.0f;
	m_flamePower = 1.0f;

	if (m_d3d)
	{
		// CItem은 텍스처 매니저만 있으면 아이템 텍스처를 미리 로드할 수 있다.
		// 실제 아이템 슬롯을 쓰는 목적이 아니라,
		// 로딩 씬에서 텍스처 캐시를 채우기 위한 전용 로더 역할이다.
		m_loaderItem.SetTextureManager(&m_d3d->GetTextureManager());

		// 3프레임마다 1개씩 아이템 텍스처를 로드하도록 시작한다.
		m_loaderItem.StartPreloadItemTextures();
	}
}

void CLoadingScene::Update(float dt)
{
	m_timer += dt;

	// 로딩 중에는 매 프레임 UpdatePreloadItemTextures()를 호출한다.
	// 실제 로드는 CItem 내부에서 3프레임마다 1개씩만 진행된다.
	// 그래서 한 프레임에 25개를 몰아서 로드하는 멈춤을 피할 수 있다.
	if (!m_loaderItem.IsPreloadDone())
	{
		m_loaderItem.UpdatePreloadItemTextures();
	}

	UpdateRocket(dt);
}

void CLoadingScene::UpdateRocket(float dt)
{
	const float startX = -50.0f;	// 화면 왼쪽 밖
	const float stopX = 2.0f;	// Loading 글자 아래 정지 위치
	const float outX = 85.0f;	// 화면 오른쪽 밖
	const float baseY = 9.5f;	// Loading 글자 아래쪽 높이

	if (m_state == LoadingState::E_ROCKET_IN)
	{
		// Ease Out 느낌:
		// 처음에는 빠르게 들어오고, 목표 위치에 가까워질수록 부드럽게 멈춘다.
		float distance = stopX - m_rocketX;
		m_rocketSpeed = distance * 4.8f;

		if (m_rocketSpeed < 8.0f) { m_rocketSpeed = 8.0f; }

		m_rocketX += m_rocketSpeed * dt;

		m_bodyWidthScale = 1.0f;
		m_bodyLengthScale = 1.0f;
		m_flamePower = 1.2f;

		if (m_rocketX >= stopX)
		{
			m_rocketX = stopX;
			m_timer = 0.0f;
			m_state = LoadingState::E_LOADING;
		}
	}

	else if (m_state == LoadingState::E_LOADING)
	{
		// 로딩 중에는 로켓이 Loading 텍스트 아래에서 살짝 떠 있는 느낌만 준다.
		m_rocketY = baseY + sinf(GetTickCount() * 0.004f) * 0.35f;

		m_bodyWidthScale = 1.0f;
		m_bodyLengthScale = 1.0f;
		m_flamePower = 0.85f;

		// 텍스처 로딩이 끝나면 바로 날아가지 않고,
		// 카툰식 압축 준비 상태로 넘어간다.
		if (m_loaderItem.IsPreloadDone() && m_timer >=1.0f)
		{
			m_timer = 0.0f;
			m_state = LoadingState::E_ROCKET_SQUASH;
		}
	}

	else if (m_state == LoadingState::E_ROCKET_SQUASH)
	{
		// 미국 카툰식 Squash:
		// 발사 직전에 로켓이 진행 방향으로 살짝 짧아지고,
		// 두께는 약간 두꺼워진다.
		// 이 짧은 정지가 있어야 다음 발사가 더 과장되어 보인다.
		float t = m_timer / 0.22f;
		t = Clamp(t, 0.0f, 1.0f);

		float pulse = sinf(t * D3DX_PI);

		m_bodyLengthScale = 1.0f - pulse * 0.35f;
		m_bodyWidthScale = 1.0f + pulse * 0.30f;
		m_flamePower = 0.55f + pulse * 0.45f;

		if (m_timer >= 0.22f)
		{
			m_timer = 0.0f;
			m_rocketSpeed = 18.0f;
			m_state = LoadingState::E_ROCKET_OUT;
		}
	}

	else if (m_state == LoadingState::E_ROCKET_OUT)
	{
		// Stretch:
		// 발사 순간에는 로켓을 길게 늘리고, 폭은 살짝 줄인다.
		// 속도도 계속 증가시켜서 화면 오른쪽으로 튀어나가는 느낌을 만든다.
		m_rocketSpeed += 120.0f * dt;
		m_rocketX += m_rocketSpeed * dt;

		m_bodyLengthScale = 1.45f;
		m_bodyWidthScale = 0.82f;
		m_flamePower = 4.0f;

		if (m_rocketX > 85.0f)
		{
			m_state = LoadingState::E_NONE;
			m_nextScene = GameScene::E_GAME;
		}
	}
}

void CLoadingScene::Render(HDC hdc)
{
	DrawRocket();

	ID3DXFont* font = m_d3d->GetUIFont();

	if (font) { DrawLoadingText(font); }
}

void CLoadingScene::DrawLoadingText(ID3DXFont* font)
{
	if (!font) { return; }

	const wchar_t* text = L"Loading....";

	// startX:
	// 첫 글자(L)가 출력될 시작 X 위치.
	// 값이 작아질수록 전체 문장이 왼쪽으로 이동한다.
	int startX = 180;

	// baseY:
	// 글자가 기본적으로 출력되는 높이.
	// 점프(jump)가 적용되기 전 기준 Y 위치이다.
	int baseY = 370;

	// charW:
	// 글자 하나가 차지하는 가로 공간.
	// 값을 키우면 글자 크기가 커 보이고
	// 글자 사이 간격도 넓어진다.
	int charW = 60;

	DWORD now = GetTickCount();

	for (int i = 0; text[i] != L'\0'; i++)
	{
		// phase:
		// 글자마다 서로 다른 타이밍으로 움직이기 위한 값.
		// i를 더해주지 않으면 모든 글자가 동시에 움직인다.
		float phase = (now * 0.012f) + ((float)i * 0.55f);

		// jump:
		// 위로 튀어오르는 높이.
		// 현재는 최대 약 22픽셀 정도 위로 움직인다.
		float jump = fabsf(sinf(phase)) * 22.0f;

		RECT rc =
		{
			startX + (i * charW),
			(int)(baseY - jump),
			startX + (i * charW) + charW,
			baseY + 140
		};

		RECT scaled = CButton::ScaleRect(rc);

		wchar_t letter[2] = { text[i], L'\0' };

		RECT shadow = scaled;
		OffsetRect(&shadow, 5, 5);

		// 그림자
		font->DrawTextW(
			nullptr,
			letter,
			-1,
			&shadow,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			D3DCOLOR_ARGB(255, 0, 0, 0));

		// 본문
		font->DrawTextW(
			nullptr,
			letter,
			-1,
			&scaled,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE,
			D3DCOLOR_ARGB(255, 180, 240, 255));
	}
}

void CLoadingScene::DrawRocket()
{
	if (!m_d3d) { return; }

	m_d3d->RenderLoadingRocket(
		m_rocketX,
		m_rocketY,
		22.0f,
		5.0f,
		m_flamePower,
		m_bodyWidthScale,
		m_bodyLengthScale);
}