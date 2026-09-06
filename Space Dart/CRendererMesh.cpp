#include "CRenderer3D.h"
#include "Math.h"

// 과녁 생성 함수
void CRenderer3D::CreateTarget()
{
	// 과녁은 40각으로 이루어진 형태 조금더 형태를 다듬어서 완벽한 원형을 목표
	const int SEG = 40;
	float radius = 1.0f;	// 과녁의 사이즈

	m_TargetMesh.vCount = SEG + 1;
	m_TargetMesh.iCount = SEG * 3;

	m_device->CreateVertexBuffer(
		m_TargetMesh.vCount * sizeof(CUSTOMVERTEX),
		0,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&m_TargetMesh.vb,
		NULL);

	CUSTOMVERTEX* v;
	m_TargetMesh.vb->Lock(0, 0, (void**)&v, 0);

	v[0].x = 0.0f;
	v[0].y = 0.0f;
	v[0].z = 0.0f;
	v[0].color = D3DCOLOR_XRGB(255, 255, 255);
	v[0].u = 0.5f;
	v[0].v = 0.5f;
	/*
	 원 생성 공식

	x = r * cos(θ)
	y = r * sin(θ)

	θ = 0 ~ 2π

	→ 원은 각도를 조금씩 증가시키면서 점을 찍으면 만들어짐

	θ = (2π * i) / SEG
	*/

	for (int i = 0; i < SEG; i++)
	{
		float angle = (2.0f * D3DX_PI * i) / SEG;

		float x = cosf(angle) * radius;
		float y = sinf(angle) * radius;

		v[i + 1].x = x;
		v[i + 1].y = y;
		v[i + 1].z = 0.0f;

		v[i + 1].color = D3DCOLOR_XRGB(255, 255, 255);

		// 텍스쳐 좌표
		v[i + 1].u = (x / (radius * 2.0f)) + 0.5f;
		v[i + 1].v = (-y / (radius * 2.0f)) + 0.5f;
	}

	m_TargetMesh.vb->Unlock();

	m_device->CreateIndexBuffer(
		m_TargetMesh.iCount * sizeof(WORD),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_TargetMesh.ib,
		NULL);

	WORD* idx;
	m_TargetMesh.ib->Lock(0, 0, (void**)&idx, 0);

	for (int i = 0; i < SEG; i++)
	{
		idx[i * 3 + 0] = 0;
		idx[i * 3 + 1] = i + 1;
		idx[i * 3 + 2] = (i + 1) % SEG + 1;
	}

	m_TargetMesh.ib->Unlock();
}

// Planet 생성 함수
// 만약 구형인 모양에 이미지파일을 입힐거면 이미지파일도 그와 똑같이 생겨야한다
// 예를들면 구형이면 이미지파일도 구형을 평면으로 !펼친! 모양이여야한다
// 더 알기 쉽게는 구 형을 그리고 그위에 지구 이미지를 입힐려면
// 그 구형을 평면으로 펼친 이미지 파일을 가지고있어야한다.
void CRenderer3D::CreatePlanet()
{
	const int STACK = 20;
	const int SLICE = 40;
	float radius = 2.5f;	// 반구의 사이즈

	m_PlanetMesh.vCount = (STACK + 1) * (SLICE + 1);

	m_PlanetMesh.iCount = STACK * SLICE * 6;

	m_device->CreateVertexBuffer(
		m_PlanetMesh.vCount * sizeof(CUSTOMVERTEX),
		0,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&m_PlanetMesh.vb,
		NULL);

	CUSTOMVERTEX* v;

	m_PlanetMesh.vb->Lock(0, 0, (void**)&v, 0);

	int index = 0;

	for (int stack = 0; stack <= STACK; stack++)
	{
		float phi = (D3DX_PI / 2.0f) * stack / STACK;

		for (int slice = 0; slice <= SLICE; slice++)
		{
			float theta = (2.0f * D3DX_PI) * slice / SLICE;

			float x = radius * sinf(phi) * cosf(theta);
			float y = radius * sinf(phi) * sinf(theta);
			float z = radius * cosf(phi) + 0.02f;

			v[index].x = x;
			v[index].y = y;
			v[index].z = z;

			v[index].color = D3DCOLOR_XRGB(255, 255, 255);
			v[index].u = (float)slice / SLICE;
			v[index].v = (float)stack / STACK;

			index++;
		}
	}

	m_PlanetMesh.vb->Unlock();

	m_device->CreateIndexBuffer(
		m_PlanetMesh.iCount * sizeof(WORD),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_PlanetMesh.ib,
		NULL);

	WORD* idx;

	m_PlanetMesh.ib->Lock(0, 0, (void**)&idx, 0);

	int id = 0;

	for (int stack = 0; stack < STACK; stack++)
	{
		for (int slice = 0; slice < SLICE; slice++)
		{
			int cur = stack * (SLICE + 1) + slice;

			int next = cur + SLICE + 1;

			idx[id++] = cur;
			idx[id++] = next;
			idx[id++] = cur + 1;

			idx[id++] = cur + 1;
			idx[id++] = next;
			idx[id++] = next + 1;
		}
	}

	m_PlanetMesh.ib->Unlock();
}
// u = SLICE / slice?, v = STACK / stack?

// 다트 생성 함수
void CRenderer3D::CreateDart()
{
	const int SEG = 20;

	float bodyRadius = 0.1f;		// 몸통 굵기
	float bodyLength = 0.7f;		// 몸통 길이
	float tipLength = 0.6f;			// 원뿔길이

	int vCount = SEG * 2 + 1 + 8; // 원통 + 팁 + 날개
	int iCount = SEG * 6 + SEG * 3 + 12;

	m_DartMesh.vCount = vCount;
	m_DartMesh.iCount = iCount;

	m_device->CreateVertexBuffer(
		vCount * sizeof(CUSTOMVERTEX),
		0,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&m_DartMesh.vb,
		NULL);

	CUSTOMVERTEX* v;
	m_DartMesh.vb->Lock(0, 0, (void**)&v, 0);

	// 원통(몸통)
	int idx = 0;

	for (int i = 0; i < SEG; i++)
	{
		float angle = (2 * D3DX_PI * i) / SEG;

		float x = cosf(angle) * bodyRadius;
		float y = sinf(angle) * bodyRadius;

		// 앞
		v[idx++] = { x, y, 0.0f,D3DCOLOR_XRGB(200,200,200), 0.0f, 0.0f };

		// 뒤
		v[idx++] = { x, y, -bodyLength, D3DCOLOR_XRGB(200,200,200), 0.0f, 0.0f };
	}

	// 팁(원뿔)
	int tipIndex = idx;

	v[idx++] = { 0.0f,0.0f,tipLength,D3DCOLOR_XRGB(255,255,0), 0.0f, 0.0f };

	// 날개(십자)
	int flightStart = idx;

	float f = 0.17f;
	float z = -bodyLength - 0.18f;

	//x축 날개
	v[idx++] = { -f, 0, z, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { f, 0, z, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { f, 0, z + 0.2f, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { -f, 0, z + 0.2f, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };

	// Y축 날개
	v[idx++] = { 0, -f, z, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { 0,  f, z, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { 0,  f, z + 0.2f, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[idx++] = { 0, -f, z + 0.2f, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };

	m_DartMesh.vb->Unlock();

	m_device->CreateIndexBuffer(
		iCount * sizeof(WORD),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_DartMesh.ib,
		NULL);

	WORD* index;
	m_DartMesh.ib->Lock(0, 0, (void**)&index, 0);

	int id = 0;

	// 원통 인덱스
	for (int i = 0; i < SEG; i++)
	{
		int i0 = i * 2;
		int i1 = (i * 2 + 2) % (SEG * 2);
		int i2 = i0 + 1;
		int i3 = i1 + 1;

		index[id++] = i0;
		index[id++] = i1;
		index[id++] = i2;

		index[id++] = i2;
		index[id++] = i1;
		index[id++] = i3;
	}

	// 팁 인덱스
	for (int i = 0; i < SEG; i++)
	{
		int i0 = i * 2;
		int i1 = (i * 2 + 2) % (SEG * 2);

		index[id++] = tipIndex;
		index[id++] = i0;
		index[id++] = i1;
	}

	// 날개 인덱스
	for (int i = 0; i < 2; i++)
	{
		int base = flightStart + i * 4;

		index[id++] = base;
		index[id++] = base + 1;
		index[id++] = base + 2;

		index[id++] = base;
		index[id++] = base + 2;
		index[id++] = base + 3;
	}

	m_DartMesh.ib->Unlock();
}

// 국룰적인 연노랑색 아님 아무런 배경없이 투명배경에 화이트톤 물음표
// 큐브를 먹었을때 랜덤한 아이템 아님 이미 랜덤한 아이템이 들어가있는 큐브
void CRenderer3D::CreateCube()
{
	m_CubeMesh.vCount = 8;
	m_CubeMesh.iCount = 36;

	float size = 0.15f;

	m_device->CreateVertexBuffer(
		m_CubeMesh.vCount * sizeof(CUSTOMVERTEX),
		0,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&m_CubeMesh.vb,
		NULL);

	CUSTOMVERTEX* v;
	m_CubeMesh.vb->Lock(0, 0, (void**)&v, 0);

	// 큐브의 8개 꼭짓점
	v[0] = { -size, -size, -size, D3DCOLOR_XRGB(255,0,0), 0.0f, 0.0f };
	v[1] = { -size,  size, -size, D3DCOLOR_XRGB(0,255,0), 0.0f, 0.0f };
	v[2] = { size,  size, -size, D3DCOLOR_XRGB(0,0,255), 0.0f, 0.0f };
	v[3] = { size, -size, -size, D3DCOLOR_XRGB(255,255,0), 0.0f, 0.0f };

	v[4] = { -size, -size,  size, D3DCOLOR_XRGB(0,255,255), 0.0f, 0.0f };
	v[5] = { -size,  size,  size, D3DCOLOR_XRGB(255,0,255), 0.0f, 0.0f };
	v[6] = { size,  size,  size, D3DCOLOR_XRGB(255,255,255), 0.0f, 0.0f };
	v[7] = { size, -size,  size, D3DCOLOR_XRGB(100,100,100), 0.0f, 0.0f };

	m_CubeMesh.vb->Unlock();

	m_device->CreateIndexBuffer(
		m_CubeMesh.iCount * sizeof(WORD),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_CubeMesh.ib,
		NULL);

	WORD* idx;
	m_CubeMesh.ib->Lock(0, 0, (void**)&idx, 0);

	WORD index[] =
	{
		// 앞면
		0,1,2, 0,2,3,
		// 뒷면
		4,6,5, 4,7,6,
		// 왼쪽
		4,5,1, 4,1,0,
		// 오른쪽
		3,2,6, 3,6,7,
		// 위
		1,5,6, 1,6,2,
		// 아래
		4,0,3, 4,3,7
	};

	for (int i = 0; i < 36; i++)
	{
		idx[i] = index[i];
	}

	m_CubeMesh.ib->Unlock();
}

void CRenderer3D::CreateTitleRocket()
{
	if (!m_device) { return; }

	if (m_TitleRocketMesh.vb)
	{
		m_TitleRocketMesh.vb->Release();
		m_TitleRocketMesh.vb = nullptr;
	}

	if (m_TitleRocketMesh.ib)
	{
		m_TitleRocketMesh.ib->Release();
		m_TitleRocketMesh.ib = nullptr;
	}

	// 기존 CreateDart(), CreateCube()처럼
	// 로켓도 한 번 메쉬로 만들어두고 DrawTitleRocket()에서는 그리기만 한다.
	// 이렇게 해야 매 프레임 정점을 새로 만드는 것보다 관리가 쉽고 안정적이다.

	vector<CUSTOMVERTEX> vertices;
	vector<WORD> indices;

	const int SEG = 32;

	// 로켓은 local z축 방향으로 앞을 보게 만든다.
	// DrawTitleRocket()에서 forward/right/up 행렬을 이용해 원하는 방향으로 돌린다.
	float bodyRadius = 0.18f;
	float bodyLength = 1.95f;
	float noseLength = 0.62f;

	float tailZ = -bodyLength * 0.48f;
	float bodyFrontZ = bodyLength * 0.34f;
	float noseTipZ = bodyFrontZ + noseLength;

	auto AddVertex = [&](float x, float y, float z, D3DCOLOR color) -> WORD
		{
			CUSTOMVERTEX v;
			v.x = x;
			v.y = y;
			v.z = z;
			v.color = color;
			v.u = 0.0f;
			v.v = 0.0f;

			vertices.push_back(v);
			return (WORD)(vertices.size() - 1);
		};

	auto AddTri = [&](WORD a, WORD b, WORD c)
		{
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		};

	// 몸체 원통
	for (int i = 0; i < SEG; i++)
	{
		float a0 = (2.0f * D3DX_PI * i) / SEG;
		float a1 = (2.0f * D3DX_PI * (i + 1)) / SEG;

		float x0 = cosf(a0) * bodyRadius;
		float y0 = sinf(a0) * bodyRadius;
		float x1 = cosf(a1) * bodyRadius;
		float y1 = sinf(a1) * bodyRadius;

		// sin 값으로 간단한 명암을 준다.
		// 위쪽은 밝고 아래쪽은 어둡게 해서 원통처럼 보이게 한다.
		float light0 = 0.55f + (0.45f * ((sinf(a0) + 1.0f) * 0.5f));
		float light1 = 0.55f + (0.45f * ((sinf(a1) + 1.0f) * 0.5f));

		D3DCOLOR c0 = D3DCOLOR_XRGB((int)(215 * light0), (int)(225 * light0), (int)(245 * light0));
		D3DCOLOR c1 = D3DCOLOR_XRGB((int)(215 * light1), (int)(225 * light1), (int)(245 * light1));

		WORD p0 = AddVertex(x0, y0, tailZ, c0);
		WORD p1 = AddVertex(x1, y1, tailZ, c1);
		WORD p2 = AddVertex(x0, y0, bodyFrontZ, c0);
		WORD p3 = AddVertex(x1, y1, bodyFrontZ, c1);

		AddTri(p0, p2, p1);
		AddTri(p1, p2, p3);
	}

	// 노즈콘
	WORD noseTip = AddVertex(
		0.0f,
		0.0f,
		noseTipZ,
		D3DCOLOR_XRGB(255, 255, 255));

	for (int i = 0; i < SEG; i++)
	{
		float a0 = (2.0f * D3DX_PI * i) / SEG;
		float a1 = (2.0f * D3DX_PI * (i + 1)) / SEG;

		WORD p0 = AddVertex(cosf(a0) * bodyRadius, sinf(a0) * bodyRadius, bodyFrontZ, D3DCOLOR_XRGB(220, 230, 250));
		WORD p1 = AddVertex(cosf(a1) * bodyRadius, sinf(a1) * bodyRadius, bodyFrontZ, D3DCOLOR_XRGB(195, 215, 245));

		AddTri(p0, noseTip, p1);
	}

	// 엔진 캡
	WORD tailCenter = AddVertex(
		0.0f,
		0.0f,
		tailZ - 0.10f,
		D3DCOLOR_XRGB(45, 50, 70));

	for (int i = 0; i < SEG; i++)
	{
		float a0 = (2.0f * D3DX_PI * i) / SEG;
		float a1 = (2.0f * D3DX_PI * (i + 1)) / SEG;

		WORD p0 = AddVertex(cosf(a0) * bodyRadius, sinf(a0) * bodyRadius, tailZ, D3DCOLOR_XRGB(145, 160, 190));
		WORD p1 = AddVertex(cosf(a1) * bodyRadius, sinf(a1) * bodyRadius, tailZ, D3DCOLOR_XRGB(100, 115, 145));

		AddTri(tailCenter, p1, p0);
	}

	// 날개
	float finBackZ = tailZ + 0.08f;
	float finFrontZ = tailZ + 0.62f;
	float finOut = bodyRadius * 2.05f;

	WORD r0 = AddVertex(bodyRadius * 0.72f, 0.0f, finBackZ, D3DCOLOR_XRGB(170, 205, 255));
	WORD r1 = AddVertex(finOut, 0.0f, finBackZ - 0.24f, D3DCOLOR_XRGB(50, 100, 210));
	WORD r2 = AddVertex(bodyRadius * 0.72f, 0.0f, finFrontZ, D3DCOLOR_XRGB(230, 245, 255));
	AddTri(r0, r1, r2);

	WORD l0 = AddVertex(-bodyRadius * 0.72f, 0.0f, finBackZ, D3DCOLOR_XRGB(170, 205, 255));
	WORD l1 = AddVertex(-finOut, 0.0f, finBackZ - 0.24f, D3DCOLOR_XRGB(50, 100, 210));
	WORD l2 = AddVertex(-bodyRadius * 0.72f, 0.0f, finFrontZ, D3DCOLOR_XRGB(230, 245, 255));
	AddTri(l2, l1, l0);

	WORD d0 = AddVertex(0.0f, -bodyRadius * 0.72f, finBackZ, D3DCOLOR_XRGB(135, 165, 230));
	WORD d1 = AddVertex(0.0f, -finOut, finBackZ - 0.24f, D3DCOLOR_XRGB(35, 70, 160));
	WORD d2 = AddVertex(0.0f, -bodyRadius * 0.72f, finFrontZ, D3DCOLOR_XRGB(200, 225, 255));
	AddTri(d0, d1, d2);

	// 창문
	auto AddWindow = [&](float localZ)
		{
			float winR = 0.06f;
			float winY = bodyRadius * 1.04f;

			WORD center = AddVertex(0.0f, winY, localZ, D3DCOLOR_XRGB(230, 250, 255));

			for (int i = 0; i < 18; i++)
			{
				float a0 = (2.0f * D3DX_PI * i) / 18;
				float a1 = (2.0f * D3DX_PI * (i + 1)) / 18;

				// 창문은 로켓 위쪽 표면에 붙은 원판처럼 만든다.
				// x/z 평면으로 원을 만들고 y는 몸체 표면 쪽으로 고정한다.
				WORD p0 = AddVertex(cosf(a0) * winR, winY + 0.01f, localZ + sinf(a0) * winR, D3DCOLOR_XRGB(35, 130, 255));
				WORD p1 = AddVertex(cosf(a1) * winR, winY + 0.01f, localZ + sinf(a1) * winR, D3DCOLOR_XRGB(10, 55, 150));

				AddTri(center, p0, p1);
			}
		};

	AddWindow(tailZ + 0.75f);
	AddWindow(tailZ + 1.10f);
	AddWindow(tailZ + 1.45f);

	m_TitleRocketMesh.vCount = (int)vertices.size();
	m_TitleRocketMesh.iCount = (int)indices.size();

	m_device->CreateVertexBuffer(
		m_TitleRocketMesh.vCount * sizeof(CUSTOMVERTEX),
		0,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
		D3DPOOL_MANAGED,
		&m_TitleRocketMesh.vb,
		NULL);

	CUSTOMVERTEX* v = nullptr;

	if (SUCCEEDED(m_TitleRocketMesh.vb->Lock(0, 0, (void**)&v, 0)))
	{
		for (int i = 0; i < m_TitleRocketMesh.vCount; i++)
		{
			v[i] = vertices[i];
		}

		m_TitleRocketMesh.vb->Unlock();
	}

	m_device->CreateIndexBuffer(
		m_TitleRocketMesh.iCount * sizeof(WORD),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_TitleRocketMesh.ib,
		NULL);

	WORD* idx = nullptr;

	if (SUCCEEDED(m_TitleRocketMesh.ib->Lock(0, 0, (void**)&idx, 0)))
	{
		for (int i = 0; i < m_TitleRocketMesh.iCount; i++)
		{
			idx[i] = indices[i];
		}

		m_TitleRocketMesh.ib->Unlock();
	}
}

//---------------------  Draw 함수 ----------------------------------

// 과녁 그리기 함수
void CRenderer3D::DrawTarget(float x, float y, float z, float radius)
{
	D3DXMATRIX matWorld, matScale, matTrans;

	// CreateTarget에서 반지름 1.0짜리 원을 만들었으므로
	// 여기서 실제 과녁 반지름만큼 스케일한다.
	D3DXMatrixScaling(&matScale, radius, radius, 1.0f);
	D3DXMatrixTranslation(&matTrans, x, y, z);

	matWorld = matScale * matTrans;

	m_device->SetTransform(D3DTS_WORLD, &matWorld);

	m_device->SetTexture(0, m_TargetTexture);
	m_device->SetStreamSource(0, m_TargetMesh.vb, 0, sizeof(CUSTOMVERTEX));
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetIndices(m_TargetMesh.ib);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		m_TargetMesh.vCount,
		0,
		m_TargetMesh.iCount / 3);

	m_device->SetTexture(0, NULL);
}

void CRenderer3D::DrawPlanet(float x, float y, float z, float radius)
{
	D3DXMATRIX matWorld, matScale, matTrans;

	D3DXMatrixScaling(&matScale, radius / 2.5f, radius / 2.5f, radius / 2.5f);
	D3DXMatrixTranslation(&matTrans, x, y, z);

	matWorld = matScale * matTrans;

	m_device->SetTransform(D3DTS_WORLD, &matWorld);

	m_device->SetTexture(0, m_PlanetTexture);
	m_device->SetStreamSource(0, m_PlanetMesh.vb, 0, sizeof(CUSTOMVERTEX));
	m_device->SetIndices(m_PlanetMesh.ib);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		0,
		m_PlanetMesh.vCount,
		0,
		m_PlanetMesh.iCount / 3 );

	m_device->SetTexture(0, NULL);
}

// 다트 그리기 함수
void CRenderer3D::DrawDart(const Vec3& pos, const Vec3& dir, DartOwner owner)
{
	CUSTOMVERTEX* v = nullptr;

	if (SUCCEEDED(m_DartMesh.vb->Lock(0, 0, (void**)&v, 0)))
	{
		D3DCOLOR dartColor =
			(owner == E_PLAYER)
			? D3DCOLOR_XRGB(0, 90, 255)
			: D3DCOLOR_XRGB(255, 40, 40);

		int tipIndex = 20 * 2;
		int flightStart = 20 * 2 + 1;

		v[tipIndex].color = dartColor;

		for (int i = 0; i < 8; i++)
		{
			v[flightStart + i].color = dartColor;
		}

		m_DartMesh.vb->Unlock();
	}

	// forward 벡터(다트가 바라보는 방향)
	Vec3 forward = dir;

	if (Length(forward) < 0.0001f) { forward = Vec3(0.0f,0.0f,1.0f); }

	// 정규화 (안전장치)
	Normalize(forward);

	// 기준 up 벡터 (월드 기준 위쪽)
	Vec3 up = { 0.0f,1.5f,0.0f };

	// right 벡터 계산 (외적 : up * forward)
	Vec3 right;
	right.x = up.y * forward.z - up.z * forward.y;
	right.y = up.z * forward.x - up.x * forward.z;
	right.z = up.x * forward.y - up.y * forward.x;

	Normalize(right);

	// 다시 up 조정 (forward * right)
	// 기존 up은 틀어졌을 수 있기 때문에 안전하게 다시 계산
	up.x = forward.y * right.z - forward.z * right.y;
	up.y = forward.z * right.x - forward.x * right.z;
	up.z = forward.x * right.y - forward.y * right.x;

	// 5. 월드 행렬 구성
	// DirectX는 행렬을 이렇게 채운다:
	//
	// [ right.x   right.y   right.z   0 ]
	// [ up.x      up.y      up.z      0 ]
	// [ forward.x forward.y forward.z 0 ]
	// [ pos.x     pos.y     pos.z     1 ]
	//
	// 이게 "회전 + 위치"를 동시에 포함한 행렬
	D3DXMATRIX matWorld =
	{
		right.x,   right.y,   right.z,   0.0f,
		up.x,      up.y,      up.z,      0.0f,
		forward.x, forward.y, forward.z, 0.0f,
		pos.x,     pos.y,     pos.z,     1.0f
	};

	// 월드 행렬 작용
	m_device->SetTransform(D3DTS_WORLD, &matWorld);

	// 랜더링
	m_device->SetStreamSource(0, m_DartMesh.vb, 0, sizeof(CUSTOMVERTEX));
	m_device->SetIndices(m_DartMesh.ib);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_DartMesh.vCount, 0, m_DartMesh.iCount / 3);
}

void CRenderer3D::DrawCube(const Instance& inst)
{
	D3DXMATRIX matWorld;

	D3DXMATRIX matT, matRX, matRY, matRZ, matS;

	D3DXMatrixScaling(&matS, inst.scale.x, inst.scale.y, inst.scale.z);

	D3DXMatrixRotationX(&matRX, inst.rot.x);
	D3DXMatrixRotationY(&matRY, inst.rot.y);
	D3DXMatrixRotationZ(&matRZ, inst.rot.z);

	D3DXMatrixTranslation(&matT, inst.pos.x, inst.pos.y, inst.pos.z);

	matWorld = matS * matRX * matRY * matRZ * matT;

	m_device->SetTransform(D3DTS_WORLD, &matWorld);

	m_device->SetStreamSource(0, m_CubeMesh.vb, 0, sizeof(CUSTOMVERTEX));
	m_device->SetIndices(m_CubeMesh.ib);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_CubeMesh.vCount, 0, m_CubeMesh.iCount / 3);
}

void CRenderer3D::CreateTitleText()
{
	if (!m_device) { return; }

	if (m_TitleTextMesh)
	{
		m_TitleTextMesh->Release();
		m_TitleTextMesh = nullptr;
	}

	m_TitleSpaceTextMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleSpaceTextMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleDartTextMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleDartTextMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	HDC hdc = CreateCompatibleDC(NULL);
	if (!hdc) { return; }

	HFONT font = CreateFontW(
		120,
		0,
		0,
		0,
		FW_HEAVY,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Arial Black");

	HFONT oldFont = (HFONT)SelectObject(hdc, font);

	HRESULT hr = D3DXCreateTextW(
		m_device,			// 3D 텍스트 메시를 만들 Direct3D 장치
		hdc,				// 어떤 폰트 모양으로 글자를 만들지 알려주는 GDI HDC
		L"SPACE DART",		// 3D 메시로 만들 실제 문자열
		0.01f,				// 글자 곡선 정밀도. 값이 작을수록 곡선이 부드럽지만 정점 수가 늘 수 있음
		0.46f,				// 글자의 두께. 2D 글자가 아니라 앞뒤 두께가 있는 3D 글자로 만들어짐
		&m_TitleTextMesh,	// 완성된 3D 글자 메시를 저장할 포인터
		nullptr,			// 인접 정보. 지금은 사용하지 않으므로 nullptr
		nullptr);			// 글자별 메시 정보. 지금은 사용하지 않으므로 nullptr

	if (SUCCEEDED(hr) && m_TitleTextMesh)
	{
		void* vertexData = nullptr;

		if (SUCCEEDED(m_TitleTextMesh->LockVertexBuffer(0, &vertexData)))
		{
			DWORD stride = m_TitleTextMesh->GetNumBytesPerVertex();

			if (stride > 0)
			{
				// 1차 BoundingBox 계산.
				// 변형 전 글자의 왼쪽/오른쪽 범위를 알아야
				// 각 정점이 SPACE 쪽인지 DART 쪽인지 판단할 수 있다.
				D3DXComputeBoundingBox(
					(D3DXVECTOR3*)vertexData,
					m_TitleTextMesh->GetNumVertices(),
					stride,
					&m_TitleTextMin,
					&m_TitleTextMax);

				float minX = m_TitleTextMin.x;
				float maxX = m_TitleTextMax.x;
				float width = maxX - minX;

				if (width <= 0.0001f) { width = 1.0f; }

				BYTE* bytes = (BYTE*)vertexData;

				for (DWORD i = 0; i < m_TitleTextMesh->GetNumVertices(); i++)
				{
					D3DXVECTOR3* pos = (D3DXVECTOR3*)(bytes + (i * stride));

					// t = 0이면 왼쪽 SPACE 쪽
					// t = 1이면 오른쪽 DART 쪽
					float t = (pos->x - minX) / width;

					if (t < 0.0f) { t = 0.0f; }
					if (t > 1.0f) { t = 1.0f; }

					float centerX = (minX + maxX) * 0.5f;

					// 왼쪽은 좁게, 오른쪽은 넓게.
					// 전체 메쉬를 사다리꼴처럼 보이게 만드는 핵심값이다.
					float yScale = 1.5f + (t * 3.0f);

					// 왼쪽 SPACE는 z축 안쪽으로 더 들어가고,
					// 오른쪽 DART는 상대적으로 앞으로 나온다.
					float zDepth = 6.0f * (2.0f - t);

					// 텍스트 전체를 중심 기준으로 양옆으로 넓힌다.
					// 1.0f = 원래 폭, 1.15f = 좌우 15% 확장
					float xWide = 1.0f;

					pos->x = centerX + ((pos->x - centerX) * xWide);

					// 오른쪽으로 갈수록 약간 밀어서
					// 단순 회전이 아니라 앞으로 뻗는 로고 느낌을 만든다.
					float xPush = 1.0f * t;

					pos->y *= yScale;
					pos->z += zDepth;
					pos->x += xPush;
				}

				// 변형 후 BoundingBox를 다시 계산한다.
				// DrawTitleText()에서 중앙 정렬할 때 변형된 실제 크기를 기준으로 써야 한다.
				D3DXComputeBoundingBox(
					(D3DXVECTOR3*)vertexData,
					m_TitleTextMesh->GetNumVertices(),
					stride,
					&m_TitleTextMin,
					&m_TitleTextMax);
			}

			m_TitleTextMesh->UnlockVertexBuffer();

			// 정점 위치를 직접 바꿨으므로 조명 계산용 노멀을 다시 계산한다.
			// 실패해도 출력 자체는 가능하므로 별도 중단은 하지 않는다.
			D3DXComputeNormals(m_TitleTextMesh, nullptr);
		}
	}

	SelectObject(hdc, oldFont);
	DeleteObject(font);
	DeleteDC(hdc);
}

void CRenderer3D::RenderTitleLogo(float titleTimer, bool startFly, float startFlyTimer)
{
	if (!m_device) { return; }

	float introRate = titleTimer / 1.35f;

	if (introRate < 0.0f) { introRate = 0.0f; }
	if (introRate > 1.0f) { introRate = 1.0f; }

	introRate = introRate * introRate * (3.0f - (2.0f * introRate));

	float flyRate = startFlyTimer / 0.85f;

	if (flyRate < 0.0f) { flyRate = 0.0f; }
	if (flyRate > 1.0f) { flyRate = 1.0f; }

	flyRate = flyRate * flyRate * (3.0f - (2.0f * flyRate));

	float logoY = 15.0f;

	// 로켓 시작위치
	float rocketX = -50.0f + (60.0f * introRate);
	float rocketY = (logoY - 5.0f) + (((logoY - 0.0f) - (logoY - 1.0f)) * introRate);
	float rocketZ = 46.0f + ((22.0f - 46.0f) * introRate);

	float rocketScale = 5.0f;
	float flamePower = 1.0f - introRate;

	// 로켓 몸체 기본 비율
	// bodyWidthScale  : 로켓 두께 방향 크기
	// bodyLengthScale : 로켓 진행 방향 길이
	float bodyWidthScale = 1.0f;
	float bodyLengthScale = 1.0f;

	if (flamePower < 0.25f) { flamePower = 0.25f; }

	Vec3 rocketDir = { 1.0f, 0.24f, 0.0f };
	Normalize(rocketDir);

	if (startFly)
	{
		// startFlyTimer 전체를 두 구간으로 나눈다.
		// 0.0 ~ 0.22초 : 카툰식 압축 준비 동작
		// 0.22초 이후 : X축 방향으로 직선 발사

		float squashTime = 0.1f;

		if (startFlyTimer < squashTime)
		{
			float squashRate = startFlyTimer / squashTime;

			if (squashRate < 0.0f) { squashRate = 0.0f; }
			if (squashRate > 1.0f) { squashRate = 1.0f; }

			// 부드러운 보간
			squashRate = squashRate * squashRate * (3.0f - (2.0f * squashRate));

			// 로켓이 날아가기 직전에 살짝 뒤로 눌렸다가 압축되는 느낌.
			// X축으로만 움직여서 z축 튀어나옴은 만들지 않는다.
			rocketX = 10.0f - (1.2f * squashRate);
			rocketY = logoY - 4.5f;
			rocketZ = 22.0f;

			// 미국 카툰식 squash.
			// 길이는 줄고, 두께는 살짝 커진다.
			bodyLengthScale = 1.2f - (0.38f * squashRate);
			bodyWidthScale = 1.2f + (0.22f * squashRate);

			flamePower = 0.5f + (0.3f * squashRate);
		}

		else
		{
			float launchRate = (startFlyTimer - squashTime) / 0.75f;

			if (launchRate < 0.0f) { launchRate = 0.0f; }
			if (launchRate > 1.0f) { launchRate = 1.0f; }

			// 0~1 사이에서 부드럽게 가속.
			// 기존 5.0f - 2.0f 방식은 1.0에서 값이 3.0까지 커질 수 있어서
			// 로켓이 너무 멀리 튀는 문제가 생길 수 있다.
			launchRate = launchRate * launchRate * (3.0f - (2.0f * launchRate));

			float stretch = 1.0f - launchRate;

			// 발사 직후에는 길게 늘어나고,
			// 시간이 지나면 원래 비율로 돌아온다.
			bodyLengthScale = 1.0f + (0.5f * stretch);
			bodyWidthScale = 1.0f - (0.1f * stretch);

			// 로켓이 바라보는 방향 기준으로 앞으로 날아간다.
			// rocketDir.x/y/z를 모두 사용하므로,
			// 로켓 방향을 바꾸면 이동 방향도 같이 바뀐다.
			float launchDistance = 75.0f;

			rocketX = 10.0f + (rocketDir.x * launchDistance * launchRate);
			rocketY = logoY + (rocketDir.y * launchDistance * launchRate) - 4.5f;
			rocketZ = 22.0f + (rocketDir.z * launchDistance * launchRate);

			flamePower = 1.2f + (5.0f * launchRate);
		}
	}

	// 네온 프레임도 텍스트 변형 방향과 같은 느낌으로 맞춘다.
	// 왼쪽은 좁고 안쪽, 오른쪽은 넓고 앞으로 나오는 비대칭 사다리꼴 기준이다.
	float logoZ = 20.0f;

	// 윗쪽 네온
	DrawTitleNeonLine(
		-14.3f, logoY + -3.0f, logoZ + 2.2f,
		18.0f, logoY + 13.0f, logoZ - 0.6f,
		D3DCOLOR_ARGB(255, 80, 170, 255));

	// 아랫쪽 네온
	DrawTitleNeonLine(
		-13.2f, logoY - 7.5f, logoZ + 2.2f,
		18.0f, logoY - 0.8f, logoZ - 0.6f,
		D3DCOLOR_ARGB(240, 80, 170, 255));

	// 텍스트 메쉬 자체가 이미 왼쪽 좁음 / 오른쪽 넓음 / z축 깊이 변형을 가진다.
	// 그래서 DrawTitleText에서는 과한 Y축 회전 없이 네온 라인 기울기에 맞춰 살짝만 돌린다.
	DrawTitleText(-1.0f, logoY + 0.45f, logoZ, 20.0f, 0.1f);

	DrawTitleRocket(rocketX, rocketY, rocketZ, rocketScale, rocketDir, flamePower, bodyWidthScale, bodyLengthScale);
}

void CRenderer3D::DrawTitleText(float x, float y, float z, float scale, float rotZ)
{
	if (!m_device) { return; }
	if (!m_TitleTextMesh) { return; }

	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// DiffuseMaterialSource:
	// 빛을 직접 받는 기본 표면 색상을 어디서 가져올지 정한다.
	// D3DMCS_MATERIAL은 아래에서 SetMaterial()로 넣은 Diffuse 색상을 사용한다는 뜻이다.
	m_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);

	// AmbientMaterialSource:
	// 직접 빛을 받지 않는 어두운 부분의 기본 색상을 어디서 가져올지 정한다.
	m_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);

	// SpecularMaterialSource:
	// 금속처럼 반짝이는 하이라이트 색상을 어디서 가져올지 정한다.
	m_device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

	// 장면 전체에 깔리는 약한 주변광.
	// 너무 낮으면 글자의 어두운 면이 완전히 죽고,
	// 너무 높으면 입체감이 사라진다.
	m_device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(175, 185, 220));

	// 글자의 앞면과 윗면에 강한 흰색 하이라이트를 만든다.
	// 로고의 주 조명이다.
	D3DLIGHT9 mainLight;
	/*
	typedef struct _D3DLIGHT9 {
	광원 종류
	D3DLIGHT_DIRECTIONAL : 태양빛처럼 무한히 먼 곳에서 오는 빛
	D3DLIGHT_POINT       : 전구처럼 한 점에서 퍼지는 빛
	D3DLIGHT_SPOT        : 손전등처럼 특정 방향으로 비추는 빛
	D3DLIGHTTYPE Type;

	물체 표면에 직접 비치는 기본 색상
	가장 눈에 잘 보이는 조명 색
	D3DCOLORVALUE Diffuse;

	반사광(하이라이트) 색상
	금속, 유광 재질에서 번쩍이는 부분
	D3DCOLORVALUE Specular;

	환경광 색상
	직접 빛을 받지 않는 부분도 완전히 검게 보이지 않도록 하는 빛
	D3DCOLORVALUE Ambient;

	광원의 위치
	POINT, SPOT 라이트에서 사용
	DIRECTIONAL 라이트에서는 무시됨
	D3DVECTOR Position;

	광원이 향하는 방향
	DIRECTIONAL, SPOT 라이트에서 사용
	D3DVECTOR Direction;

	광원이 영향을 주는 최대 거리
	POINT, SPOT 라이트에서 사용
	float Range;

	SpotLight 가장자리 감쇠 곡선
	보통 1.0f 사용
	float Falloff;

	거리 감쇠 상수항
	값이 높을수록 거리 영향을 덜 받음
	float Attenuation0;

	거리 감쇠 1차항
	거리 증가에 따라 밝기가 감소
	float Attenuation1;

	거리 감쇠 2차항
	현실적인 조명에서 가장 많이 사용
	float Attenuation2;

	SpotLight 내부 각도 (라디안)
	이 각도 안은 최대 밝기
	float Theta;

	SpotLight 외부 각도 (라디안)
	Theta ~ Phi 구간은 점점 어두워짐
	float Phi;

	} D3DLIGHT9;
	*/
	ZeroMemory(&mainLight, sizeof(mainLight));

	// Type:
	// D3DLIGHT_DIRECTIONAL은 태양빛처럼 방향만 있는 빛이다.
	// 위치가 없고, 모든 물체에 같은 방향으로 빛을 쏜다.
	mainLight.Type = D3DLIGHT_DIRECTIONAL;

	// Direction:
	// 빛이 향하는 방향이다.
	// 값 자체는 위치가 아니라 "빛이 진행하는 방향 벡터"라고 보면 된다.
	// 여기서는 왼쪽 위쪽에서 글자를 때리는 느낌을 만들기 위해 대각선 방향을 준다.
	mainLight.Direction = D3DXVECTOR3(-0.35f, -0.45f, 1.0f);

	// Diffuse:
	// 물체 표면이 직접 빛을 받을 때 보이는 색.
	// 흰색에 가까울수록 은색/금속 느낌이 선명해진다.
	mainLight.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	// Ambient:
	// 이 라이트가 주는 약한 주변광.
	// 어두운 면도 살짝 보이게 만든다.
	mainLight.Ambient = D3DXCOLOR(0.68f, 0.74f, 0.92f, 1.0f);

	// Specular:
	// 반짝이는 하이라이트 색.
	// 금속 재질에서는 이 값이 밝을수록 반짝임이 강하게 보인다.
	mainLight.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	// 설정한 라이트를 적용한다.
	m_device->SetLight(0, &mainLight);
	m_device->LightEnable(0, TRUE);

	// 반대쪽/뒤쪽에서 약한 푸른빛을 넣어 글자 두께와 옆면을 살린다.
	// 3D 텍스트가 평면처럼 보이지 않게 하는 보조 조명이다.
	D3DLIGHT9 rimLight;
	ZeroMemory(&rimLight, sizeof(rimLight));

	rimLight.Type = D3DLIGHT_DIRECTIONAL;

	// 메인 라이트와 다른 방향에서 들어오는 보조광.
	// 너무 강하면 글자가 번들거리기만 하므로 Diffuse는 낮게 잡는다.
	rimLight.Direction = D3DXVECTOR3(0.55f, 0.35f, -0.75f);

	// 푸른 계열 Diffuse를 약하게 줘서 우주/네온 느낌을 더한다.
	rimLight.Diffuse = D3DXCOLOR(0.22f, 0.38f, 0.95f, 1.0f);

	// 보조광의 Ambient는 낮게 둔다.
	// 너무 높이면 전체가 푸르게 떠서 금속 명암이 약해진다.
	rimLight.Ambient = D3DXCOLOR(0.03f, 0.05f, 0.14f, 1.0f);

	// 옆면에 살짝 푸른 반짝임을 만들기 위한 Specular.
	rimLight.Specular = D3DXCOLOR(0.35f, 0.55f, 1.0f, 1.0f);

	m_device->SetLight(1, &rimLight);
	m_device->LightEnable(1, TRUE);

	// D3DXCreateTextW로 만든 메쉬는 원점 기준이 화면 중앙이 아니다.
	// 그래서 BoundingBox로 실제 중심을 구한 뒤,
	// 그 중심을 원점으로 옮긴 다음 원하는 위치로 이동시킨다.
	float centerX = (m_TitleTextMin.x + m_TitleTextMax.x) * 0.5f;
	float centerY = (m_TitleTextMin.y + m_TitleTextMax.y) * 0.5f;
	float centerZ = (m_TitleTextMin.z + m_TitleTextMax.z) * 0.5f;

	float textWidth = m_TitleTextMax.x - m_TitleTextMin.x;
	if (textWidth <= 0.0001f) { textWidth = 1.0f; }

	// scale은 "최종 로고가 화면에서 차지할 가로 크기"처럼 사용한다.
	// 실제 메쉬 폭으로 나누어 크기를 일정하게 맞춘다.
	float finalScale = scale / textWidth;

	D3DXMATRIX matCenter, matS, matRZ, matT, matWorld;

	// 메쉬 중심을 원점으로 이동
	D3DXMatrixTranslation(&matCenter, -centerX, -centerY, -centerZ);

	// 전체 크기 조절
	// xWide는 로고 전체를 좌우로 더 넓히기 위한 값이다.
	// CreateTitleText()에서 만든 사다리꼴/깊이감은 그대로 유지하고,
	// 최종 출력 단계에서 X축만 조금 더 넓힌다.
	float xWide = 1.5f;

	D3DXMatrixScaling(&matS, finalScale * xWide, finalScale, finalScale);
	// 전체 로고 기울기
	// 네온 라인과 같은 방향으로 살짝 기울이는 용도다.
	D3DXMatrixRotationZ(&matRZ, rotZ);

	// 같은 텍스트 메시를 그림자, 외곽선, 본문 순서로 여러 번 그린다.
	// 각각 Material과 위치 오프셋만 바꿔서 레이어처럼 쌓는다.
	auto DrawPass = [&](const D3DMATERIAL9& material, float offsetX, float offsetY, float offsetZ)
	{
		m_device->SetMaterial(&material);

		// 최종 위치 이동
		D3DXMatrixTranslation(&matT, x + offsetX, y + offsetY, z + offsetZ);

		matWorld = matCenter * matS * matRZ * matT;

		m_device->SetTransform(D3DTS_WORLD, &matWorld);
		m_TitleTextMesh->DrawSubset(0);
	};

	// 그림자
	D3DMATERIAL9 shadowMat;
	ZeroMemory(&shadowMat, sizeof(shadowMat));

	// Diffuse / Ambient를 거의 검정으로 두어 그림자처럼 보이게 한다.
	shadowMat.Diffuse = D3DXCOLOR(0.003f, 0.004f, 0.010f, 1.0f);
	shadowMat.Ambient = D3DXCOLOR(0.003f, 0.004f, 0.010f, 1.0f);
	// 그림자는 반짝임이 필요 없으므로 Power만 낮게 둔다.
	shadowMat.Power = 1.0f;

	// 텍스트가 깊이 변형된 상태라 그림자는 너무 멀리 두지 않는다.
	DrawPass(shadowMat, 0.36f, -0.30f, 0.22f);

	// 파란 외곽선
	D3DMATERIAL9 outlineMat;
	ZeroMemory(&outlineMat, sizeof(outlineMat));

	// 외곽선은 푸른 네온 느낌을 주기 위한 재질이다.
	outlineMat.Diffuse = D3DXCOLOR(0.08f, 0.42f, 1.0f, 1.0f);
	outlineMat.Ambient = D3DXCOLOR(0.04f, 0.22f, 0.85f, 1.0f);
	outlineMat.Specular = D3DXCOLOR(0.45f, 0.85f, 1.0f, 1.0f);
	// Emissive:
	// 빛을 받지 않아도 스스로 살짝 빛나는 색.
	// 네온 외곽선 느낌을 위해 약하게 준다.
	outlineMat.Emissive = D3DXCOLOR(0.015f, 0.08f, 0.28f, 1.0f);
	// Power:
	// Specular 하이라이트의 날카로움.
	// 높을수록 좁고 강한 반짝임이 된다.
	outlineMat.Power = 110.0f;

	const float outlineOffset = 0.026f;

	// 같은 메시를 네 방향으로 살짝 어긋나게 그려 외곽선을 만든다.
	DrawPass(outlineMat, -outlineOffset, 0.0f, 0.08f);
	DrawPass(outlineMat, outlineOffset, 0.0f, 0.08f);
	DrawPass(outlineMat, 0.0f, -outlineOffset, 0.08f);
	DrawPass(outlineMat, 0.0f, outlineOffset, 0.08f);

	// 금속 텍스트 패스
	D3DMATERIAL9 mainMat;
	ZeroMemory(&mainMat, sizeof(mainMat));

	// Diffuse:
	// 글자의 기본 표면색. 은색/화이트 메탈 느낌.
	mainMat.Diffuse = D3DXCOLOR(0.92f, 0.95f, 1.0f, 1.0f);

	// Ambient:
	// 직접 빛을 받지 않는 부분의 기본 밝기.
	mainMat.Ambient = D3DXCOLOR(0.78f, 0.83f, 0.95f, 1.0f);

	// Specular:
	// 금속 반짝임. 흰색에 가까울수록 깨끗한 하이라이트가 생긴다.
	mainMat.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	// Emissive:
	// 아주 약한 자체 발광.
	// 완전한 네온은 아니고, 어두운 우주 배경에서 글자가 묻히지 않게 하는 정도다.
	mainMat.Emissive = D3DXCOLOR(0.04f, 0.05f, 0.08f, 1.0f);

	// Power:
	// 하이라이트 날카로움.
	// 금속 느낌을 위해 높은 값을 사용한다.
	mainMat.Power = 210.0f;

	DrawPass(mainMat, 0.0f, 0.0f, 0.0f);
	// 이 함수에서 켠 라이트는 여기서 반드시 끈다.
	// 다른 3D 오브젝트 렌더에 조명 상태가 섞이는 것을 막는다.
	m_device->LightEnable(0, FALSE);
	m_device->LightEnable(1, FALSE);

	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
}

void CRenderer3D::DrawTitleNeonLine(float x1, float y1, float z1, float x2, float y2, float z2, D3DCOLOR color)
{
	if (!m_device) { return; }

	struct LINE_VERTEX
	{
		float x, y, z;
		D3DCOLOR color;
	};

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;

	float len = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
	if (len <= 0.0001f) { return; }

	float dirX = dx / len;
	float dirY = dy / len;
	float dirZ = dz / len;

	float nx = -dirY;
	float ny = dirX;

	// 끝부분이 너무 삐져나오지 않도록 살짝 안쪽에서 그린다.
	float cap = 0.08f;

	float sx = x1 + (dirX * cap);
	float sy = y1 + (dirY * cap);
	float sz = z1 + (dirZ * cap);

	float ex = x2 - (dirX * cap);
	float ey = y2 - (dirY * cap);
	float ez = z2 - (dirZ * cap);

	auto DrawGlowQuad = [&](float thickness, D3DCOLOR quadColor)
		{
			float hx = nx * thickness;
			float hy = ny * thickness;

			LINE_VERTEX v[6] =
			{
				{ sx - hx, sy - hy, sz, quadColor },
				{ sx + hx, sy + hy, sz, quadColor },
				{ ex + hx, ey + hy, ez, quadColor },

				{ sx - hx, sy - hy, sz, quadColor },
				{ ex + hx, ey + hy, ez, quadColor },
				{ ex - hx, ey - hy, ez, quadColor }
			};

			m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, v, sizeof(LINE_VERTEX));
		};

	BYTE a = (BYTE)((color >> 24) & 0xff);
	BYTE r = (BYTE)((color >> 16) & 0xff);
	BYTE g = (BYTE)((color >> 8) & 0xff);
	BYTE b = (BYTE)(color & 0xff);

	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	// 네온 블러 강화.
	// 바깥쪽은 더 넓고 약하게, 안쪽은 진하게 쌓아서 블러처럼 보이게 한다.
	DrawGlowQuad(1.00f, D3DCOLOR_ARGB((BYTE)(a * 0.08f), r, g, b));
	DrawGlowQuad(0.70f, D3DCOLOR_ARGB((BYTE)(a * 0.14f), r, g, b));
	DrawGlowQuad(0.42f, D3DCOLOR_ARGB((BYTE)(a * 0.28f), r, g, b));
	DrawGlowQuad(0.22f, D3DCOLOR_ARGB((BYTE)(a * 0.55f), r, g, b));
	DrawGlowQuad(0.09f, D3DCOLOR_ARGB((BYTE)(a * 0.90f), r, g, b));

	// 중심선은 FakeBloom이 잘 먹도록 밝고 선명하게 둔다.
	DrawGlowQuad(0.04f, D3DCOLOR_ARGB(255, 235, 248, 255));

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void CRenderer3D::DrawTitleRocket(float x, float y, float z, float scale, const Vec3& dir, float flamePower, float bodyWidthScale, float bodyLengthScale)
{
	if (!m_device) { return; }
	if (!m_TitleRocketMesh.vb || !m_TitleRocketMesh.ib) { return; }

	struct FLAME_VERTEX
	{
		float x, y, z;
		D3DCOLOR color;
	};

	// 로켓이 바라보는 방향
	Vec3 forward = dir;

	if (Length(forward) < 0.0001f) { forward = Vec3(1.0f, 0.0f, 0.0f); }
	Normalize(forward);

	// 월드 기준 위쪽.
	// forward와 up을 이용해서 로켓의 right/up/forward 축을 만든다.
	Vec3 up = { 0.0f, 1.0f, 0.0f };

	Vec3 right;
	right.x = up.y * forward.z - up.z * forward.y;
	right.y = up.z * forward.x - up.x * forward.z;
	right.z = up.x * forward.y - up.y * forward.x;

	if (Length(right) < 0.0001f)
	{
		right = Vec3(1.0f, 0.0f, 0.0f);
	}

	Normalize(right);

	up.x = forward.y * right.z - forward.z * right.y;
	up.y = forward.z * right.x - forward.x * right.z;
	up.z = forward.x * right.y - forward.y * right.x;

	Normalize(up);

	float widthScale = bodyWidthScale;
	float lengthScale = bodyLengthScale;

	if (widthScale < 0.2f) { widthScale = 0.2f; }
	if (lengthScale < 0.2f) { lengthScale = 0.2f; }

	D3DXMATRIX matBase =
	{
		right.x,   right.y,   right.z,   0.0f,
		up.x,      up.y,      up.z,      0.0f,
		forward.x, forward.y, forward.z, 0.0f,
		x,         y,         z,         1.0f
	};

	D3DXMATRIX matScale;

	// bodyWidthScale:
	// 로켓의 두께 방향 크기
	// 발사 직전 압축될 때 두꺼워지는 카툰 효과에 사용
	
	// bodyLengthScale:
	// 로켓 진행 방향 길이
	// 발사 직전에는 줄어들고,
	// 발사 순간에는 늘어나는 squash/stretch 효과에 사용
	D3DXMatrixScaling(
		&matScale,
		scale * widthScale,
		scale * widthScale,
		scale * lengthScale);

	D3DXMATRIX matWorld = matScale * matBase;

	// 로켓 본체 출력
	m_device->SetTransform(D3DTS_WORLD, &matWorld);
	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	m_device->SetStreamSource(0, m_TitleRocketMesh.vb, 0, sizeof(CUSTOMVERTEX));
	m_device->SetIndices(m_TitleRocketMesh.ib);
	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	m_device->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0, 0,
		m_TitleRocketMesh.vCount, 0,
		m_TitleRocketMesh.iCount / 3);

	DrawTitleRocketFlame(x, y, z, scale, forward, right, up, flamePower, widthScale, lengthScale);
}

void CRenderer3D::DrawTitleRocketFlame(float x, float y, float z, float scale, const Vec3& forward, const Vec3& right, const Vec3& up, float flamePower, float bodyWidthScale, float bodyLengthScale)
{
	if (!m_device) { return; }

	struct FLAME_VERTEX
	{
		float x, y, z;
		D3DCOLOR color;
	};

	auto MakePoint = [&](float lx, float ly, float lz) -> Vec3
		{
			Vec3 p;

			p.x = x + (right.x * lx) + (up.x * ly) + (forward.x * lz);
			p.y = y + (right.y * lx) + (up.y * ly) + (forward.y * lz);
			p.z = z + (right.z * lx) + (up.z * ly) + (forward.z * lz);

			return p;
		};

	auto MakeFlameVertex = [&](const Vec3& p, D3DCOLOR color) -> FLAME_VERTEX
		{
			FLAME_VERTEX v;
			v.x = p.x;
			v.y = p.y;
			v.z = p.z;
			v.color = color;
			return v;
		};

	float power = flamePower;
	if (power < 0.2f) { power = 0.2f; }

	// tailZ:
	// 로켓 뒤쪽 엔진 캡 위치.
	// 불꽃은 로켓 중심 기준 뒤쪽에서 시작해야 하므로 음수 방향으로 둔다.
	float tailZ = -1.95f * 0.48f * scale * bodyLengthScale;

	// baseWidth:
	// 불꽃의 기본 폭.
	// bodyWidthScale이 커지면 로켓 두께에 맞춰 불꽃도 같이 넓어진다.
	float baseWidth = 0.46f * scale * bodyWidthScale;

	// baseLength:
	// 불꽃 전체 길이.
	// flamePower가 커질수록 뒤로 길게 뻗는 추진 화염이 된다.
	float baseLength = scale * bodyLengthScale * (1.5f + power * 1.65f);

	// time:
	// sin/cos 기반 흔들림에 사용하는 시간값.
	// 매 프레임 값이 바뀌면서 불꽃이 살아 움직이는 것처럼 보인다.
	float time = GetTickCount() * 0.001f;

	// shakePower:
	// 불꽃 전체가 흔들리는 강도.
	// 너무 크면 화염이 물결 리본처럼 보이므로 폭 기준으로 제한한다.
	float shakePower = baseWidth * 0.18f;

	// roughPower:
	// 가장자리 찢김 강도.
	// 기존 불꽃이 너무 매끈해 보이는 문제를 줄이기 위한 값이다.
	float roughPower = baseWidth * 10.0f;

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);

	m_device->SetTransform(D3DTS_WORLD, &matIdentity);
	m_device->SetTexture(0, nullptr);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Additive Blend:
	// 밝은 색을 누적해서 Bloom에 잘 걸리게 만든다.
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// 텍스처 없이 정점 색상만 사용한다.
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

	auto DrawEngineCoreDisc = [&](float radius, float zOffset, D3DCOLOR centerColor, D3DCOLOR edgeColor)
		{
			const int CORE_SEG = 28;
			FLAME_VERTEX core[CORE_SEG * 3];
			int coreCount = 0;

			Vec3 center = MakePoint(0.0f, 0.0f, tailZ + zOffset);

			for (int i = 0; i < CORE_SEG; i++)
			{
				float a0 = (2.0f * D3DX_PI * i) / CORE_SEG;
				float a1 = (2.0f * D3DX_PI * (i + 1)) / CORE_SEG;

				Vec3 p0 = MakePoint(cosf(a0) * radius, sinf(a0) * radius, tailZ + zOffset);
				Vec3 p1 = MakePoint(cosf(a1) * radius, sinf(a1) * radius, tailZ + zOffset);

				core[coreCount++] = MakeFlameVertex(center, centerColor);
				core[coreCount++] = MakeFlameVertex(p0, edgeColor);
				core[coreCount++] = MakeFlameVertex(p1, edgeColor);
			}

			m_device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, coreCount / 3, core, sizeof(FLAME_VERTEX));
		};

	// 가장자리의 원판
	DrawEngineCoreDisc(
		baseWidth * 1.1f,
		-baseWidth * 0.10f,
		D3DCOLOR_ARGB(245, 130, 210, 255),
		D3DCOLOR_ARGB(60, 45, 90, 200));
	// 중간자리의 원판
	DrawEngineCoreDisc(
		baseWidth * 0.72f,
		-baseWidth * 0.16f,
		D3DCOLOR_ARGB(255, 220, 245, 255),
		D3DCOLOR_ARGB(100, 100, 190, 255));
	// 가장안쪽의 원판
	DrawEngineCoreDisc(
		baseWidth * 0.38f,
		-baseWidth * 0.22f,
		D3DCOLOR_ARGB(255, 255, 255, 255),
		D3DCOLOR_ARGB(170, 255, 230, 130));


	auto DrawPlume = [&](float widthMul, float lengthMul, bool vertical, float phase, D3DCOLOR nearColor, D3DCOLOR midColor, D3DCOLOR farColor)
		{
			const int PLUME_SEG = 34;
			FLAME_VERTEX plume[PLUME_SEG * 2];

			float plumeWidth = baseWidth * widthMul;
			float plumeLength = baseLength * lengthMul;

			for (int i = 0; i < PLUME_SEG; i++)
			{
				float t = (float)i / (float)(PLUME_SEG - 1);

				// fade:
				// t가 0이면 엔진 근처, t가 1이면 불꽃 끝.
				// 끝으로 갈수록 흐려지고 작아지게 하기 위한 값이다.
				float fade = 1.0f - t;

				// swell:
				// 불꽃 중간을 부풀리는 값.
				// sin(0)=0, sin(PI/2)=1, sin(PI)=0 이므로
				// 시작과 끝은 작고, 중간은 크게 만든다.
				float swell = sinf(t * D3DX_PI);

				// endShake:
				// 끝부분으로 갈수록 흔들림이 강해지게 만든다.
				// t*t라서 엔진 근처는 거의 안 흔들리고, 끝으로 갈수록 많이 흔들린다.
				float endShake = t * t;

				float waveA = sinf((time * 18.0f) + (t * 7.0f) + phase) * shakePower * endShake;
				float waveB = cosf((time * 15.0f) + (t * 6.0f) + phase) * shakePower * 0.55f * endShake;

				float currentWidth = plumeWidth * (0.36f + (swell * 0.92f)) * (0.25f + (fade * 0.75f));

				// turbulence:
				// 역할:
				// 불꽃 전체 폭이 숨 쉬듯이 살짝 커졌다 작아졌다 하게 만든다.

				// 아주 쉽게 말하면:
				// currentWidth가 그냥 10, 10, 10, 10으로 일정하면 매끈한 풍선처럼 보인다.
				// 그런데 turbulence를 곱하면 10, 11, 9, 10.5처럼 조금씩 달라진다.
				// 그래서 불꽃 안쪽에 공기가 섞여서 울렁이는 느낌이 생긴다.

				// 계산 방식:
				// sin, cos는 -1부터 1 사이를 왔다 갔다 하는 값이다.
				// 여기에 작은 숫자 0.06, 0.04를 곱해서 변화량을 작게 만든다.

				// time:
				// 시간이 지나면서 값이 바뀌게 한다.

				// t:
				// 불꽃의 위치마다 다른 모양이 나오게 한다.

				// phase:
				// 여러 겹의 plume이 서로 똑같이 흔들리지 않게 구분해준다.

				// 결과:
				// currentWidth *= 1.0f + turbulence;
				// turbulence가 0.1이면 폭이 10% 커지고,
				// turbulence가 -0.1이면 폭이 10% 작아진다.
				float turbulence =
					(sinf((time * 34.0f) + (t * 29.0f) + phase) * 0.16f) +
					(cosf((time * 47.0f) + (t * 41.0f) + phase) * 0.12f) +
					(sinf((time * 71.0f) + (t * 67.0f) + phase) * 0.07f);

				currentWidth *= (1.0f + turbulence);

				// surfaceBreak:
				// 불꽃 표면이 얼마나 강하게 찢어질지 정하는 값.
				
				// 엔진 바로 근처는 불꽃이 안정적으로 뿜어져 나와야 한다.
				// 그래서 t가 작은 앞부분에서는 약하게 적용한다.

				// 중간 이후부터는 불꽃이 공기와 섞이며 거칠어지는 느낌이 필요하므로
				// t가 커질수록 강하게 적용한다.
				float surfaceBreak = t * 1.75f;
				if (surfaceBreak > 1.0f) { surfaceBreak = 1.0f; }

				// edgeNoise:
				// 역할:
				// 불꽃 가장자리를 톱니처럼 찢어 보이게 만든다.

				// 아주 쉽게 말하면:
				// 불꽃의 왼쪽 끝과 오른쪽 끝이 매끈한 선이면 종이 리본처럼 보인다.
				// edgeNoise는 그 선을 살짝 깎았다 붙였다 해서
				// "불이 갈라지고 찢기는 느낌"을 만든다.

				// 중요:
				// 여기서는 불꽃 중심 좌표 waveA, waveB는 크게 건드리지 않는다.
				// 중심을 과하게 건드리면 불꽃 전체가 이상한 방향으로 밀려 보일 수 있다.

				// 그래서 edgeNoise는 좌표 위치가 아니라 "폭"에만 적용한다.

				// 계산 방식:
				// sin/cos로 빠르게 변하는 값을 만든다.
				// t * 113, t * 157처럼 큰 값을 쓰면
				// 구간마다 변화가 빨라져서 더 잘게 찢어진 실루엣이 나온다.

				// edgeAmount:
				// 엔진 근처는 안정적이어야 하므로 적게 적용하고,
				// 불꽃 끝부분으로 갈수록 강하게 적용한다.

				// 결과:
				// leftWidth와 rightWidth를 서로 다르게 만들어서
				// 양쪽 가장자리가 똑같지 않은 불규칙한 불꽃이 된다.
				float edgeNoise =
					(sinf((time * 82.0f) + (t * 113.0f) + phase) * 0.52f) +
					(cosf((time * 101.0f) + (t * 157.0f) + phase) * 0.38f) +
					(sinf((time * 137.0f) + (t * 191.0f) + phase) * 0.25f);

				// biteNoise:
				// 불꽃 표면을 바깥으로만 키우는 게 아니라,
				// 일부 구간을 안쪽으로 파먹는 느낌을 주는 값.

				// 쉽게 말하면:
				// edgeNoise만 있으면 불꽃이 뚱뚱해졌다 얇아졌다 하는 느낌이 강하다.
				// biteNoise는 한쪽 외곽을 순간적으로 안쪽으로 깎아서
				// 진짜 불꽃처럼 갈라지고 뜯긴 실루엣을 만든다.
				float biteNoise =
					(sinf((time * 149.0f) + (t * 223.0f) + phase) * 0.5f) +
					(cosf((time * 173.0f) + (t * 199.0f) + phase) * 0.5f);

				if (biteNoise < 0.0f) { biteNoise = -biteNoise; }

				// bite:
				// 실제로 표면을 안쪽으로 깎는 비율.
				// surfaceBreak를 곱해서 엔진 근처보다 끝부분에서 더 많이 깎이게 한다.
				float bite = biteNoise * 0.50f * surfaceBreak;

				float leftWidth = currentWidth * (1.0f + edgeNoise * surfaceBreak);
				float rightWidth = currentWidth * (1.0f - edgeNoise * surfaceBreak);

				// 한쪽 가장자리를 랜덤하게 안쪽으로 깎는다.
				// 이 처리가 "불꽃 표면 위에서 찢기는 느낌"을 만드는 핵심이다.
				if (edgeNoise > 0.0f)	{ leftWidth *= (1.0f - bite); }
				else					{ rightWidth *= (1.0f - bite); }

				// centerTear:
				// 중심선을 아주 조금만 흔들어준다.
				// 너무 많이 흔들면 불꽃 전체가 따로 노는 것처럼 보이므로
				// baseWidth 기준으로 작게 제한한다.

				// 목적:
				// 폭만 찢으면 한 방향으로만 거칠어 보일 수 있다.
				// 중심선을 약간만 흔들면 표면 찢김이 더 자연스럽게 보인다.
				float centerTear =
					sinf((time * 65.0f) + (t * 89.0f) + phase) * baseWidth * 0.055f * surfaceBreak;

				if (vertical)	{ waveA += centerTear; }
				else			{ waveB += centerTear; }

				if (leftWidth < baseWidth * 0.014f) { leftWidth = baseWidth * 0.014f; }
				if (rightWidth < baseWidth * 0.014f) { rightWidth = baseWidth * 0.014f; }

				// widthLimit:
				// edgeNoise가 강해졌기 때문에 폭이 순간적으로 너무 커질 수 있다.
				// 불꽃이 로켓과 분리되어 보이지 않도록 최대 폭을 제한한다.
				float widthLimit = plumeWidth * 1.95f;

				if (leftWidth > widthLimit) { leftWidth = widthLimit; }
				if (rightWidth > widthLimit) { rightWidth = widthLimit; }

				float currentZ = tailZ - (plumeLength * t);

				Vec3 p0;
				Vec3 p1;

				if (vertical)
				{
					p0 = MakePoint(waveA, waveB - leftWidth, currentZ);
					p1 = MakePoint(waveA, waveB + rightWidth, currentZ);
				}

				else
				{
					p0 = MakePoint(waveA - leftWidth, waveB, currentZ);
					p1 = MakePoint(waveA + rightWidth, waveB, currentZ);
				}

				D3DCOLOR color;

				if (t < 0.22f)		{ color = nearColor; }
				else if (t < 0.66f) { color = midColor; }
				else				{ color = farColor; }

				plume[i * 2 + 0] = MakeFlameVertex(p0, color);
				plume[i * 2 + 1] = MakeFlameVertex(p1, color);
			}

			m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, (PLUME_SEG * 2) - 2, plume, sizeof(FLAME_VERTEX));
		};

		DrawPlume(
			1.55f,
			1.22f,
			false,
			0.0f,
			D3DCOLOR_ARGB(105, 95, 170, 255),
			D3DCOLOR_ARGB(70, 70, 130, 255),
			D3DCOLOR_ARGB(0, 20, 60, 170));

		DrawPlume(
			1.08f,
			1.10f,
			true,
			1.7f,
			D3DCOLOR_ARGB(80, 80, 150, 255),
			D3DCOLOR_ARGB(55, 60, 115, 255),
			D3DCOLOR_ARGB(0, 15, 45, 150));

		DrawPlume(
			0.92f,
			0.92f,
			false,
			3.2f,
			D3DCOLOR_ARGB(230, 255, 225, 105),
			D3DCOLOR_ARGB(185, 255, 130, 35),
			D3DCOLOR_ARGB(0, 230, 55, 10));

		DrawPlume(
			0.42f,
			0.56f,
			false,
			5.1f,
			D3DCOLOR_ARGB(255, 255, 255, 245),
			D3DCOLOR_ARGB(230, 255, 235, 145),
			D3DCOLOR_ARGB(25, 255, 115, 35));

		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}

// 3D 타이틀 텍스트 출력 구조
// 1. CreateTitleText()
//	- 일반적인 UI 글자처럼 바로 화면에 그리는 방식이 아니고
//	- D3DXCreateTextW()라는 함수를 이용해서 타이틀 로고의 문자열을 실제 3D 메쉬로 만든다.
//	- 즉, 글자의 외곽선을 따서 앞면, 뒷면, 옆면이 있는 3D 오브젝트로 변환한다.
//	- 만들어진 메시 포인터는 m_TitleTextMesh에 저장되며, 리소스이므로 반드시 해체도 해야한다.

// 2. BoundingBox 계산
//	- D3DXCreateTextW()로 만들어진 글자 메시의 원점은 화면 중앙이 아니고
//	- 보통 글자의 왼쪽 아래 기준에 가까운 위치를 가진다.
//	- 그래서 그대로 그리면 화면 밖으로 밀리거나 위치가 이상해질수 있다.
//	- LockVretexBuffer()로 메쉬 정점 데이터를 읽고, 
//	- D3DXComputeBoundingBox()로 글자 전체의 최소/최대 좌표를 구하고,
//	- 이 값으로 글자 중심점을 계산하여 DrawTitleText()에서 중앙 정렬해서 저장한다.

// 3. DrawTitleText()
//	- 저장해둔 m_TitleTextMesh를 실제 화면에 그리는 함수이며,
//	- 먼저 메쉬 중심을 원점으로 옮기고, 그 다음 스케일, 회전, 이동 행렬을 차례대로 적용한다.
//	- 최종 행렬 순서는 다음과 같다.
//	- 중심 보정 → 크기 조절 → 기울이기 → 원하는 위치로 이동
//	- 마지막에 m_TitleTextMesh->DrawSubset(0)을 호출하면 실제 3D 글자가 출력된다

// 4. 조명과 재질
//	- D3DXCreateTextW()로 만든 메쉬에는 우리가 직접 넣은 정점 색상이 없다.
//	- 그래서 조명과 재질 설정이 제대로 없으면 글자가 검게 나오거나 안보일수도 있다.
//	- DrawTitleText()에서는 LIGHTINHG을 켜고, D3DMATERIAL9로 글자의 색상과 반사광 느낌을 설정한다.
//	- COLORVERTEX를 FALSE로 두면 정점색보다 재질 색을 우선 사용하게 된다.

// 5. RenderTitleLogo()
//	- 타이틀 화면에서 매 프레임 호출되는 전체 로고 출력 함사이다.
//	- 여기서 네온 라인, 3D 텍스트, 로켓을 순서대로 그린다.
//	- titleTimer로 등장 애니메이션을 만들고, startFlyTimer로 START 클릭후 로켓이 앞으로 날아가는 연출을 만든다.

// D3DXCreateTextW()란
// 일반 텍스트처럼 화면에 글자를 바로 출력하는 함수가 아니다.
// 문자열을 실제 3D 모델(메쉬)로 변환해주는 함수이다.
// 만들어진 글자는 이후 DrawSubset()으로 일반 3D 오브젝트처럼 출력한다.

// BoundingBox란
// 생성된 3D 글자가 공간상에서 어느 범위를 차지하는지 계산하는 기능이다.
// 실제 박스를 생성하는 것이 아니라
// 글자의 최소 좌표(Min)와 최대 좌표(Max)를 구한다.
// 이 값을 이용해서 글자의 중심 위치를 계산하고
// 화면 중앙 정렬이나 회전 기준점 계산에 사용한다.

// 그럼 중심을 찾았으면 왜 -centerX를 하는가?
// 원점
// ↓
// (0,0) [ S P A C E   D A R T -------------------- ] 글자가 이렇게 있다고 생각해보면
// 글자의 중심은 오른쪽에 있다. 그럼 먼저 글자를 왼쪽으로 당겨서 중심이 원점에 오게 만들고
// D3DXMatrixTranslation(&matCenter, -centerX, -centerY, -centerZ); 이렇게 함수를 이용해서 글자를 직접 이동시킨다
// 그래서 이제부터는 글자를 어디에 놓든 수치로 조금씩 중앙을 찾아가면된다.
// 그리고 행렬 순서가 중요한 이유는
// 1. matCenter
// 글자 중심을 원점으로 맞춘다.
// 2. matS
// 글자 크기를 조절한다.
// 3. matRZ
// 글자를 살짝 기울인다.
// 4. matT
// 원하는 위치로 옮긴다.

// 행렬 순서가 바뀌면 결과도 완전히 달라진다.
// 예를 들어 회전 후 이동과 이동 후 회전은 전혀 다른 결과를 만든다.

// 메쉬란 무엇인가
// 메쉬는 쉽게 말해서 3D 물체의 뼈대와 표면 정보이다.
// 3D 게임에서 컴퓨터는 "로켓", "다트", "글자" 같은 개념을 이해하지 못한다. 대신

// 정점(Vertex)
// = 점

// 인덱스(Index)
// = 점을 연결하는 순서

// 삼각형(Triangle)
// = 점 3개로 이루어진 면을 이용해서 물체를 만든다.

// 결국 메쉬란
// "정점들과 삼각형들의 집합" 이라고 생각하면 된다.
// D3DXCreateTextW()도
//
// SPACE DART 문자열
// ↓
// 수많은 정점 생성
// ↓
// 수많은 삼각형 생성
// ↓
// 3D 글자 메쉬 완성 의 과정을 거친다.

// 메쉬 위에 텍스트를 생성하면
// 더 이상 단순한 문자열이 아니라 하나의 3D 오브젝트가 된다.

// 즉 과녁, 다트, 행성, 큐브처럼
// 월드 좌표를 가지고 공간 안에 배치할 수 있다.

// 따라서 위치 이동(Translation), 회전(Rotation), 크기 변경(Scaling),
// 조명(Light), 그림자(Shadow), 충돌 판정(Collision)
// 등의 일반적인 3D 오브젝트 처리와 동일하게 사용할 수 있다.

// 예를 들어
// "SPACE DART" 로고를 회전시키거나,
// 앞으로 날아오게 하거나, 빛나는 재질을 적용하거나,
// 로켓과 함께 이동시키는 등의 연출이 가능하다.

// 즉 D3DXCreateTextW()는 문자열을 화면에 출력하는 것이 아니라
// 문자열을 실제 3D 모델(메쉬)로 변환하는 과정이라고 볼 수 있다.

// 문자열(String)
// ↓
// 정점(Vertex) 생성
// ↓
// 삼각형(Triangle) 생성
// ↓
// 메쉬(Mesh) 생성
// ↓
// 일반 3D 오브젝트처럼 렌더링