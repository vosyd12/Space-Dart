#ifndef CLOADINGSCENE_H
#define CLOADINGSCENE_H

#include "CScene.h"
#include "CItem.h"

enum class LoadingState
{
	E_ROCKET_IN,
	E_LOADING,
	E_ROCKET_SQUASH,
	E_ROCKET_OUT,
	E_NONE
};

class CLoadingScene : public CScene
{
	private:
		CItem m_loaderItem;

		LoadingState m_state;

		float m_timer;
		float m_rocketX;
		float m_rocketY;
		float m_rocketSpeed;

		float m_bodyWidthScale;
		float m_bodyLengthScale;
		float m_flamePower;

		void UpdateRocket(float dt);
		void DrawLoadingText(ID3DXFont* font);
		void DrawRocket();

	public:
		CLoadingScene();
		~CLoadingScene();

		void Init(HWND hwnd, CD3D* d3d) override;
		void Update(float dt) override;
		void Render(HDC hdc) override;

		bool IsD3DScene() const override { return true; }
};

#endif