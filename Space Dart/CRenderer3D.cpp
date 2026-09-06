#include "CRenderer3D.h"
#include "CDart.h"
#include "ResourcePath.h"
#include <math.h>

void CRenderer3D::Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager)
{
	m_device = device;
	m_TextureManager = textureManager;
	m_LastCubeUpdateTime = GetTickCount();

	m_TitleTextMesh = nullptr;

	m_TitleSpaceTextMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleSpaceTextMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleDartTextMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_TitleDartTextMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_TargetTexture = m_TextureManager->LoadTexture(L"dart_target");
	m_PlanetTexture = m_TextureManager->LoadTexture(L"planet");

	CreateTarget();
	CreatePlanet();
	CreateDart();
	CreateCube();
	CreateTitleText();
	CreateTitleRocket();

	for (int i = 0; i < 50; i++)
	{
		m_Cubes[i].pos.x = RandomRange(-5.0f, 5.0f);
		m_Cubes[i].pos.y = RandomRange(-5.0f, 5.0f);
		m_Cubes[i].pos.z = RandomRange(-5.0f, -75.0f);

		float s = RandomRange(0.5f, 2.0f);
		m_Cubes[i].scale = { s,s,s };
		m_Cubes[i].rot = { 0,0,0 };
	}
}

// 큐브 생성시 정중앙으로부터 특정한 위치까지는 생성이 안되게 하기
void CRenderer3D::Release()
{
	MeshBuffer* meshes[] =
	{
		&m_TargetMesh,
		&m_PlanetMesh,
		&m_DartMesh,
		&m_CubeMesh,
		&m_TitleRocketMesh
	};

	for (int i = 0; i < 5; i++)
	{
		if (meshes[i]->vb)
		{
			meshes[i]->vb->Release();
			meshes[i]->vb = nullptr;
		}

		if (meshes[i]->ib)
		{
			meshes[i]->ib->Release();
			meshes[i]->ib = nullptr;
		}

		meshes[i]->vCount = 0;
		meshes[i]->iCount = 0;
	}
}

void CRenderer3D::Update(float deltaTime)
{
	DWORD currentTime = GetTickCount();

	// 큐브 회전
	for (int i = 0; i < 50; i++)
	{
		m_Cubes[i].rot.y += 0.09f;
	}

	// 10초마다 위치 변경
	if (currentTime - m_LastCubeUpdateTime > 10000)
	{
		m_LastCubeUpdateTime = currentTime;

		for (int i = 0; i < 50; i++)
		{
			m_Cubes[i].pos.x = RandomRange(-5.0f, 5.0f);
			m_Cubes[i].pos.y = RandomRange(-5.0f, 5.0f);
			m_Cubes[i].pos.z = RandomRange(5.0f, 75.0f);
		}
	}
}

void CRenderer3D::Render(const DartState& dart, Vec3 targetPos, DartOwner owner, float targetRadius)
{
	static bool cubeInit = false;

	// 최초 1회 큐브 위치 초기화
	if (!cubeInit)
	{
		cubeInit = true;

		for (int i = 0; i < 50; i++)
		{
			RandomizeCube(i);
		}
	}

	DrawPlanet(targetPos.x, targetPos.y, targetPos.z, targetRadius);
	DrawTarget(targetPos.x, targetPos.y, targetPos.z, targetRadius);
	DrawDart(dart.pos, dart.dir, owner);

	for (int i = 0; i < 50; i++)
	{
		DrawCube(m_Cubes[i]);
	}

	// 3D 오브젝트 렌더 최종 상태 복구
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

const Instance& CRenderer3D::GetCube(int index) const
{
	// 잘못된 index가 들어오면 0번 큐브 반환
	// 충돌 검사에서 out of range 방지용
	if (index < 0 || index >= 50)
	{
		return m_Cubes[0];
	}

	return m_Cubes[index];
}

void CRenderer3D::RandomizeCube(int index)
{
	if (index < 0 || index >= 50) { return; }

	// 큐브를 새 위치로 재배치
	// 충돌 후 같은 위치에서 계속 맞는 문제 방지
	m_Cubes[index].pos.x = RandomRange(-5.0f, 5.0f);
	m_Cubes[index].pos.y = RandomRange(-5.0f, 5.0f);
	m_Cubes[index].pos.z = RandomRange(5.0f, 55.0f);

	// 크기도 다시 랜덤 설정
	float s = RandomRange(0.5f, 2.0f);
	m_Cubes[index].scale = Vec3(s, s, s);

	// 회전 초기화
	m_Cubes[index].rot = Vec3(0.0f, 0.0f, 0.0f);
}