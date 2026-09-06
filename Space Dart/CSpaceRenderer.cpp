#include "CSpaceRenderer.h"
#include "GameSetting.h"
#include "Math.h"

void CSpaceRenderer::Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager)
{
	m_device = device;
	m_textureManager = textureManager;
	m_billboardVB = nullptr;

	m_device->CreateVertexBuffer(
		sizeof(CUSTOMVERTEX) * 4,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_DEFAULT,
		&m_billboardVB,
		nullptr);
}

void CSpaceRenderer::Release()
{
	if (m_billboardVB)
	{
		m_billboardVB->Release();
		m_billboardVB = nullptr;
	}
}

// 전체 우주 그리기
void CSpaceRenderer::DrawSpace(const CSpaceBackground& bg, const CCamera& camera)
{
	DrawGalaxyBackground();

	BeginSpotLight();

	DrawStars(bg, camera);
	DrawNebulas(bg, camera);
	DrawMeteors(bg, camera);
	DrawDust(bg, camera);

	if (bg.GetWarp().active) { DrawWarpStars(bg, camera); }

	EndSpotLight();

	// DrawSpace 최종 상태 복구
	m_device->SetTexture(0, nullptr);

	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
}

// 우주배경 그리기
void CSpaceRenderer::DrawGalaxyBackground()
{
	LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(L"galaxy");

	if (!tex) { return; }

	D3DVIEWPORT9 vp;
	m_device->GetViewport(&vp);

	float width = (float)vp.Width;
	float height = (float)vp.Height;

	float scrollU = GetTickCount() * 0.000003f;
	float scrollV = GetTickCount() * 0.000001f;

	struct SCREENVERTEX
	{
		float x, y, z, rhw;
		D3DCOLOR color;
		float u, v;
	};

	SCREENVERTEX quad[4];

	quad[0] = { -0.5f,   -0.5f,    0.0f, 1.0f, D3DCOLOR_ARGB(ApplyLumi(100), ApplyLumi(255), ApplyLumi(255), ApplyLumi(255)), 0.0f, 0.0f };
	quad[1] = { width - 0.5f,   -0.5f,    0.0f, 1.0f, D3DCOLOR_ARGB(ApplyLumi(100), ApplyLumi(255), ApplyLumi(255), ApplyLumi(255)), 1.0f, 0.0f };
	quad[2] = { -0.5f,   height - 0.5f,   0.0f, 1.0f, D3DCOLOR_ARGB(ApplyLumi(100), ApplyLumi(255), ApplyLumi(255), ApplyLumi(255)), 0.0f, 1.0f };
	quad[3] = { width - 0.5f,   height - 0.5f,   0.0f, 1.0f, D3DCOLOR_ARGB(ApplyLumi(100), ApplyLumi(255), ApplyLumi(255), ApplyLumi(255)), 1.0f, 1.0f };

	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	m_device->SetTexture(0, tex);
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(SCREENVERTEX));

	m_device->SetTexture(0, nullptr);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

// 별 그리기
void CSpaceRenderer::DrawStars(const CSpaceBackground& bg, const CCamera& camera)
{
	LPDIRECT3DTEXTURE9 starTex = m_textureManager->LoadTexture(L"star");

	if (!starTex) { return; }

	for (const auto& star : bg.GetStars())
	{
		float scale = star.size * (2500.0f / star.pos.z);
		scale = Clamp(scale, 0.35f, 9.0f);

		if		(star.brightness > 230.0f)	{ scale *= 2.2f; }
		else if (star.brightness > 190.0f)	{ scale *= 1.5f; }

		float fade = DistanceFade(star.pos.z, 0.0f, 7000.0f);
		int alpha = (int)(star.brightness * fade);

		alpha = (int)Clamp((float)alpha, 25.0f, 255.0f);

		int r = (star.color >> 16) & 0xff;
		int g = (star.color >> 8) & 0xff;
		int b = star.color & 0xff;

		DrawBillboard(
			star.pos,
			scale,
			D3DCOLOR_ARGB(ApplyLumi(alpha), ApplyLumi(r), ApplyLumi(g), ApplyLumi(b)),
			starTex,
			camera);
	}
}

// 성운 그리기
void CSpaceRenderer::DrawNebulas(const CSpaceBackground& bg, const CCamera& camera)
{
	for (const auto& nebula : bg.GetNebulas())
	{
		wstring texName = L"nebula" + to_wstring(nebula.texIndex);

		LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(texName);

		if (!tex) { continue; }

		int alpha = (int)(255.0f * nebula.alpha);

		DrawBillboard(
			nebula.pos,
			nebula.size + 200.0f,
			D3DCOLOR_ARGB(ApplyLumi(alpha), ApplyLumi(140), ApplyLumi(140), ApplyLumi(140)),
			tex,
			camera);
	}
}

// 회전 은하 그리기
void CSpaceRenderer::DrawGalaxy(const CSpaceBackground& bg, const CCamera& camera)
{

}

// 유성 그리기
void CSpaceRenderer::DrawMeteors(const CSpaceBackground& bg, const CCamera& camera)
{
	for (const auto& meteor : bg.GetMeteors())
	{
		if (!meteor.active) { continue; }

		DrawMeteorLine(meteor, camera);
	}
}

// 유성 출력 함수
// 여러 개의 반투명 폴리곤을 겹쳐서 유성의 꼬리, 잔상, 머리 Glow를 표현했다.

// 원리 : 
// 유성의 현재 위치를 Head로 잡는다.
// 유성 이동 방향의 반대 방향(Vec3 tailDir)을 tailDir로 잡는다
// Head에서 tailDir 방향으로 꼬리를 길게 만든다.
// 카메라 기준 Right/Up 방향을 이용해서 꼬리의 폭을 만든다.
// 넒은 외곽 꼬리, 밝은 중심 꼬리, 긴 잔상 꼬리를 삼각형 폴리곤으로 출력한다.
// 머리 부분은 DrawMetoeroGlowCircle()을 여러번 호출해서 
// 중심은 밝고 바깥으로 갈수록 희미해지는 원형 Glow를 만든다.
void CSpaceRenderer::DrawMeteorLine(const Meteor& meteor, const CCamera& camera)
{
	if (!m_device) return;

	// 유성의 머리 위치
	// 유성에서 가장 밝게 빛나게 할 중심점
	Vec3 head = meteor.pos;

	// 꼬리 방향 계산
	// 유성은 m.dir 방향으로 이동하므로
	// 꼬리는 그 반대 방향으로 생겨야 한다.
	Vec3 tailDir = meteor.dir * -2.5f;

	// 가까운 유성인지 먼 유성인지 나타내는 값
	// 0에 가까우면 먼 유성, 1에 가까우면 가까운 유성
	float nearRate = meteor.nearRate;
	if (nearRate < 0.0f) nearRate = 0.0f;
	if (nearRate > 1.0f) nearRate = 1.0f;

	// 유성 전체 밝기
	// 너무 낮으면 안보이고, 너무 높으면 하얗게 뭉개지므로 범위를 제한한다.
	int alpha = (int)meteor.brightness;
	if (alpha < 100) alpha = 100;
	if (alpha > 245) alpha = 245;

	// D3DCOLOR에서 RGB 값 분리하기 위한 계산
	int r = (meteor.color >> 16) & 0xff;
	int g = (meteor.color >> 8) & 0xff;
	int b = meteor.color & 0xff;

	// 꼬리의 좌우 폭 방향
	// camera.GetRight()만 쓰면 화면 기준으로 안정적인 폭이 나오고,
	// Up을 조금 섞으면 완전히 딱딱한 수평 리본 느낌이 줄어든다.
	Vec3 side = camera.GetRight() + (camera.GetUp() * 0.5f);
	Normalize(side);

	float realTailLength = meteor.tailLength * meteor.tailScale;
	float realSize = meteor.size * meteor.glowScale;

	// 머리 쪽 폭과 꼬리 끝 폭
	// 머리는 넒고, 꼬리 끝은 좁게 해서 유성 모양을 만든다.
	float headWidth = realSize * 0.48f;
	float tailWidth = realSize * 0.045f;

	Vec3 headSide = side * headWidth;
	Vec3 tailSide = side * tailWidth;

	// 꼬리 끝 위치
	Vec3 tailStart = head + (tailDir * (realSize * 0.1f));
	Vec3 tail = head + (tailDir * realTailLength);

	struct METEORVERT
	{
		float x, y, z;
		D3DCOLOR color;
	};

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	// 이미 월드 좌표로 직접 정점을 만들기 때문에 월드 행렬은 항등행렬을 사용
	m_device->SetTransform(D3DTS_WORLD, &identity);
	// 텍스처 없이 색상 폴리곤만 사용
	m_device->SetTexture(0, nullptr);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	// 반투명 + Additive Blend
	// 밝은 색이 배경 위에 더해져서 빛나는 느낌이 난다.
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// 1. 넓은 외곽 꼬리
	// 가장 넒은 빛 번짐 영역
	// 머리 부분은 넒고 밝게, 꼬리 끝은 좁고 약하게 만든다.
	METEORVERT outer[4];

	outer[0] =
	{
		tailStart.x + headSide.x * 1.4f,
		tailStart.y + headSide.y * 1.4f,
		tailStart.z + headSide.z * 1.4f,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 2),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b))
	};

	outer[1] =
	{
		tailStart.x - headSide.x * 1.4f,
		tailStart.y - headSide.y * 1.4f,
		tailStart.z - headSide.z * 1.4f,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 2),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b))
	};

	outer[2] =
	{
		tail.x + tailSide.x,
		tail.y + tailSide.y,
		tail.z + tailSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(45),
			ApplyLumi(r / 3),
			ApplyLumi(g / 3),
			ApplyLumi(b / 3))
	};

	outer[3] =
	{
		tail.x - tailSide.x,
		tail.y - tailSide.y,
		tail.z - tailSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(45),
			ApplyLumi(r / 3),
			ApplyLumi(g / 3),
			ApplyLumi(b / 3))
	};

	// TRIANGLESTRIP 정점 순서:
	// 0---1
	// |  /|
	// | / |
	// 2---3
	// 사각형 1개를 삼각형 2개로 그린다.
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, outer, sizeof(METEORVERT));

	// 2. 중심 밝은 꼬리
	// 유성의 핵심 빛줄기
	// 외곽 꼬리보다 좁고 휠씬 밝게 만든다.
	Vec3 coreTail = head + (tailDir * (meteor.tailLength * 0.75f));
	Vec3 coreSide = side * (meteor.size * 0.22f);
	Vec3 coreEndSide = side * (meteor.size * 0.025f);

	METEORVERT coreTailPoly[4];

	coreTailPoly[0] =
	{
		tailStart.x + coreSide.x,
		tailStart.y + coreSide.y,
		tailStart.z + coreSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha),
			ApplyLumi(245),
			ApplyLumi(250),
			ApplyLumi(255))
	};

	coreTailPoly[1] =
	{
		tailStart.x - coreSide.x,
		tailStart.y - coreSide.y,
		tailStart.z - coreSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha),
			ApplyLumi(245),
			ApplyLumi(250),
			ApplyLumi(255))
	};

	coreTailPoly[2] =
	{
		coreTail.x + coreEndSide.x,
		coreTail.y + coreEndSide.y,
		coreTail.z + coreEndSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(70),
			ApplyLumi(r / 2),
			ApplyLumi(g / 2),
			ApplyLumi(b / 2))
	};

	coreTailPoly[3] =
	{
		coreTail.x - coreEndSide.x,
		coreTail.y - coreEndSide.y,
		coreTail.z - coreEndSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(70),
			ApplyLumi(r / 2),
			ApplyLumi(g / 2),
			ApplyLumi(b / 2))
	};

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, coreTailPoly, sizeof(METEORVERT));

	// 3. 긴 잔상 꼬리
	// 유성이 지나간 자리에 남는 긴 빛줄기
	// 실제 꼬리보다 더 길고 얆게 만들어 속도감을 준다.
	Vec3 longTail = head + (tailDir * (meteor.tailLength * 1.35f));
	Vec3 longSideStart = side * (meteor.size * 0.12f);
	Vec3 longSideEnd = side * (meteor.size * 0.012f);

	METEORVERT longTrail[4];

	longTrail[0] =
	{
		tailStart.x + longSideStart.x,
		tailStart.y + longSideStart.y,
		tailStart.z + longSideStart.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 2),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b))
	};

	longTrail[1] =
	{
		tailStart.x - longSideStart.x,
		tailStart.y - longSideStart.y,
		tailStart.z - longSideStart.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 2),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b))
	};

	longTrail[2] =
	{
		longTail.x + longSideEnd.x,
		longTail.y + longSideEnd.y,
		longTail.z + longSideEnd.z,
		D3DCOLOR_ARGB(
			ApplyLumi(35),
			ApplyLumi(r / 4),
			ApplyLumi(g / 4),
			ApplyLumi(b / 4))
	};

	longTrail[3] =
	{
		longTail.x - longSideEnd.x,
		longTail.y - longSideEnd.y,
		longTail.z - longSideEnd.z,
		D3DCOLOR_ARGB(
			ApplyLumi(35),
			ApplyLumi(r / 4),
			ApplyLumi(g / 4),
			ApplyLumi(b / 4))
	};

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, longTrail, sizeof(METEORVERT));

	// 4. 원형 Glow
	// 사각형 빌보드 대신 삼각형 팬으로 원형에 가까운 Glow를 만든다.
	// 큰 원, 중간 원, 작은 원 3겹을 겹처서
	// 흰색 중심부 -> 푸른/보라 외곽 -> 투명한 끝 느낌을 만든다.

	// 큰 원 : 색이 넒게 퍼지는 외곽
	DrawMeteorGlowCircle(
		head,
		meteor.size * 1.9f,
		20,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 3),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b)),
		D3DCOLOR_ARGB(
			0,
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b)),
		camera);

	// 중간 원: 밝은 컬러 중심부
	DrawMeteorGlowCircle(
		head,
		meteor.size * 1.0f,
		18,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 2),
			ApplyLumi(210),
			ApplyLumi(230),
			ApplyLumi(255)),
		D3DCOLOR_ARGB(
			0,
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b)),
		camera);

	// 작은 원: 강한 흰색 핵
	DrawMeteorGlowCircle(
		head,
		meteor.size * 0.35f,
		16,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha),
			255,
			255,
			255),
		D3DCOLOR_ARGB(
			ApplyLumi(alpha / 6),
			ApplyLumi(230),
			ApplyLumi(240),
			ApplyLumi(255)),
		camera);

	m_device->SetTexture(0, nullptr);

	// 하위 함수이므로 알파 블렌드는 끄지 않는다.
	// DrawSpace()의 EndSpotLight()가 전체 상태를 정리한다.
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
}

// 원형 Glow 출력 함수
// 텍스처 없이 삼각형 팬(TRIANGLEFAN)을 사용해서
// 중심이 밝고 바깥으로 갈수록 투명해지는 원형 빛 번짐을 만든다.
// 1. 첫 번째 정점은 원의 중심.
// 2. 나머지 정점들은 원 둘레를 segment 개수만큼 나눈 위치.
// 3. 중심 정점에는 centerColor를 준다.
// 4. 둘레 정점에는 edgeColor를 준다.
// 5. TRIANGLEFAN으로 그리면 중심에서 둘레로 색상이 보간되어 부드러운 원형 Glow처럼 보인다.
void CSpaceRenderer::DrawMeteorGlowCircle(const Vec3& center, float radius, int segment, D3DCOLOR centerColor, D3DCOLOR edgeColor, const CCamera& camera)
{
	if (!m_device) return;
	// 너무 낮은 segment는 원이 아니라 다각형처럼 보이므로 최소 8 보장.
	if (segment < 8) segment = 8;

	struct GLOWVERT
	{
		float x, y, z;
		D3DCOLOR color;
	};

	// 카메라 기준 평면 생성.
	// right/up을 이용해서 항상 카메라를 바라보는 원형 평면을 만든다.
	Vec3 right = camera.GetRight();
	Vec3 up = camera.GetUp();

	// 중심 1개 + 둘레 segment + 마지막 닫는 점 1개
	GLOWVERT* v = new GLOWVERT[segment + 2];

	// 중심 정점
	v[0] =
	{
		center.x,
		center.y,
		center.z,
		centerColor
	};

	for (int i = 0; i <= segment; i++)
	{
		// 0 ~ 2PI까지 둘레를 돈다.
		float t = (2.0f * D3DX_PI * i) / segment;

		float c = cosf(t);
		float s = sinf(t);

		// 카메라 기준 right/up 방향으로 원 둘레 위치 계산
		Vec3 p =
			center +
			(right * (c * radius)) +
			(up * (s * radius));

		// 둘레 정점은 edgeColor
		// 보통 알파 0에 가깝게 줘서 바깥쪽이 자연스럽게 사라지게 한다.
		v[i + 1] =
		{
			p.x,
			p.y,
			p.z,
			edgeColor
		};
	}

	m_device->SetTexture(0, nullptr);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	m_device->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		segment,
		v,
		sizeof(GLOWVERT));

	delete[] v;
}

// 우주 먼지
void CSpaceRenderer::DrawDust(const CSpaceBackground& bg, const CCamera& camera)
{
	LPDIRECT3DTEXTURE9 tex = m_textureManager->LoadTexture(L"dust");

	if (!tex) { return; }

	for (const auto& dust : bg.GetDusts())
	{
		int alpha = (int)Clamp(dust.brightness, 10.0f, 70.0f);

		DrawBillboard(
			dust.pos,
			dust.size,
			D3DCOLOR_ARGB(ApplyLumi(alpha), ApplyLumi(120), ApplyLumi(160), ApplyLumi(255)),
			tex,
			camera);
	}
}

// 워프 별
void CSpaceRenderer::DrawWarpStars(const CSpaceBackground& bg, const CCamera& camera)
{
	const WarpEffect& warp = bg.GetWarp();

	if (!warp.active) return;

	int index = 0;

	for (const auto& star : bg.GetStars())
	{
		// 너무 많으면 화면이 하얗게 터지므로 절반만 출력
		if (index % 8 != 0)
		{
			index++;
			continue;
		}

		DrawWarpLine(star, camera, warp);

		index++;
	}
}

void CSpaceRenderer::DrawWarpLine(const StarParticle& star, const CCamera& camera, const WarpEffect& warp)
{
	if (!m_device) return;

	float power = warp.power;

	if (power < 0.0f) power = 0.0f;
	if (power > 1.0f) power = 1.0f;

	// 별 위치가 너무 중심에 있으면 방향 계산이 이상해지므로 제외
	Vec3 radial2D = Vec3(star.pos.x, star.pos.y, 0.0f);

	float centerDist = Length(radial2D);

	if (centerDist < 180.0f) { return; }

	Normalize(radial2D);

	// 화면 중심에서 바깥으로 뻗는 방향
	Vec3 outDir = (camera.GetRight() * radial2D.x) + (camera.GetUp() * radial2D.y);

	Normalize(outDir);

	// 라인의 두께 방향
	Vec3 side = (camera.GetRight() * -radial2D.y) + (camera.GetUp() * radial2D.x);

	Normalize(side);

	// 중심에 가까울수록 짧게, 바깥쪽 별일수록 길게
	float distRate = centerDist / 3200.0f;
	if (distRate < 0.0f) distRate = 0.0f;
	if (distRate > 1.0f) distRate = 1.0f;

	// 속도감 핵심 수치
	float length = 35.0f + (star.Stretch * 0.8f) + (power * 180.0f * distRate);
	// 너무 긴 선 방지
	if (length > 260.0f) { length = 260.0f; }

	float width = star.size * (0.8f + power * 1.2f);
	width = Clamp(width, 0.5f, 3.2f);

	Vec3 head = star.pos + (outDir * (length * 0.08f));
	// 꼬리는 화면 중심 쪽으로 남긴다.
	Vec3 tail = star.pos - (outDir * (length * 0.92f));

	Vec3 headSide = side * width;
	Vec3 tailSide = side * (width * 0.18f);

	int r = (star.color >> 16) & 0xff;
	int g = (star.color >> 8) & 0xff;
	int b = star.color & 0xff;

	int alpha = (int)(star.brightness * (0.8f + power));
	alpha = (int)Clamp((float)alpha, 60.0f, 255.0f);

	struct WARPVERTEX
	{
		float x, y, z;
		D3DCOLOR color;
	};

	WARPVERTEX quad[4];

	quad[0] =
	{
		head.x + headSide.x,
		head.y + headSide.y,
		head.z + headSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha),
			ApplyLumi(120),
			ApplyLumi(190),
			ApplyLumi(255))
	};

	quad[1] =
	{
		head.x - headSide.x,
		head.y - headSide.y,
		head.z - headSide.z,
		D3DCOLOR_ARGB(
			ApplyLumi(alpha),
			ApplyLumi(r),
			ApplyLumi(g),
			ApplyLumi(b))
	};

	quad[2] =
	{
		tail.x + tailSide.x,
		tail.y + tailSide.y,
		tail.z + tailSide.z,
		D3DCOLOR_ARGB(
			40,
			ApplyLumi(r / 3),
			ApplyLumi(g / 3),
			ApplyLumi(b / 3))
	};

	quad[3] =
	{
		tail.x - tailSide.x,
		tail.y - tailSide.y,
		tail.z - tailSide.z,
		D3DCOLOR_ARGB(
			40,
			ApplyLumi(r / 3),
			ApplyLumi(g / 3),
			ApplyLumi(b / 3))
	};

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);

	m_device->SetTransform(D3DTS_WORLD, &identity);
	m_device->SetTexture(0, nullptr);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	m_device->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP,
		2,
		quad,
		sizeof(WARPVERTEX));
}

// 일반 빌보드
// 빌보드는 3D 공간에 있는 평면 이미지가 항상 카메라를 바라보게 만드는 방식이야
// 예를 들면 별, 먼지, 성운, 이펙트, 체력바 같은 것들이 실제 3D 모델이 아니라 사각형 이미지인데도 자연스럽게 보이는 이유
void CSpaceRenderer::DrawBillboard(Vec3 pos, float size, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera)
{
	if (!tex) { return; }
	if (!m_device) { return; }

	Vec3 right = camera.GetRight() * (size * 0.5f);
	Vec3 up = camera.GetUp() * (size * 0.5f);
	// 카메라는 보통 3개의 방향 벡터가 존재하다.
	// forward : 카메라가 바라보는 방향
	// Right : 카메라 기준 오른쪽 방향
	// Up : 카메라 기존 위쪽 방향
	// 위에서는 forward에 대한 계산이 없지만
	// Right와 Up이 이미 Forward를 기준으로 만들어졌기 때문에
	// Forward를 직접 사용하지 않아도
	// 결과적으로 카메라 정면을 향하는 평면이 만들어진다.

	// pos는 빌보드 중심점을 뜻한다.
	Vec3 p0 = pos - right + up;		// 왼쪽 위
	Vec3 p1 = pos + right + up;		// 오른쪽 위
	Vec3 p2 = pos - right - up;		// 왼쪽 아래
	Vec3 p3 = pos + right - up;		// 오른쪽 아래
	// 카메라 기준 오른쪽/ 위쪽 방향으로 사각형 네개의 꼭짓점을 만든다.
	//그럼 왜 항상 카메라를 보게 되냐?
	// 카메라가 오른쪽으로 돌면 camera.GetRight()와 camera.GetUp()도 같이 바뀐다.
	// 그러면 빌보드 사각형의 네점도 카메라 회전에 맞춰 새로 계산된다.
	// 그래서 사각형이 월드에 고정된 평면이 아니라, 매 프레임 카메라 방향에 맞춰 다시 만들어진다.

	// Vec3 worldUp = Vec3(0.0f, 1.0f, 0.0f);
	// Right = worldUp x Forward;
	// Up = Forward x Right;

	CUSTOMVERTEX quad[4];

	quad[0] = { p0.x, p0.y, p0.z, color, 0.0f, 0.0f };
	quad[1] = { p1.x, p1.y, p1.z, color, 1.0f, 0.0f };
	quad[2] = { p2.x, p2.y, p2.z, color, 0.0f, 1.0f };
	quad[3] = { p3.x, p3.y, p3.z, color, 1.0f, 1.0f };

	D3DXMATRIX identity;
	// 월드에 아무 변환도 하지 않는 행렬
	// 위에서 이미 모든 좌표 계산이 완료되었기에
	// 만약 여기서 한번더 계산을 하면 리소스 낭비가 된다.
	D3DXMatrixIdentity(&identity);

	m_device->SetTransform(D3DTS_WORLD, &identity);

	m_device->SetTexture(0, tex);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_device->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP,
		2,
		quad,
		sizeof(CUSTOMVERTEX));

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

// 길쭉한 워프 빌보드
void CSpaceRenderer::DrawStretchBillboard( const Vec3& pos, float width, float height, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera, const Vec3& stretchDir)
{
	if (!tex) { return; }

	D3DXMATRIX identity;

	Vec3 right = camera.GetRight() * width;
	Vec3 up = camera.GetUp() * width;

	Vec3 stretch = stretchDir;
	Normalize(stretch);
	stretch = stretch * height;

	Vec3 p0 = pos - right + up;
	Vec3 p1 = pos + right + up;
	Vec3 p2 = pos - right - up - stretch;
	Vec3 p3 = pos + right - up - stretch;

	CUSTOMVERTEX quad[4];

	quad[0] = { p0.x, p0.y, p0.z, color, 0.0f, 0.0f };
	quad[1] = { p1.x, p1.y, p1.z, color, 1.0f, 0.0f };
	quad[2] = { p2.x, p2.y, p2.z, color, 0.0f, 1.0f };
	quad[3] = { p3.x, p3.y, p3.z, color, 1.0f, 1.0f };

	D3DXMatrixIdentity(&identity);

	m_device->SetTransform(D3DTS_WORLD, &identity);
	m_device->SetTexture(0, tex);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(CUSTOMVERTEX));

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CSpaceRenderer::DrawRotatedBillboard(Vec3 pos, float width, float height, float angle, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera)
{
	if (!tex) { return; }
	if (!m_device) { return; }

	Vec3 right = camera.GetRight();
	Vec3 up = camera.GetUp();

	float c = cosf(angle);
	float s = sinf(angle);

	Vec3 localRight = (right * c) + (up * s);
	Vec3 localUp = (right * -s) + (up * c);

	localRight = localRight * (width * 0.5f);
	localUp = localUp* (height * 0.5f);

	Vec3 p0 = pos - localRight + localUp;
	Vec3 p1 = pos + localRight + localUp;
	Vec3 p2 = pos - localRight - localUp;
	Vec3 p3 = pos + localRight - localUp;

	CUSTOMVERTEX quad[4];

	quad[0] = { p0.x, p0.y, p0.z, color, 0.0f, 0.0f };
	quad[1] = { p1.x, p1.y, p1.z, color, 1.0f, 0.0f };
	quad[2] = { p2.x, p2.y, p2.z, color, 0.0f, 1.0f };
	quad[3] = { p3.x, p3.y, p3.z, color, 1.0f, 1.0f };

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);

	m_device->SetTransform(D3DTS_WORLD, &identity);
	m_device->SetTexture(0, tex);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(CUSTOMVERTEX));

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

// Additive 시작
void CSpaceRenderer::BeginAdditive()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
}

// Additive 종료
void CSpaceRenderer::EndAdditive()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

// Alpha 시작
void CSpaceRenderer::BeginAlpha()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

// Alpha 종료
void CSpaceRenderer::EndAlpha()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CSpaceRenderer::BeginSpotLight()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	// 검은 배경 날리고 밝은 부분만 더함
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

void CSpaceRenderer::EndSpotLight()
{
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

// 배경 밝기 조절용 함수
int CSpaceRenderer::ApplyLumi(int value)
{
	float t = g_GameSetting.luminosity; // 0.0f ~ 1.0f

	int result = value;

	if (t < 0.5f)
	{
		// 0.0 ~ 0.5 구간: 어둡게
		float dark = 0.25f + (t * 1.5f);
		result = (int)(value * dark);
	}

	else
	{
		// 0.5 ~ 1.0 구간: 밝게
		float bright = (t - 0.5f) * 2.0f; // 0 ~ 1
		result = value + (int)((255 - value) * bright);
	}

	if (result < 0) result = 0;
	if (result > 255) result = 255;

	return result;
}