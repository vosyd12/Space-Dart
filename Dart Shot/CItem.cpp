#include "CItem.h"
#include "Math.h"
#include <stdlib.h>

CItem::CItem()
{
	Init();
}

CItem::~CItem() 
{
	// 텍스처는 CTextureManager가 소유하므로 여기서 Release하지 않는다.
	m_TextureManager = nullptr;
}

void CItem::Init()
{
	// 텍스처는 CTextureManager가 소유하므로 여기서 Release하지 않는다.
	m_TextureManager = nullptr;

	m_MaxSlotCount = ITEM_SLOT_MAX;
	m_preloadIndex = 0;
	m_preloadDone = true;
	m_preloadFrameCounter = 0;
}

void CItem::Reset()
{
	ClearAll();
}

void CItem::SetTextureManager(CTextureManager* textureManager)
{
	// 아이템 아이콘 출력을 위해 텍스처 매니저를 연결
	// 실제 텍스처 생성/해제는 CTextureManager가 담당
	m_TextureManager = textureManager;
}

void CItem::DrawItemIcon(LPDIRECT3DDEVICE9 device, ItemType type, const RECT& rc, D3DCOLOR color)
{
	if (!device) { return; }
	if (!m_TextureManager) { return; }
	if (type == ItemType::E_NONE) { return; }

	wstring textureKey = MakeItemName(type);

	if (textureKey.empty()) { return; }
	if (textureKey == L"none") { return; }

	LPDIRECT3DTEXTURE9 texture = m_TextureManager->LoadTexture(textureKey);

	if (!texture) { return; }

	RECT dst = CButton::ScaleRect(rc);

	CButton::DrawTexture(device, dst, texture, color);
}

void CItem::DrawSlotIcon(LPDIRECT3DDEVICE9 device, int slotIndex, const RECT& rc)
{
	if (!IsValidSlot(slotIndex)) { return; }

	ItemType type = m_Slot[slotIndex].type;
	if (type == ItemType::E_NONE) { return; }

	// 슬롯에 들어있는 아이템 타입에 맞는 아이콘 출력
	DrawItemIcon(device, type, rc);
}

bool CItem::IsValidSlot(int index) const
{
	int slotCount;

	if (g_GameSetting.step == Step::E_EASY)			{ slotCount = 5; }
	else if (g_GameSetting.step == Step::E_NOMAL)	{ slotCount = 4; }
	else											{ slotCount = 3; }

	if (slotCount > ITEM_SLOT_MAX) { slotCount = ITEM_SLOT_MAX; }

	return index >= 0 && index < slotCount;
}

bool CItem::IsEmptySlot(int index) const
{
	if (!IsValidSlot(index)) { return true; }
	return m_Slot[index].type == ItemType::E_NONE;
}

void CItem::ClearSlot(int index)
{
	if (!IsValidSlot(index)) { return; }
	m_Slot[index] = ItemSlot();
}

void CItem::ClearAll()
{
	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		m_Slot[i] = ItemSlot();
	}
}

void CItem::SetSlot(int index, ItemType type, int count)
{
	if (!IsValidSlot(index)) { return; }

	if (type == ItemType::E_NONE)
	{
		ClearSlot(index);
		return;
	}

	// 슬롯 하나당 아이템 하나만 저장되도록한다.
	if (count <= 0) { count = 1; }

	m_Slot[index].type = type;
	m_Slot[index].count = 1;
	m_Slot[index].itemName = MakeItemName(type);
}

void CItem::SetSlotName(int index, const wstring& name)
{
	if (!IsValidSlot(index)) { return; }
	m_Slot[index].itemName = name;
}

bool CItem::AddItem(ItemType type, int count)
{
	if (type == ItemType::E_NONE) { return false; }

	// 슬롯 하나당 아이템 하나만 허용
	// count는 무조건 1로 고정
	count = 1;

	// 빈 슬롯에만 추가
	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		if (!IsValidSlot(i)) { continue; }

		if (m_Slot[i].type == ItemType::E_NONE)
		{
			SetSlot(i, type, count);
			return true;
		}
	}

	// 빈 슬롯이 없으면 획득 실패
	return false;
}

bool CItem::AddRandomItem()
{
	return AddItem(RandomItemType(), 1);
}

void CItem::RandomizeSlot(int index)
{
	if (!IsValidSlot(index)) { return; }
	SetSlot(index, RandomItemType(), 1);
}

void CItem::RandomizeAll()
{
	// ITEMCHARGE 규칙:
	// 비어있는 슬롯은 제외한다.
	// 비어있는 슬롯은 계속 비어있어야 한다.
	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		if (!IsValidSlot(i)) { continue; }
		if (m_Slot[i].type == ItemType::E_NONE) { continue; }

		RandomizeSlot(i);
	}
}

ItemEffectState CItem::UseSlot(int index)
{
	ItemEffectState effect;

	if (!IsValidSlot(index)) { return effect; }
	if (m_Slot[index].type == ItemType::E_NONE) { return effect; }

	if (m_Slot[index].count <= 0)
	{
		ClearSlot(index);
		return effect;
	}

	ItemType type = m_Slot[index].type;

	effect.active = true;
	effect.sourceItem = type;
	effect.lockSlot = -1;

	switch (type)
	{
		case ItemType::E_POINT_UP:
		{
			effect.pointUp = true;
			effect.pointValue = MakeRandomPointValue();
			break;
		}

		case ItemType::E_POINT_DOWN:
		{
			effect.pointDown = true;
			effect.pointValue = MakeRandomPointValue();
			break;
		}

		case ItemType::E_EXCHANGE:		{ effect.exchangePoint = true; break; }
		case ItemType::E_ITEMCHARGE:
		{
			effect.itemCharge = true;

			// 현재 난이도 기준 실제 슬롯 수 계산.
			int slotCount;

			if (g_GameSetting.step == Step::E_EASY) { slotCount = 5; }
			else if (g_GameSetting.step == Step::E_NOMAL) { slotCount = 4; }
			else { slotCount = 3; }

			if (slotCount > ITEM_SLOT_MAX) { slotCount = ITEM_SLOT_MAX; }

			effect.itemChargeSlotCount = slotCount;

			// 중요:
			// ITEMCHARGE가 들어있던 슬롯도 사용 직전에는 아이템이 있었던 슬롯이다.
			// 그러므로 해당 슬롯도 슬롯머신 대상에 포함해야 한다.
			for (int i = 0; i < slotCount; i++)
			{
				if (!IsValidSlot(i)) { continue; }

				if (m_Slot[i].type != ItemType::E_NONE)
				{
					ItemType newItem = RandomItemType();

					effect.itemChargeSlotActive[i] = true;
					effect.itemChargeFinalItems[i] = newItem;

					// 실제 사용 가능한 최종 아이템으로 슬롯을 교체한다.
					SetSlot(i, newItem, 1);
				}
			}

			// ITEMCHARGE는 위에서 이미 슬롯을 교체했다.
			// 아래 공통 count 감소 / ClearSlot 로직을 타면
			// ITEMCHARGE가 있던 자리가 다시 비어버릴 수 있다.
			// 그래서 여기서 바로 return한다.
			return effect;
		}
		case ItemType::E_ITEM_STEAL:	{ effect.stealItem = true; break; }
		case ItemType::E_TARGET_SLOW:	{ effect.slowTarget = true; break; }
		case ItemType::E_TARGET_STOP:	{ effect.stopTarget = true; break; }
		case ItemType::E_EXTRA_CHANCE:	{ effect.extraChance = true; break; }
		case ItemType::E_SNEAK_PEEK:	{ effect.sneakPeek = true; break; }
		case ItemType::E_TARGET_SIZEUP:	{ effect.sizeUpTarget = true; break; }
		case ItemType::E_SHIELD:		{ effect.shield = true; break; }
		case ItemType::E_ITEM_COPY:		{ effect.copyItem = true; break; }
		case ItemType::E_MAGNET:		{ effect.magnet = true; break; }
		case ItemType::E_ITEM_LOCK:		{ effect.lockItem = true; break; }
		case ItemType::E_MIRROR:		{ effect.mirror = true; break; }
		case ItemType::E_CURSE:			{ effect.curse = true; break; }
		case ItemType::E_ITEM_BLOCK:	{ effect.item_block = true; break; }
		case ItemType::E_CONFUSE:		{ effect.confuse = true; break; }
		case ItemType::E_SHAKE:			{ effect.shake = true; break; }
		case ItemType::E_HEAVY:			{ effect.heavy = true; break; }
		case ItemType::E_DRAIN:			{ effect.drain = true; break; }
		case ItemType::E_CURSED_CUBE:	{ effect.cursedCube = true; break; }
		case ItemType::E_BLACK_HOLE:	{ effect.blackHole = true; break; }
		case ItemType::E_ITEM_BOMB:		{ effect.itemBomb = true; break; }
		case ItemType::E_PARASITE:		{ effect.parasite = true; break; }
		case ItemType::E_LUCKY_SEVEN:	{ effect.luckySeven = true; break; }
		case ItemType::E_NOVA:			{ effect.nova = true; break; }
		default:						{ effect.Reset(); return effect; }
	}

	m_Slot[index].count--;

	if (m_Slot[index].count <= 0)
	{
		ClearSlot(index);
	}

	return effect;
}

const ItemSlot& CItem::GetSlot(int index) const
{
	static ItemSlot emptySlot;

	if (!IsValidSlot(index)) { return emptySlot; }

	return m_Slot[index];
}

ItemType CItem::GetSlotType(int index) const
{
	if (!IsValidSlot(index)) { return ItemType::E_NONE; }
	return m_Slot[index].type;
}

int CItem::GetSlotCount(int index) const
{
	if (!IsValidSlot(index)) { return 0; }
	return m_Slot[index].count;
}

wstring CItem::GetSlotName(int index) const
{
	if (!IsValidSlot(index)) { return L""; }

	return m_Slot[index].itemName;
}

void CItem::StartPreloadItemTextures()
{
	m_preloadIndex = (int)ItemType::E_EXCHANGE;
	m_preloadDone = false;
	m_preloadFrameCounter = 0;
}

void CItem::UpdatePreloadItemTextures()
{
	if (m_preloadDone) { return; }
	if (!m_TextureManager) { return; }

	m_preloadFrameCounter++;

	// 3프레임마다 1개씩 로드한다.
	if (m_preloadFrameCounter < 3) { return; }

	m_preloadFrameCounter = 0;

	int maxValue = (int)ItemType::E_NOVA;

	while (m_preloadIndex <= maxValue)
	{
		ItemType type = (ItemType)m_preloadIndex;
		m_preloadIndex++;

		wstring textureKey = MakeItemName(type);

		if (textureKey.empty()) { continue; }
		if (textureKey == L"none") { continue; }

		m_TextureManager->LoadTexture(textureKey);
		return;
	}

	m_preloadDone = true;
}

bool CItem::HasItem(ItemType type) const
{
	return FindItemSlot(type) >= 0;
}

int CItem::GetItemCount(ItemType type) const
{
	if (type == ItemType::E_NONE) { return 0; }

	int count = 0;

	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		if (m_Slot[i].type == type)
		{
			count += m_Slot[i].count;
		}
	}

	return count;
}

int CItem::FindItemSlot(ItemType type) const
{
	if (type == ItemType::E_NONE) { return -1; }

	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		if (m_Slot[i].type == type) { return i; }
	}

	return -1;
}

int CItem::MakeRandomPointValue() const
{
	return 10 + (rand() % 51);
}

ItemType CItem::RandomItemType()
{
	// E_NONE은 제외하고 E_EXCHANGE ~ E_SIZEUP 사이에서 랜덤.
	int minValue = (int)ItemType::E_EXCHANGE;
	int maxValue = (int)ItemType::E_TARGET_SIZEUP;

	int value = minValue + (rand() % (maxValue - minValue + 1));

	value = ClampInt(value, minValue, maxValue);

	return (ItemType)value;
}

wstring CItem::MakeItemName(ItemType type) const
{
	switch (type)
	{
		case ItemType::E_EXCHANGE:		return L"exchange";
		case ItemType::E_POINT_UP:		return L"point_up";
		case ItemType::E_POINT_DOWN:	return L"point_down";
		case ItemType::E_ITEMCHARGE:	return L"itemcharge";
		case ItemType::E_ITEM_STEAL:	return L"item_steal";
		case ItemType::E_TARGET_SLOW:	return L"target_slow";
		case ItemType::E_TARGET_STOP:	return L"target_stop";
		case ItemType::E_EXTRA_CHANCE:	return L"extra_chance";
		case ItemType::E_SNEAK_PEEK:	return L"sneak_peek";
		case ItemType::E_TARGET_SIZEUP:	return L"target_sizeup";
		case ItemType::E_SHIELD:		return L"shield";
		case ItemType::E_ITEM_COPY:		return L"item_copy";
		case ItemType::E_MAGNET:		return L"magnet";
		case ItemType::E_ITEM_LOCK:		return L"item_lock";
		case ItemType::E_MIRROR:		return L"mirror";
		case ItemType::E_CURSE:			return L"curse";
		case ItemType::E_ITEM_BLOCK:	return L"item_block";
		case ItemType::E_CONFUSE:		return L"confuse";
		case ItemType::E_SHAKE:			return L"shake";
		case ItemType::E_HEAVY:			return L"heavy";
		case ItemType::E_DRAIN:			return L"drain";
		case ItemType::E_CURSED_CUBE:	return L"cursed_cube";
		case ItemType::E_BLACK_HOLE:	return L"black_hole";
		case ItemType::E_ITEM_BOMB:		return L"item_bomb";
		case ItemType::E_PARASITE:		return L"parasite";
		case ItemType::E_LUCKY_SEVEN:	return L"lucky_seven";
		case ItemType::E_NOVA:			return L"nova";
		default:						return L"none";
	}
}