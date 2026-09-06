#include "CPostProcess.h"

CPostProcess::CPostProcess()
{
	m_device = nullptr;

	m_sceneTex = nullptr;
	m_sceneSurface = nullptr;

	m_brightTex = nullptr;
	m_brightSurface = nullptr;

	m_bloomTex = nullptr;
	m_bloomSurface = nullptr;

	m_blurTex = nullptr;
	m_blurSurface = nullptr;

	m_oldRT = nullptr;

	m_textureManager = nullptr;

	m_width = 0;
	m_height = 0;

	m_bloomIntensity = 2.0f;	// Bloom 강도 값이 높을수록 화면 번짐이 강해진다
	m_blurPower = 5.0;			// Blur 퍼짐 거리 값이 높을수록 Fake Bloom이 더 넓게 퍼진다.
	m_threshold = 180.0f;		// 밝은 부분 추출 기준값

	m_enableBloom = true;
	m_enableLensDirt = true;
	m_enableDust = true;
}

CPostProcess::~CPostProcess()
{
	Release();
}

void CPostProcess::Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager, int width, int height)
{
	m_device = device;
	m_textureManager = textureManager;

	m_width = width;
	m_height = height;

	if (!m_device) { return; }

	HRESULT hr = m_device->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_sceneTex,
		nullptr);

	if (FAILED(hr) || !m_sceneTex) { return; }

	hr = m_sceneTex->GetSurfaceLevel(0, &m_sceneSurface);

	if (FAILED(hr))
	{
		m_sceneSurface = nullptr;
		return;
	}

	// Bright-pass RenderTarget
	// 원본 장면에서 threshold 이상으로 밝은 픽셀만 저장한다.
	hr = m_device->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_brightTex,
		nullptr);

	if (FAILED(hr) || !m_brightTex) { return; }

	hr = m_brightTex->GetSurfaceLevel(0, &m_brightSurface);

	if (FAILED(hr))
	{
		m_brightSurface = nullptr;
		return;
	}

	// Bloom용 RenderTarget
	// 최종적으로 블러된 빛 번짐 결과가 들어간다.
	hr = m_device->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_bloomTex,
		nullptr);

	if (FAILED(hr) || !m_bloomTex) { return; }

	hr = m_bloomTex->GetSurfaceLevel(0, &m_bloomSurface);

	if (FAILED(hr))
	{
		m_bloomSurface = nullptr;
		return;
	}

	// Blur 중간 저장용 RenderTarget
	// 가로 블러 결과를 저장하고, 그 결과를 다시 세로 블러에 사용한다.
	hr = m_device->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_blurTex,
		nullptr);

	if (FAILED(hr) || !m_blurTex) { return; }

	hr = m_blurTex->GetSurfaceLevel(0, &m_blurSurface);

	if (FAILED(hr))
	{
		m_blurSurface = nullptr;
		return;
	}

	// Bright-pass Pixel Shader
	// 역할:
	// - srcTex에서 픽셀 색을 읽는다.
	// - 밝기 = R*0.299 + G*0.587 + B*0.114 로 계산한다.
	// - 밝기가 threshold보다 낮으면 검정색으로 버린다.
	// - 밝으면 원래 색을 유지한다.
	const char* shaderCode =
		// Pixel Shader 2.0 문법을 사용한다.
		// 이 셰이더는 "밝은 픽셀만 남기고 어두운 픽셀은 검정으로 버리는" Bright-pass용이다.
		"ps_2_0\n"	// == Pixel Shader Model 2.0

		// t0.xy:
		// 화면 사각형 정점에서 넘어온 텍스처 좌표.
		// DrawFullscreenQuadOffset()에서 u, v로 넣어준 좌표를 여기서 받는다.
		"dcl t0.xy\n"	// 텍스처 좌표 입력 선언,  GPU가 화면을 그릴때 Rasterizer의 픽셀마다 텍스처의 UV좌표를 만들어준다.

		// s0:
		// 0번 텍스처 샘플러 선언.
		// 여기에는 m_sceneTex 같은 원본 장면 텍스처가 들어간다.
		"dcl_2d s0\n"	// 2D 텍스처 샘플러 선언

		// r0 = tex2D(s0, t0)
		// 현재 픽셀 위치의 원본 장면 색상을 읽는다.
		// r0에는 RGBA 값이 들어간다.
		"texld r0, t0, s0\n"	// 텍스처 읽기 (== r0 = tex2D(s0, t0);)

		// r1.r = dot(r0.rgb, c0.rgb)
		// 현재 픽셀의 밝기를 계산한다.
		// c0에는 { 0.299, 0.587, 0.114, 0 } 값이 들어간다.
		// 사람 눈은 초록색을 더 밝게 느끼므로 G 비중이 가장 높다.
		"dp3 r1.r, r0, c0\n"	// Dot Product3(내적 계산), (r1.r = r0.r * c0.r + r0.g * c0.g + r0.b * c0.b)

		// r1.r = brightness - threshold
		// c1.r에는 threshold 값이 들어간다.
		// 결과가 0 이상이면 밝은 픽셀,
		// 0보다 작으면 어두운 픽셀로 판단한다.
		"add r1.r, r1.r, -c1.r\n"	// Threshold 적용, (r1.r = brightness - threshold)
									// 양수 -> 밝음, 음수 -> 어두움

		// if r1.r >= 0 then r0 else c2
		// cmp는 조건 선택 명령이다.
		// r1.r이 0 이상이면 원본 색 r0를 유지하고,
		// 0보다 작으면 c2, 즉 검정색으로 바꾼다.
		// 결과적으로 밝은 부분만 남고 어두운 부분은 검정이 된다.
		"cmp r0, r1.r, r0, c2\n" 

		// 최종 출력 색상.
		// 위에서 살아남은 밝은 픽셀만 RenderTarget에 기록된다.
		"mov oC0, r0\n";	// 최종 출력

	LPD3DXBUFFER codeBuffer = nullptr;
	LPD3DXBUFFER errorBuffer = nullptr;

	hr = D3DXAssembleShader(
		shaderCode,
		(UINT)strlen(shaderCode),
		nullptr,
		nullptr,
		0,
		&codeBuffer,
		&errorBuffer);

	if (SUCCEEDED(hr) && codeBuffer)
	{
		m_device->CreatePixelShader(
			(DWORD*)codeBuffer->GetBufferPointer(),
			&m_brightPassShader);
	}

	if (codeBuffer)
	{
		codeBuffer->Release();
		codeBuffer = nullptr;
	}

	if (errorBuffer)
	{
		errorBuffer->Release();
		errorBuffer = nullptr;
	}
}

void CPostProcess::Release()
{
	if (m_oldRT)
	{
		m_oldRT->Release();
		m_oldRT = nullptr;
	}

	if (m_brightPassShader)
	{
		m_brightPassShader->Release();
		m_brightPassShader = nullptr;
	}

	if (m_blurSurface)
	{
		m_blurSurface->Release();
		m_blurSurface = nullptr;
	}

	if (m_blurTex)
	{
		m_blurTex->Release();
		m_blurTex = nullptr;
	}

	if (m_bloomSurface)
	{
		m_bloomSurface->Release();
		m_bloomSurface = nullptr;
	}

	if (m_bloomTex)
	{
		m_bloomTex->Release();
		m_bloomTex = nullptr;
	}

	if (m_brightSurface)
	{
		m_brightSurface->Release();
		m_brightSurface = nullptr;
	}

	if (m_brightTex)
	{
		m_brightTex->Release();
		m_brightTex = nullptr;
	}

	if (m_sceneSurface)
	{
		m_sceneSurface->Release();
		m_sceneSurface = nullptr;
	}

	if (m_sceneTex)
	{
		m_sceneTex->Release();
		m_sceneTex = nullptr;
	}
}

// 렌더 시작
void CPostProcess::BeginScene()
{
	if (!m_device) { return; }
	if (!m_sceneSurface) { return; }

	HRESULT hr = m_device->TestCooperativeLevel();

	if (hr != D3D_OK) { return; }

	// 이전 RenderTarget이 남아있으면 정리
	if (m_oldRT)
	{
		m_oldRT->Release();
		m_oldRT = nullptr;
	}

	// 현재 RenderTarget 저장
	hr = m_device->GetRenderTarget( 0, &m_oldRT);

	if (FAILED(hr))
	{
		m_oldRT = nullptr;
		return;
	}

	// 앞으로 그리는 장면은 m_sceneTex에 저장된다.
	hr = m_device->SetRenderTarget( 0, m_sceneSurface);

	if (FAILED(hr))
	{
		if (m_oldRT)
		{
			m_oldRT->Release();
			m_oldRT = nullptr;
		}

		return;
	}

	m_device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0),
		1.0f,
		0);
}

// 렌더 종료
void CPostProcess::EndScene()
{
	if (!m_device) { return; }
	if (!m_oldRT) { return; }

	// RenderTarget을 다시 BackBuffer로 복구
	m_device->SetRenderTarget( 0, m_oldRT);

	m_oldRT->Release();
	m_oldRT = nullptr;

	// Scene RenderTarget에 그려둔 최종 장면을 BackBuffer에 출력
	ApplyPostProcess();
}

void CPostProcess::ApplyPostProcess()
{
	// 원본 장면 출력
	DrawSceneTexture();

	// Real Bloom
	// 원본 화면을 약간씩 밀어서 여러 번 Additive로 합성한다.
	// 셰이더 없이 빛 번짐 느낌을 내는 방식.
	if (m_enableBloom) { DrawRealBloom(); }

	// Lens Dirt
	// 밝은 부분 위에 렌즈 먼지 느낌을 추가
	if (m_enableLensDirt) { DrawLensDirt(); }

	// Dust Overlay
	// 화면 전체에 우주 먼지 / 필름 노이즈 느낌 추가
	if (m_enableDust) { DrawDustOverlay(); }

	// 최종 상태 정리
	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CPostProcess::DrawSceneTexture()
{
	if (!m_device) { return; }
	if (!m_sceneTex) { return; }

	// 원본 장면은 절대 어두워지면 안 된다.
	// 그래서 AlphaBlend를 끄고, 텍스처를 그대로 덮어쓴다.
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	DrawFullscreenQuad(
		m_sceneTex,
		D3DCOLOR_ARGB(255, 255, 255, 255));
}

void CPostProcess::DrawRealBloom()
// Fake Bloom 원리
// 원본 화면을 여러 방향으로 조금씩 밀어서 다시 그린다.
// Additive Blend로 더하면 밝은 부분이 주변으로 퍼진 것처럼 보인다.
//
// m_bloomIntensity:
// - Bloom 밝기 강도
// - 높을수록 번짐이 강해짐
//
// m_blurPower:
// - 화면을 밀어 그리는 거리
// - 높을수록 번짐 범위가 넓어짐
//
// m_threshold:
// - 현재 Fake Bloom에서는 사용 안 함
// - 진짜 Bloom에서는 밝은 픽셀만 추출할 때 사용
{
	if (!m_device) { return; }
	if (!m_sceneTex) { return; }
	if (!m_brightTex || !m_brightSurface) { return; }
	if (!m_bloomTex || !m_bloomSurface) { return; }
	if (!m_blurTex || !m_blurSurface) { return; }

	// Bright-pass
	// 원본 전체를 블러하지 않고,
	// threshold 이상으로 밝은 픽셀만 m_brightTex에 남긴다.
	DrawBrightPass(
		m_sceneTex,
		m_brightSurface);

	// 가로 블러
	// 밝은 픽셀만 남은 m_brightTex를 좌우로 퍼뜨린다.
	DrawBlurPass(
		m_brightTex,
		m_blurSurface,
		m_blurPower,
		0.0f);

	// 세로 블러
	// 가로 블러 결과를 위아래로 다시 퍼뜨려 실제 번짐처럼 만든다.
	DrawBlurPass(
		m_blurTex,
		m_bloomSurface,
		0.0f,
		m_blurPower);

	// 블러된 Bloom 텍스처를 원본 화면 위에 Additive 합성
	// 여기서 합성되는 것은 원본 전체가 아니라 밝은 부분만 추출해서 블러한 결과다.
	int alpha = (int)(75.0f * m_bloomIntensity);

	if (alpha < 0) { alpha = 0; }
	if (alpha > 255) { alpha = 255; }

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// 흰색으로 합성하면 원본 색을 덜 오염시킨다.
	// 기존 160,210,255는 화면 전체가 푸르게 떠 보일 수 있다.
	DrawFullscreenQuad(
		m_bloomTex,
		D3DCOLOR_ARGB(alpha, 255, 255, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CPostProcess::DrawBrightPass(LPDIRECT3DTEXTURE9 srcTex, LPDIRECT3DSURFACE9 dstSurface)
{
	if (!m_device) { return; }
	if (!srcTex || !dstSurface) { return; }

	LPDIRECT3DSURFACE9 oldRT = nullptr;

	if (FAILED(m_device->GetRenderTarget(0, &oldRT)))
	{
		oldRT = nullptr;
		return;
	}

	m_device->SetRenderTarget(0, dstSurface);

	m_device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 0),
		1.0f,
		0);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	// Linear Filter
	// 밝은 부분 추출용이므로 텍스처를 부드럽게 샘플링한다.
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	if (m_brightPassShader)
	{
		// c0 = 밝기 계산 가중치
		// 사람 눈은 초록색을 가장 밝게 느끼기 때문에 G 비중이 가장 높다.
		float weight[4] = { 0.299f, 0.587f, 0.114f, 0.0f };

		// c1 = threshold
		// m_threshold는 0~255 기준이므로 Pixel Shader에서는 0~1로 변환해서 사용한다.
		float threshold = m_threshold / 255.0f;

		if (threshold < 0.0f) { threshold = 0.0f; }
		if (threshold > 1.0f) { threshold = 1.0f; }

		float thresholdConst[4] = { threshold, threshold, threshold, threshold };

		// c2 = 검정색
		// threshold보다 어두운 픽셀은 이 값으로 대체된다.
		float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		m_device->SetPixelShaderConstantF(0, weight, 1);
		m_device->SetPixelShaderConstantF(1, thresholdConst, 1);
		m_device->SetPixelShaderConstantF(2, black, 1);

		m_device->SetPixelShader(m_brightPassShader);

		DrawFullscreenQuad(
			srcTex,
			D3DCOLOR_ARGB(255, 255, 255, 255));

		m_device->SetPixelShader(nullptr);
	}
	else
	{
		// Shader 생성 실패 시 기존 기능이 완전히 죽지 않도록 fallback.
		// 이 경우는 예전처럼 전체 장면이 Bloom 대상이 된다.
		DrawFullscreenQuad(
			srcTex,
			D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	m_device->SetRenderTarget(0, oldRT);

	oldRT->Release();
	oldRT = nullptr;
}

void CPostProcess::DrawBlurPass(LPDIRECT3DTEXTURE9 srcTex, LPDIRECT3DSURFACE9 dstSurface, float offsetX, float offsetY)
{
	if (!m_device) { return; }
	if (!srcTex || !dstSurface) { return; }

	LPDIRECT3DSURFACE9 oldRT = nullptr;

	if (FAILED(m_device->GetRenderTarget(0, &oldRT)))
	{
		oldRT = nullptr;
		return;
	}

	m_device->SetRenderTarget(0, dstSurface);

	m_device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 0),
		1.0f,
		0);

	// Additive 방식으로 여러 장을 겹쳐서
	// Gaussian Blur와 비슷한 분포를 만든다.
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// 가운데 샘플이 가장 강하다.
	DrawFullscreenQuadOffset(
		srcTex,
		0.0f,
		0.0f,
		D3DCOLOR_ARGB(80, 255, 255, 255));

	// 가까운 샘플
	DrawFullscreenQuadOffset(
		srcTex,
		-offsetX * 1.0f,
		-offsetY * 1.0f,
		D3DCOLOR_ARGB(54, 255, 255, 255));

	DrawFullscreenQuadOffset(
		srcTex,
		offsetX * 1.0f,
		offsetY * 1.0f,
		D3DCOLOR_ARGB(54, 255, 255, 255));

	// 중간 샘플
	DrawFullscreenQuadOffset(
		srcTex,
		-offsetX * 2.0f,
		-offsetY * 2.0f,
		D3DCOLOR_ARGB(32, 255, 255, 255));

	DrawFullscreenQuadOffset(
		srcTex,
		offsetX * 2.0f,
		offsetY * 2.0f,
		D3DCOLOR_ARGB(32, 255, 255, 255));

	// 먼 샘플
	DrawFullscreenQuadOffset(
		srcTex,
		-offsetX * 3.0f,
		-offsetY * 3.0f,
		D3DCOLOR_ARGB(16, 255, 255, 255));

	DrawFullscreenQuadOffset(
		srcTex,
		offsetX * 3.0f,
		offsetY * 3.0f,
		D3DCOLOR_ARGB(16, 255, 255, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	m_device->SetRenderTarget(0, oldRT);

	oldRT->Release();
	oldRT = nullptr;
}

void CPostProcess::DrawGlow()
{
	if (!m_textureManager) { return; }

	LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(L"glow");

	if (!tex) { return; }

	// Glow 원리
	// glow.png 같은 발광 텍스처를 화면 전체에 Additive로 더한다.
	// 별빛, 우주 배경, 유성, 네온 느낌을 강화한다.

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	DrawFullscreenQuad(
		tex,
		D3DCOLOR_ARGB(90, 160, 210, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CPostProcess::DrawLensDirt()
{
	if (!m_textureManager) { return; }

	LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(L"lens_dirt");

	if (!tex) { return; }

	// Lens Dirt 원리
	// 렌즈 먼지 텍스처를 밝은 화면 위에 살짝 더한다.
	// 우주 장면에서 카메라 렌즈에 빛이 묻는 느낌을 준다.

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	DrawFullscreenQuad(
		tex,
		D3DCOLOR_ARGB(110, 255, 255, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}
// 전경 오버레이 방식으로 수정해야할듯

void CPostProcess::DrawDustOverlay()
{
	if (!m_textureManager) { return; }

	LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(L"dust");

	if (!tex) { return; }

	// Dust Overlay 원리
	// 화면 전체에 아주 옅은 먼지 텍스처를 Alpha Blend로 덮는다.
	// 필름 노이즈, 우주 먼지, 화면 질감 효과를 만든다.

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	DrawFullscreenQuad(
		tex,
		D3DCOLOR_ARGB(35, 180, 200, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CPostProcess::DrawFullscreenQuad(LPDIRECT3DTEXTURE9 tex, D3DCOLOR color)
{
	DrawFullscreenQuadOffset(tex, 0.0f, 0.0f, color);
}

void CPostProcess::DrawFullscreenQuadOffset(LPDIRECT3DTEXTURE9 tex, float offsetX, float offsetY, D3DCOLOR color)
{
	if (!m_device) { return; }
	if (!tex) { return; }

	struct SCREENVERTEX
	{
		float x, y, z, rhw;
		D3DCOLOR color;
		float u, v;
	};

	float width = (float)m_width;
	float height = (float)m_height;

	SCREENVERTEX quad[4];

	quad[0] =
	{
		-0.5f + offsetX,
		-0.5f + offsetY,
		0.0f, 1.0f, color, 0.0f, 0.0f
	};

	quad[1] =
	{
		width - 0.5f + offsetX,
		-0.5f + offsetY,
		0.0f, 1.0f, color, 1.0f, 0.0f
	};

	quad[2] =
	{
		-0.5f + offsetX,
		height - 0.5f + offsetY,
		0.0f, 1.0f, color, 0.0f, 1.0f
	};

	quad[3] =
	{
		width - 0.5f + offsetX,
		height - 0.5f + offsetY,
		0.0f, 1.0f, color, 1.0f, 1.0f
	};

	// 2D 화면좌표 렌더링
	m_device->SetTexture(0, tex);
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	// 텍스처 색상 * 정점 색상
	// color의 알파와 RGB로 전체 텍스처의 밝기와 투명도를 조절할 수 있다.
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(SCREENVERTEX));

	m_device->SetTexture(0, nullptr);
}