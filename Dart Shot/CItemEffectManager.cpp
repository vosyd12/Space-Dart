#include "CItemEffectManager.h"

CItemEffectManager::CItemEffectManager()
{
	m_DrainUsed = false;
}

void CItemEffectManager::Reset()
{
	m_DrainUsed = false;
}

bool CItemEffectManager::IsDrainUsed() const
{
	return m_DrainUsed;
}

void CItemEffectManager::SetDrainUsed()
{
	m_DrainUsed = true;
}

void CItemEffectManager::ApplyItem(
	ItemType type,
	ItemEffectState& self,
	ItemEffectState& enemy)
{
	// 실드 우선 처리
	if (enemy.shield)
	{
		enemy.shield = false;
		return;
	}

	switch (type)
	{
		case ItemType::E_SHIELD:
		{
			self.shield = true;
			break;
		}

		case ItemType::E_MIRROR:
		{
			self.mirror = true;
			break;
		}

		case ItemType::E_ITEM_BLOCK:
		{
			enemy.item_block = true;
			break;
		}

		case ItemType::E_CONFUSE:
		{
			enemy.confuse = true;
			break;
		}

		case ItemType::E_SHAKE:
		{
			enemy.shake = true;
			break;
		}

		case ItemType::E_HEAVY:
		{
			enemy.heavy = true;
			break;
		}

		case ItemType::E_MAGNET:
		{
			self.magnet = true;
			break;
		}

		case ItemType::E_CURSED_CUBE:
		{
			enemy.cursedCube = true;
			break;
		}

		case ItemType::E_BLACK_HOLE:
		{
			enemy.blackHole = true;
			break;
		}

		case ItemType::E_PARASITE:
		{
			self.parasite = true;
			break;
		}

		case ItemType::E_LUCKY_SEVEN:
		{
			self.luckySeven = true;
			break;
		}

		default:
		{
			break;
		}
	}
}

void CItemEffectManager::EndTurn(ItemEffectState& effect)
{
	effect.Reset();
}

void CItemEffectManager::RemoveAllBuff(ItemEffectState& effect)
{
	effect.shield = false;
	effect.mirror = false;
	effect.magnet = false;
	effect.parasite = false;
	effect.blackHole = false;
	effect.luckySeven = false;
}