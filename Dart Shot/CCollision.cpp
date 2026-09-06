#include "CCollision.h"

CCollision::CCollision()
{
	Init();
}

CCollision::~CCollision() {}

void CCollision::Init()
{
	m_TargetRadius = 3.0f;
	m_CubeRadius = 0.7f;
	m_TargetDepthRange = 1.0f;

	m_WorldLimitX = 80.0f;
	m_WorldLimitY = 80.0f;
	m_WorldLimitZ = 59.6f;

	m_InnerBullRate = 0.055f;
	m_OuterBullRate = 0.130f;

	m_TripleInnerRate = 0.440f;
	m_TripleOuterRate = 0.520f;

	m_DoubleInnerRate = 0.830f;
	m_DoubleOuterRate = 0.950f;
}

void CCollision::SetDartBoardRate(float innerBull, float outerBull, float tripleIn, float tripleOut, float doubleIn, float doubleOut)
{
	m_InnerBullRate = innerBull;
	m_OuterBullRate = outerBull;

	m_TripleInnerRate = tripleIn;
	m_TripleOuterRate = tripleOut;

	m_DoubleInnerRate = doubleIn;
	m_DoubleOuterRate = doubleOut;
}

bool CCollision::CheckOutofBounds(Vec3 pos)
{
	if (pos.x < -m_WorldLimitX || pos.x > m_WorldLimitX) return true;
	if (pos.y < -m_WorldLimitY || pos.y > m_WorldLimitY) return true;
	if (pos.z < -10.0f || pos.z > m_WorldLimitZ)		 return true;

	return false;
}

bool CCollision::CheckHitTarget(Vec3 dartPos, Vec3 targetPos, float targetRadius)
{
	float dz = fabsf(dartPos.z - targetPos.z);

	if (dz > m_TargetDepthRange)
	{
		return false;
	}

	return IsCircleHit2D(dartPos, targetPos, targetRadius);
}

bool CCollision::CheckCubeCollision(Vec3 dartPos, Vec3 cubePos, float cubeRadius)
{
	return IsSphereHit3D(dartPos, cubePos, cubeRadius);
}

TargetHitResult CCollision::CheckTargetHitAndScore(Vec3 dartPos, Vec3 targetPos, float targetRadius, int currentPoint, bool zeroOneMode)
{
	TargetHitResult result;

	result.hitPos = dartPos;

	// 과녁 이미지 전체 반지름이 아니라
	// 실제 점수판 영역만 판정 반지름으로 사용한다.
	// 업로드한 dart_target 이미지 기준으로 숫자/외곽 검은 영역을 제외하면 약 0.77 정도가 맞음.
	float scoreRadius = targetRadius * 0.74f;

	float dx = dartPos.x - targetPos.x;
	float dy = dartPos.y - targetPos.y;

	result.distance = sqrtf((dx * dx) + (dy * dy));
	result.rate = result.distance / scoreRadius;

	// 과녁 z축 두께 판정
	float dz = fabsf(dartPos.z - targetPos.z);

	if (dz > m_TargetDepthRange)
	{
		result.remainPoint = currentPoint;
		return result;
	}

	// 실제 점수판 바깥이면 MISS
	if (result.rate > 1.0f)
	{
		result.hit = false;
		result.area = DartHitArea::E_MISS;
		result.remainPoint = currentPoint;
		return result;
	}

	result.hit = true;

	// 각도 계산
	// 기준:
	// 12시 방향 = 20
	// 시계 방향 = 1,18,4,13...
	//
	// world 좌표에서 +Y를 위쪽, +X를 오른쪽으로 보고 계산한다.
	float angle = atan2f(dx, dy) * 180.0f / D3DX_PI;

	if (angle < 0.0f) { angle += 360.0f; }

	result.angle = angle;

	result.area = CalculateHitArea(result.rate);
	result.multiplier = CalculateMultiplier(result.area);

	if (result.area == DartHitArea::E_INNER_BULL)
	{
		result.bull = true;
		result.baseScore = 50;
		result.score = 50;
	}

	else if (result.area == DartHitArea::E_OUTER_BULL)
	{
		result.bull = true;
		result.baseScore = 25;
		result.score = 25;
	}

	else if (result.area == DartHitArea::E_MISS)
	{
		result.hit = false;
		result.bull = false;
		result.baseScore = 0;
		result.multiplier = 0;
		result.score = 0;
		result.remainPoint = currentPoint;
		return result;
	}

	else
	{
		// 표준 다트보드 숫자 배열
		// 12시 방향부터 시계방향
		int boardNumbers[20] =
		{
			20, 1, 18, 4, 13,
			6, 10, 15, 2, 17,
			3, 19, 7, 16, 8,
			11, 14, 9, 12, 5
		};

		// 각 섹터는 18도.
		// 20점 섹터가 12시 중앙에 오도록 9도 보정.
		int sectorIndex = (int)((angle + 9.0f) / 18.0f);

		if (sectorIndex < 0) { sectorIndex = 0; }
		if (sectorIndex > 19) { sectorIndex = 19; }

		result.bull = false;
		result.baseScore = boardNumbers[sectorIndex];
		result.score = result.baseScore * result.multiplier;
		result.score = ClampInt(result.score, 0, 60);
	}

	if (zeroOneMode)
	{
		result.bust = CheckBust(currentPoint, result.score);

		if (result.bust)	{ result.remainPoint = currentPoint; }
		else				{ result.remainPoint = currentPoint - result.score; }
	}

	else
	{
		result.bust = false;
		result.remainPoint = currentPoint + result.score;
	}

	return result;
}

CubeHitResult CCollision::CheckCubeHit(Vec3 dartPos, Vec3 cubePos, int cubeIndex, float cubeRadius)
{
	CubeHitResult result;

	result.hit = CheckCubeCollision(dartPos, cubePos, cubeRadius);

	if (result.hit)
	{
		result.cubeIndex = cubeIndex;
		result.cubePos = cubePos;
	}

	else
	{
		result.cubeIndex = -1;
		result.cubePos = { 0.0f, 0.0f, 0.0f };
	}

	return result;
}

DartHitArea CCollision::CalculateHitArea(float rate)
{
	// rate는 실제 점수판 반지름 기준이다.
	// 0.0 = 중앙, 1.0 = 점수판 바깥쪽 기준

	if (rate > 1.0f) { return DartHitArea::E_MISS; }

	// 불 영역은 작기 때문에 기존 설정값 그대로 사용한다.
	if (rate <= m_InnerBullRate) { return DartHitArea::E_INNER_BULL; }
	if (rate <= m_OuterBullRate) { return DartHitArea::E_OUTER_BULL; }

	// 트리플/더블 링은 이미지상으로 얇아서 좌표 오차가 조금만 생겨도 싱글로 빠진다.
	// 기존 설정값은 유지하되, 판정 여유값만 추가해서 링 판정 누락을 줄인다.
	float ringMargin = 0.015f;

	float tripleInner = m_TripleInnerRate - ringMargin;
	float tripleOuter = m_TripleOuterRate + ringMargin;

	float doubleInner = m_DoubleInnerRate - ringMargin;
	float doubleOuter = m_DoubleOuterRate + ringMargin;

	// 범위가 비정상적으로 바깥으로 나가지 않도록 보정한다.
	if (tripleInner < m_OuterBullRate) { tripleInner = m_OuterBullRate; }
	if (tripleOuter > doubleInner) { tripleOuter = doubleInner; }

	if (doubleInner < tripleOuter) { doubleInner = tripleOuter; }
	if (doubleOuter > 1.0f) { doubleOuter = 1.0f; }

	// 트리플 링 판정
	if (rate >= tripleInner && rate <= tripleOuter) { return DartHitArea::E_TRIPLE; }
	// 더블 링 판정
	if (rate >= doubleInner && rate <= doubleOuter) { return DartHitArea::E_DOUBLE; }

	return DartHitArea::E_SINGLE;
}

int CCollision::CalculateMultiplier(DartHitArea area)
{
	if (area == DartHitArea::E_DOUBLE)			{ return 2; }
	else  if (area == DartHitArea::E_TRIPLE)	{ return 3; }
	else if (area == DartHitArea::E_SINGLE)		{ return 1; }

	return 0;
}

int CCollision::CalculateTargetScore(Vec3 dartPos, Vec3 targetPos, float targetRadius)
{
	float scoreRadius = targetRadius * 0.74f;

	float dx = dartPos.x - targetPos.x;
	float dy = dartPos.y - targetPos.y;

	float distance = sqrtf((dx * dx) + (dy * dy));
	float rate = distance / scoreRadius;

	DartHitArea area = CalculateHitArea(rate);

	if (area == DartHitArea::E_MISS)
	{
		return 0;
	}

	if (area == DartHitArea::E_INNER_BULL)
	{
		return 50;
	}

	if (area == DartHitArea::E_OUTER_BULL)
	{
		return 25;
	}

	float angle = atan2f(dx, dy) * 180.0f / D3DX_PI;

	if (angle < 0.0f)
	{
		angle += 360.0f;
	}

	int boardNumbers[20] =
	{
		20, 1, 18, 4, 13,
		6, 10, 15, 2, 17,
		3, 19, 7, 16, 8,
		11, 14, 9, 12, 5
	};

	int sectorIndex = (int)((angle + 9.0f) / 18.0f);

	if (sectorIndex < 0) { sectorIndex = 0; }
	if (sectorIndex > 19) { sectorIndex = 19; }

	int baseScore = boardNumbers[sectorIndex];
	int multiplier = CalculateMultiplier(area);

	return ClampInt(baseScore * multiplier, 0, 60);
}

bool CCollision::CheckBust(int currentPoint, int score)
{
	return score > currentPoint;
}