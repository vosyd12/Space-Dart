#ifndef CRENDERER3D_H
#define CRENDERER3D_H

#include "CTextureManager.h"
#include "Common.h"

struct MeshBuffer
{
	LPDIRECT3DVERTEXBUFFER9 vb;
	LPDIRECT3DINDEXBUFFER9  ib;
	int vCount = 0;
	int iCount = 0;
};

class CRenderer3D
{
	private:
		MeshBuffer m_TargetMesh;
		MeshBuffer m_PlanetMesh;
		MeshBuffer m_DartMesh;
		MeshBuffer m_CubeMesh;
		MeshBuffer m_TitleRocketMesh;

		// 텍스처 매니저
		CTextureManager* m_TextureManager;

		// 실제 텍스처
		LPDIRECT3DTEXTURE9 m_TargetTexture;
		LPDIRECT3DTEXTURE9 m_PlanetTexture;
		LPDIRECT3DDEVICE9  m_device;

		LPD3DXMESH m_TitleTextMesh;

		D3DXVECTOR3 m_TitleTextMin;
		D3DXVECTOR3 m_TitleTextMax;

		D3DXVECTOR3 m_TitleSpaceTextMin;
		D3DXVECTOR3 m_TitleSpaceTextMax;
		D3DXVECTOR3 m_TitleDartTextMin;
		D3DXVECTOR3 m_TitleDartTextMax;

		DWORD m_LastCubeUpdateTime;
		Instance m_Cubes[50];

	public:
		void Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager);
		void Update(float deltaTime);
		void Render(const DartState& dart, Vec3 targetPos, DartOwner owner, float targetRadius);

		void Release();
		int GetCubeCount() const				{ return 50; }
		const Instance* GetCubeArray() const	{ return m_Cubes; }

		const Instance& GetCube(int index) const;
		void RandomizeCube(int index);

		// 생성 함수
		void CreateTarget();
		void CreatePlanet();
		void CreateDart();
		void CreateCube();
		void CreateTitleRocket();
		// 그리기 함수
		void DrawTarget(float x, float y, float z, float radius);
		void DrawPlanet(float x, float y, float z, float radius);
		void DrawDart(const Vec3& pos, const Vec3& dir, DartOwner owner);
		void DrawCube(const Instance& inst);

		void CreateTitleText();
		void RenderTitleLogo(float titleTimer, bool startFly, float startFlyTimer);
		void DrawTitleText(float x, float y, float z, float scale, float rotZ);
		void DrawTitleRocket(float x, float y, float z, float scale, const Vec3& dir, float flamePower, float bodyWidthScale, float bodyLengthScale);
		void DrawTitleRocketFlame(float x, float y, float z, float scale, const Vec3& forward, const Vec3& right, const Vec3& up, float flamePower, float bodyWidthScale, float bodyLengthScale);
		void DrawTitleNeonLine(float x1, float y1, float z1, float x2, float y2, float z2, D3DCOLOR color);
};

#endif

// FOV값에 따라서 타이틀 로고의 사이즈 변경