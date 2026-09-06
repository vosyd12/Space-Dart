#include "CPlayScene.h"
#include "GameSetting.h"
#include "KeyProc.h"

CPlayScene::CPlayScene() 
{
	// 부모 CScene의 m_nextScene을 사용한다.
	// 별도 m_NextScene을 쓰면 ResetNextScene()과 값이 엇갈릴 수 있다.
	m_nextScene = GameScene::E_NONE;
}

CPlayScene::~CPlayScene() {}

void CPlayScene::Init(HWND hwnd, CD3D* d3d)
{
	CScene::Init(hwnd, d3d);
	m_controlMode = ControlMode::E_KEYBOARD;

	m_isPause = false;
	m_isFly = false;
	m_noCountThrow = false;
	m_currentTurn = E_PLAYER;

	m_isGameOver = false;
	m_gameResult = GameResult::E_NONE;

	m_retryBtn.Init(270, 650, 150, 60, L"");
	m_exitBtn.Init(480, 650, 150, 60, L"");

	m_lastViewChangeTime = 0;
	m_selectedItem = 0;

	m_aimVelX = 0.0f;
	m_aimVelY = 0.0f;

	m_effect.Init(30.0f);	// 타임 설정

	// 과녁 초기 위치
	m_Target.Init( 0.0f, 0.0f, 80.0f );
	m_Collision.Init();

	// 다트 초기 위치
	m_Player.Init(E_PLAYER);
	m_Com.Init(E_COMPUTER, g_GameSetting.step);

	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		m_item_Slot[i].Init(50 + (i * 60), 900, 50, 50, L"");
	}

	m_settingButton.Init(835, 25, 45, 45, L"");


	int startChance = 8, startPoint = 301;

	// 난이도가 높아질수록 점수를 더 높고 잡게 위해서 찬스를 더 주었다.
	if (g_GameSetting.step == Step::E_EASY)			{ startChance = 6; }
	else if (g_GameSetting.step == Step::E_NOMAL)	{ startChance = 8; }
	else if (g_GameSetting.step == Step::E_HARD)	{ startChance = 10; }

	// ZERO ONE은 난이도별 시작 점수에서 시작한다.
	// 찬스는 던질수록 증가하는 카운트 방식이므로 0에서 시작한다.
	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		if (g_GameSetting.step == Step::E_EASY)			{ startPoint = 201; }
		else if (g_GameSetting.step == Step::E_NOMAL)	{ startPoint = 301; }
		else if (g_GameSetting.step == Step::E_HARD)	{ startPoint = 401; }

		m_Player.SetPoint(startPoint);
		m_Com.SetPoint(startPoint);

		m_Player.SetChance(0);
		m_Com.SetChance(0);
	}

	// COUNT UP은 기존 방식 유지
	else
	{
		m_Player.SetPoint(0);
		m_Com.SetPoint(0);

		m_Player.SetChance(startChance);
		m_Com.SetChance(startChance);
	}

	if (m_d3d)
	{
		m_d3d->GetCamera().Init();
		m_d3d->GetCamera().Update(m_Player.GetDart().GetLocation(), 0.0f);
		m_Player.GetItem().SetTextureManager(&m_d3d->GetTextureManager());
		m_Com.GetItem().SetTextureManager(&m_d3d->GetTextureManager());

		// 테스트용:
		// 플레이어 1번 아이템 슬롯에 EXCHANGE 아이템을 강제로 넣는다.
		// 1번 슬롯은 배열 인덱스 기준으로 0번이다.
		//m_Player.GetItem().SetSlot(0, ItemType::E_SNEAK_PEEK, 1);
		//m_Player.GetItem().SetSlot(1, ItemType::E_EXTRA_CHANCE, 1);
		//m_Player.GetItem().SetSlot(2, ItemType::E_TARGET_STOP, 1);

		// 시작 선택 슬롯도 1번 슬롯으로 맞춘다.
		m_selectedItem = 0;
	}

	m_Target.Stop();
}

void CPlayScene::Update(float dt)
{
	if (m_isGameOver)
	{
		UpdateResultWindow();
		return;
	}

	m_Player.GetItem().UpdatePreloadItemTextures();

	// 설정 버튼은 일시정지 상태와 상관없이 반응하게 한다.
	bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	m_settingButton.Update(g_mousePos, mouseDown);

	if (m_settingButton.IsClicked())
	{
		m_nextScene = GameScene::E_SETTING;
		return;
	}

	// 일시정지 토글
	if (IsActionTriggered(InputAction::E_PAUSE)) { m_isPause = !m_isPause; }

	if (m_isPause) { return; }

	// 화면 이펙트는 다트 비행 여부와 상관없이 업데이트
	m_effect.UpdateEffect(dt);

	// 조준 중일 때만 타이머 감소
	UpdateTurnTimer(dt);

	// 플레이어 입력 처리
	HandlePlayerInput();

	HandleComputerTurn(dt);

	// 발사 처리
	HandleDartFire();

	// 다트 비행 상태에 맞춰 과녁 이동 상태를 먼저 동기화한다.
	// 이 처리를 과녁 업데이트보다 먼저 해야 현재 프레임의 이동 여부가 정확히 반영된다.
	UpdateTargetMoveState();

	// 과녁을 먼저 업데이트한다.
	// 그래야 충돌 판정과 렌더링이 같은 프레임의 과녁 좌표를 사용한다.
	m_Target.Update(dt);

	// 다트 이동, 과녁 충돌, 턴 종료 처리
	HandleDartUpdateAndCollision();

	// 카메라 업데이트
	if (m_d3d)
	{
		DartState dart = GetCurrentDart().GetState();
		m_d3d->GetCamera().Update(dart.pos, dt);
	}
}

void CPlayScene::UpdateTurnTimer(float dt)
{
	// 다트가 날아가는 중이면 타이머 정지
	// 다트가 안 날아가는 조준 상태이면 플레이어/컴퓨터 모두 타이머 감소
	if (m_isFly) { return; }

	m_effect.UpdateTimer(dt);

	if (m_effect.IsTimeOver()) { OnTurnTimeOver(); }
}

void CPlayScene::UpdateTargetMoveState()
{
	CDart& currentDart = GetCurrentDart();
	// 다트가 날아가는 중이면 과녁 이동
	// 다트가 안 날아가는 조준 상태이면 과녁 정지
	if (m_isFly && currentDart.GetFired() && currentDart.GetMoving())	{ m_Target.Resume(); }
	else																{ m_Target.Stop(); }
}

int CPlayScene::GetCurrentItemSlotCount() const
{
	if (g_GameSetting.step == Step::E_EASY) { return 5; }
	if (g_GameSetting.step == Step::E_NOMAL) { return 4; }

	return 3;
}

const wchar_t* CPlayScene::GetItemDisplayName(ItemType type) const
{
	switch (type)
	{
	case ItemType::E_EXCHANGE:		return L"EXCHANGE";
	case ItemType::E_POINT_UP:		return L"POINT UP";
	case ItemType::E_POINT_DOWN:	return L"POINT DOWN";
	case ItemType::E_ITEMCHARGE:	return L"ITEM CHARGE";
	case ItemType::E_ITEM_STEAL:	return L"ITEM STEAL";
	case ItemType::E_TARGET_SLOW:	return L"TARGET SLOW";
	case ItemType::E_TARGET_STOP:	return L"TARGET STOP";
	case ItemType::E_EXTRA_CHANCE:	return L"EXTRA CHANCE";
	case ItemType::E_SNEAK_PEEK:	return L"SNEAK PEEK";
	case ItemType::E_TARGET_SIZEUP:	return L"TARGET SIZE UP";
	default:						return L"UNKNOWN ITEM";
	}
}

void CPlayScene::RenderCenterTextureItemEffect(LPDIRECT3DDEVICE9 device, EffectType type)
{
	if (!m_d3d) { return; }

	const wchar_t* textureName = L"";

	if (type == EffectType::E_TARGET_SLOW) { textureName = L"target_slow_effect"; }
	else if (type == EffectType::E_TARGET_STOP) { textureName = L"target_stop_effect"; }
	else { return; }

	LPDIRECT3DTEXTURE9 texture = m_d3d->GetTextureManager().LoadTexture(textureName);

	m_effect.RenderCenterTextureEffect(device, texture);
}

void CPlayScene::ApplyUsedItemEffect(const ItemEffectState& result, int usedSlotIndex)
{
	if (!result.active) { return; }

	switch (result.sourceItem)
	{
		case ItemType::E_EXCHANGE:
		{
			int playerPoint = m_Player.GetPoint();
			int comPoint = m_Com.GetPoint();

			m_Player.SetPoint(comPoint);
			m_Com.SetPoint(playerPoint);

			m_effect.PlayExchange();
			break;
		}

		case ItemType::E_POINT_UP:
		{
			bool targetIsPlayer = false;

			if (g_GameSetting.type == GameType::E_ZERO_ONE) { targetIsPlayer = (m_currentTurn != E_PLAYER); }
			else { targetIsPlayer = (m_currentTurn == E_PLAYER); }

			ApplyPointItemEffect(ItemType::E_POINT_UP, result.pointValue);
			m_effect.PlayPointUp(result.pointValue, targetIsPlayer);
			break;
		}

		case ItemType::E_POINT_DOWN:
		{
			bool targetIsPlayer = false;

			if (g_GameSetting.type == GameType::E_ZERO_ONE) { targetIsPlayer = (m_currentTurn == E_PLAYER); }
			else { targetIsPlayer = (m_currentTurn != E_PLAYER); }

			bool jackpot = false;

			if (g_GameSetting.type == GameType::E_ZERO_ONE)
			{
				jackpot = (result.pointValue == GetCurrentStatus().GetPoint());
			}

			ApplyPointItemEffect(ItemType::E_POINT_DOWN, result.pointValue);
			m_effect.PlayPointDown(result.pointValue, targetIsPlayer, jackpot);
			break;
		}

		case ItemType::E_ITEMCHARGE:
		{
			m_effect.PlayItemCharge(
				result.itemChargeSlotActive,
				result.itemChargeFinalItems,
				result.itemChargeSlotCount);

			break;
		}

		case ItemType::E_ITEM_STEAL:
		{
			ItemType stolenItem = ItemType::E_NONE;
			bool stealSuccess = false;

			ApplyStealItemEffect(
				usedSlotIndex,
				stolenItem,
				stealSuccess);

			m_effect.PlaySteal(
				stolenItem,
				usedSlotIndex,
				stealSuccess);

			break;
		}

		case ItemType::E_TARGET_SLOW:
		{
			m_Target.ApplySlow(0.0f);

			// 중앙에 target_slow_effect 텍스처가 나타났다가 사라지는 이펙트.
			m_effect.PlayCenterTextureEffect(EffectType::E_TARGET_SLOW);
			break;
		}

		case ItemType::E_TARGET_STOP:
		{
			m_Target.ApplyStop(0.0f);

			// 중앙에 target_stop_effect 텍스처가 나타났다가 사라지는 이펙트.
			m_effect.PlayCenterTextureEffect(EffectType::E_TARGET_STOP);
			break;
		}

		case ItemType::E_EXTRA_CHANCE:
		{
			// 사용 효과:
			// 이번 투척은 찬스를 소모하지 않도록 표시한다.
			// 실제 찬스 차감 여부는 턴 종료 처리 쪽에서 m_noCountThrow를 보고 판단한다.
			m_noCountThrow = true;

			// 사용 이펙트:
			// 화면 중앙에 보호막 링 + EXTRA CHANCE / NO COUNT 텍스트 출력.
			m_effect.PlayExtraChance();

			break;
		}

		case ItemType::E_SNEAK_PEEK:
		{
			m_Target.ApplySneakPeek(0.0f);

			// 실제 과녁이 사용할 방향 번호를 그대로 넘긴다.
			// 이 값을 기반으로 화살표가 직접 그려진다.
			m_effect.PlaySneakPeek(m_Target.GetDirectionIndex());
			break;
		}

		case ItemType::E_TARGET_SIZEUP:
		{
			m_Target.ApplySizeUp(0.0f, 1.5f);
			break;
		}

		default:
		{
			break;
		}
	}
}

void CPlayScene::HandlePlayerInput()
{
	// 컴퓨터 턴에는 플레이어 입력 무시
	if (m_currentTurn != E_PLAYER) { return; }

	CDart& currentDart = GetCurrentDart();

	// 조작 방식 변경
	if (IsActionTriggered(InputAction::E_CONTROL_CHANGE))
	{
		if (m_controlMode == ControlMode::E_KEYBOARD)	{ m_controlMode = ControlMode::E_MOUSE; }
		else											{ m_controlMode = ControlMode::E_KEYBOARD; }
	}

	// 키보드 조작
	if (m_controlMode == ControlMode::E_KEYBOARD)
	{
		float inputX = 0.0f;
		float inputY = 0.0f;

		// else if를 쓰지 않고 각각 검사한다.
		// 이렇게 해야 W+D 같은 대각선 입력이 가능하다.
		if (IsActionPressed(InputAction::E_MOVE_UP))    { inputY += 1.0f; }
		if (IsActionPressed(InputAction::E_MOVE_DOWN))  { inputY -= 1.0f; }
		if (IsActionPressed(InputAction::E_MOVE_LEFT))  { inputX -= 1.0f; }
		if (IsActionPressed(InputAction::E_MOVE_RIGHT)) { inputX += 1.0f; }

		// 키 반전은 WSAD 전체 방향에 적용한다.
		// W/S뿐 아니라 A/D도 반대로 움직이게 한다.
		if (g_GameSetting.invertMouse)
		{
			inputX *= -1.0f;
			inputY *= -1.0f;
		}

		// 대각선 입력 시 속도가 더 빨라지는 문제 방지
		float len = sqrtf((inputX * inputX) + (inputY * inputY));

		if (len > 0.0001f)
		{
			inputX /= len;
			inputY /= len;
		}

		// 설정창의 감도 슬라이더를 키보드 조준 감도에 적용한다.
		// 0.5일 때 기존 속도와 비슷하게 맞추고,
		// 0.0이면 느리게, 1.0이면 빠르게 움직이게 한다.
		float keySensitivity = 0.65f + (g_GameSetting.mouseSensitivity * 1.25f);

		// 입력이 있을 때 조준 속도가 서서히 증가한다.
		float accel = 0.42f * keySensitivity;

		// 조준 속도의 최대값
		float maxSpeed = 2.35f * keySensitivity;

		// 감속값
		// 키를 떼면 바로 멈추지 않고 부드럽게 줄어든다.
		float damping = 0.78f;

		m_aimVelX += inputX * accel;
		m_aimVelY += inputY * accel;

		// 최대 속도 제한
		if (m_aimVelX > maxSpeed) { m_aimVelX = maxSpeed; }
		if (m_aimVelX < -maxSpeed) { m_aimVelX = -maxSpeed; }

		if (m_aimVelY > maxSpeed) { m_aimVelY = maxSpeed; }
		if (m_aimVelY < -maxSpeed) { m_aimVelY = -maxSpeed; }

		// 입력이 없어도 매 프레임 감속
		m_aimVelX *= damping;
		m_aimVelY *= damping;

		// 아주 작은 값은 0으로 처리해서 미세 떨림 방지
		if (fabsf(m_aimVelX) < 0.001f) { m_aimVelX = 0.0f; }
		if (fabsf(m_aimVelY) < 0.001f) { m_aimVelY = 0.0f; }

		// 최종적으로 누적된 조준 속도를 다트 방향 변경에 반영
		currentDart.Move(m_aimVelX, m_aimVelY);
	}

	// 마우스 조작
	else if (m_controlMode == ControlMode::E_MOUSE)
	{
		float sensitivity = 0.02f + (g_GameSetting.mouseSensitivity * 0.12f);

		float moveX = g_mouseDeltaX * sensitivity;
		float moveY = g_mouseDeltaY * sensitivity;

		if (g_GameSetting.invertMouse)  { currentDart.Move(moveX, moveY); }
		else							{ currentDart.Move(moveX, -moveY); }
	}

	int slotCount = GetCurrentItemSlotCount();

	if (slotCount > ITEM_SLOT_MAX) { slotCount = ITEM_SLOT_MAX; }

	// 아이템 슬롯 선택
	if (IsActionTriggered(InputAction::E_ITEM0))		{ m_selectedItem = 0; }
	else if (IsActionTriggered(InputAction::E_ITEM1))	{ m_selectedItem = 1; }
	else if (IsActionTriggered(InputAction::E_ITEM2))	{ m_selectedItem = 2; }
	else if (IsActionTriggered(InputAction::E_ITEM3))	{ m_selectedItem = 3; }
	else if (IsActionTriggered(InputAction::E_ITEM4))	{ m_selectedItem = 4; }

	if (m_selectedItem >= slotCount)	{ m_selectedItem = slotCount - 1; }
	if (m_selectedItem < 0)				{ m_selectedItem = 0; }

	// 아이템 사용
	if (IsActionTriggered(InputAction::E_USEITEM))
	{
		if (m_isFly) { return; }

		ItemEffectState result = GetCurrentItem().UseSlot(m_selectedItem);

		// 플레이어 / 컴퓨터 공통 아이템 처리 함수 사용.
		ApplyUsedItemEffect(result, m_selectedItem);
	}

	// 시점 변환
	if (IsActionPressed(InputAction::E_VIEWPOINTCONVERSION))
	{
		DWORD now = GetTickCount();

		if (now - m_lastViewChangeTime > 250)
		{
			if (m_d3d) { m_d3d->GetCamera().ToggleViewMode(); }
			m_lastViewChangeTime = now;
		}
	}
	
	// 우클릭 드래그 카메라 회전
	if (IsActionPressed(InputAction::E_MOUSE_RIGHT))
	{
		if (m_d3d)
		{
			m_d3d->GetCamera().Rotate(
				g_mouseDeltaX * 0.005f,
				g_mouseDeltaY * 0.005f);
		}
	}
}

void CPlayScene::HandleComputerTurn(float dt)
{
	if (m_currentTurn != E_COMPUTER) { return; }

	if (m_isFly)
	{
		if (m_d3d)
		{
			CRenderer3D& renderer = m_d3d->GetRenderer3D();

			m_Com.UpdateRealtimeAI(
				dt,
				m_Target.GetLocation(),
				m_Target.GetDirection(),
				m_Target.GetSpeed(),
				m_Target.GetRadius(),
				m_Target.IsMoving(),
				renderer.GetCubeArray(),
				renderer.GetCubeCount());
		}

		return;
	}

	if (!m_Com.IsThinking() && !m_Com.IsReadyThrow())
	{
		m_Com.StartThink(
			m_Com.GetPoint(),
			m_Com.GetChance(),
			m_Player.GetPoint(),
			m_Player.GetChance());
	}

	m_Com.UpdateThink(dt);

	UseComputerItemByAI();

	// COUNT UP은 찬스가 있어야 던진다.
	// ZERO ONE은 찬스가 투척 횟수 카운트라 0이어도 던진다.
	bool canThrow = m_Com.HasChance();

	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		canThrow = true;
	}

	if (m_Com.IsReadyThrow() && canThrow)
	{
		if (m_noCountThrow)
		{
			m_Com.ThrowByAI(false);
			m_noCountThrow = false;
		}

		else
		{
			m_Com.ThrowByAI(true);
		}

		m_isFly = true;

		if (m_d3d)
		{
			m_d3d->GetSpaceBackground().StarWarp(0.80f);
		}
	}
}

void CPlayScene::HandleDartFire()
{
	// 플레이어 턴이 아니면 입력 발사 불가
	if (m_currentTurn != E_PLAYER) { return; }
	// 이미 날아가는 중이면 중복 발사 방지
	if (m_isFly) { return; }
	// 마우스 왼클릭이 이번 프레임에 눌렸는지 확인
	if (!IsActionTriggered(InputAction::E_FLY)) { return; }

	CStatus& currentStatus = GetCurrentStatus();

	// COUNT UP은 기존처럼 남은 찬스가 있어야 던질 수 있다.
	// ZERO ONE은 찬스가 사용 횟수 카운트라서 0이어도 던질 수 있다.
	if (g_GameSetting.type != GameType::E_ZERO_ONE)
	{
		if (!currentStatus.HasChance()) { return; }
	}

	CDart& currentDart = GetCurrentDart();

	// 다트 발사
	currentDart.Fly(m_Target.GetLocation());

	// 비행 상태 시작
	m_isFly = true;

	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		// ZERO ONE은 던질수록 찬스 카운트가 증가한다.
		// EXTRA CHANCE 사용 중이면 이번 투척은 카운트하지 않는다.
		if(m_noCountThrow)	{ m_noCountThrow = false; }
		else				{ currentStatus.AddChance(1); }

	}

	else
	{
		// 노카운트 상태가 아니면 찬스를 줄인다.
		// 노카운트 상태라면 이번 투척은 무료로 처리하고 플래그를 끈다.
		if (m_noCountThrow) { m_noCountThrow = false; }
		else				{ currentStatus.DecreaseChance(); }
	}

	// 과녁 이동 즉시 시작
	m_Target.Resume();

	// 배경 워프 효과
	if (m_d3d) { m_d3d->GetSpaceBackground().StarWarp(0.85f); }
}

void CPlayScene::HandleDartUpdateAndCollision()
{
	CDart& currentDart = GetCurrentDart();
	CStatus& currentStatus = GetCurrentStatus();
	CItem& currentItem = GetCurrentItem();

	// 다트 이동
	currentDart.Update();

	// 다트가 발사되어 이동 중일 때만 판정
	if (currentDart.GetFired() && currentDart.GetMoving())
	{
		Vec3 dartPos = currentDart.GetLocation();
		Vec3 dartDir = currentDart.GetDirection();

		Vec3 dartTipPos =
		{
			dartPos.x + (dartDir.x * 0.6f),
			dartPos.y + (dartDir.y * 0.6f),
			dartPos.z + (dartDir.z * -0.2f)
		};

		Vec3 targetPos = m_Target.GetLocation();

		// 1. 큐브 충돌 검사
		if (m_d3d)
		{
			CRenderer3D& renderer = m_d3d->GetRenderer3D();

			for (int i = 0; i < renderer.GetCubeCount(); i++)
			{
				const Instance& cube = renderer.GetCube(i);

				// 큐브 기본 크기 0.15에 scale이 적용됨
				// 너무 빡빡하면 충돌이 어려우므로 약간 넉넉하게 0.35 사용
				float cubeRadius = 0.35f * cube.scale.x;

				CubeHitResult cubeHit = m_Collision.CheckCubeHit(
					dartPos,
					cube.pos,
					i,
					cubeRadius);

				if (cubeHit.hit)
				{
					// 현재 턴의 아이템 슬롯에 랜덤 아이템 추가
					bool added = currentItem.AddRandomItem();

					// 아이템 획득 성공 시 이펙트 출력
					if (added) { m_effect.PlayCube(); }

					// 맞은 큐브는 새 위치로 이동
					// 같은 큐브가 같은 자리에서 계속 맞는 문제 방지
					renderer.RandomizeCube(i);

					return;
				}
			}
		}

		// 다트가 과녁 z 위치까지 도달했는지 확인
		bool reachedTargetZ = dartTipPos.z >= targetPos.z;

		// 점수 계산용 좌표
		// 기존에는 Update 이후의 dartPos를 그대로 사용해서,
		// 다트가 한 프레임에 과녁 평면을 살짝 지나간 경우
		// 실제 눈으로 보이는 위치보다 바깥쪽 좌표로 점수가 계산될 수 있었다.
		Vec3 scorePos = dartTipPos;

		// 과녁 판정 두께 안에 들어왔을 때만
		// 현재 다트 방향을 이용해서 정확히 과녁 z평면 위의 좌표로 보정한다.
		if (reachedTargetZ)
		{
			if (fabsf(dartDir.z) > 0.0001f)
			{
				float moveBack = (dartTipPos.z - targetPos.z) / dartDir.z;

				scorePos.x = dartTipPos.x - (dartDir.x * moveBack);
				scorePos.y = dartTipPos.y - (dartDir.y * moveBack);
				scorePos.z = targetPos.z;
			}
		}

		TargetHitResult hit = m_Collision.CheckTargetHitAndScore(
			scorePos,
			targetPos,
			m_Target.GetRadius(),
			currentStatus.GetPoint(),
			g_GameSetting.type == GameType::E_ZERO_ONE);

		if (hit.hit)
		{
			currentDart.Stop();
			currentStatus.SetPoint(hit.remainPoint);

			if (hit.bull) { m_effect.PlayBull(); }
			else { m_effect.PlayHit(hit.score); }
		}

		else if (reachedTargetZ)
		{
			currentDart.Stop();
			m_effect.PlayMiss();
		}
	}

	// 다트가 발사된 뒤 멈췄다면 이펙트가 끝난 후 턴 종료
	if (currentDart.GetFired() && !currentDart.GetMoving())
	{
		if (m_effect.IsPlaying()) { return; }
		EndCurrentTurn();
	}
}

void CPlayScene::EndCurrentTurn()
{
	// 턴이 끝난 직후 승패 확인
	CheckGameResult();

	// 게임이 끝났다면 턴 교체를 하지 않는다.
	if (m_isGameOver)
	{
		m_isFly = false;
		m_noCountThrow = false;
		m_Target.Stop();

		if (m_d3d) { m_d3d->GetSpaceBackground().StopWarp(); }
		return;
	}
	// 현재 턴 정리
	m_isFly = false;

	m_noCountThrow = false;

	// 턴 한정 아이템 효과 제거
	m_Target.ClearItemEffects();

	// 타이머 리셋
	m_effect.Reset();

	// 워프 효과 종료
	if (m_d3d) { m_d3d->GetSpaceBackground().StopWarp(); }

	// 턴 교체 및 다음 턴 다트 초기화
	if (m_currentTurn == E_PLAYER)
	{
		m_currentTurn = E_COMPUTER;
		m_Com.ResetDart();
		m_Com.EndThink();
	}

	else
	{
		m_currentTurn = E_PLAYER;
		m_Player.ResetDart();
	}

	// 플레이어의 시점이 어떠하든 컴퓨터의 시점은 숄더뷰로 고정한다.
	m_d3d->GetCamera().SetViewMode(CameraViewMode::E_SHOULDER);

	// 다음 턴은 조준 상태이므로 과녁 정지
	m_Target.PrepareNextTurn();
}

// 컴퓨터 아이템 사용
bool CPlayScene::UseComputerItemByAI()
{
	// 컴퓨터 턴이 아니면 사용하지 않음
	if (m_currentTurn != E_COMPUTER) { return false; }

	// 다트가 날아가는 중에는 아이템 사용 불가
	// 컴퓨터도 플레이어와 동일하게 발사 전 조준 상태에서만 아이템을 사용한다.
	if (m_isFly) { return false; }

	const AIAimData& aim = m_Com.GetAimData();

	// AI가 아이템 사용을 원하지 않으면 종료
	if (!aim.useItem) { return false; }

	// 사용할 슬롯이 유효하지 않으면 종료
	if (aim.useItemSlot < 0 || aim.useItemSlot >= ITEM_SLOT_MAX) { return false; }

	ItemEffectState result = m_Com.UseItemSlot(aim.useItemSlot);

	if (!result.active) { return false; }

	ApplyUsedItemEffect(result, aim.useItemSlot);

	return true;
}

int CPlayScene::GetStealFailRate() const
{
	// 난이도별 STEAL 실패 확률.
	// 플레이어가 선택 슬롯 위 말풍선으로 미리 볼 수 있게 할 값이다.
	if (g_GameSetting.step == Step::E_EASY) { return 5; }
	if (g_GameSetting.step == Step::E_NOMAL) { return 10; }

	return 15;
}

bool CPlayScene::RollStealSuccess() const
{
	int failRate = GetStealFailRate();

	int roll = rand() % 100;

	// roll이 실패 확률보다 작으면 실패.
	// 예: failRate = 10이면 0~9가 실패, 10~99가 성공.
	return roll >= failRate;
}

int CPlayScene::FindRandomStealableSlot(CItem& item) const
{
	int candidate[ITEM_SLOT_MAX];
	int count = 0;

	for (int i = 0; i < ITEM_SLOT_MAX; i++)
	{
		if (!item.IsValidSlot(i)) { continue; }

		ItemType type = item.GetSlotType(i);

		if (type == ItemType::E_NONE) { continue; }

		candidate[count] = i;
		count++;
	}

	if (count <= 0) { return -1; }

	int index = rand() % count;

	return candidate[index];
}

bool CPlayScene::ApplyStealItemEffect(int useSlotIndex, ItemType& stolenItem, bool& success)
{
	stolenItem = ItemType::E_NONE;
	success = false;

	// 현재 턴 기준으로 훔치는 쪽 / 당하는 쪽을 결정한다.
	CItem& myItem = GetCurrentItem();
	CItem& enemyItem = (m_currentTurn == E_PLAYER) ? m_Com.GetItem() : m_Player.GetItem();

	// 실패 확률 판정.
	// 실패하면 STEAL 아이템은 이미 UseSlot()에서 소모된 상태이므로
	// 내 슬롯은 비어 있고, 상대 아이템은 건드리지 않는다.
	if (!RollStealSuccess())
	{
		success = false;
		return false;
	}

	// 상대의 비어있지 않은 슬롯 중 하나를 랜덤 선택한다.
	int enemySlotIndex = FindRandomStealableSlot(enemyItem);

	if (enemySlotIndex < 0)
	{
		// 훔칠 아이템이 없으면 실패 처리.
		success = false;
		return false;
	}

	stolenItem = enemyItem.GetSlotType(enemySlotIndex);

	if (stolenItem == ItemType::E_NONE)
	{
		success = false;
		return false;
	}

	// 상대 슬롯은 비운다.
	enemyItem.ClearSlot(enemySlotIndex);

	// 중요:
	// STEAL 아이템을 사용한 바로 그 슬롯에 훔친 아이템을 넣는다.
	// UseSlot()에서 STEAL은 이미 소모되어 해당 슬롯이 비어있으므로,
	// 이 자리에 SetSlot()으로 다시 채우는 방식이 가장 자연스럽다.
	myItem.SetSlot(useSlotIndex, stolenItem, 1);

	success = true;
	return true;
}

void CPlayScene::OnTurnTimeOver()
{
	CStatus& status = GetCurrentStatus();

	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		// ZERO ONE은 찬스가 남은 기회가 아니라 투척 횟수 카운트다.
		// 시간 초과도 턴을 사용한 것으로 보고 찬스를 1 증가시킨다.
		status.AddChance(1);
	}
	else
	{
		// COUNT UP은 기존 방식대로 시간 초과 시 남은 찬스를 1 줄인다.
		if (status.HasChance()) { status.DecreaseChance(); }
	}

	// 시간 초과도 현재 턴 종료로 처리
	EndCurrentTurn();
}

void CPlayScene::Render(HDC hdc)
{
	if (!m_d3d) return;

	Vec3 targetPos = m_Target.GetLocation();
	DartState dart = GetCurrentDart().GetState();

	m_d3d->RenderGameObjects(targetPos, dart, m_currentTurn, m_Target.GetRenderRadius());

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();
	ID3DXFont* font = m_d3d->GetUIFont();

	if (device && font) 
	{ 
		DrawPlayHUD(device, font); 

		if (m_isGameOver)
		{
			DrawResultWindow(device, font);
		}
	}

	// 일시 정지중이라는 텍스트 출력
	if (m_isPause)
	{
		m_d3d->DrawGameStateMessage(
			L"PAUSED",
			D3DCOLOR_XRGB(255, 255, 255),
			D3DCOLOR_XRGB(100, 180, 255),
			360,
			true);
	}
}

CDart& CPlayScene::GetCurrentDart()
{
	if (m_currentTurn == E_PLAYER) { return m_Player.GetDart(); }
	return m_Com.GetDart();
}

CItem& CPlayScene::GetCurrentItem()
{
	if (m_currentTurn == E_PLAYER) { return m_Player.GetItem(); }
	return m_Com.GetItem();
}

CStatus& CPlayScene::GetCurrentStatus()
{
	if (m_currentTurn == E_PLAYER) { return m_Player; }
	return m_Com;
}

CStatus& CPlayScene::GetOpponentStatus()
{
	if (m_currentTurn == E_PLAYER) { return m_Com; }
	return m_Player;
}

void CPlayScene::ApplyPointItemEffect(ItemType type, int pointValue)
{
	CStatus& currentStatus = GetCurrentStatus();
	CStatus& opponentStatus = GetOpponentStatus();

	if (pointValue < 10) { pointValue = 10; }
	if (pointValue > 60) { pointValue = 60; }

	if (type == ItemType::E_POINT_UP)
	{
		if (g_GameSetting.type == GameType::E_ZERO_ONE)
		{
			// ZERO ONE:
			// 남은 점수가 늘어나는 것은 불리하므로 상대에게 적용.
			opponentStatus.AddPoint(pointValue);
		}

		else
		{
			// COUNT UP:
			// 점수가 늘어나는 것은 유리하므로 자신에게 적용.
			currentStatus.AddPoint(pointValue);
		}
	}

	else if (type == ItemType::E_POINT_DOWN)
	{
		if (g_GameSetting.type == GameType::E_ZERO_ONE)
		{
			// ZERO ONE:
			// 남은 점수가 줄어드는 것은 유리하므로 자신에게 적용.
			//
			// 단, 남은 점수보다 큰 값이면 BUST처럼 실패 처리.
			// 예: 12점 남았는데 13이 나오면 12 유지.
			//
			// 정확히 같은 값이면 0점까지 허용.
			// 예: 12점 남았는데 12가 나오면 0점.
			if (pointValue <= currentStatus.GetPoint())
			{
				currentStatus.SubPoint(pointValue);
			}
		}

		else
		{
			// COUNT UP:
			// 점수가 줄어드는 것은 불리하므로 상대에게 적용.
			opponentStatus.SubPoint(pointValue);
		}
	}
}

void CPlayScene::CheckGameResult()
{
	if (m_isGameOver) { return; }

	// 제로 원
	if (g_GameSetting.type == GameType::E_ZERO_ONE)
	{
		if (m_Player.GetPoint() == 0)
		{
			m_isGameOver = true;
			m_gameResult = GameResult::E_WIN;
			return;
		}

		else  if (m_Com.GetPoint() == 0)
		{
			m_isGameOver = true;
			m_gameResult = GameResult::E_LOSE;
			return;
		}

		else if (m_Player.GetChance() <= 0 && m_Com.GetChance() <= 0)
		{
			m_isGameOver = true;
			m_gameResult = GameResult::E_TIE;
			return;
		}

		return;
	}

	// 카운터 업
	if (!m_Player.HasChance() && !m_Com.HasChance())
	{
		m_isGameOver = true;

		int playerPoint = m_Player.GetPoint();
		int comPoint = m_Com.GetPoint();

		if (playerPoint > comPoint)			{ m_gameResult = GameResult::E_WIN; }
		else if (playerPoint < comPoint)	{ m_gameResult = GameResult::E_LOSE; }
		else								{ m_gameResult = GameResult::E_TIE; }
	}
}

void CPlayScene::UpdateResultWindow()
{
	// 결과창 버튼 텍스트 갱신
	// 언어 설정이 바뀌어도 현재 설정에 맞게 출력되도록
	// 결과창 업데이트 시점에서 텍스트를 넣는다.

	m_retryBtn.SetText(g_GameSetting.isEnglish ? L"RETRY" : L"다시하기");
	m_exitBtn.SetText(g_GameSetting.isEnglish ? L"EXIT" : L"나가기");

	POINT virtualMouse = CButton::ToVirtualPoint(g_mousePos);

	bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	m_retryBtn.Update(virtualMouse, mouseDown);
	m_exitBtn.Update(virtualMouse, mouseDown);

	if (m_retryBtn.IsClicked())
	{
		m_nextScene = GameScene::E_TITLE;
		return;
	}

	else if (m_exitBtn.IsClicked()) { PostQuitMessage(0); }
}

const wchar_t* CPlayScene::GetResultText() const
{
	switch (m_gameResult)
	{
		case GameResult::E_WIN:  return g_GameSetting.isEnglish ? L"WIN" : L"승리";
		case GameResult::E_LOSE: return g_GameSetting.isEnglish ? L"LOSE" : L"패배";
		case GameResult::E_TIE:  return g_GameSetting.isEnglish ? L"DRAW" : L"무승부";
		default:				 return L"";
	}
}

void CPlayScene::DrawResultWindow(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	if (!device || !font) { return; }

	// 화면 전체를 살짝 어둡게 덮는다.
	RECT darkBg = { 0,0,900,1000 };
	RECT scaledDarkBg = CButton::ScaleRect(darkBg);

	CButton::DrawRect(device, scaledDarkBg, D3DCOLOR_ARGB(155, 0, 0, 0));

	// 결과 텍스트 출력
	RECT resultRc = { 0, 360,900,500 };
	RECT scaledResultRc = CButton::ScaleRect(resultRc);

	DrawTextEx(device, GetResultText(), scaledResultRc, 
			   D3DCOLOR_XRGB(255, 255, 255), 90, 
		       DT_CENTER | DT_VCENTER | DT_SINGLELINE, false);

	// 버튼 출력
	m_retryBtn.Render(device, font);
	m_exitBtn.Render(device, font);

	// UI 랜더 상태 복구
	device->SetTexture(0, nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}