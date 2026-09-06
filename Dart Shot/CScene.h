#ifndef CSCENE_H
#define CSCENE_H

#include <windows.h>
#include "Common.h"
#include "CD3D.h"
#include "CButton.h"

enum class GameScene
{
	E_NONE,
	E_TITLE,
	E_SETTING,
	E_MODE_SELECT,
	E_LOADING,
	E_GAME,
};

class CScene
{
	protected:
		HWND m_hwnd;
		GameScene m_nextScene;
		CD3D* m_d3d;

	public:
		virtual ~CScene() {}

		virtual void Init(HWND hwnd, CD3D* d3d)
		{
			m_hwnd = hwnd;
			m_d3d = d3d;
			m_nextScene = GameScene::E_NONE;
		}
		virtual void Update(float deltaTime) = 0;
		virtual void Render(HDC hdc) = 0;

		virtual GameScene GetNextScene() const	{ return m_nextScene; }
		virtual bool IsD3DScene() const			{ return false; }
		void ResetNextScene()					{ m_nextScene = GameScene::E_NONE; }
		virtual bool IsPaused() const			{ return false; }
};

#endif