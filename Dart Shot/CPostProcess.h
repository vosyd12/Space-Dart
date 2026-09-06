#ifndef CPOSTPROCESS_H
#define CPOSTPROCESS_H

#include "CTextureManager.h"

class CPostProcess
{
	private:
		// Render Target
		LPDIRECT3DTEXTURE9 m_sceneTex;
		LPDIRECT3DSURFACE9 m_sceneSurface;

		// 밝은 부분만 추출한 결과 저장용
		// 진짜 Bloom은 원본 전체가 아니라 밝은 픽셀만 블러해야 한다.
		LPDIRECT3DTEXTURE9 m_brightTex;
		LPDIRECT3DSURFACE9 m_brightSurface;

		// 블룸용
		LPDIRECT3DTEXTURE9 m_bloomTex;
		LPDIRECT3DSURFACE9 m_bloomSurface;

		// 블러 중간 저장용
		// 가로 블러 결과를 임시로 저장하고,
		// 그 결과를 다시 세로 블러에 사용한다.
		LPDIRECT3DTEXTURE9 m_blurTex;
		LPDIRECT3DSURFACE9 m_blurSurface;

		// 밝은 부분만 남기는 Pixel Shader
		// m_threshold보다 어두운 픽셀은 검정색으로 버린다.
		LPDIRECT3DPIXELSHADER9 m_brightPassShader;

		LPDIRECT3DSURFACE9 m_oldRT;

		LPDIRECT3DDEVICE9	m_device;
		CTextureManager* m_textureManager;

		int m_width;
		int m_height;

		float m_bloomIntensity;
		float m_blurPower;
		float m_threshold;

		bool m_enableBloom;
		bool m_enableLensDirt;
		bool m_enableDust;

	public:
		CPostProcess();
		~CPostProcess();

		void Init(LPDIRECT3DDEVICE9 device, CTextureManager* textureManager, int width, int height);
		void Release();

		// 후처리 시작
		// 현재 화면를 PostProcess용 텍스처에 렌더링 시작
		void BeginScene();

		// 후처리 종료
		// Scene 렌더링 종료 후 원본 화면으로 복귀
		void EndScene();

		void ApplyPostProcess();

		void DrawSceneTexture();
		void DrawRealBloom();

		// 원본 장면에서 밝은 픽셀만 m_brightTex로 추출한다.
		void DrawBrightPass(LPDIRECT3DTEXTURE9 srcTex, LPDIRECT3DSURFACE9 dstSurface);

		void DrawBlurPass(LPDIRECT3DTEXTURE9 srcTex, LPDIRECT3DSURFACE9 dstSurface, float offsetX, float offsetY);
		// Glow 출력
		// 강한 발광 영역을 강조해서 화면에 추가
		void DrawGlow();
		// Lens Dirt 출력
		// 카메라 렌즈에 먼지 낀 듯한 효과
		void DrawLensDirt();
		// 먼지 OverLay 출력
		// 화면 전체에 우주 먼지 / 필름 노이즈 느낌 추가
		void DrawDustOverlay();

		// 전체 화면 텍스처 출력
		void DrawFullscreenQuad(LPDIRECT3DTEXTURE9 tex, D3DCOLOR color);
		void DrawFullscreenQuadOffset(LPDIRECT3DTEXTURE9 tex, float offsetX, float offsetY, D3DCOLOR color);

		// 값 설정
		void SetBloomIntensity(float value)		{ m_bloomIntensity = value; }
		void SetBlurPower(float value)			{ m_blurPower = value; }
		// 0~255 기준.
		// 값이 낮으면 더 많은 부분이 Bloom 대상이 되고,
		// 값이 높으면 네온/불꽃/강한 하이라이트만 Bloom 대상이 된다.
		void SetThreshold(float value)			{ m_threshold = value; }


		void SetBloomEnable(bool enable)		{ m_enableBloom = enable; }
		void SetLensDirtEnable(bool enable)		{ m_enableLensDirt = enable; }
		void SetDustEnable(bool enable)			{ m_enableDust = enable; }
};

#endif

// 참고 사이트 : https://rhksgml78.tistory.com/390 (블러처리)
//				 https://nellfamily.tistory.com/50 (블룸처리)
//				 https://kyuhwang.tistory.com/38   (블룸처리)