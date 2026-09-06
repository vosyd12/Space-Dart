#include "CD3D.h"
#include "CButton.h"
#include "GameSetting.h"

CD3D::CD3D()
{
	m_d3d = nullptr;
	m_device = nullptr;
	m_uiFont = nullptr;

	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	ZeroMemory(&m_windowRect, sizeof(m_windowRect));

	m_screenWidth = 900;
	m_screenHeight = 1000;

	m_hwnd = nullptr;
	m_isFullscreen = false;
	m_isDeviceResetting = false;
	m_lastTime = 0;
}

CD3D::~CD3D()
{
	CleanUp();
}

HRESULT CD3D::Init(HWND hwnd)
{
	m_hwnd = hwnd;
	m_isFullscreen = false;
	GetWindowRect(hwnd, &m_windowRect);

	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!m_d3d) { return E_FAIL; }

	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));

	m_d3dpp.Windowed = TRUE;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.hDeviceWindow = hwnd;

	m_d3dpp.BackBufferWidth = 0;
	m_d3dpp.BackBufferHeight = 0;
	m_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	m_d3dpp.EnableAutoDepthStencil = TRUE;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	HRESULT hr =
		m_d3d->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&m_d3dpp,
			&m_device);

	if (FAILED(hr)) { return hr; }

	m_camera.Init();
	m_camera.Update(Vec3(0.0f, 0.0f, 50.0f), 0.0f);

	RECT rc;
	GetClientRect(hwnd, &rc);

	m_screenWidth = rc.right - rc.left;
	m_screenHeight = rc.bottom - rc.top;

	// UI 가상 좌표 900x1000을 현재 화면 크기에 맞게 스케일링하기 위해 필요
	CButton::SetScreenSize(m_screenWidth, m_screenHeight);

	// 설정창 FOV 값을 기준으로 Projection 설정
	SetFOV(g_GameSetting.fov);

	// Render State
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// 텍스처 시스템
	m_textureManager.Init(m_device);
	m_textureManager.BuildTextureDatabase(L"./Data");
	// 우주 데이터
	m_spaceBG.Init(m_screenWidth, m_screenHeight);
	// Renderer 초기화
	m_renderer3D.Init(m_device, &m_textureManager);
	m_spaceRenderer.Init(m_device, &m_textureManager);
	m_postProcess.Init(m_device, &m_textureManager, m_screenWidth, m_screenHeight);

	m_lastTime = GetTickCount();

	// 폰트 생성
	D3DXCreateFont(
		m_device,
		28,
		0,
		FW_BOLD,
		1,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"my",
		&m_uiFont);

	return S_OK;
}

void CD3D::Update(float deltaTime)
{
	Vec3 cameraMove =
	{
		0.08f * deltaTime,
		0.02f * deltaTime,
		0.0f
	};

	m_spaceBG.Update(deltaTime, cameraMove);
	m_renderer3D.Update(deltaTime);
}

bool CD3D::BeginFrame()
{
	if (!m_device) return false;

	m_device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0),
		1.0f,
		0);

	if (FAILED(m_device->BeginScene())) { return false; }

	m_postProcess.BeginScene();

	return true;
}

void CD3D::EndFrame()
{
	if (!m_device) { return; }

	m_postProcess.EndScene();

	m_device->EndScene();

	m_device->Present(nullptr, nullptr, nullptr, nullptr);
}

void CD3D::DrawSpace()
{
	if (!m_device) { return; }

	m_camera.Apply(m_device);

	m_spaceRenderer.DrawSpace(m_spaceBG, m_camera);
}

void CD3D::RenderGameObjects(const Vec3& targetPos, const DartState& dart, DartOwner owner, float targetRadius)
{
	if (!m_device) { return; }

	m_camera.Apply(m_device);

	// 과녁의 실제 판정 반지름을 렌더러에 넘긴다.
	m_renderer3D.Render(dart, targetPos, owner, targetRadius);
}

void CD3D::RenderSpaceOnly()
{
	BeginFrame();
	DrawSpace();
	EndFrame();
}

void CD3D::RenderTitleLogo(float titleTimer, bool startFly, float startFlyTimer)
{
	if (!m_device) { return; }

	// 타이틀 전용 카메라.
	// 버튼 UI와 별도로 상단 로고를 안정적으로 잡기 위한 고정 카메라다.
	D3DXVECTOR3 eye(0.0f, 0.0f, -28.0f);
	D3DXVECTOR3 at(0.0f, 2.5f, 22.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	m_device->SetTransform(D3DTS_VIEW, &matView);

	SetFOV(g_GameSetting.fov);

	m_renderer3D.RenderTitleLogo(titleTimer, startFly, startFlyTimer);

	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CD3D::RenderLoadingRocket(float rocketX, float rocketY, float rocketZ, float rocketScale, float flamePower, float bodyWidthScale, float bodyLengthScale)
{
	if (!m_device) { return; }

	// 타이틀 로고와 같은 전용 카메라를 사용한다.
	// LoadingScene에서 CRenderer3D::DrawTitleRocket()을 직접 호출하면
	// 이전에 DrawSpace()가 적용한 게임 카메라 상태가 그대로 남아 있어서
	// 로켓이 화면 밖에 있거나 카메라에 안 잡힐 수 있다.
	D3DXVECTOR3 eye(0.0f, 0.0f, -28.0f);
	D3DXVECTOR3 at(0.0f, 2.5f, 22.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	m_device->SetTransform(D3DTS_VIEW, &matView);

	// 타이틀 로켓과 같은 Projection/FOV 사용
	SetFOV(g_GameSetting.fov);

	Vec3 dir = { 1.0f, 0.24f, 0.0f };
	Normalize(dir);

	m_renderer3D.DrawTitleRocket(
		rocketX,
		rocketY,
		rocketZ,
		rocketScale,
		dir,
		flamePower,
		bodyWidthScale,
		bodyLengthScale);

	// 다음 UI / 씬 렌더에 영향이 가지 않도록 기본 상태 복구
	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CD3D::CleanUp()
{
	m_postProcess.Release();
	m_spaceRenderer.Release();
	m_renderer3D.Release();
	m_textureManager.Release();

	if (m_uiFont)
	{
		m_uiFont->Release();
		m_uiFont = nullptr;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}

	if (m_d3d)
	{
		m_d3d->Release();
		m_d3d = nullptr;
	}
}

void CD3D::SetFOV(float degree)
{
	if (!m_device) return;

	float aspect = (float)(m_screenWidth) / (float)(m_screenHeight);

	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(degree), aspect, 0.1f, 10000.0f);
	m_device->SetTransform(D3DTS_PROJECTION, &matProj);
}

void CD3D::SetFullscreen(bool enable)
{
	if (!m_hwnd) return;
	if (m_isFullscreen == enable) return;

	m_isFullscreen = enable;

	if (enable)
	{
		GetWindowRect(m_hwnd, &m_windowRect);

		SetWindowLong(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		SetWindowPos(
			m_hwnd,
			HWND_TOP,
			0,
			0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}

	else
	{
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		SetWindowPos(
			m_hwnd,
			HWND_TOP,
			m_windowRect.left,
			m_windowRect.top,
			m_windowRect.right - m_windowRect.left,
			m_windowRect.bottom - m_windowRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}

	UpdateScreenSize();
	UpdateWindow(m_hwnd);
}

void CD3D::UpdateScreenSize()
{
	if (!m_hwnd || !m_device) return;

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_screenWidth = rc.right - rc.left;
	m_screenHeight = rc.bottom - rc.top;

	if (m_screenWidth <= 0 || m_screenHeight <= 0) return;

	CButton::SetScreenSize(m_screenWidth, m_screenHeight);

	// 폰트 리셋 준비
	if (m_uiFont)
	{
		m_uiFont->OnLostDevice();
	}

	// 렌더 리소스 해제
	m_spaceRenderer.Release();
	m_postProcess.Release();

	// 백버퍼 크기 갱신
	m_d3dpp.BackBufferWidth = m_screenWidth;
	m_d3dpp.BackBufferHeight = m_screenHeight;

	// 리셋
	HRESULT hr = m_device->Reset(&m_d3dpp);

	if (FAILED(hr))
	{
		return;
	}

	// 폰트 복구
	if (m_uiFont)
	{
		m_uiFont->OnResetDevice();
	}

	// 렌더러 재초기화
	m_spaceRenderer.Init(m_device, &m_textureManager);

	m_postProcess.Init(
		m_device,
		&m_textureManager,
		m_screenWidth,
		m_screenHeight);

	// Viewport 갱신
	D3DVIEWPORT9 vp;

	vp.X = 0;
	vp.Y = 0;
	vp.Width = m_screenWidth;
	vp.Height = m_screenHeight;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	m_device->SetViewport(&vp);

	// Projection 갱신
	SetFOV(g_GameSetting.fov);
}


void CD3D::ResetDevice()
{
	if (!m_device) return;
	if (m_screenWidth <= 0 || m_screenHeight <= 0) return;

	// Reset 전에 D3DPOOL_DEFAULT 리소스 해제
	m_postProcess.Release();
	m_spaceRenderer.Release();
	m_renderer3D.Release();

	if (m_uiFont)m_uiFont->OnLostDevice();

	m_d3dpp.BackBufferWidth = m_screenWidth;
	m_d3dpp.BackBufferHeight = m_screenHeight;

	HRESULT hr = m_device->Reset(&m_d3dpp);

	if (FAILED(hr)) return;
	if (m_uiFont) m_uiFont->OnResetDevice();

	m_spaceRenderer.Init(m_device, &m_textureManager);
	m_renderer3D.Init(m_device, &m_textureManager);
	m_postProcess.Init(m_device, &m_textureManager, m_screenWidth, m_screenHeight);

	SetFOV(g_GameSetting.fov);

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = m_screenWidth;
	vp.Height = m_screenHeight;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	m_device->SetViewport(&vp);
}

void CD3D::DrawGameStateMessage(const wstring& text, D3DCOLOR mainColor, D3DCOLOR glowColor, int y, bool darkOverlay)
{
	if (!m_device) return;
	if (!m_uiFont) return;
	if (text.empty()) return;

	if (darkOverlay)
	{
		RECT overlay = { 0, 0, 900, 1000 };
		overlay = CButton::ScaleRect(overlay);

		CButton::DrawRect(
			m_device,
			overlay,
			D3DCOLOR_ARGB(150, 0, 0, 0));
	}

	// glow 느낌: 주변에 여러 번 찍어서 두껍게 보이게 함
	RECT glow1 = { -4, y - 4, 904, y + 140 };
	RECT glow2 = { 4, y - 4, 904, y + 140 };
	RECT glow3 = { -4, y + 4, 904, y + 140 };
	RECT glow4 = { 4, y + 4, 904, y + 140 };

	glow1 = CButton::ScaleRect(glow1);
	glow2 = CButton::ScaleRect(glow2);
	glow3 = CButton::ScaleRect(glow3);
	glow4 = CButton::ScaleRect(glow4);

	CButton::DrawTextUI(m_uiFont, text, glow1, glowColor);
	CButton::DrawTextUI(m_uiFont, text, glow2, glowColor);
	CButton::DrawTextUI(m_uiFont, text, glow3, glowColor);
	CButton::DrawTextUI(m_uiFont, text, glow4, glowColor);

	RECT main = { 0, y, 900, y + 140 };
	main = CButton::ScaleRect(main);

	CButton::DrawTextUI(
		m_uiFont,
		text,
		main,
		mainColor);
}