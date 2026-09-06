#include "CThink.h"
#include <algorithm>

void CThink::BuildTargetCandidates()
{
	m_Candidates.clear();

	if (g_GameSetting.type == GameType::E_COUNT_UP) { BuildCountUpCandidates(); }
	else											{ BuildZeroOneCandidates(); }
}

void CThink::BuildCountUpCandidates()
{
	AddCandidate(60, AITargetType::E_SCORE);
	AddCandidate(57, AITargetType::E_SCORE);
	AddCandidate(54, AITargetType::E_SCORE);
	AddCandidate(50, AITargetType::E_SCORE);
	AddCandidate(40, AITargetType::E_SAFE);
	AddCandidate(25, AITargetType::E_SAFE);
	AddCandidate(20, AITargetType::E_SAFE);

	if (ShouldPlayAggressive()) { AddCandidate(60, AITargetType::E_RISKY); }
}

void CThink::BuildZeroOneCandidates()
{
	int remain = m_Self.score;

	if (remain <= 0)
	{
		AddCandidate(1, AITargetType::E_SAFE);
		return;
	}

	// 정확히 끝낼 수 있으면 가장 우선
	if (remain <= 60)
	{
		AddCandidate(remain, AITargetType::E_FINISH);
	}

	// 남은 점수를 다음 턴에 끝내기 쉽게 만드는 후보
	if (remain > 60)
	{
		int setupScores[] =
		{
			60, 57, 54, 51, 50,
			48, 45, 42, 40,
			39, 36, 33, 30,
			25, 20
		};

		for (int i = 0; i < sizeof(setupScores) / sizeof(int); i++)
		{
			int s = setupScores[i];

			if (s < remain)
			{
				int nextRemain = remain - s;

				if (nextRemain <= 60)	{ AddCandidate(s, AITargetType::E_SETUP); }
				else					{ AddCandidate(s, AITargetType::E_SCORE); }
			}
		}
	}

	// 위험할 때 안전 후보
	if (remain <= 20)
	{
		AddCandidate(1, AITargetType::E_SAFE);
		AddCandidate(5, AITargetType::E_SAFE);
		AddCandidate(10, AITargetType::E_SAFE);
	}
	else
	{
		AddCandidate(20, AITargetType::E_SAFE);
		AddCandidate(25, AITargetType::E_SAFE);
	}

	// 상대가 끝내기 직전이면 공격 후보 추가
	if (EnemyCanFinishNextTurn())
	{
		AddCandidate(60, AITargetType::E_RISKY);
		AddCandidate(50, AITargetType::E_RISKY);
	}
}

void CThink::AddCandidate(int score, AITargetType type)
{
	if (score <= 0) return;

	AITargetCandidate c;

	c.score = score;
	c.type = type;
	c.targetPos = CalculateTargetPosition(score);
	c.bust = IsBust(score);
	c.finish = IsFinish(score);

	m_Candidates.push_back(c);
}

void CThink::EvaluateCandidates()
{
	for (auto& c : m_Candidates)
	{
		c.scoreValue = CalculateScoreValue(c.score);
		c.finishValue = CalculateFinishValue(c.score);
		c.safetyValue = CalculateSafetyValue(c.score);
		c.riskValue = CalculateBustRisk(c.score);

		c.finalValue = 0.0f;

		c.finalValue += c.scoreValue;
		c.finalValue += c.finishValue;
		c.finalValue += c.safetyValue;

		c.finalValue -= c.riskValue * m_RiskWeight;

		if (c.type == AITargetType::E_RISKY)		{ c.finalValue += 20.0f * m_AggressiveWeight; }
		else if (c.type == AITargetType::E_SETUP)   { c.finalValue += 15.0f; }
		else if (c.type == AITargetType::E_FINISH)  { c.finalValue += 100.0f; }

		// Easy는 가끔 이상한 판단을 하도록 노이즈 추가
		if (m_Level == Step::E_EASY)		{ c.finalValue += RandomRangeAI(-25.0f, 15.0f); }
		else if (m_Level == Step::E_NOMAL)	{ c.finalValue += RandomRangeAI(-8.0f, 8.0f); }
		else								{ c.finalValue += RandomRangeAI(-2.0f, 2.0f); }
	}
}

void CThink::SelectBestCandidate()
{
	if (m_Candidates.empty())
	{
		m_BestCandidate.score = 20;
		m_BestCandidate.targetPos = CalculateTargetPosition(20);
		return;
	}

	m_BestCandidate = m_Candidates[0];

	for (size_t i = 1; i < m_Candidates.size(); i++)
	{
		if (m_Candidates[i].finalValue > m_BestCandidate.finalValue)
		{
			m_BestCandidate = m_Candidates[i];
		}
	}
}

Vec3 CThink::CalculateTargetPosition(int score)
{
	Vec3 pos;
	pos.z = 60.0f;

	// 임시 과녁 좌표
	// 실제 과녁 점수 좌표 함수가 생기면 여기만 교체하면 됨.

	if (score == 50)
	{
		pos.x = 0.0f;
		pos.y = 0.0f;
	}
	else if (score == 60)
	{
		pos.x = 0.0f;
		pos.y = 0.75f;
	}
	else if (score == 57)
	{
		pos.x = -0.35f;
		pos.y = 0.65f;
	}
	else if (score == 54)
	{
		pos.x = 0.35f;
		pos.y = 0.65f;
	}
	else if (score >= 40)
	{
		pos.x = RandomRangeAI(-0.45f, 0.45f);
		pos.y = RandomRangeAI(0.25f, 0.7f);
	}
	else if (score >= 20)
	{
		pos.x = RandomRangeAI(-0.8f, 0.8f);
		pos.y = RandomRangeAI(-0.2f, 0.5f);
	}
	else
	{
		pos.x = RandomRangeAI(-1.0f, 1.0f);
		pos.y = RandomRangeAI(-0.7f, 0.7f);
	}

	return pos;
}