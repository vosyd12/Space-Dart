#ifndef CCOLLISION_H
#define CCOLLISION_H

#include "Common.h"
#include "Math.h"

enum class DartHitArea
{
	E_MISS,
	E_SINGLE,
	E_DOUBLE,
	E_TRIPLE,
	E_OUTER_BULL,
	E_INNER_BULL
};

// 과녁 충돌 결과
struct TargetHitResult
{
	bool hit;
	bool bull;
	bool bust;

	int score;
	int baseScore;
	int multiplier;
	int remainPoint;

	DartHitArea area;

	Vec3 hitPos;
	float distance;
	float rate;
	float angle;

	TargetHitResult()
	{
		hit = false;
		bull = false;
		bust = false;

		score = 0;
		baseScore = 0;
		multiplier = 0;
		remainPoint = 0;

		area = DartHitArea::E_MISS;

		hitPos = { 0.0f, 0.0f, 0.0f };
		distance = 0.0f;
		rate = 0.0f;
		angle = 0.0f;
	}
};

// 큐브 충돌 결과
struct CubeHitResult
{
	bool hit;
	int cubeIndex;
	Vec3 cubePos;

	CubeHitResult()
	{
		hit = false;
		cubeIndex = -1;
		cubePos = { 0.0f, 0.0f, 0.0f };
	}
};

class CCollision
{
	private:
		float m_TargetRadius;	// 과녁 기본 충돌 반지름
		float m_BullRadius;		// 불스아이 반지름
		float m_CubeRadius;		// 큐브 충돌 반지름
		float m_TargetDepthRange;

		float m_WorldLimitX;		// 다트 월드 X 제한
		float m_WorldLimitY;		// 다트 월드 Y 제한
		float m_WorldLimitZ;		// 다트 월드 Z 제한

		// 과녁 링 비율
		float m_InnerBullRate;
		float m_OuterBullRate;

		float m_TripleInnerRate;
		float m_TripleOuterRate;

		float m_DoubleInnerRate;
		float m_DoubleOuterRate;

	public:
		CCollision();
		~CCollision();

		void Init();

		void SetTargetRadius(float radius) { m_TargetRadius = radius; }
		void SetCubeRadius(float radius) { m_CubeRadius = radius; }

		void SetWorldLimit(float x, float y, float z)
		{
			m_WorldLimitX = x;
			m_WorldLimitY = y;
			m_WorldLimitZ = z;
		}

		void SetDartBoardRate(
			float innerBull,
			float outerBull,
			float tripleIn,
			float tripleOut,
			float doubleIn,
			float doubleOut);

		float GetTargetRadius() const				{ return m_TargetRadius; }
		float GetCubeRadius() const					{ return m_CubeRadius; }
		float GetTargetDepthRange() const			{ return m_TargetDepthRange; }

		void SetTargetDepthRange(float range)		{ m_TargetDepthRange = range; }

		bool CheckOutofBounds(Vec3 pos);

		bool CheckHitTarget(Vec3 dartPos, Vec3 targetPos, float targetRadius);
		bool CheckHitTarget(Vec3 dartPos, Vec3 targetPos) { return CheckHitTarget(dartPos, targetPos, m_TargetRadius); }

		bool CheckCubeCollision(Vec3 dartPos, Vec3 cubePos, float cubeRadius);
		bool CheckCubeCollision(Vec3 dartPos, Vec3 cubePos) { return CheckCubeCollision(dartPos, cubePos, m_CubeRadius); }

		TargetHitResult CheckTargetHitAndScore(Vec3 dartPos, Vec3 targetPos, float targetRadius, int currentPoint, bool zeroOneMode);
		CubeHitResult CheckCubeHit(Vec3 dartPos, Vec3 cubePos, int cubeIndex, float cubeRadius);

		CubeHitResult CheckCubeHit(Vec3 dartPos, Vec3 cubePos, int cubeIndex) { return CheckCubeHit(dartPos, cubePos, cubeIndex, m_CubeRadius); }

		int CalculateTargetScore(Vec3 dartPos, Vec3 targetPos, float targetRadius);
		DartHitArea CalculateHitArea(float rate);
		int CalculateMultiplier(DartHitArea area);

		bool CheckBust(int currentPoint, int score);
};

#endif

// 여기는 충돌 처리를 위해서 만든곳