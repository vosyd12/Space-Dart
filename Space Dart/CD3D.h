#ifndef CD3D_H
#define CD3D_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "Common.h"

#include "CCamera.h"
#include "CTextureManager.h"
#include "CRenderer3D.h"
#include "CSpaceRenderer.h"
#include "CPostProcess.h"
#include "CSpaceBackground.h"

class CD3D
{
	private:
		LPDIRECT3D9			m_d3d ;
		LPDIRECT3DDEVICE9	m_device;
		D3DPRESENT_PARAMETERS m_d3dpp;

		// 카메라
		CCamera				m_camera;
		// 우주 데이터
		CSpaceBackground	m_spaceBG;
		// 텍스처 시스템
		CTextureManager m_textureManager;
		// 3D 오브젝트 렌더러
		CRenderer3D		m_renderer3D;
		// 우주 전용 렌더러
		CSpaceRenderer	m_spaceRenderer;
		// 후처리 시스템
		CPostProcess	m_postProcess;

		DWORD m_lastTime;

		int m_screenWidth;
		int m_screenHeight;

		HWND m_hwnd;
		RECT m_windowRect;
		bool m_isFullscreen;
		bool m_isDeviceResetting;

		ID3DXFont* m_uiFont;

	public:
		CD3D();
		~CD3D();

		HRESULT Init(HWND hwnd);
		void Update(float deltaTime);

		// 프레임 시작
		// 화면 초기화 및 BeginScene 호출
		bool BeginFrame();
		// 프레임 종료
		void EndFrame();
		// 우주 배경 전체 랜더링
		void DrawSpace();
		// 실제 게임 오브젝트 렌더링
		void RenderGameObjects(const Vec3& targetPos, const DartState& dart, DartOwner owner, float targetRadius);
		// 카메라 시야각 변경
		void SetFOV(float degree);
		// 전체 화면 / 창모드 전환
		void SetFullscreen(bool enable);

		void RenderSpaceOnly();		// 배경랜더전용 함수
		void RenderTitleLogo(float titleTimer, bool startFly, float startFlyTimer);
		void RenderLoadingRocket(float rocketX, float rocketY, float rocketZ, float rocketScale, float flamePower, float bodyWidthScale, float bodyLengthScale);
		void CleanUp();
		// 화면 크기 갱신
		void UpdateScreenSize();
		// Device Reset 함수
		void ResetDevice();

		void DrawGameStateMessage(const wstring& text, D3DCOLOR mainColor, D3DCOLOR glowColor, int y = 360, bool darkOverlay = true);

		CCamera& GetCamera()					{ return m_camera; }
		CSpaceBackground& GetSpaceBackground()	{ return m_spaceBG; }
		ID3DXFont* GetUIFont()					{ return m_uiFont; }
		LPDIRECT3DDEVICE9 GetDevice()			{ return m_device; }
		CTextureManager& GetTextureManager()	{ return m_textureManager; }
		CRenderer3D& GetRenderer3D()			{ return m_renderer3D; }
};

#endif

// 랜더관련한거는 모두 처리

// 패럴랙스
// moveSpeed = cameraSpeed×depth

/*
 거리 기반 크기
		baseSize
size =  --------
			z
*/