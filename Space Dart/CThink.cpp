#include "CThink.h"

CThink::CThink()
{
	m_Level = Step::E_NOMAL;
	m_State = AIState::E_IDLE;

	m_ThinkTimer = 0.0f;
	m_ThinkDelay = 0.8f;

	m_Accuracy = 0.75f;
	m_MistakeRate = 0.15f;
	m_RiskWeight = 1.0f;
	m_AggressiveWeight = 1.0f;
	m_ItemWeight = 1.0f;
	m_CubeWeight = 1.0f;

	m_ReactionDelay = 0.15f;
	m_ReThinkTimer = 0.0f;
	m_ReThinkInterval = 0.15f;

	m_SteerSmooth = 0.065f;
	m_PanicRate = 0.15f;
	m_GiveUpThreshold = 0.45f;

	m_TargetPos = { 0.0f, 0.0f, 60.0f };
	m_TargetDir = { 0.0f, 0.0f, 0.0f };
	m_TargetSpeed = 0.0f;
	m_TargetRadius = 3.0f;
	m_TargetMoving = false;

	m_DartPos = { 0.0f, 0.0f, 1.0f };
	m_DartDir = { 0.0f, 0.0f, 1.0f };
	m_DartSpeed = 0.15f;
	m_DartFlying = false;

	m_CubeCount = 0;

	m_CurrentPlanTarget = { 0.0f, 0.0f, 60.0f };
	m_CurrentPlanScore = 20;
	m_TargetHoldTimer = 0.0f;
}

CThink::~CThink() {}

void CThink::Init(Step level)
{
	m_Level = level;
	m_State = AIState::E_IDLE;

	ApplyDifficulty();
	ResetAim();
}

void CThink::Update(float dt)
{
	switch (m_State)
	{
		case AIState::E_IDLE:
		{
			break;
		}

		case AIState::E_THINK:
		{
			m_ThinkTimer += dt;

			if (m_ThinkTimer >= m_ThinkDelay)
			{
				Think();
				m_State = AIState::E_AIM;
			}

			break;
		}

		case AIState::E_AIM:
		{
			m_State = AIState::E_THROW;
			break;
		}

		case AIState::E_THROW:
		{
			m_Aim.shouldThrow = true;
			m_State = AIState::E_WAIT;
			break;
		}

		case AIState::E_WAIT:
		{
			break;
		}
	}
}

void CThink::SetSelfData(int score, int chance)
{
	m_Self.score = score;
	m_Self.chance = chance;
}

void CThink::SetEnemyData(int score, int chance)
{
	m_Enemy.score = score;
	m_Enemy.chance = chance;
}

void CThink::SetWorldData(
	Vec3 targetPos,
	Vec3 targetDir,
	float targetSpeed,
	float targetRadius,
	bool targetMoving,
	Vec3 dartPos,
	Vec3 dartDir,
	float dartSpeed,
	bool dartFlying)
{
	// 현재 과녁 상태를 AI 판단용으로 저장
	m_TargetPos = targetPos;
	m_TargetDir = targetDir;
	m_TargetSpeed = targetSpeed;
	m_TargetRadius = targetRadius;
	m_TargetMoving = targetMoving;

	// 현재 다트 상태를 AI 판단용으로 저장
	m_DartPos = dartPos;
	m_DartDir = dartDir;
	m_DartSpeed = dartSpeed;
	m_DartFlying = dartFlying;
}

void CThink::SetCubeData(const Instance* cubes, int cubeCount)
{
	if (!cubes)
	{
		m_CubeCount = 0;
		return;
	}

	if (cubeCount < 0) { cubeCount = 0; }
	if (cubeCount > AI_MAX_CUBE_COUNT) { cubeCount = AI_MAX_CUBE_COUNT; }

	m_CubeCount = cubeCount;

	for (int i = 0; i < m_CubeCount; i++)
	{
		m_Cubes[i] = cubes[i];
	}
}

void CThink::StartTurn()
{
	m_ThinkTimer = 0.0f;
	m_ReThinkTimer = 0.0f;
	m_TargetHoldTimer = 0.0f;

	m_State = AIState::E_THINK;

	ResetAim();
	ApplyDifficulty();
}

void CThink::EndTurn()
{
	m_State = AIState::E_IDLE;

	ResetAim();
}

void CThink::ResetAim()
{
	m_Aim.targetPos = { 0.0f, 0.0f, 60.0f };
	m_Aim.targetScore = 0;
	m_Aim.expectedScore = 0;
	m_Aim.offsetX = 0.0f;
	m_Aim.offsetY = 0.0f;
	m_Aim.accuracy = m_Accuracy;
	m_Aim.useItem = false;
	m_Aim.useCube = false;
	m_Aim.shouldThrow = false;

	m_Aim.useItemSlot = -1;
	m_Aim.useItemType = ItemType::E_NONE;

	m_CurrentPlanTarget = { 0.0f, 0.0f, 60.0f };
	m_CurrentPlanScore = 20;
}

void CThink::Think()
{
	m_Candidates.clear();

	BuildTargetCandidates();
	EvaluateCandidates();
	SelectBestCandidate();

	m_Aim.targetScore = m_BestCandidate.score;
	m_Aim.expectedScore = m_BestCandidate.score;
	m_Aim.targetPos = m_BestCandidate.targetPos;

	// 처음 정한 목표를 현재 계획으로 저장
	m_CurrentPlanTarget = m_Aim.targetPos;
	m_CurrentPlanScore = m_Aim.targetScore;

	m_Aim.useItem = ShouldUseItem();
	m_Aim.useCube = ShouldGetCube();

	ApplyAimError();

	m_Aim.accuracy = m_Accuracy;
}

void CThink::UpdateRealtime(float dt)
{
	// 다트가 날아가는 중이 아닐 때는 실시간 판단하지 않음
	if (!m_DartFlying) { return; }

	m_ReThinkTimer += dt;
	m_TargetHoldTimer += dt;

	// 너무 자주 판단하면 다트가 떨리므로 일정 간격마다만 재판단
	if (m_ReThinkTimer < m_ReThinkInterval) { return; }

	m_ReThinkTimer = 0.0f;

	ThinkRealtime();
}

void CThink::ThinkRealtime()
{
	// 비행 중에도 아이템과 큐브 판단은 계속 갱신
	m_Aim.useItem = ShouldUseItem();
	m_Aim.useCube = ShouldGetCube();

	Vec3 finalTarget = SelectRealtimeTarget();

	float errorScale = GetDifficultyErrorScale();
	float jitterScale = GetHumanJitterScale();

	// 난이도에 따른 조준 오차
	float errorX = RandomRangeAI(-1.0f, 1.0f) * errorScale;
	float errorY = RandomRangeAI(-1.0f, 1.0f) * errorScale;

	// 사람처럼 보이기 위한 아주 작은 흔들림
	float jitterX = RandomRangeAI(-1.0f, 1.0f) * jitterScale;
	float jitterY = RandomRangeAI(-1.0f, 1.0f) * jitterScale;

	// 쉬운 난이도일수록 가끔 판단이 크게 흔들림
	if (Random01() < m_PanicRate)
	{
		errorX *= 1.7f;
		errorY *= 1.7f;
	}

	finalTarget.x += errorX + jitterX;
	finalTarget.y += errorY + jitterY;

	m_Aim.targetPos = finalTarget;
	m_Aim.offsetX = errorX;
	m_Aim.offsetY = errorY;
	m_Aim.accuracy = m_Accuracy;
}

Vec3 CThink::SelectRealtimeTarget()
{
	Vec3 cubeTarget;

	// 큐브를 먹을 가치가 있고 아이템칸도 비어 있으면 큐브를 우선 목표로 선택
	if (m_Aim.useCube && HasUsefulCubePath(cubeTarget))
	{
		m_Aim.useCube = true;
		m_CurrentPlanTarget = cubeTarget;
		return cubeTarget;
	}

	m_Aim.useCube = false;

	// 큐브를 노리지 않는다면 과녁의 미래 위치를 예측
	Vec3 predictedTarget = PredictTargetPosition();

	// 현재 목표를 계속 노릴 수 있으면 그대로 유지
	if (!ShouldGiveUpCurrentTarget(predictedTarget)) { return predictedTarget; }

	// 현재 목표가 너무 어려워졌으면 다른 안전한 목표로 변경
	return ChooseFallbackTarget();
}

Vec3 CThink::PredictTargetPosition()
{
	Vec3 predicted = m_TargetPos;

	if (!m_TargetMoving) { return predicted; }

	float zDist = m_TargetPos.z - m_DartPos.z;

	if (zDist < 0.0f) { zDist = 0.0f; }

	float speed = m_DartSpeed;

	if (speed < 0.001f) { speed = 0.15f; }

	// 현재 구조에서는 다트 이동이 프레임 단위 속도라서 도착까지 걸리는 프레임 수로 예측
	float arriveFrame = zDist / speed;

	float predictScale = 1.0f;

	if (m_Level == Step::E_EASY) { predictScale = 0.45f; }
	else if (m_Level == Step::E_NOMAL) { predictScale = 0.75f; }
	else { predictScale = 1.0f; }

	predicted.x += m_TargetDir.x * m_TargetSpeed * arriveFrame * predictScale;
	predicted.y += m_TargetDir.y * m_TargetSpeed * arriveFrame * predictScale;

	// 예측값이 너무 멀리 튀는 것을 방지
	if (predicted.x > 10.0f)	{ predicted.x = 10.0f; }
	if (predicted.x < -10.0f)	{ predicted.x = -10.0f; }
	if (predicted.y > 10.0f)	{ predicted.y = 10.0f; }
	if (predicted.y < -10.0f)	{ predicted.y = -10.0f; }

	return predicted;
}

bool CThink::ShouldGiveUpCurrentTarget(Vec3 targetPos)
{
	float reach = EvaluateReachability(targetPos);

	// 현재 방향에서 목표까지 자연스럽게 도달하기 어렵다면 목표 포기
	if (reach < m_GiveUpThreshold) { return true; }

	return false;
}

float CThink::EvaluateReachability(Vec3 targetPos)
{
	Vec3 desired = targetPos - m_DartPos;
	if (Length(desired) < 0.0001f) { return 1.0f; }

	Normalize(desired);

	Vec3 current = m_DartDir;
	if (Length(current) < 0.0001f) { current = { 0.0f, 0.0f, 1.0f }; }

	Normalize(current);

	// 현재 방향과 목표 방향이 얼마나 비슷한지 계산
	float dot = (current.x * desired.x) + (current.y * desired.y) + (current.z * desired.z);

	if (dot < -1.0f) { dot = -1.0f; }
	if (dot > 1.0f) { dot = 1.0f; }

	// 1에 가까울수록 현재 궤도에서 목표에 도달하기 쉬움
	return dot;
}

Vec3 CThink::ChooseFallbackTarget()
{
	Vec3 fallback = m_TargetPos;

	// 목표를 포기했을 때 종목에 맞는 안전한 목표를 선택
	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		if (CanFinishThisTurn())		{ fallback = CalculateTargetPosition(m_Self.score); }
		else if (m_Self.score > 100)	{ fallback = CalculateTargetPosition(20); }
		else if (m_Self.score > 60)		{ fallback = CalculateTargetPosition(25); }
		else							{ fallback = CalculateTargetPosition(10); }
	}
	else
	{
		if (m_Level == Step::E_HARD)		{ fallback = CalculateTargetPosition(50); }
		else if (m_Level == Step::E_NOMAL)	{ fallback = CalculateTargetPosition(25); }
		else								{ fallback = CalculateTargetPosition(20); }
	}

	// CalculateTargetPosition은 과녁 중심 기준 좌표이므로 현재 과녁 위치를 더해준다.
	fallback.x += m_TargetPos.x;
	fallback.y += m_TargetPos.y;
	fallback.z = m_TargetPos.z;

	m_CurrentPlanTarget = fallback;

	return fallback;
}

float CThink::GetDifficultyErrorScale() const
{
	if (m_Level == Step::E_EASY)		{ return 0.35f; }
	else if (m_Level == Step::E_NOMAL)	{ return 0.15f; }
	else								{ return 0.045f; }
}

float CThink::GetHumanJitterScale() const
{
	if (m_Level == Step::E_EASY)		{ return 0.055f; }
	else if (m_Level == Step::E_NOMAL)	{ return 0.028f; }
	else								{ return 0.010f; }
}