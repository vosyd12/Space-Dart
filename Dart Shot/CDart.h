#ifndef CDART_H
#define CDART_H

#include <windows.h>
#include <iostream>
#include "Common.h"
#include "CCollision.h"
#include "Math.h"
#pragma warning(disable : 4996)

class CDart
{
	private:
		Instance m_transform; // 위치 + 회전 + 스케일 통합
		Vec3 m_Direction;	  // 이동 방향

		float m_Speed;		// 속도

		bool m_isMoving;	// 움직임 여부
		bool m_isCollision; // 충돌여부
		bool m_isFired;

		DartOwner m_Owner;	// 누구 다트인지

		float m_Yaw;
		float m_Pitch;

	public:
		void Init(float x, float y, float z, Vec3 dir, DartOwner owner);
		void Update();

		void Stop();
		void Fly(Vec3 targetPos);
		void Move(float x, float y);
		void AimAt(Vec3 targetPos);
		void SteerToward(Vec3 targetPos, float steerRate);

		// 앞에 있는 const는 읽기 전용 인터페이스만 제공한다는 뜻
		// &sms 원본에 접근 + 수정은 가능하다는 뜻
		// 하지만 앞에 있는 const때문에 원본에 접근은 가능하되, 읽기만 가능하다 즉 값만 받아온다
		const Vec3& GetLocation() const			{ return m_transform.pos; }
		DartOwner GetOwner() const				{ return m_Owner; }

		// 랜더용
		const Instance& GetTransform() const	{ return m_transform; }
		DartState GetState() const				{ return { m_transform.pos, m_Direction }; }
		const bool GetMoving() const			{ return m_isMoving; }
		bool GetFired() const					{ return m_isFired; }
		float GetSpeed() const					{ return m_Speed; }
		Vec3 GetDirection() const				{ return m_Direction; }
};

#endif

// 다트는 과녁에서 온 위치값을 가지고 현재 그위치에 해당하는 점수를 계산한다.