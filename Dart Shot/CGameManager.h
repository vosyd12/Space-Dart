#ifndef CGAMEMANAGER_H
#define CGAMEMANAGER_H

#include <windows.h>
#include "CScene.h"
#include "CD3D.h"
#include "KeyProc.h"

class CGameManager
{
	private:
		HWND m_hwnd;
		CScene* m_CurrentScene;		// 현재 씬
		CScene* m_SavedGameScene;
		GameScene m_CurrentSceneType;
		CD3D m_d3d;
		
		bool m_initialized;

	public:
		CGameManager();
		~CGameManager();

		void Init(HWND hwnd);
		void Update();
		void Render(HDC hdc);
		void OnResize();

		void ChangeScene(GameScene scene);

		CD3D& GetD3D() { return m_d3d; }
};

#endif

// 게임플레이의 전체적인 흐름을 관리