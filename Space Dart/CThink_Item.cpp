#include "CThink.h"

bool CThink::ShouldUseItem()
{
	int bestSlot = SelectBestItem();

	if (bestSlot < 0)
	{
		m_Aim.useItemSlot = -1;
		m_Aim.useItemType = ItemType::E_NONE;
		return false;
	}

	ItemType bestItem = m_Self.itemSlot[bestSlot];

	float value = EvaluateItemValue(bestItem);

	// 난이도별 아이템 사용 판단 보정
	if (m_Level == Step::E_EASY)
	{
		value *= 0.55f;
		value += RandomRangeAI(-25.0f, 15.0f);
	}
	else if (m_Level == Step::E_NOMAL)
	{
		value *= 0.85f;
		value += RandomRangeAI(-10.0f, 10.0f);
	}
	else if (m_Level == Step::E_HARD)
	{
		value *= 1.15f;
		value += RandomRangeAI(-3.0f, 3.0f);
	}

	value *= m_ItemWeight;

	if (value < 0.0f) value = 0.0f;
	if (value > 100.0f) value = 100.0f;

	float roll = RandomRangeAI(0.0f, 100.0f);

	if (roll < value)
	{
		m_Aim.useItemSlot = bestSlot;
		m_Aim.useItemType = bestItem;
		return true;
	}

	m_Aim.useItemSlot = -1;
	m_Aim.useItemType = ItemType::E_NONE;

	return false;
}

int CThink::SelectBestItem()
{
	int bestSlot = -1;
	float bestValue = -9999.0f;

	for (int i = 0; i < 3; i++)
	{
		ItemType item = m_Self.itemSlot[i];

		// 빈 칸은 절대 선택하지 않음
		if (item == ItemType::E_NONE)
		{
			continue;
		}

		float value = EvaluateItemValue(item);

		if (value > bestValue)
		{
			bestValue = value;
			bestSlot = i;
		}
	}

	// 가치가 너무 낮으면 사용하지 않음
	if (bestValue < 20.0f) { return -1; }

	return bestSlot;
}

float CThink::EvaluateItemValue(ItemType item)
{
	// 빈 아이템은 가치 없음
	if (item == ItemType::E_NONE)
	{
		return 0.0f;
	}

	float value = 0.0f;

	bool zeroOne = (g_GameSetting.type == GameType::E_ZERO_ONE);
	bool countUp = (g_GameSetting.type == GameType::E_COUNT_UP);

	int myScore = m_Self.score;
	int enemyScore = m_Enemy.score;

	bool canFinish = CanFinishThisTurn();
	bool enemyCanFinish = EnemyCanFinishNextTurn();
	bool danger = IsDangerState();

	switch (item)
	{
		case ItemType::E_EXCHANGE:
		{
			if (zeroOne)
			{
				// ZERO ONE에서는 점수가 낮을수록 유리
				if (enemyScore < myScore)	{ value += 85.0f; }
				if (enemyCanFinish)			{ value += 35.0f; }
				if (canFinish)				{ value -= 80.0f; }
			}

			else if (countUp)
			{
				// COUNT UP에서는 점수가 높을수록 유리
				if (enemyScore > myScore)	{ value += 80.0f; }
				else						{ value -= 60.0f; }
			}

			break;
		}

		case ItemType::E_POINT_UP:
		{
			// ZERO ONE에서는 남은 점수가 늘어나므로 보통 손해
			if (zeroOne)
			{
				if (enemyCanFinish) { value += 20.0f; }
				if (canFinish)		{ value -= 75.0f; }
				if (myScore > 120)	{ value += 15.0f; }
			}
			else if (countUp)
			{
				value += 70.0f;

				if (m_Self.chance <= 1) { value += 25.0f; }
			}

			break;
		}

		case ItemType::E_POINT_DOWN:
		{
			if (zeroOne)
			{
				if (myScore > 60)		{ value += 65.0f; }
				else if (myScore <= 60) { value += 30.0f; }
				else if (canFinish)		{ value -= 30.0f; }
			}

			else if (countUp)			{ value -= 55.0f; }

			break;
		}

		case ItemType::E_ITEMCHARGE:
		{
			value += 15.0f;

			if (!HasItem(ItemType::E_TARGET_STOP) &&
				!HasItem(ItemType::E_TARGET_SLOW) &&
				!HasItem(ItemType::E_TARGET_SIZEUP) &&
				!HasItem(ItemType::E_EXCHANGE) &&
				!HasItem(ItemType::E_EXTRA_CHANCE))
			{
				value += 40.0f;
			}

			if (danger)		{ value += 15.0f; }
			if (canFinish)	{ value -= 45.0f; }

			break;
		}

		case ItemType::E_ITEM_STEAL:
		{
			value += 25.0f;

			if (enemyCanFinish) { value += 55.0f; }
			else if (danger)	{ value += 20.0f; }

			break;
		}

		case ItemType::E_TARGET_SLOW:
		{
			value += 35.0f;

			if (canFinish)						{ value += 45.0f; }
			else if (m_Level == Step::E_EASY)	{ value += 15.0f; }

			break;
		}

		case ItemType::E_TARGET_STOP:
		{
			value += 45.0f;

			if (canFinish)						{ value += 70.0f; }
			else if (m_Self.chance <= 1)		{ value += 35.0f; }
			else if (zeroOne && myScore <= 60)	{ value += 40.0f; }

			break;
		}

		case ItemType::E_EXTRA_CHANCE:
		{
			if (enemyCanFinish) { value += 95.0f; }
			else if (danger)	{ value += 40.0f; }
			else				{ value += 15.0f; }

			if (canFinish) { value -= 50.0f; }

			break;
		}

		case ItemType::E_SNEAK_PEEK:
		{
			value += 25.0f;

			if (canFinish)						{ value += 30.0f; }
			else if (m_Level == Step::E_HARD)	{ value += 15.0f; }

			break;
		}

		case ItemType::E_TARGET_SIZEUP:
		{
			value += 40.0f;

			if (canFinish)					{ value += 50.0f; }
			else if (m_Self.chance <= 1)	{ value += 20.0f; }

			break;
		}

		default:
		{
			value = 0.0f;
			break;
		}
	}

	if (value < 0.0f) value = 0.0f;
	if (value > 100.0f) value = 100.0f;

	return value;
}

bool CThink::HasItem(ItemType item)
{
	for (int i = 0; i < 3; i++)
	{
		if (m_Self.itemSlot[i] == item)
		{
			return true;
		}
	}

	return false;
}

void CThink::SetSelfItem(int slot, ItemType type)
{
	if (slot < 0 || slot >= 3) { return; }

	m_Self.itemSlot[slot] = type;
}