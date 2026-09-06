#ifndef CPLAYSCENE_H
#define CPLAYSCENE_H

#include <string>
#include "CScene.h"
#include "CTarget.h"
#include "CPlayer.h"
#include "CCom.h"
#include "CDart.h"
#include "CCamera.h"
#include "CD3D.h"
#include "CEffect.h"
#include "CCollision.h"
#include "CItemEffectManager.h"

using namespace std;

enum class ControlMode
{
	E_KEYBOARD,
	E_MOUSE
};

enum class GameResult
{
	E_NONE,
	E_WIN,
	E_LOSE,
	E_TIE
};

class CPlayScene : public CScene
{
	private:
		CTarget m_Target;
		CEffect m_effect;
		CPlayer m_Player;
		CCom m_Com;
		DartOwner m_currentTurn;
		CCollision m_Collision;
		CItemEffectManager m_ItemEffectManager;

		int m_selectedItem;
		// 키보드 조준 속도
		// 키보드는 0/1 입력이라 바로 Move하면 움직임이 딱딱해진다.
		// 이 값을 이용해서 조준에 가속/감속을 준다.
		float m_aimVelX;
		float m_aimVelY;
		
		bool m_isPause;
		bool m_isFly;
		bool m_noCountThrow;
		DWORD m_lastViewChangeTime;

		ControlMode m_controlMode;	// 다트 조작키에 대한 변수

		CButton m_item_Slot[ITEM_SLOT_MAX];
		CButton m_settingButton;

		CDart& GetCurrentDart();
		CItem& GetCurrentItem();
		CStatus& GetCurrentStatus();

		CStatus& GetOpponentStatus();

		bool m_isGameOver;
		GameResult m_gameResult;

		CButton m_retryBtn;
		CButton m_exitBtn;

		void ApplyPointItemEffect(ItemType type, int pointValue);

		int GetCurrentItemSlotCount() const;
		const wchar_t* GetItemDisplayName(ItemType type) const;

		void ApplyUsedItemEffect(const ItemEffectState& result, int usedSlotIndex);
		void RenderCenterTextureItemEffect(LPDIRECT3DDEVICE9 device, EffectType type);

		void UpdateTurnTimer(float dt);
		void UpdateTargetMoveState();
		void HandlePlayerInput();
		void HandleComputerTurn(float dt);
		void HandleDartFire();
		void HandleDartUpdateAndCollision();
		void EndCurrentTurn();
		bool UseComputerItemByAI();

		int GetStealFailRate() const;
		bool RollStealSuccess() const;
		int FindRandomStealableSlot(CItem& item) const;
		bool ApplyStealItemEffect(int useSlotIndex, ItemType& stolenItem, bool& success);

		void CheckGameResult();
		void DrawResultWindow(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void UpdateResultWindow();
		const wchar_t* GetResultText() const;

	public:
		CPlayScene();
		~CPlayScene();

		void Init(HWND hwnd, CD3D* d3d) override;
		void Update(float) override;
		void Render(HDC hdc) override;

		void DrawPlayHUD(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void DrawScoreUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void DrawItemUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void DrawChanceUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void DrawEffectUI(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		void DrawHUDPanel(LPDIRECT3DDEVICE9 device, const RECT& rc, D3DCOLOR lineColor);
		void DrawSettingGearIcon(LPDIRECT3DDEVICE9 device);
		void DrawTextEx(LPDIRECT3DDEVICE9 device, const wchar_t* text, const RECT& rc, D3DCOLOR color, int fontSize, DWORD format, bool shadow);

		void OnTurnTimeOver();

		bool IsD3DScene() const override		{ return true; }
		bool IsPaused() const override			{ return m_isPause; }
};

#endif

// 여기는 현재에 게임 상태를 출력하면 된다.

// 큐브 충돌시에 이펙트 추가

// 게임의 종목을 제로원을 결정했을시 0점을 맟출때까지 계속 턴이 돌지만
// 그럴경우 너무 루즈해질수 있으므로 AI난이도도 쉬움이라도 제로원일경우 찬스가 늘어날수록 점점더 똑똑해 지고
// 카운트업일경우 찬스가 줄어들수록 점점 똑똑해지는 지도록한다.
// 지금의 AI이동을 좀더 스무스하게 하는 방법이 필요해 보임

// 지금 다트와 과녁사이에 충돌시에 좌표 계산이 약간 어긋나 있는것같음
// 그리고 지금 아이템에 의한 과녁 사이즈업을 했을시에도 약간 어긋나 있는것 같음
// 지금의 다트와 과녁의 충돌에 대한 좌표는 매우 세밀하고 정밀해야 할듯
// 지금 핀 부분을 다트의 앞부분으로 했는데 약간 각도에 따른 핀부분이 찍히는게 달라지는것 같음 
// 그런 모든 경우의 수를 배재하고 찍히는 다트의 핀은 매우 정밀해야함
