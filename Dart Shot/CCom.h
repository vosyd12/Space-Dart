#ifndef CCOM_H
#define CCOM_H

#include "CStatus.h"
#include "CDart.h"
#include "CItem.h"
#include "CThink.h"
#include "Common.h"
#include "GameSetting.h"

class CCom : public CStatus
{
	private:
		CDart m_Dart;		
		DartOwner m_Owner;		// 다트 소유자

		CThink m_Think;
		CItem m_Item;

		bool	  m_isThinking;	// AI 생각 중인지
		bool	  m_isReadyThrow; // AI가 던질 준비 완료했는지

		// 현재 보유 아이템 정보를 CThink 쪽으로 넘긴다.
		void SyncItemToThink();
	public:
		CCom();
		virtual ~CCom();

		// 초기화
		void Init();
		void Init(DartOwner owner, Step difficulty);

		// AI 판단
		void StartThink(int selfScore, int selfChance, int enemyScore, int enemyChance);
		void UpdateThink(float dt);
		void EndThink();

		bool IsThinking() const								{ return m_isThinking; }
		bool IsReadyThrow() const							{ return m_isReadyThrow; }

		AIState GetAIState() const							{ return m_Think.GetState(); }
		const AIAimData& GetAimData() const					{ return m_Think.GetAimData(); }

		CThink& GetThink()									{ return m_Think; }
		const CThink& GetThink() const						{ return m_Think; }

		// AI 결과 실행
		bool ShouldUseItem() const							{ return m_Think.GetAimData().useItem; }
		int GetUseItemSlot() const							{ return m_Think.GetAimData().useItemSlot; }
		ItemType GetUseItemType() const						{ return m_Think.GetAimData().useItemType; }
		bool ShouldThrow() const							{ return m_Think.GetAimData().shouldThrow; }

		// 다트 조작
		void ThrowDart(Vec3 targetPos, bool consumeChance = true);
		void ThrowByAI(bool consumeChance = true);

		void UpdateDart();
		void ResetDart();

		void UpdateRealtimeAI(float dt, Vec3 targetPos, Vec3 targetDir, float targetSpeed, float targetRadius, bool targetMoving, const Instance* cubes, int cubeCount);
		void ApplyRealtimeAim();

		bool IsDartMoving() const							{ return m_Dart.GetMoving(); }
		bool IsDartFired() const							{ return m_Dart.GetFired(); }

		CDart& GetDart()									{ return m_Dart; }
		const CDart& GetDart() const						{ return m_Dart; }

		DartState GetDartState() const						{ return m_Dart.GetState(); }
		Vec3 GetDartLocation() const						{ return m_Dart.GetLocation(); }

		// 아이템
		CItem& GetItem()									{ return m_Item; }
		const CItem& GetItem() const						{ return m_Item; }

		bool AddItem(ItemType type, int count = 1)			{ return m_Item.AddItem(type, count); }
		bool AddRandomItem()								{ return m_Item.AddRandomItem(); }
		ItemEffectState UseItemSlot(int slotIndex)			{ return m_Item.UseSlot(slotIndex); }

		// 기본 정보
		DartOwner GetOwner() const							{ return m_Owner; }

		// 상태 초기화
		virtual void Reset() override;
};

#endif

// 아이템은 날아가기 전에만 사용하는 방식으로