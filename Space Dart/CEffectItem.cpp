#include "CEffect.h"

static ItemType MakeRandomChargeDisplayItem()
{
	int minValue = (int)ItemType::E_EXCHANGE;
	int maxValue = (int)ItemType::E_TARGET_SIZEUP;

	int value = minValue + (rand() % (maxValue - minValue + 1));

	if (value < minValue) { value = minValue; }
	if (value > maxValue) { value = maxValue; }

	return (ItemType)value;
}

void CEffect::PlayExchange()
{
	// 점수 교환 아이템 이펙트 시작
	m_Type = EffectType::E_EXCHANGE;
	m_isPlaying = true;

	// 이펙트 시간 초기화
	m_EffectTimer = 0.0f;

	// 점수판 폭죽은 짧고 눈에 띄게 처리
	m_EffectDuration = 1.0f;
}

void CEffect::PlayPointUp(int finalPoint, bool targetIsPlayer)
{
	m_Type = EffectType::E_POINT_UP;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;

	// 슬롯머신 숫자가 빠르게 돌다가 멈추는 연출이 필요하므로
	// 기존보다 약간 길게 잡는다.
	m_EffectDuration = 2.0f;

	// POINT_UP:
	// positive = true
	// jackpot = false
	StartNumberSlotEffect(finalPoint, true, targetIsPlayer, false);
}

void CEffect::PlayPointDown(int finalPoint, bool targetIsPlayer, bool jackpot)
{
	m_Type = EffectType::E_POINT_DOWN;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;

	// jackpot이면 더 짜릿하게 보여주기 위해 조금 더 오래 출력한다.
	if (jackpot) { m_EffectDuration = 2.0f; }
	else { m_EffectDuration = 2.0f; }

	// POINT_DOWN:
	// positive = false
	// jackpot = ZERO ONE에서 정확히 0점이 되는 특수 상황
	StartNumberSlotEffect(finalPoint, false, targetIsPlayer, jackpot);
}

void CEffect::PlaySneakPeek(int directionIndex)
{
	m_Type = EffectType::E_SNEAK_PEEK;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 1.5f;

	m_DirectionIndex = ClampInt(directionIndex, 0, 3);
}

void CEffect::PlaySteal(ItemType item, int slotIndex, bool success)
{
	m_Type = EffectType::E_STEAL;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;
	m_EffectDuration = 1.6f;

	// 성공 시 훔친 아이템.
	// 실패 시 E_NONE이 들어와도 된다.
	m_StealItem = item;

	// 슬롯 플래시를 터트릴 위치.
	// 플레이어가 실제로 사용한 STEAL 슬롯 번호다.
	m_StealSlotIndex = slotIndex;

	// 중앙 텍스트에서 성공 / 실패를 구분하기 위한 값.
	m_StealSuccess = success;

	m_StealProgress = 0.0f;
}

void CEffect::PlayItemCharge(const bool slotActive[], const ItemType finalItems[], int slotCount)
{
	m_Type = EffectType::E_ITEMCHARGE;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;

	// 슬롯머신이 빠르게 돌다가 천천히 멈추고,
	// 마지막 결과를 잠깐 보여줘야 하므로 기존보다 조금 길게 둔다.
	m_EffectDuration = 1.65f;

	StartItemChargeSlotEffect(slotActive, finalItems, slotCount);
}

void CEffect::PlayExtraChance()
{
	// EXTRA_CHANCE 이펙트 시작.
	// 실제 사용 효과는 CPlayScene에서 m_noCountThrow = true로 처리하고,
	// 여기서는 화면에 "이번 투척은 찬스를 소모하지 않는다"는 느낌을 보여준다.
	m_Type = EffectType::E_EXTRA_CHANCE;
	m_isPlaying = true;

	m_EffectTimer = 0.0f;

	// 너무 짧으면 플레이어가 의미를 확인하기 어렵다.
	// 1.45초 정도면 "보호막이 켜졌다"는 느낌을 보기 충분하다.
	m_EffectDuration = 1.45f;
}

float CEffect::GetEffectRate() const
{
	// 이펙트가 없으면 0 반환
	if (m_EffectDuration <= 0.0f) { return 0.0f; }

	float rate = m_EffectTimer / m_EffectDuration;

	// 0 ~ 1 사이로 고정
	if (rate < 0.0f) { rate = 0.0f; }
	if (rate > 1.0f) { rate = 1.0f; }

	return rate;
}

void CEffect::StartNumberSlotEffect(int finalValue, bool positive, bool targetIsPlayer, bool jackpot)
{
	m_NumberSlot.Reset();

	// finalValue:
	// 슬롯머신이 마지막에 멈출 최종 숫자.
	// POINT_UP / POINT_DOWN은 10 ~ 60 사이 값을 사용한다.
	m_NumberSlot.finalValue = ClampInt(finalValue, 10, 60);

	// 최종 숫자를 10의 자리와 1의 자리로 나눈다.
	// 예:
	// 57이면 tensFinal = 5, onesFinal = 7
	// 10이면 tensFinal = 1, onesFinal = 0
	// 60이면 tensFinal = 6, onesFinal = 0
	m_NumberSlot.tensFinal = m_NumberSlot.finalValue / 10;
	m_NumberSlot.onesFinal = m_NumberSlot.finalValue % 10;

	// 처음 화면에 보일 숫자도 랜덤으로 시작한다.
	// 최종값과 연관성이 있으면 예측 가능한 느낌이 나므로,
	// 시작값도 그냥 무작위로 잡는다.
	m_NumberSlot.tensDisplay = 1 + (rand() % 6);
	m_NumberSlot.onesDisplay = rand() % 10;

	// 다음에 위에서 내려올 숫자도 따로 랜덤으로 준비한다.
	// 현재 숫자와 다음 숫자가 반드시 이어질 필요는 없다.
	// 예: 2 다음 6, 6 다음 1, 0 다음 9처럼 완전히 랜덤이어야 한다.
	m_NumberSlot.tensNext = 1 + (rand() % 6);
	m_NumberSlot.onesNext = rand() % 10;

	// 기존 GetCurrentPoint(), GetFinalPoint()와 호환되도록
	// displayValue / m_CurrentPoint / m_FinalPoint도 계속 유지한다.
	m_NumberSlot.displayValue = (m_NumberSlot.tensDisplay * 10) + m_NumberSlot.onesDisplay;

	m_CurrentPoint = m_NumberSlot.displayValue;
	m_FinalPoint = m_NumberSlot.finalValue;

	// 10의 자리 릴 타이머.
	// 이 값은 10의 자리 숫자가 한 칸 굴러가는 시간을 누적한다.
	m_NumberSlot.tensTimer = 0.0f;

	// 1의 자리 릴 타이머.
	// 10의 자리와 별도로 움직여야 하므로 따로 둔다.
	m_NumberSlot.onesTimer = 0.0f;

	// 10의 자리 릴 속도.
	// 1의 자리보다 조금 느리게 시작하면 두 릴이 서로 다르게 움직여 보인다.
	m_NumberSlot.tensInterval = 0.030f;

	// 1의 자리 릴 속도.
	// 1의 자리는 더 빠르게 돌게 해서 슬롯머신의 긴장감을 만든다.
	m_NumberSlot.onesInterval = 0.020f;

	// scrollRate:
	// 현재 숫자와 다음 숫자가 얼마나 이동했는지 나타내는 값.
	// 0.0이면 현재 숫자가 중앙.
	// 0.5이면 현재 숫자가 반쯤 내려가고 다음 숫자가 반쯤 들어온 상태.
	// 1.0이면 다음 숫자가 중앙에 도착한 상태.
	m_NumberSlot.tensScrollRate = 0.0f;
	m_NumberSlot.onesScrollRate = 0.0f;

	// 멈춤 여부.
	// 현실 슬롯머신처럼 10의 자리와 1의 자리가 서로 다른 타이밍에 멈추게 한다.
	m_NumberSlot.tensStopped = false;
	m_NumberSlot.onesStopped = false;

	// positive:
	// true면 POINT_UP 색상 계열.
	// false면 POINT_DOWN 색상 계열.
	m_NumberSlot.isPositive = positive;

	// targetIsPlayer:
	// true면 TO YOU.
	// false면 TO COMPUTER.
	m_NumberSlot.targetIsPlayer = targetIsPlayer;

	// jackpot:
	// ZERO ONE에서 POINT_DOWN 값이 내 남은 점수와 정확히 같을 때 true.
	m_NumberSlot.jackpot = jackpot;
}

void CEffect::UpdateNumberSlotEffect(float dt, float rate)
{
	// 숫자 릴 이론:
	// 기존 방식은 13 -> 52 -> 27처럼 숫자 전체가 순간적으로 바뀌었다.
	// 그러면 슬롯머신 느낌은 나지만 실제로 "굴러간다"는 느낌은 약하다.

	// 이번 방식은 숫자 전체를 하나로 보지 않고,
	// 10의 자리 릴과 1의 자리 릴을 따로 굴린다.

	// 예:
	// +57이라면
	// [5] [7]
	// 이렇게 두 개의 작은 슬롯이 따로 존재하는 것처럼 처리한다.

	// 10의 자리:
	// 1 ~ 6 중 랜덤.

	// 1의 자리:
	// 0 ~ 9 중 랜덤.

	// 중요한 점:
	// 숫자는 순차적으로 증가/감소하면 안 된다.
	// 20 다음 19, 60 다음 10처럼 나오는 것이 아니라,
	// 각 자리 숫자가 매번 완전히 랜덤하게 바뀌어야 한다.

	// 즉 10의 자리는 1, 5, 2, 6, 3처럼 랜덤.
	// 1의 자리는 7, 0, 9, 2, 8처럼 랜덤.

	// 최종적으로는 finalValue의 10의 자리와 1의 자리로 멈춘다.

	const float tensStopRate = 0.60f;
	const float onesStopRate = 0.65f;

	// 10의 자리 먼저 멈춤.
	// 현실 슬롯머신도 릴들이 동시에 멈추지 않고
	// 하나씩 멈추면서 긴장감을 만든다.
	if (rate >= tensStopRate && !m_NumberSlot.tensStopped)
	{
		m_NumberSlot.tensDisplay = m_NumberSlot.tensFinal;
		m_NumberSlot.tensNext = m_NumberSlot.tensFinal;
		m_NumberSlot.tensScrollRate = 0.0f;
		m_NumberSlot.tensStopped = true;
	}

	// 1의 자리는 조금 더 늦게 멈춤.
	// 마지막 자리까지 굴러가면 결과를 기다리는 느낌이 더 강해진다.
	if (rate >= onesStopRate && !m_NumberSlot.onesStopped)
	{
		m_NumberSlot.onesDisplay = m_NumberSlot.onesFinal;
		m_NumberSlot.onesNext = m_NumberSlot.onesFinal;
		m_NumberSlot.onesScrollRate = 0.0f;
		m_NumberSlot.onesStopped = true;
	}

	if (!m_NumberSlot.tensStopped)
	{
		// 10의 자리 진행률.
		// 0에서 tensStopRate까지를 0~1로 변환한다.
		float slowRate = rate / tensStopRate;
		slowRate = Clamp(slowRate, 0.0f, 1.0f);

		// ease:
		// 시간이 지날수록 더 느려지게 만드는 값.
		// 제곱을 사용하면 초반에는 빠르고 후반에는 확실히 느려진다.
		float ease = slowRate * slowRate;

		// 10의 자리 릴 속도.
		// 초반에는 빠르게, 멈추기 직전에는 느리게.
		m_NumberSlot.tensInterval = 0.028f + (ease * 0.16f);

		m_NumberSlot.tensTimer += dt;

		// scrollRate 계산.
		// 현재 숫자가 아래로 내려가고,
		// 다음 숫자가 위에서 내려오는 진행률이다.
		m_NumberSlot.tensScrollRate =
			m_NumberSlot.tensTimer / m_NumberSlot.tensInterval;

		m_NumberSlot.tensScrollRate =
			Clamp(m_NumberSlot.tensScrollRate, 0.0f, 1.0f);

		if (m_NumberSlot.tensTimer >= m_NumberSlot.tensInterval)
		{
			m_NumberSlot.tensTimer = 0.0f;
			m_NumberSlot.tensScrollRate = 0.0f;

			// 다음 숫자가 중앙에 도착했으므로 현재 숫자로 승격한다.
			m_NumberSlot.tensDisplay = m_NumberSlot.tensNext;

			// 다음에 내려올 숫자는 새로 랜덤 선택한다.
			// 이전 값과 연관 없이 1~6 중 아무 숫자나 나온다.
			m_NumberSlot.tensNext = 1 + (rand() % 6);
		}
	}

	if (!m_NumberSlot.onesStopped)
	{
		// 1의 자리 진행률.
		// 1의 자리는 더 늦게 멈추므로 onesStopRate를 기준으로 감속한다.
		float slowRate = rate / onesStopRate;
		slowRate = Clamp(slowRate, 0.0f, 1.0f);

		float ease = slowRate * slowRate;

		// 1의 자리는 더 빠르게 시작한다.
		// 그래서 마지막까지 좀 더 활발하게 돌아가는 느낌이 난다.
		m_NumberSlot.onesInterval = 0.018f + (ease * 0.20f);

		m_NumberSlot.onesTimer += dt;

		m_NumberSlot.onesScrollRate =
			m_NumberSlot.onesTimer / m_NumberSlot.onesInterval;

		m_NumberSlot.onesScrollRate =
			Clamp(m_NumberSlot.onesScrollRate, 0.0f, 1.0f);

		if (m_NumberSlot.onesTimer >= m_NumberSlot.onesInterval)
		{
			m_NumberSlot.onesTimer = 0.0f;
			m_NumberSlot.onesScrollRate = 0.0f;

			// 다음 숫자가 중앙에 도착했으므로 현재 숫자로 승격한다.
			m_NumberSlot.onesDisplay = m_NumberSlot.onesNext;

			// 1의 자리는 0~9 중 완전 랜덤.
			m_NumberSlot.onesNext = rand() % 10;
		}
	}

	// 기존 코드와 호환되도록 displayValue를 계속 갱신한다.
	m_NumberSlot.displayValue =
		(m_NumberSlot.tensDisplay * 10) + m_NumberSlot.onesDisplay;

	if (m_NumberSlot.tensStopped && m_NumberSlot.onesStopped)
	{
		m_NumberSlot.displayValue = m_NumberSlot.finalValue;
	}

	m_CurrentPoint = m_NumberSlot.displayValue;
}

void CEffect::StartItemChargeSlotEffect(const bool slotActive[], const ItemType finalItems[], int slotCount)
{
	m_ItemChargeSlot.Reset();

	if (slotCount < 0) { slotCount = 0; }
	if (slotCount > ITEM_EFFECT_SLOT_MAX) { slotCount = ITEM_EFFECT_SLOT_MAX; }

	m_ItemChargeSlot.slotCount = slotCount;

	for (int i = 0; i < slotCount; i++)
	{
		m_ItemChargeSlot.active[i] = slotActive[i];
		m_ItemChargeSlot.finalItems[i] = finalItems[i];

		if (slotActive[i])
		{
			// 처음 중앙에 보일 아이콘.
			m_ItemChargeSlot.displayItems[i] = MakeRandomChargeDisplayItem();

			// 다음에 위에서 내려올 아이콘.
			// 이 값이 있어야 아이콘이 순간 교체되지 않고
			// 위에서 아래로 흘러오는 것처럼 보인다.
			m_ItemChargeSlot.nextItems[i] = MakeRandomChargeDisplayItem();

			m_ItemChargeSlot.spinTimer[i] = 0.0f;
			m_ItemChargeSlot.scrollRate[i] = 0.0f;

			// 슬롯마다 속도를 살짝 다르게 해서
			// 모든 슬롯이 동시에 딱딱 끊기는 느낌을 줄인다.
			m_ItemChargeSlot.spinInterval[i] = 0.022f + ((float)(i % 3) * 0.005f);
		}

		else
		{
			m_ItemChargeSlot.displayItems[i] = ItemType::E_NONE;
			m_ItemChargeSlot.nextItems[i] = ItemType::E_NONE;
			m_ItemChargeSlot.scrollRate[i] = 0.0f;
		}
	}

	// 최종 결과에서 3개 이상 연속 같은 아이템이 있는지 검사한다.
	// 예: [POINT_UP][POINT_UP][POINT_UP]
	for (int i = 0; i < slotCount; i++)
	{
		if (!m_ItemChargeSlot.active[i]) { continue; }

		ItemType baseType = m_ItemChargeSlot.finalItems[i];
		if (baseType == ItemType::E_NONE) { continue; }

		int count = 1;

		for (int j = i + 1; j < slotCount; j++)
		{
			if (!m_ItemChargeSlot.active[j]) { break; }
			if (m_ItemChargeSlot.finalItems[j] != baseType) { break; }

			count++;
		}

		if (count >= 3)
		{
			m_ItemChargeSlot.jackpot = true;
			m_ItemChargeSlot.jackpotStartIndex = i;
			m_ItemChargeSlot.jackpotCount = count;
			break;
		}
	}
}

void CEffect::UpdateItemChargeSlotEffect(float dt, float rate)
{
	// 릴 데이터 이론:
	// 현실 슬롯머신은 아이콘 하나가 갑자기 A -> B로 바뀌는 것이 아니라,
	// 여러 아이콘이 붙어있는 긴 띠(reel)가 위/아래로 스크롤되는 구조다.

	// 여기서는 완전한 릴 배열을 만들지는 않고,
	// 현재 아이콘(displayItems)과 다음 아이콘(nextItems) 두 장만 사용해서
	// "가짜 릴 스크롤"을 만든다.

	// displayItems:
	// 현재 슬롯 중앙에 있는 아이콘.

	// nextItems:
	// 위쪽에서 내려오는 다음 아이콘.

	// scrollRate = 0.0 : 현재 아이콘이 슬롯 중앙에 있음
	// scrollRate = 0.5 : 현재 아이콘은 반쯤 내려가고, 다음 아이콘이 반쯤 들어옴
	// scrollRate = 1.0 : 다음 아이콘이 중앙에 도착함

	// scrollRate가 1.0이 되면:
	// displayItems = nextItems
	// nextItems = 새 랜덤 아이템
	// scrollRate = 0

	// 이렇게 하면 "순간 교체"가 아니라 "실제 위치 이동"이 생겨서
	// 뚝뚝 끊기는 느낌이 훨씬 줄어든다.

	if (rate >= 0.72f)
	{
		for (int i = 0; i < m_ItemChargeSlot.slotCount; i++)
		{
			if (!m_ItemChargeSlot.active[i]) { continue; }

			m_ItemChargeSlot.displayItems[i] = m_ItemChargeSlot.finalItems[i];
			m_ItemChargeSlot.nextItems[i] = ItemType::E_NONE;
			m_ItemChargeSlot.scrollRate[i] = 0.0f;

			m_CurrentItems[i] = m_ItemChargeSlot.finalItems[i];
		}

		return;
	}

	float slowRate = rate / 0.72f;
	slowRate = Clamp(slowRate, 0.0f, 1.0f);

	// ease:
	// 초반에는 빠르게, 후반에는 느리게 만들기 위한 감속값.
	// slowRate를 제곱하면 후반으로 갈수록 더 확실하게 느려진다.
	float ease = slowRate * slowRate;

	for (int i = 0; i < m_ItemChargeSlot.slotCount; i++)
	{
		if (!m_ItemChargeSlot.active[i]) { continue; }

		// spinInterval:
		// 한 칸이 스크롤되는 데 걸리는 시간.
		// 초반에는 짧아서 빠르게 내려가고,
		// 후반에는 길어져서 천천히 멈추는 느낌이 난다.
		float baseInterval = 0.022f + ((float)(i % 3) * 0.005f);
		m_ItemChargeSlot.spinInterval[i] = baseInterval + (ease * 0.20f);

		m_ItemChargeSlot.spinTimer[i] += dt;

		// 현재 구간에서 얼마나 스크롤됐는지 계산.
		m_ItemChargeSlot.scrollRate[i] = m_ItemChargeSlot.spinTimer[i] / m_ItemChargeSlot.spinInterval[i];

		m_ItemChargeSlot.scrollRate[i] = Clamp(m_ItemChargeSlot.scrollRate[i], 0.0f, 1.0f);

		if (m_ItemChargeSlot.spinTimer[i] >= m_ItemChargeSlot.spinInterval[i])
		{
			m_ItemChargeSlot.spinTimer[i] = 0.0f;
			m_ItemChargeSlot.scrollRate[i] = 0.0f;

			// 다음 아이콘이 중앙에 도착했으므로 현재 아이콘으로 승격.
			m_ItemChargeSlot.displayItems[i] = m_ItemChargeSlot.nextItems[i];

			// 다음 스크롤에 사용할 새 아이콘 준비.
			m_ItemChargeSlot.nextItems[i] = MakeRandomChargeDisplayItem();

			m_CurrentItems[i] = m_ItemChargeSlot.displayItems[i];
		}
	}
}

ItemType CEffect::GetItemChargeNextItem(int index) const
{
	if (index < 0 || index >= ITEM_EFFECT_SLOT_MAX) { return ItemType::E_NONE; }
	return m_ItemChargeSlot.nextItems[index];
}

float CEffect::GetItemChargeScrollRate(int index) const
{
	if (index < 0 || index >= ITEM_EFFECT_SLOT_MAX) { return 0.0f; }
	return m_ItemChargeSlot.scrollRate[index];
}