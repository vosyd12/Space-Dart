#ifndef CTARGET_H
#define CTARGET_H

#include "Common.h"
#include "windows.h"
#include <stdlib.h>

class CTarget
{
	private:
		Vec3 m_Location;	// 다트의 현재 위치정보
		Vec3 m_Direction;

		float m_Speed;
		float m_BaseSpeed;
		float m_Limit;

		float m_Radius;		// 현재 과녁 크기
		float m_BaseRadius;	// 기본 과녁 크기

		float m_SizeUpFlashTimer;
		float m_SizeUpFlashDuration;

		bool m_isMoving;

		bool m_isSlow;
		bool m_isStop;
		bool m_isSizeUp;
		bool m_isSneakPeek;

		int m_DirectionIndex; // SneakPeek 이미지 선택용 0~3

		void ChangeRandomDirection();

	public:
		CTarget();
		~CTarget();

		void Init(float x, float y, float z);
		void Update(float dt);

		void Stop(); // 맞았을 때
		void Resume();
		void PrepareNextTurn();
		float GetRenderRadius() const;

		Vec3 GetLocation() const			{ return m_Location; }
		Vec3 GetDirection() const			{ return m_Direction; }

		// 이동 상태
		bool IsMoving() const				{ return m_isMoving; }

		// 속도/크기
		float GetSpeed() const				{ return m_Speed; }
		float GetRadius() const				{ return m_Radius; }

		int GetDirectionIndex() const		{ return m_DirectionIndex; }
		// AI/렌더/디버그용
		Vec3 GetNextDirectionHint() const	{ return m_Direction; }

		bool IsSlow() const					{ return m_isSlow; }
		bool IsStop() const					{ return m_isStop; }
		bool IsSizeUp() const				{ return m_isSizeUp; }
		bool IsSneakPeek() const			{ return m_isSneakPeek; }

		void SetLocation(const Vec3& pos)	{ m_Location = pos; }
		void SetSpeed(float speed)			{ m_Speed = speed; }
		void SetRadius(float radius)		{ m_Radius = radius; }

		// 아이템 효과
		void ApplySlow(float duration);
		void ApplyStop(float duration);
		void ApplySizeUp(float duration, float scale);
		void ApplySneakPeek(float duration);

		void ClearItemEffects();

		void Reset();
};

#endif

// 과녁은 위치값만 제공을한다.