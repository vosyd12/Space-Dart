#ifndef CITEM_H
#define CITEM_H

#include "Common.h"
#include "CButton.h"
#include "CTextureManager.h"
#include "CItemEffect.h"
#include "GameSetting.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <windows.h>

using namespace std;

const int ITEM_SLOT_MAX = 5;

struct ItemSlot
{
	ItemType type;
	int count;
	wstring itemName;

	ItemSlot()
	{
		type = ItemType::E_NONE;
		count = 0;
		itemName = L"";
	}
};

class CItem
{
	private:
		ItemSlot m_Slot[ITEM_SLOT_MAX];
		int m_MaxSlotCount;

		// 아이템 아이콘을 찾기 위한 텍스처 매니저 포인터
		// 실제 텍스처 소유권은 CTextureManager가 가진다.
		CTextureManager* m_TextureManager;

		ItemType RandomItemType();
		wstring MakeItemName(ItemType type) const;
		int MakeRandomPointValue() const;

		int m_preloadIndex;
		bool m_preloadDone;

		int m_preloadFrameCounter;

	public :
		CItem();
		~CItem();

		void Init();
		void Reset();

		// 텍스처 매니저 연결
		// CPlayScene 또는 CD3D 초기화 이후 한 번만 호출하면 됨
		void SetTextureManager(CTextureManager* textureManager);

		// 슬롯 직접 관리
		bool IsValidSlot(int index) const;
		bool IsEmptySlot(int index) const;

		void ClearSlot(int index);
		void ClearAll();

		void SetSlot(int index, ItemType type, int count = 1);
		void SetSlotName(int index, const wstring& name);

		// 아이템 획득
		bool AddItem(ItemType type, int count = 1);
		bool AddRandomItem();

		// ITEMCHARGE 전용
		void RandomizeSlot(int index);
		void RandomizeAll();

		// 아이템 사용
		ItemEffectState UseSlot(int index);

		// 아이템 슬롯 아이콘 출력
		// CPlayScene은 이 함수만 호출하면 됨
		void DrawSlotIcon(LPDIRECT3DDEVICE9 device, int slotIndex, const RECT& rc);
		// 실제 아이콘 한 장 출력
		void DrawItemIcon(LPDIRECT3DDEVICE9 device, ItemType type, const RECT& rc, D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255));

		// 조회
		const ItemSlot& GetSlot(int index) const;
		ItemType GetSlotType(int index) const;
		int GetSlotCount(int index) const;
		wstring GetSlotName(int index) const;
		// ITEMCHARGE 슬롯머신용 아이콘들을 미리 로드한다.
		// 이걸 아이템 사용 전에 한 번 호출하면,
		// 슬롯머신 도중 LoadTexture로 인한 순간 프레임 드랍을 줄일 수 있다.
		void StartPreloadItemTextures();
		void UpdatePreloadItemTextures();
		bool IsPreloadDone() const				{ return m_preloadDone; }

		bool HasItem(ItemType type) const;
		int GetItemCount(ItemType type) const;
		int FindItemSlot(ItemType type) const;
};

#endif

// 아이템의 리스트는 외부 텍스트에 존재한다.
// E_ITEMCHARGE 아이템같은경우 슬롯머신과 같이 아이템칸에서 실제로 돌아가는 모션을 취한다.
// 아이템에 대한 전체적인 관리
// 다트의 종류를 

// 아이템을 사용할시에 따른 구조는 상하관계가 필요할듯하다
// 예를들면 과녁을 멈추게했지만 내가 과녁 슬로우 아이템을 사용해서 과녁에 대입을 시키면 
// 전에 사용한 과녁 스톱아이템은 무용지물이 되므로 
// 이런경우에는 한가지의 상태만 가질수 있게 되어야할듯하다.
