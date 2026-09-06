#include "CThink.h"
#include <cstdlib>

void CThink::ApplyDifficulty()
{
	// 난이도별 AI 성향값 세팅
	// 이 값들이 실시간 판단, 오차, 반응속도, 목표 포기 기준에 영향을 준다.
	switch (m_Level)
	{
	case Step::E_EASY:
	{
		m_Accuracy = 0.55f;
		m_MistakeRate = 0.35f;
		m_RiskWeight = 0.65f;
		m_AggressiveWeight = 0.75f;
		m_ItemWeight = 0.6f;
		m_CubeWeight = 0.85f;

		// Easy는 생각이 느리고, 목표를 쉽게 포기하고, 흔들림이 많다.
		m_ThinkDelay = 1.2f;
		m_ReactionDelay = 0.28f;
		m_ReThinkInterval = 0.24f;
		m_SteerSmooth = 0.035f;
		m_PanicRate = 0.32f;
		m_GiveUpThreshold = 0.72f;
		break;
	}

	case Step::E_NOMAL:
	{
		m_Accuracy = 0.75f;
		m_MistakeRate = 0.16f;
		m_RiskWeight = 1.0f;
		m_AggressiveWeight = 1.0f;
		m_ItemWeight = 1.0f;
		m_CubeWeight = 1.0f;

		// Normal은 적당한 반응속도와 적당한 오차를 가진다.
		m_ThinkDelay = 0.75f;
		m_ReactionDelay = 0.16f;
		m_ReThinkInterval = 0.15f;
		m_SteerSmooth = 0.065f;
		m_PanicRate = 0.16f;
		m_GiveUpThreshold = 0.55f;
		break;
	}

	case Step::E_HARD:
	{
		m_Accuracy = 0.93f;
		m_MistakeRate = 0.04f;
		m_RiskWeight = 1.45f;
		m_AggressiveWeight = 1.35f;
		m_ItemWeight = 1.35f;
		m_CubeWeight = 1.2f;

		// Hard는 반응이 빠르고, 목표를 오래 유지하며, 오차가 작다.
		m_ThinkDelay = 0.35f;
		m_ReactionDelay = 0.06f;
		m_ReThinkInterval = 0.08f;
		m_SteerSmooth = 0.11f;
		m_PanicRate = 0.04f;
		m_GiveUpThreshold = 0.35f;
		break;
	}

	default:
	{
		m_Accuracy = 0.75f;
		m_MistakeRate = 0.16f;
		m_RiskWeight = 1.0f;
		m_AggressiveWeight = 1.0f;
		m_ItemWeight = 1.0f;
		m_CubeWeight = 1.0f;

		m_ThinkDelay = 0.8f;
		m_ReactionDelay = 0.16f;
		m_ReThinkInterval = 0.15f;
		m_SteerSmooth = 0.065f;
		m_PanicRate = 0.16f;
		m_GiveUpThreshold = 0.55f;
		break;
	}
	}
}

void CThink::ApplyAimError()
{
	// 최초 발사 전 목표에 적용되는 기본 오차
	float range = 1.0f - m_Accuracy;

	float errorScale = 2.2f;

	if (m_Level == Step::E_EASY) { errorScale = 8.0f; }
	else if (m_Level == Step::E_NOMAL) { errorScale = 4.0f; }
	else { errorScale = 1.3f; }

	float errorX = RandomRangeAI(-1.0f, 1.0f) * range * errorScale;
	float errorY = RandomRangeAI(-1.0f, 1.0f) * range * errorScale;

	// 실수 확률에 걸리면 오차를 크게 만든다.
	if (Random01() < m_MistakeRate)
	{
		errorX *= 3.0f;
		errorY *= 3.0f;
	}

	m_Aim.offsetX = errorX;
	m_Aim.offsetY = errorY;

	m_Aim.targetPos.x += errorX;
	m_Aim.targetPos.y += errorY;
}

float CThink::Random01()
{
	return (float)(rand() % 10000) / 10000.0f;
}

float CThink::RandomRangeAI(float minValue, float maxValue)
{
	float t = Random01();

	return minValue + ((maxValue - minValue) * t);
}