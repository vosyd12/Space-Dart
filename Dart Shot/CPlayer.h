#ifndef CPLAYER_H
#define CPLAYER_H

#include "CStatus.h"
#include "CDart.h"
#include "CItem.h"
#include "Common.h"

class CPlayer : public CStatus
{
	private:
		DartOwner m_Owner;
		CDart m_Dart;
		CItem m_Item;
		
	public:
		CPlayer();
		virtual ~CPlayer();

		// 기본 초기화
		void Init();

		// 소유자 지정 초기화
		void Init(DartOwner owner);

		// 다트 조작
		void ThrowDart(Vec3 targetPos);
		void MoveDart(float x, float y);
		void UpdateDart();
		void ResetDart();

		bool IsDartMoving() const					{ return m_Dart.GetMoving(); }
		bool IsDartFired() const					{ return m_Dart.GetFired(); }

		CDart& GetDart()							{ return m_Dart; }
		const CDart& GetDart() const				{ return m_Dart; }

		DartState GetDartState() const				{ return m_Dart.GetState(); }
		Vec3 GetDartLocation() const				{ return m_Dart.GetLocation(); }

		// 아이템
		CItem& GetItem()							{ return m_Item; }
		const CItem& GetItem() const				{ return m_Item; }

		bool AddItem(ItemType type, int count = 1)	{ return m_Item.AddItem(type, count); }
		bool AddRandomItem()						{ return m_Item.AddRandomItem(); }
		ItemEffectState UseItemSlot(int slotIndex)  { return m_Item.UseSlot(slotIndex); }

		// 기본 정보
		DartOwner GetOwner() const { return m_Owner; }

		// 전체 상태 초기화
		virtual void Reset() override;
};

#endif