#include "CCom.h"

CCom::CCom()
{
	m_Owner = E_COMPUTER;

	m_isThinking = false;
	m_isReadyThrow = false;
}

CCom::~CCom() {}

void CCom::Init()
{
	Init(E_COMPUTER, g_GameSetting.step);
}

void CCom::Init(DartOwner owner, Step difficulty)
{
	m_Owner = owner;

	CStatus::Reset();
	SetAlive(true);

	m_Item.Init();

	m_Dart.Init(
		0.0f,
		0.0f,
		1.0f,
		{ 0.0f, 0.0f, 1.0f },
		m_Owner);

	m_Think.Init(difficulty);

	m_isThinking = false;
	m_isReadyThrow = false;
}

void CCom::SyncItemToThink()
{
	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		m_Think.SetSelfItem(i, m_Item.GetSlotType(i));
	}
}

void CCom::StartThink(
	int selfScore,
	int selfChance,
	int enemyScore,
	int enemyChance)
{
	m_isThinking = true;
	m_isReadyThrow = false;

	m_Think.SetSelfData(selfScore, selfChance);
	m_Think.SetEnemyData(enemyScore, enemyChance);

	SyncItemToThink();

	m_Think.StartTurn();
}

void CCom::UpdateThink(float dt)
{
	if (!m_isThinking) { return; }

	m_Think.Update(dt);
	const AIAimData& aim = m_Think.GetAimData();

	if (aim.shouldThrow)
	{
		m_isThinking = false;
		m_isReadyThrow = true;
	}
}

void CCom::EndThink()
{
	m_isThinking = false;
	m_isReadyThrow = false;

	m_Think.EndTurn();
}

void CCom::ThrowDart(Vec3 targetPos, bool consumeChance)
{
	// COUNT UP은 기존처럼 찬스가 없으면 던지지 않는다.
	// ZERO ONE은 찬스가 투척 횟수 카운트라서 0이어도 던질 수 있다.
	if (g_GameSetting.type != GameType::E_ZERO_ONE)
	{
		if (!HasChance()) { return; }
	}

	// 이미 다트가 움직이는 중이면 중복 발사 방지
	if (m_Dart.GetMoving()) { return; }

	// 컴퓨터는 목표 위치를 향해 자동 조준
	m_Dart.AimAt(targetPos);

	// 조준된 방향으로 발사
	m_Dart.Fly(targetPos);

	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		// ZERO ONE은 던질수록 찬스 카운트를 증가시킨다.
		AddChance(1);
	}

	// consumeChance가 true일 때만 찬스를 줄인다.
	else
	{
		DecreaseChance();
	}

	// 발사 준비 상태 해제
	m_isReadyThrow = false;
}

void CCom::ThrowByAI(bool consumeChance)
{
	const AIAimData& aim = m_Think.GetAimData();

	if (!aim.shouldThrow) { return; }

	ThrowDart(aim.targetPos, consumeChance);
}

void CCom::UpdateDart()
{
	m_Dart.Update();
}

void CCom::ResetDart()
{
	m_Dart.Init(
		0.0f,
		0.0f,
		1.0f,
		{ 0.0f, 0.0f, 1.0f },
		m_Owner);
}

void CCom::UpdateRealtimeAI(float dt, Vec3 targetPos, Vec3 targetDir, float targetSpeed, float targetRadius, bool targetMoving, const Instance* cubes, int cubeCount)
{
	// 현재 보유 아이템을 CThink에 계속 동기화한다.
	// 큐브를 먹어서 아이템이 추가된 직후에도 AI 판단에 반영되게 하기 위함.
	SyncItemToThink();

	// 현재 점수와 기회 상태도 계속 갱신한다.
	m_Think.SetSelfData(GetPoint(), GetChance());

	// 현재 월드 상태를 CThink에 전달한다.
	m_Think.SetWorldData(
		targetPos,
		targetDir,
		targetSpeed,
		targetRadius,
		targetMoving,
		m_Dart.GetLocation(),
		m_Dart.GetDirection(),
		m_Dart.GetSpeed(),
		m_Dart.GetMoving());

	// 큐브 위치를 CThink에 전달한다.
	m_Think.SetCubeData(cubes, cubeCount);

	// 비행 중 실시간 판단 실행
	m_Think.UpdateRealtime(dt);

	// 판단 결과를 실제 다트 움직임에 반영
	ApplyRealtimeAim();
}

void CCom::ApplyRealtimeAim()
{
	if (!m_Dart.GetMoving()) { return; }

	const AIAimData& aim = m_Think.GetAimData();

	float steerRate = 0.065f;

	// 난이도별 조준 보정 속도
	if (g_GameSetting.step == Step::E_EASY) { steerRate = 0.035f; }
	else if (g_GameSetting.step == Step::E_NOMAL) { steerRate = 0.065f; }
	else if (g_GameSetting.step == Step::E_HARD) { steerRate = 0.11f; }

	// AimAt은 즉시 방향 변경이라 기계적으로 보인다.
	// SteerToward는 목표 방향으로 조금씩 꺾어서 부드럽게 움직인다.
	m_Dart.SteerToward(aim.targetPos, steerRate);
}


void CCom::Reset()
{
	CStatus::Reset();

	SetAlive(true);

	m_Item.Reset();

	ResetDart();

	m_isThinking = false;
	m_isReadyThrow = false;

	m_Think.EndTurn();
}
