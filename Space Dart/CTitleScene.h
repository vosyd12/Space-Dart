#ifndef CTITLESCENE_H
#define CTITLESCENE_H

#include "CScene.h"
#include "CButton.h"
#include "CD3D.h"

class CTitleScene : public CScene
{
	private:
		CButton m_startBtn;
		CButton m_settingBtn;
		CButton m_exitBtn;

		GameScene m_nextScene;

		// 3D 타이틀 연출용
		float m_titleTimer;
		float m_startFlyTimer;
		bool m_startFly;

	public:
		CTitleScene();
		~CTitleScene();

		void Init(HWND hwnd, CD3D* d3d) override;
		void Update(float dt) override;
		void Render(HDC hdc) override;

		GameScene GetNextScene() const override { return m_nextScene; }
};

#endif

// 타이틀 로고에서 S자가 너무 뒤로 누워서 가시성이 떨어져보임