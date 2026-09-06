#include "CThink.h"

bool CThink::HasEmptyItemSlot() const
{
	// 아이템칸이 하나라도 비어 있으면 큐브를 먹을 가치가 있다.
	for (int i = 0; i < 3; i++)
	{
		if (m_Self.itemSlot[i] == ItemType::E_NONE) { return true; }
	}

	return false;
}

bool CThink::ShouldGetCube()
{
	// 아이템칸이 가득 차 있으면 큐브를 먹지 않는다.
	if (!HasEmptyItemSlot())
	{
		m_Aim.useCube = false;
		return false;
	}

	float value = CalculateCubeValue();
	float roll = RandomRangeAI(0.0f, 100.0f);

	return roll < value;
}

float CThink::CalculateCubeValue()
{
	// 빈 슬롯이 없으면 큐브 가치는 0이다.
	if (!HasEmptyItemSlot()) { return 0.0f; }

	float value = 0.0f;

	if (m_Level == Step::E_EASY) { value = 15.0f; }
	else if (m_Level == Step::E_NOMAL) { value = 35.0f; }
	else { value = 55.0f; }

	// 마지막 기회면 큐브보다 점수를 우선한다.
	if (m_Self.chance <= 1) { value -= 25.0f; }

	// 이번 턴에 끝낼 수 있으면 큐브 욕심을 줄인다.
	if (CanFinishThisTurn()) { value -= 60.0f; }

	// 상대가 끝낼 수 있는 상태면 큐브보다 점수/방해를 우선한다.
	if (EnemyCanFinishNextTurn()) { value -= 40.0f; }

	// COUNT UP은 점수 누적 게임이라 아이템 획득 가치가 조금 더 높다.
	if (g_GameSetting.type == GameType::E_COUNT_UP) { value += 15.0f; }

	value *= m_CubeWeight;

	if (value < 0.0f) { value = 0.0f; }
	if (value > 100.0f) { value = 100.0f; }

	return value;
}

bool CThink::HasUsefulCubePath(Vec3& outCubePos)
{
	// 아이템칸이 가득 차 있으면 큐브 경로를 찾지 않는다.
	if (!HasEmptyItemSlot()) { return false; }

	if (m_CubeCount <= 0) { return false; }

	float bestValue = -9999.0f;
	int bestIndex = -1;

	for (int i = 0; i < m_CubeCount; i++)
	{
		Vec3 cubePos = m_Cubes[i].pos;

		// 이미 다트가 지나간 큐브는 제외
		if (cubePos.z <= m_DartPos.z) { continue; }

		// 과녁보다 뒤쪽의 큐브는 제외
		if (cubePos.z >= m_TargetPos.z) { continue; }

		float dx = cubePos.x - m_DartPos.x;
		float dy = cubePos.y - m_DartPos.y;
		float distance2D = sqrtf((dx * dx) + (dy * dy));

		// 현재 다트 위치에서 가까울수록 가치가 높다.
		float value = 100.0f - (distance2D * 18.0f);

		if (m_Self.chance <= 1) { value -= 35.0f; }
		if (CanFinishThisTurn()) { value -= 70.0f; }

		// 난이도별 판단 흔들림
		if (m_Level == Step::E_EASY) { value += RandomRangeAI(-30.0f, 10.0f); }
		else if (m_Level == Step::E_NOMAL) { value += RandomRangeAI(-12.0f, 12.0f); }
		else { value += RandomRangeAI(-3.0f, 8.0f); }

		if (value > bestValue)
		{
			bestValue = value;
			bestIndex = i;
		}
	}

	if (bestIndex < 0) { return false; }

	// 가치가 낮으면 큐브를 무리해서 먹지 않는다.
	if (bestValue < 45.0f) { return false; }

	outCubePos = m_Cubes[bestIndex].pos;
	return true;
}