#include "CTarget.h"
#include "GameSetting.h"
#include "CModScene.h"

// 움직임대한 위치값을 저장하기 위해서 만듬
// 움직임을 8방향으로 만들지 않고 4방향으로 정의후 사용한다.
Vec3 Directions[4] =
{
	{ 1,  0, 0 }, // 좌우
	{ 0,  1, 0 }, // 상하
	{ 1,  1, 0 }, // ↙↗
	{-1,  1, 0 }  // ↖↘
};

float GetBaseTargetRadiusByDifficulty()
{
	// 과녁 이미지 전체 반지름 기준
	// 실제 점수판 판정은 CCollision에서 targetRadius * 0.77f로 계산된다.
	if (g_GameSetting.step == Step::E_EASY)			{ return 7.0f; }
	else if (g_GameSetting.step == Step::E_NOMAL)	{ return 6.0f; }
	else if (g_GameSetting.step == Step::E_HARD)	{ return 5.0f; }

	return 4.0f;
}

CTarget::CTarget()
{
	m_Location = { 0.0f, 0.0f, 60.0f };
	m_Direction = { 1.0f, 0.0f, 0.0f };

	m_Speed = 0.0f;
	m_BaseSpeed = 0.0f;
	m_Limit = 12.0f;

	m_Radius = 3.0f;
	m_BaseRadius = 3.0f;

	m_SizeUpFlashTimer = 0.0f;
	m_SizeUpFlashDuration = 0.0f;

	m_isMoving = true;

	m_isSlow = false;
	m_isStop = false;
	m_isSizeUp = false;
	m_isSneakPeek = false;

	m_DirectionIndex = 0;
}

CTarget::~CTarget() {}

// 이부분에서 난이도에 따른 이동속도및 움직이는 거리를 정의한다.
void CTarget::Init(float x, float y, float z)
{
	m_Location = { x,y,z };

	m_SizeUpFlashTimer = 0.0f;
	m_SizeUpFlashDuration = 0.0f;

	m_isMoving = true;

	switch (g_GameSetting.step)
	{
		case Step::E_EASY:
		{
			m_BaseSpeed = 0.1f;
			m_Limit = 5.0f;
			break;
		}

		case Step::E_NOMAL:
		{
			m_BaseSpeed = 0.15f;
			m_Limit = 7.0f;
			break;
		}

		case Step::E_HARD:
		{
			m_BaseSpeed = 0.2f;
			m_Limit = 10.0f;
			break;
		}

		default:
		{
			m_BaseSpeed = 0.15f;
			m_Limit = 7.0;
			break;
		}
	}

	m_Speed = m_BaseSpeed;

	m_BaseRadius = GetBaseTargetRadiusByDifficulty();
	m_Radius = m_BaseRadius;

	m_isSlow = false;
	m_isStop = false;
	m_isSizeUp = false;
	m_isSneakPeek = false;

	ChangeRandomDirection();
}

void CTarget::ChangeRandomDirection()
{
	m_DirectionIndex = rand() % 4;

	m_Direction = Directions[m_DirectionIndex];

	Normalize(m_Direction);
}

// 움직임이 한쪽끝에 도착하면 다른 방향으로 이동한다.
void CTarget::Update(float dt)
{
	// SIZEUP 플래시 타이머 갱신.
	// 과녁이 움직이지 않는 상태여도 이펙트 시간은 흘러야 하므로
	// 이동 return보다 먼저 처리한다.
	if (m_SizeUpFlashTimer > 0.0f)
	{
		m_SizeUpFlashTimer -= dt;

		if (m_SizeUpFlashTimer < 0.0f)
		{
			m_SizeUpFlashTimer = 0.0f;
		}
	}

	if (!m_isMoving) { return; }
	if (m_isStop)    { return; }

	float speed = m_Speed;

	if (m_isSlow) { speed = m_BaseSpeed * 0.5f; }

	// 기존 코드의 이동 감각을 살리기 위해 dt는 사용하지 않는다.
	// 나중에 완전한 프레임 독립 이동으로 바꾸고 싶으면 speed * dt 형태로 변경하면 된다.
	m_Location.x += m_Direction.x * speed;
	m_Location.y += m_Direction.y * speed;

	if (m_Location.x > m_Limit)
	{
		m_Location.x = m_Limit;
		m_Direction.x *= -1.0f;
	}

	if (m_Location.x < -m_Limit)
	{
		m_Location.x = -m_Limit;
		m_Direction.x *= -1.0f;
	}

	if (m_Location.y > m_Limit)
	{
		m_Location.y = m_Limit;
		m_Direction.y *= -1.0f;
	}

	if (m_Location.y < -m_Limit)
	{
		m_Location.y = -m_Limit;
		m_Direction.y *= -1.0f;
	}
}

void CTarget::Stop()
{
	m_isMoving = false;
}

void CTarget::Resume()
{
	m_isMoving = true;
}

void CTarget::PrepareNextTurn()
{
	// 턴 한정 아이템 효과 제거
	ClearItemEffects();

	// 과녁 위치를 중앙으로 복구
	// 현재 플레이 씬에서 Init(0,0,80)을 사용하므로 z도 80으로 맞춘다.
	m_Location = { 0.0f, 0.0f, 80.0f };

	// 난이도별 기본 크기를 다시 적용
	// 혹시 설정값이 바뀌었거나 SizeUp 효과가 끝났을 때 기준 크기로 복구하기 위함
	m_BaseRadius = GetBaseTargetRadiusByDifficulty();
	m_Radius = m_BaseRadius;

	// 현재 속도 복구
	m_Speed = m_BaseSpeed;

	// 새 턴마다 이동 방향을 다시 뽑는다.
	ChangeRandomDirection();

	// 다음 턴 시작 직후에는 조준 상태이므로 멈춰둔다.
	m_isMoving = false;
	m_isStop = false;
}

float CTarget::GetRenderRadius() const
{
	// 플래시가 없으면 실제 과녁 반지름 그대로 출력한다.
	if (m_SizeUpFlashTimer <= 0.0f)		{ return m_Radius; }
	if (m_SizeUpFlashDuration <= 0.0f)	{ return m_Radius; }

	// progress:
	// 플래시 이펙트가 얼마나 진행되었는지.

	// 시작 직후: 0.0
	// 끝날 때  : 1.0
	float progress =
		(m_SizeUpFlashDuration - m_SizeUpFlashTimer) /
		m_SizeUpFlashDuration;

	if (progress < 0.0f) { progress = 0.0f; }
	if (progress > 1.0f) { progress = 1.0f; }

	// flashCount:
	// 기본 크기와 커진 크기를 몇 번 교차해서 보여줄지.
	// 6이면 0.8초 동안 대략 6번 번쩍이는 느낌이다.
	const int flashCount = 6;

	// phase:
	// progress를 flashCount만큼 나누어 깜빡임 구간을 만든다.

	// 예:
	// progress = 0.00 ~ 0.16 : 0번 구간
	// progress = 0.16 ~ 0.33 : 1번 구간
	// progress = 0.33 ~ 0.50 : 2번 구간

	// 짝수 구간은 기본 크기,
	// 홀수 구간은 커진 크기로 출력한다.
	int phase = (int)(progress * flashCount);

	// 마지막 순간에는 반드시 커진 크기로 고정한다.
	if (progress >= 1.0f) { return m_Radius; }

	// 짝수 구간: 기본 크기.
	// 홀수 구간: 커진 크기.
	if ((phase % 2) == 0) { return m_BaseRadius; }

	return m_Radius;
}

void CTarget::ApplySlow(float duration)
{
	m_isSlow = true;
	m_Speed = m_BaseSpeed * 0.5f;
}

void CTarget::ApplyStop(float duration)
{
	m_isStop = true;
	m_isMoving = false;
}

void CTarget::ApplySizeUp(float duration, float scale)
{
	m_isSizeUp = true;

	if (scale < 1.0f) { scale = 1.0f; }

	// 실제 게임 판정 크기.
	// 아이템을 사용한 순간부터 과녁은 실제로 커진 상태가 된다.
	m_Radius = m_BaseRadius * scale;

	// duration을 따로 넘기지 않으면 기본 0.8초 사용.
	// 이 시간 동안 화면에 보이는 과녁 크기만
	// 기본 크기 ↔ 커진 크기로 번갈아 출력된다.
	if (duration <= 0.0f) { duration = 0.8f; }

	m_SizeUpFlashDuration = duration;
	m_SizeUpFlashTimer = duration;
}

void CTarget::ApplySneakPeek(float duration)
{
	m_isSneakPeek = true;
}

void CTarget::ClearItemEffects()
{
	m_isSlow = false;
	m_isStop = false;
	m_isSizeUp = false;
	m_isSneakPeek = false;

	m_SizeUpFlashTimer = 0.0f;
	m_SizeUpFlashDuration = 0.0f;

	m_Speed = m_BaseSpeed;
	m_Radius = m_BaseRadius;
}

void CTarget::Reset()
{
	ClearItemEffects();

	m_Location = { 0.0f, 0.0f, 80.0f };

	switch (g_GameSetting.step)
	{
		case Step::E_EASY:
		{
			m_BaseSpeed = 0.1f;
			m_Limit = 5.0f;
			break;
		}

		case Step::E_NOMAL:
		{
			m_BaseSpeed = 0.15f;
			m_Limit = 7.0f;
			break;
		}

		case Step::E_HARD:
		{
			m_BaseSpeed = 0.2f;
			m_Limit = 10.0f;
			break;
		}

		default:
		{
			m_BaseSpeed = 0.15f;
			m_Limit = 7.0;
			break;
		}
	}

	// 현재 속도 복구
	m_Speed = m_BaseSpeed;

	// 크기 복구
	m_BaseRadius = GetBaseTargetRadiusByDifficulty();
	m_Radius = m_BaseRadius;

	// 이동 가능 상태로 복구
	m_isMoving = true;
	m_isStop = false;

	// 이동 방향 재설정
	ChangeRandomDirection();
}