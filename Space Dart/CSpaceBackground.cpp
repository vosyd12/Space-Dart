#include "CSpaceBackground.h"

void CSpaceBackground::Init(int width, int height)
{
	m_Width = width;
	m_Height = height;

	m_GalaxyRotation = 0.0f;
	m_CameraOffset = Vec3(0, 0, 0);

	m_Warp.active = false;
	m_Warp.power = 0.0f;
	m_Warp.starStrectch = 1.0f;
	m_Warp.moveSpeed = 1.0f;

	m_meteorSpawnTimer = 0.0f;
	// 유성 생성 변수
	m_meteorSpawnRate = 0.45f;

	CreateStars();
	CreateNebulas();
	CreateGalaxy();
	CreateDust();
}

void CSpaceBackground::Update(float deltaTime, Vec3 cameraMove)
{
	// 카메라 이동 누적
	m_CameraOffset += cameraMove;

	// 워프 보간
	float targetStretch = Lerp(1.0f, 18.0f, m_Warp.power);
	m_Warp.starStrectch = Lerp(m_Warp.starStrectch, targetStretch, deltaTime * 3.0f);
	
	// 별 업데이트
	for (auto& s : m_Stars)
	{
		// 실시간 시간 * 보정값 * 반짝임속도 = 별의 반짝임
		float time = GetTickCount() * 0.001f;

		float twinkle = sinf(time * s.twinkleSpeed);

		// 0~1 범위로 변환
		float twinkle01 = (twinkle + 1.0f) * 0.5f;

		// 기본 밝기 + 반짝임 강도
		s.brightness = s.baseBrightness + (twinkle01 * s.twinklePower);

		s.brightness = Clamp(s.brightness, 25.0f, 255.0f);

		s.Stretch = WarpStretch(m_Warp.moveSpeed, m_Warp.starStrectch);

		// 패럴랙스 이동
		// 카메라 이동에 맞게 이동
		s.pos.x -= cameraMove.x * s.depth;
		s.pos.y -= cameraMove.y * s.depth;
		s.pos.z -= cameraMove.z * s.depth;

		// 기본 이동 + 워프 이동
		if (m_Warp.active)
		{
			// 다트가 앞으로 날아가는 동안
			// 별은 반대 방향으로 빠르게 지나가는 것처럼 보이게 한다.
			float warpSpeed = 900.0f * m_Warp.power;

			s.pos.z -= warpSpeed * deltaTime;

			// 워프 중에는 별 꼬리를 더 길게 만든다.
			s.Stretch = 40.0f + (m_Warp.power * 180.0f);
		}

		else
		{
			s.pos.z -= 20.0f * deltaTime;
			s.Stretch = 1.0f;
		}

		// 거리 기반 밝기 감소
		float fade = DistanceFade(s.pos.z, 0.0f, 4000.0f);
		s.brightness *= fade;

		// 너무 가까워지면 재배치
		if (s.pos.z < -1000.0f)
		{
			s.pos.z = RandomRange(4500.0f, 9000.0f);
			s.pos.x = RandomRange(-3000.0f, 3000.0f);
			s.pos.y = RandomRange(-2200.0f, 2200.0f);
		}
	}

	m_GalaxyRotation += deltaTime * 0.3f;

	if (m_GalaxyRotation > 360.0f) { m_GalaxyRotation -= 360.0f; }

	m_meteorSpawnTimer += deltaTime;
	if (m_meteorSpawnTimer >= m_meteorSpawnRate)
	{
		SpawnMeteor();
		m_meteorSpawnTimer = 0.0f;
	}

	// 유성 이동
	for (auto& m : m_Meteors)
	{
		if (!m.active) continue;

		// 유성 이동 (유성의 시작에서 이동해야할 이동 좌표 * 스피드 * 실시간 타임)
		// 수학 : 현재위치 = 현재위치 + (방향벡터 * 속도 * 시간)
		m.pos += m.dir * m.speed * deltaTime * m_Warp.moveSpeed;

		// 거리 기반 밝기
		float distFade = DistanceFade(m.pos.z, 0.0f, 10000.0f);

		// 가까울수록 더 밝게
		float nearBoost = 0.45f + (m.nearRate * 0.55f);
		m.brightness = m.baseBrightness * distFade * nearBoost;

		// 화면 밖으로 완전히 나갈 때 제거
		if (m.pos.y < -5000.0f ||
			m.pos.x < -7000.0f ||
			m.pos.x > 7000.0f ||
			m.pos.z < -3000.0f ||
			m.pos.z > 13000.0f)
		{
			m.active = false;
		}
	}

	// 우주 먼지
	for (auto& d : m_Dusts)
	{
		d.pos += d.dir * d.speed * deltaTime * m_Warp.moveSpeed;

		d.pos.x -= cameraMove.x * d.depth;
		d.pos.y -= cameraMove.y * d.depth;
		d.pos.z -= cameraMove.z * d.depth;

		float fade = DistanceFade(d.pos.z, 0.0f, 3000.0f);

		d.brightness = 90.0f * fade;

		if (d.pos.z < -1000.0f)
		{
			d.pos.x = RandomRange(-2500.0f, 2500.0f);
			d.pos.y = RandomRange(-1800.0f, 1800.0f);
			d.pos.z = RandomRange(-5000.0f, 5000.0f);
		}
	}

	// 성운 이동
	for (auto& n : m_Nebulas)
	{
		n.pos.x -= cameraMove.x * 0.015f;
		n.pos.y -= cameraMove.y * 0.015f;
		n.pos.z -= cameraMove.z * 0.015f;

		n.pos.z -= n.moveSpeed * deltaTime * m_Warp.moveSpeed;

		if (n.pos.z < -1000.0f) 
		{ 
			n.pos.x = RandomRange(-9000.0f, 9000.0f);
			n.pos.y = RandomRange(-9000.0f, 9000.0f);
			n.pos.z = RandomRange(-9000.0f, 9000.0f);
		}
	}
}

void CSpaceBackground::StarWarp(float power)
{
	if (power < 0.0f) power = 0.0f;
	if (power > 1.0f) power = 1.0f;

	m_Warp.active = true;
	m_Warp.power = power;

	// 별 이동 속도 배율
	m_Warp.moveSpeed = 1.0f + (power * 8.0f);

	// 별이 길게 늘어나는 정도
	m_Warp.starStrectch = 1.0f + (power * 10.0f);
}

void CSpaceBackground::StopWarp()
{
	m_Warp.active = false;
	m_Warp.power = 0.0f;

	m_Warp.starStrectch = 1.0f;
	m_Warp.moveSpeed = 1.0f;
}