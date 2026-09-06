#ifndef __CSATUS_H__
#define __CSATUS_H__

#include <windows.h>
#include "CItemEffect.h"

class CStatus
{
	private :
		int m_Point;
		int m_Chance;
		bool m_isAlive;

		ItemEffectState m_ItemEffect;
		
	public:
		CStatus();
		virtual ~CStatus();

		// 점수
		void AddPoint(int point)	{ m_Point += point; }
		void SubPoint(int point)	{ m_Point -= point; if (m_Point < 0) { m_Point = 0; } }
		void SetPoint(int point)	{ m_Point = point; if (m_Point < 0) { m_Point = 0; } }
		void SetChance(int chance)	{ m_Chance = chance; }
		void AddChance(int chance)  { m_Chance += chance; }
		void SetAlive(bool alive)	{ m_isAlive = alive; }

		int GetPoint() const		{ return m_Point; }
		int GetChance() const		{ return m_Chance; }
		bool HasChance() const		{ return m_Chance > 0; }
		bool IsAlive() const		{ return m_isAlive; }

		void DecreaseChance() { if (m_Chance > 0) { m_Chance--; } }

		// 아이템 효과 접근
		ItemEffectState& GetItemEffect()				{ return m_ItemEffect; }
		const ItemEffectState& GetItemEffect() const	{ return m_ItemEffect; }
		// 1턴 효과 전체 제거
		void ClearItemEffect()							{ m_ItemEffect.Reset(); }

		virtual void Reset();
};

#endif

// 필요한 스텟에 대한 클라스
// 부모 클래스에서 가상 함수를 자식 클래스에서 재정의하 않을시, 
// 부모 클래스에 정의된 기본 가상 함수가 그대로 실행된다.