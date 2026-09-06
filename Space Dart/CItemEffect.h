#ifndef CITEMEFFECT_H
#define CITEMEFFECT_H

#include "Common.h"

const int ITEM_EFFECT_SLOT_MAX = 5;

struct ItemEffectState
{
	bool active;

	ItemType sourceItem;

	int pointValue;
	int lockSlot;

	// ITEMCHARGE 전용 정보
	// 사용 직전에 아이템이 있었던 슬롯만 true.
	bool itemChargeSlotActive[ITEM_EFFECT_SLOT_MAX];

	// ITEMCHARGE 사용 후 최종적으로 들어갈 아이템.
	ItemType itemChargeFinalItems[ITEM_EFFECT_SLOT_MAX];

	// 현재 난이도에서 실제로 사용하는 슬롯 개수.
	int itemChargeSlotCount;

	bool exchangePoint;
	bool pointUp;
	bool pointDown;
	bool itemCharge;
	bool stealItem;

	bool slowTarget;
	bool stopTarget;
	bool sneakPeek;
	bool sizeUpTarget;

	bool extraChance;

	bool shield;
	bool copyItem;
	bool magnet;
	bool lockItem;
	bool mirror;
	bool curse;
	bool item_block;
	bool confuse;
	bool shake;
	bool heavy;
	bool drain;
	bool cursedCube;
	bool blackHole;
	bool itemBomb;
	bool parasite;
	bool luckySeven;
	bool nova;

	ItemEffectState()
	{
		Reset();
	}

	void Reset()
	{
		active = false;

		sourceItem = ItemType::E_NONE;

		pointValue = 0;
		lockSlot = -1;

		for (int i = 0; i < ITEM_EFFECT_SLOT_MAX; i++)
		{
			itemChargeSlotActive[i] = false;
			itemChargeFinalItems[i] = ItemType::E_NONE;
		}

		itemChargeSlotCount = 0;

		exchangePoint = false;
		pointUp = false;
		pointDown = false;
		itemCharge = false;
		stealItem = false;

		slowTarget = false;
		stopTarget = false;
		sneakPeek = false;
		sizeUpTarget = false;

		extraChance = false;

		shield = false;
		copyItem = false;
		magnet = false;
		lockItem = false;
		mirror = false;
		curse = false;
		item_block = false;
		confuse = false;
		shake = false;
		heavy = false;
		drain = false;
		cursedCube = false;
		blackHole = false;
		itemBomb = false;
		parasite = false;
		luckySeven = false;
		nova = false;
	}
};

#endif