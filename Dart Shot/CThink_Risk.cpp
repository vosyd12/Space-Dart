#include "CThink.h"

// 현재 점수를 맞추면 버스트인지 검사
// ZERO ONE 에서만 의미가 있다.
bool CThink::IsBust(int score)
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE) { return false; }

	// 남은 점수보다 큰 점수를 맞추면 버스트
	return score > m_Self.score;
}

// 현재 점수를 정확히 끝낼 수 있는지 검사
bool CThink::IsFinish(int score)
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE) { return false; }

	// 남은 점수와 맞출 점수가 같으면 종료
	return score == m_Self.score;
}

// 이번 턴에 게임을 끝낼 수 있는 상태인지 검사
// 현재 룰은 최대 60점 마무리 기준
bool CThink::CanFinishThisTurn()
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE) { return false; }

	return m_Self.score > 0 && m_Self.score <= 60;
}

// 상대가 다음 턴에 끝낼 가능성이 있는지 검사
bool CThink::EnemyCanFinishNextTurn()
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE) { return false; }

	return m_Enemy.score > 0 && m_Enemy.score <= 60;
}

// 현재 상황이 위험한 상태인지 판단
// 위험하면 공격적 플레이나 아이템 사용 빈도가 올라간다.
bool CThink::IsDangerState()
{
	// 마지막 기회면 위험
	if (m_Self.chance <= 1) { return true; }

	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		// 상대가 끝낼 수 있으면 위험
		if (EnemyCanFinishNextTurn())	{ return true; }

		// 남은 점수가 너무 적어도 위험
		if (m_Self.score <= 20)			{ return true; }
	}

	if (g_GameSetting.type == GameType::E_COUNT_UP)
	{
		// 상대와 점수 차이가 크게 벌어졌으면 위험
		if (m_Enemy.score > m_Self.score + 80) { return true; }
	}

	return false;
}

// 공격적으로 플레이할지 판단
// Hard 난이도일수록 공격적인 선택을 많이 한다.
bool CThink::ShouldPlayAggressive()
{
	if (m_Level == Step::E_HARD)
	{
		// 상대가 끝낼 수 있으면 강하게 승부
		if (EnemyCanFinishNextTurn())	{ return true; }

		// 마지막 기회도 공격적으로 처리
		if (m_Self.chance <= 1)			{ return true; }
	}

	if (g_GameSetting.type == GameType::E_COUNT_UP)
	{
		// 점수가 뒤지고 있으면 공격 모드
		if (m_Enemy.score > m_Self.score) { return true; }
	}

	return false;
}

// 특정 점수를 노렸을 때 버스트 위험도 계산
// 값이 높을수록 위험하다.
float CThink::CalculateBustRisk(int score)
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE) { return 0.0f; }
	// 버스트면 사실상 최악의 선택
	if (score > m_Self.score)	{ return 999.0f; }

	int remainAfter = m_Self.score - score;

	// 정확히 종료
	if (remainAfter == 0)		{ return 0.0f; }
	// 애매한 숫자가 남을수록 위험
	else if (remainAfter <= 5)	{ return 35.0f; }
	else if (remainAfter <= 10) { return 20.0f; }
	else if (remainAfter <= 20) { return 10.0f; }

	return 0.0f;
}

// 해당 점수를 노렸을 때 안전한지 계산
// 높을수록 AI가 선호한다.
float CThink::CalculateSafetyValue(int score)
{
	// COUNT UP 은 안전성보다 평균 점수 유지 위주
	if (g_GameSetting.type == GameType::E_COUNT_UP)
	{
		if (score >= 50) return 10.0f;
		if (score >= 20) return 20.0f;

		return 5.0f;
	}

	// 버스트는 최악
	if (IsBust(score)) { return -999.0f; }

	int remainAfter = m_Self.score - score;

	// 정확히 종료
	if (remainAfter == 0)		{ return 100.0f; }
	// 다음 턴 마무리 가능한 구간
	else if (remainAfter <= 10) { return 15.0f; }
	else if (remainAfter <= 60) { return 30.0f; }

	// 그 외 일반 상태
	return 10.0f;
}

// 해당 점수를 노렸을 때 마무리 가치 계산
float CThink::CalculateFinishValue(int score)
{
	if (g_GameSetting.type != GameType::E_ZERO_ONE)
	{
		return 0.0f;
	}

	// 즉시 종료 가능
	if (score == m_Self.score)
	{
		return 200.0f;
	}

	int remainAfter = m_Self.score - score;

	// 다음 턴 피니시 세팅
	if (remainAfter > 0 &&
		remainAfter <= 60)
	{
		return 60.0f;
	}

	return 0.0f;
}

// 순수 점수 가치 계산
// 얼마나 높은 점수를 얻는지 평가
float CThink::CalculateScoreValue(int score)
{
	if (g_GameSetting.type == GameType::E_COUNT_UP)
	{
		// COUNT UP은 점수가 곧 가치
		return (float)score * 1.5f;
	}

	// 버스트는 가치 없음
	if (IsBust(score))
	{
		return -999.0f;
	}

	return (float)score;
}