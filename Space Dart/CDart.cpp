#include "CDart.h"

CCollision g_collision;

void CDart::Init(float x, float y, float z, Vec3 dir, DartOwner owner)
{
	// 위치 초기화
	m_transform.pos.x = x;
	m_transform.pos.y = y;
	m_transform.pos.z = z;

	// 회전 / 스케일 초기화
	m_transform.rot = { 0.0f, 0.0f, 0.0f };
	m_transform.scale = { 1.0f, 1.0f, 1.0f };

	// 조준 각도 초기화
	m_Yaw = 0.0f;
	m_Pitch = 0.0f;

	// 방향 초기화
	if (Length(dir) < 0.0001f) { m_Direction = { 0.0f, 0.0f, 1.0f }; }
	else
	{
		m_Direction = dir;
		Normalize(m_Direction);
	}

	// 상태 초기화
	m_Speed = 0.15f;
	m_isMoving = false;
	m_isCollision = false;
	m_isFired = false;

	m_Owner = owner;
}

void CDart::Update()
{
	if (!m_isMoving) return;

	Normalize(m_Direction);

	// 이동
	m_transform.pos += m_Direction * m_Speed;
}

void CDart::Stop()
{
	m_isMoving = false;
	m_isCollision = true;
}

void CDart::Fly(Vec3 targetPos)
{
    /* 디버그용
    char buf[128];
    sprintf(buf, "Offset: %f %f", m_Aimoffset.x, m_Aimoffset.y);
    MessageBoxA(NULL, buf, "debug", MB_OK);
    */

	if (m_isMoving) { return; }

	// 혹시 방향값이 비정상일 경우 기본 정면 방향으로 보정
	if (Length(m_Direction) < 0.0001f) { m_Direction = { 0.0f, 0.0f, 1.0f }; }

	Normalize(m_Direction);

	// 발사 상태로 변경
	m_isMoving = true;
	m_isFired = true;
	m_isCollision = false;
}

// 다트의 뱡향전환을 위한 함수
void CDart::Move(float x, float y)
{
   // MessageBoxA(NULL, "Move called", "debug", MB_OK); 디버그용

	float turnSpeed = 0.03f;

	// 조준 각도 변경
	m_Yaw += x * turnSpeed;
	m_Pitch += y * turnSpeed;

	// 조준 가능 각도 제한
	float maxYaw = 0.55f;
	float maxPitch = 0.32f;

	if (m_Yaw > maxYaw)		m_Yaw = maxYaw;
	if (m_Yaw < -maxYaw)	m_Yaw = -maxYaw;

	if (m_Pitch > maxPitch)		m_Pitch = maxPitch;
	if (m_Pitch < -maxPitch)	m_Pitch = -maxPitch;

	// 렌더링은 m_Direction을 보고 다트를 그리기 때문에
	// 조준 중에도 방향 벡터를 즉시 갱신해야 한다.
	m_Direction.x = cosf(m_Pitch) * sinf(m_Yaw);
	m_Direction.y = sinf(m_Pitch);
	m_Direction.z = cosf(m_Pitch) * cosf(m_Yaw);

	Normalize(m_Direction);

	// transform 회전을 쓰는 코드가 나중에 생길 경우를 대비
	m_transform.rot.x = -m_Pitch;
	m_transform.rot.y = m_Yaw;
	m_transform.rot.z = 0.0f;
}

void CDart::AimAt(Vec3 targetPos)
{
	// 현재 다트 위치에서 목표 위치까지의 방향 계산
	Vec3 dir = targetPos - m_transform.pos;

	// 방향이 너무 작으면 기본 정면 방향 사용
	if (Length(dir) < 0.0001f) { dir = { 0.0f, 0.0f, 1.0f };}

	Normalize(dir);

	// AI가 던질 방향을 실제 다트 방향으로 반영
	m_Direction = dir;

	// 렌더링 회전 보정용
	m_Yaw = atan2f(m_Direction.x, m_Direction.z);
	m_Pitch = asinf(m_Direction.y);

	m_transform.rot.x = -m_Pitch;
	m_transform.rot.y = m_Yaw;
	m_transform.rot.z = 0.0f;
}

void CDart::SteerToward(Vec3 targetPos, float steerRate)
{
	Vec3 desired = targetPos - m_transform.pos;

	if (Length(desired) < 0.0001f) { return; }

	Normalize(desired);

	if (steerRate < 0.0f) { steerRate = 0.0f; }
	if (steerRate > 1.0f) { steerRate = 1.0f; }

	// 현재 방향에서 목표 방향으로 천천히 보간한다.
	m_Direction.x = m_Direction.x + (desired.x - m_Direction.x) * steerRate;
	m_Direction.y = m_Direction.y + (desired.y - m_Direction.y) * steerRate;
	m_Direction.z = m_Direction.z + (desired.z - m_Direction.z) * steerRate;

	Normalize(m_Direction);

	// 렌더링 방향도 현재 이동 방향에 맞춘다.
	m_Yaw = atan2f(m_Direction.x, m_Direction.z);
	m_Pitch = asinf(m_Direction.y);

	m_transform.rot.x = -m_Pitch;
	m_transform.rot.y = m_Yaw;
	m_transform.rot.z = 0.0f;
}