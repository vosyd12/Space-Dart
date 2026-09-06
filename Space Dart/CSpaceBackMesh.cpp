#include "CSpaceBackground.h"

// 별 생성
void CSpaceBackground::CreateStars()
{
	m_Stars.clear();

	// 생성할 별 갯수
	const int Star_Count = 5000;

	for (int i = 0; i < Star_Count; i++)
	{
		StarParticle s;

		int layer = rand() % 100;

		if (layer < 38)
		{
			// 먼 별
			s.pos.x = RandomRange(-7000.0f, 7000.0f);
			s.pos.y = RandomRange(-7000.0f, 7000.0f);
			s.pos.z = RandomRange(-7000.0f, 7000.0f);

			s.size = RandomRange(2.0f, 3.0f);
			s.brightness = RandomRange(90.0f, 170.0f);
			s.depth = RandomRange(0.02f, 0.12f);
		}

		else if (layer < 55)
		{
			// 중간 별
			s.pos.x = RandomRange(-5000.0f, 5000.0f);
			s.pos.y = RandomRange(-5000.0f, 5000.0f);
			s.pos.z = RandomRange(-4500.0f, 4500.0f);

			s.size = RandomRange(4.0f, 6.0f);
			s.brightness = RandomRange(140.0f, 230.0f);
			s.depth = RandomRange(0.15f, 0.45f);
		}

		else
		{
			// 가까운 강조 별
			s.pos.x = RandomRange(-3000.0f, 2300.0f);
			s.pos.y = RandomRange(-3000.0f, 3000.0f);
			s.pos.z = RandomRange(-2200.0f, 2200.0f);

			s.size = RandomRange(10.0f, 20.0f);
			s.brightness = RandomRange(210.0f, 255.0f);
			s.depth = RandomRange(0.5f, 1.0f);
		}

		s.twinkleSpeed = RandomRange(2.0f, 5.0f);
		s.Stretch = 1.0f;

		int type = rand() % 5;

		if (type == 0)		{ s.color = D3DCOLOR_XRGB(255, 255, 255); }
		else if (type == 1) { s.color = D3DCOLOR_XRGB(190, 225, 255); }
		else if (type == 2) { s.color = D3DCOLOR_XRGB(160, 200, 255); }
		else if (type == 3) { s.color = D3DCOLOR_XRGB(230, 210, 255); }
		else				{ s.color = D3DCOLOR_XRGB(210, 235, 255); }

		s.baseBrightness = s.brightness;

		int twinkleType = rand() % 100;

		if (twinkleType < 40)
		{
			// 대부분: 아주 미세한 반짝임
			s.twinklePower = RandomRange(15.0f, 30.0f);
			s.twinkleSpeed = RandomRange(2.0f, 3.5f);
		}

		else if (twinkleType < 60)
		{
			// 일부: 눈에 보이는 반짝임
			s.twinklePower = RandomRange(50.0f, 100.0f);
			s.twinkleSpeed = RandomRange(3.0f, 5.0f);
		}

		else
		{
			// 극소수: 강하게 번쩍임
			s.twinklePower = RandomRange(120.0f, 240.0f);
			s.twinkleSpeed = RandomRange(4.0f, 7.0f);
		}

		m_Stars.push_back(s);
	}
}

// 성운 생성
void CSpaceBackground::CreateNebulas()
{
	m_Nebulas.clear();

	const int Nebula_Count = 40; // 성운 갯수

	for (int i = 0; i < Nebula_Count; i++)
	{
		Nebula n;

		// 성운 위치
		n.pos.x = RandomRange(-9000.0f, 9000.0f);
		n.pos.y = RandomRange(-9000.0f, 9000.0f);
		n.pos.z = RandomRange(-9000.0f, 9000.0f);

		// 성운 크기 
		n.size = RandomRange(200.0f, 2000.0f);

		// 랜덤 텍스처 
		n.texIndex = rand() % 4;

		// 성운 투명도 낮을수록 희미함 높을수록 진해짐
		n.alpha = RandomRange(0.025f, 0.08f);
		n.moveSpeed = RandomRange(10.0f, 20.0f);
		n.color = D3DCOLOR_XRGB(255, 255, 255);

		m_Nebulas.push_back(n);
	}
}

// 회전하는 은하 생성
void CSpaceBackground::CreateGalaxy()
{
	m_Galaxy.clear();
}

// 유성 생성
void CSpaceBackground::SpawnMeteor()
{
	Meteor m;

	m.active = true;

	int roll = rand() % 100;

	if (roll < 45)
	{
		m.type = MeteorType::E_NORMAL;
		m.tailScale = 1.0f;
		m.glowScale = 1.0f;
		m.alphaScale = 1.0f;
	}

	else if (roll < 60)
	{
		m.type = MeteorType::E_LONG;
		m.tailScale = 1.7f;
		m.glowScale = 0.9f;
		m.alphaScale = 0.9f;
	}

	else if (roll < 80)
	{
		m.type = MeteorType::E_FAR;
		m.tailScale = 0.75f;
		m.glowScale = 0.45f;
		m.alphaScale = 0.45f;
	}

	else
	{
		m.type = MeteorType::E_CINEMATIC;
		m.tailScale = 2.4f;
		m.glowScale = 1.8f;
		m.alphaScale = 1.25f;
	}

	m.pos.z = RandomRange(700.0f, 8500.0f);

	if (m.type == MeteorType::E_FAR) { m.pos.z = RandomRange(5000.0f, 10000.0f); }

	m.nearRate = 1.0f - ((m.pos.z - 700.0f) / (10000.0f - 700.0f));

	if (m.nearRate < 0.0f) m.nearRate = 0.0f;
	if (m.nearRate > 1.0f) m.nearRate = 1.0f;

	m.pos.x = RandomRange(4500.0f, 7000.0f);
	m.pos.y = RandomRange(2500.0f, 4200.0f);

	m.dir = Vec3(
		RandomRange(-1.45f, -0.55f),
		RandomRange(-0.75f, -0.18f),
		RandomRange(-0.22f, 0.18f));

	Normalize(m.dir);

	m.speed = 420.0f + (m.nearRate * 1300.0f);
	m.size = 18.0f + (m.nearRate * 48.0f);
	m.width = 7.0f + (m.nearRate * 24.0f);
	m.tailLength = 460.0f + (m.nearRate * 1250.0f);

	if (m.type == MeteorType::E_CINEMATIC)
	{
		m.speed *= 0.85f;
		m.size *= 1.35f;
		m.width *= 1.45f;
		m.tailLength *= 1.25f;
	}

	m.baseBrightness = 160.0f + (m.nearRate * 95.0f);
	m.brightness = m.baseBrightness;

	int colorType = rand() % 4;

	if (colorType == 0)		 m.color = D3DCOLOR_ARGB(255, 120, 180, 255);
	else if (colorType == 1) m.color = D3DCOLOR_ARGB(255, 180, 130, 255);
	else if (colorType == 2) m.color = D3DCOLOR_ARGB(255, 220, 240, 255);
	else					 m.color = D3DCOLOR_ARGB(255, 90, 210, 255);

	m_Meteors.push_back(m);
}

void CSpaceBackground::CreateDust()
{
	m_Dusts.clear();

	const int DUST_COUNT = 450;

	for (int i = 0; i < DUST_COUNT; i++)
	{
		SpaceDust d;

		d.pos.x = RandomRange(-2500.0f, 2500.0f);
		d.pos.y = RandomRange(-1800.0f, 1800.0f);
		d.pos.z = RandomRange(-5000.0f, 5000.0f);

		d.dir = Vec3(0, 0, -1);

		d.speed = RandomRange(2.0f, 8.0f);
		d.size = RandomRange(0.4f, 1.2f);

		d.brightness = RandomRange(20.0f, 80.0f);
		d.depth = RandomRange(0.1f, 0.4f);
		d.color = D3DCOLOR_ARGB(80, 150, 180, 255);

		m_Dusts.push_back(d);
	}
}