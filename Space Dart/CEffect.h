#ifndef CEFFECT_H
#define CEFFECT_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Common.h"
#include "Math.h"
#include "CButton.h"
#include "CItemEffect.h"

using namespace std;

enum class EffectType
{
	E_NONE,

	E_HIT,
	E_BULL,
	E_CUBE,
	E_MISS,

	E_EXCHANGE,
	E_ITEMCHARGE,
	E_POINT_UP,
	E_POINT_DOWN,
	E_STEAL,
	E_SNEAK_PEEK,

	E_EXTRA_CHANCE,

	E_TARGET_SLOW,
	E_TARGET_STOP
};

struct SlotEffect
{
	int displayValue;
	int finalValue;

	// 10의 자리 릴
	int tensDisplay;
	int tensNext;
	int tensFinal;
	float tensTimer;
	float tensInterval;
	float tensScrollRate;
	bool tensStopped;

	// 1의 자리 릴
	int onesDisplay;
	int onesNext;
	int onesFinal;
	float onesTimer;
	float onesInterval;
	float onesScrollRate;
	bool onesStopped;

	bool isPositive;
	bool targetIsPlayer;
	bool jackpot;

	void Reset()
	{
		displayValue = 10;
		finalValue = 10;

		tensDisplay = 1;
		tensNext = 2;
		tensFinal = 1;
		tensTimer = 0.0f;
		tensInterval = 0.025f;
		tensScrollRate = 0.0f;
		tensStopped = false;

		onesDisplay = 0;
		onesNext = 7;
		onesFinal = 0;
		onesTimer = 0.0f;
		onesInterval = 0.018f;
		onesScrollRate = 0.0f;
		onesStopped = false;

		isPositive = true;
		targetIsPlayer = true;
		jackpot = false;
	}
};

struct ItemChargeSlotEffect
{
	bool active[ITEM_EFFECT_SLOT_MAX];

	// displayItems:
	// 현재 슬롯 중앙에 보이는 아이콘.
	ItemType displayItems[ITEM_EFFECT_SLOT_MAX];

	// nextItems:
	// 위쪽에서 내려오는 다음 아이콘.
	// 현실 슬롯머신처럼 보이게 하려면
	// 현재 아이콘 하나만 바꾸는 게 아니라,
	// 현재 아이콘과 다음 아이콘을 동시에 움직여야 한다.
	ItemType nextItems[ITEM_EFFECT_SLOT_MAX];

	// finalItems:
	// 슬롯머신이 마지막에 멈출 최종 아이템.
	ItemType finalItems[ITEM_EFFECT_SLOT_MAX];

	float spinTimer[ITEM_EFFECT_SLOT_MAX];
	float spinInterval[ITEM_EFFECT_SLOT_MAX];

	// scrollRate:
	// 0.0f이면 현재 아이콘이 중앙.
	// 0.5f이면 현재 아이콘은 반쯤 내려가고 다음 아이콘도 반쯤 들어온 상태.
	// 1.0f이면 다음 아이콘이 중앙에 도착한 상태.
	float scrollRate[ITEM_EFFECT_SLOT_MAX];

	int slotCount;

	bool jackpot;
	int jackpotStartIndex;
	int jackpotCount;

	void Reset()
	{
		for (int i = 0; i < ITEM_EFFECT_SLOT_MAX; i++)
		{
			active[i] = false;

			displayItems[i] = ItemType::E_NONE;
			nextItems[i] = ItemType::E_NONE;
			finalItems[i] = ItemType::E_NONE;

			spinTimer[i] = 0.0f;
			spinInterval[i] = 0.035f;

			scrollRate[i] = 0.0f;
		}

		slotCount = 0;

		jackpot = false;
		jackpotStartIndex = -1;
		jackpotCount = 0;
	}
};

class CEffect
{
	private:
		float m_timer;
		float m_limitTime;

		EffectType m_Type;
		bool m_isPlaying;

		float m_EffectTimer;
		float m_EffectDuration;

		int m_DisplayScore;

		int m_CurrentPoint;
		int m_FinalPoint;

		ItemType m_CurrentItems[ITEM_EFFECT_SLOT_MAX];
		ItemType m_FinalItems[ITEM_EFFECT_SLOT_MAX];

		SlotEffect m_NumberSlot;
		ItemChargeSlotEffect m_ItemChargeSlot;

		int m_DirectionIndex;

		ItemType m_StealItem;
		float m_StealProgress;

		bool m_StealSuccess;
		int m_StealSlotIndex;

	public:
		CEffect();
		virtual ~CEffect();

		void Init(float limitTime);
		void UpdateTimer(float dt);
		void UpdateEffect(float dt);
		void Update(float dt);
		void Reset() { m_timer = m_limitTime; }

		void SetLimitTime(float time);
		bool IsTimeOver() const						{ return m_timer <= 0.0f; }
		float GetRemainTime() const					{ return m_timer; }

		void PlayHit(int score);
		void PlayBull();
		void PlayCube();
		void PlayMiss();
		void PlayCenterTextureEffect(EffectType type);

		void DrawGlowCircle(LPDIRECT3DDEVICE9 device, float x, float y, float radius, D3DCOLOR centerColor, D3DCOLOR edgeColor);

		void PlayItemCharge(const bool slotActive[], const ItemType finalItems[], int slotCount);
		void PlayPointUp(int finalPoint, bool targetIsPlayer);
		void PlayPointDown(int finalPoint, bool targetIsPlayer, bool jackpot);
		void PlaySneakPeek(int directionIndex);
		void PlayExchange();
		void PlayExtraChance();
		float GetEffectRate() const;

		//--------------- 아이템 이펙트 --------------
		void RenderExchangeEffect(LPDIRECT3DDEVICE9 device);
		void StartNumberSlotEffect(int finalValue, bool positive, bool targetIsPlayer, bool jackpot);
		void UpdateNumberSlotEffect(float dt, float rate);
		void RenderPointSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
		
		void StartItemChargeSlotEffect(const bool slotActive[], const ItemType finalItems[], int slotCount);
		void UpdateItemChargeSlotEffect(float dt, float rate);
		bool IsItemChargeSlotActive(int index) const;
		bool IsItemChargeJackpotSlot(int index) const;
		ItemType GetItemChargeDisplayItem(int index) const;
		ItemType GetItemChargeNextItem(int index) const;
		float GetItemChargeScrollRate(int index) const;
		void RenderItemChargeSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font, int slotIndex, const RECT& slotRc);
		void RenderItemChargeCenterEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font);

		void PlaySteal(ItemType item, int slotIndex, bool success);
		void RenderStealSlotEffect(LPDIRECT3DDEVICE9 device, ID3DXFont* font, const RECT& slotRc);
		void RenderCenterTextureEffect(LPDIRECT3DDEVICE9 device, LPDIRECT3DTEXTURE9 texture);

		void RenderExtraChanceEffect(LPDIRECT3DDEVICE9 device);
		void RenderSneakPeekEffect(LPDIRECT3DDEVICE9 device);

		bool IsStealSuccess() const					{ return m_StealSuccess; }
		int GetStealSlotIndex() const				{ return m_StealSlotIndex; }
		bool IsPlaying() const						{ return m_isPlaying; }
		EffectType GetType() const					{ return m_Type; }
		int GetDisplayScore() const					{ return m_DisplayScore; }
		int GetCurrentPoint() const					{ return m_CurrentPoint; }
		int GetFinalPoint() const					{ return m_FinalPoint; }
		int GetDirectionIndex() const				{ return m_DirectionIndex; }
		ItemType GetStealItem() const				{ return m_StealItem; }
		float GetStealProgress() const				{ return m_StealProgress; }
		const ItemType* GetCurrentItems() const		{ return m_CurrentItems; }
		const ItemType* GetFinalItems() const		{ return m_FinalItems; }

		void RenderTimer(ID3DXFont* font);
};

#endif 

// 각종 이펙트관련 전용 클라스
// 아이템에 사용에 대한 이펙트

// 큐브와 충돌시 큐브가 터지면서 빛이 사방을 뿌려지는 효과

// 참고 사이트 : https://codingfarm.tistory.com/562 (상수 버퍼) https://myoung-min.tistory.com/10 (리소스)