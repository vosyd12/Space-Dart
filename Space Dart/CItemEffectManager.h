#ifndef CITEMEFFECTMANAGER_H
#define CITEMEFFECTMANAGER_H

#include "CItemEffect.h"
#include "CPlayer.h"
#include "CCom.h"

class CItemEffectManager
{
private:

	bool m_DrainUsed;

public:

	CItemEffectManager();

	void Reset();

	// 아이템 효과 적용
	void ApplyItem(
		ItemType type,
		ItemEffectState& self,
		ItemEffectState& enemy);

	// 턴 종료
	void EndTurn(ItemEffectState& effect);

	// NOVA
	void RemoveAllBuff(ItemEffectState& effect);

	// DRAIN
	bool IsDrainUsed() const;
	void SetDrainUsed();
};

#endif