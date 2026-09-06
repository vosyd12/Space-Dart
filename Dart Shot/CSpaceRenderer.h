#ifndef CSPACERENDERER_H
#define CSPACERENDERER_H

#include "CSpaceBackground.h"
#include "CCamera.h"
#include "CTextureManager.h"

struct METEORVERTEX
{
	float x, y, z;
	D3DCOLOR color;
};

#define D3DFVF_METEORVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

class CSpaceRenderer
{
	private:
		// 텍스처 매니저 참조
		CTextureManager* m_textureManager;
		// 내부 공용 버텍스
		LPDIRECT3DVERTEXBUFFER9 m_billboardVB;
		LPDIRECT3DDEVICE9	m_device;

	public:
		// 초기화
		void Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager);

		// 메모리 해제
		void Release();

		// 우주 전체 Draw
		void DrawSpace( const CSpaceBackground& bg, const CCamera& camera);
		void DrawGalaxyBackground();

		// 개별 Draw
		void DrawStars( const CSpaceBackground& bg, const CCamera& camera);
		void DrawNebulas( const CSpaceBackground& bg, const CCamera& camera);
		void DrawGalaxy( const CSpaceBackground& bg, const CCamera& camera);
		void DrawMeteors( const CSpaceBackground& bg, const CCamera& camera);
		void DrawMeteorLine( const Meteor& meteor, const CCamera& camera);
		void DrawMeteorGlowCircle(const Vec3& center, float radius, int segment, D3DCOLOR centerColor, D3DCOLOR edgeColor, const CCamera& camera);
		void DrawDust( const CSpaceBackground& bg, const CCamera& camera);
		// 워프 별
		void DrawWarpStars(const CSpaceBackground& bg, const CCamera& camera);
		// 워프 별 1개를 직접 폴리곤으로 출력
		void DrawWarpLine(const StarParticle & star, const CCamera & camera, const WarpEffect & warp);
		// 일반 빌보드
		void DrawBillboard( Vec3 pos, float size, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera);
		// 길쭉한 워프 빌보드
		void DrawStretchBillboard( const Vec3& pos, float width, float height, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera, const Vec3& stretchDir);
		void DrawRotatedBillboard(Vec3 pos, float width, float height, float angle, D3DCOLOR color, LPDIRECT3DTEXTURE9 tex, const CCamera& camera);

		// Additive Blend 시작
		// 색상을 서로 더해서 강한 빛 표현
		void BeginAdditive();
		// Additive Blend 종료
		// 기본 렌더 상태로 복구
		void EndAdditive();

		// Alpha Blend 시작
		// 알파값(투명도)을 이용한 일반 반투명 출력
		void BeginAlpha();
		//Alpha Blend 종료
		// 기본 렌더 상태로 복구
		void EndAlpha();

		// SpotLight Blend 시작
		// 검은 배경은 제거하고 밝은 부분만 강조
		void BeginSpotLight();
		// SpotLight Blend 종료
		// 렌더 상태 복구
		void EndSpotLight();

		// 현재 설정된 Luminosity 값을 적용해서
		// 색상값을 밝게 / 어둡게 보정
		int ApplyLumi(int value);
};

#endif