#ifndef __CTHINK_H__
#define __CTHINK_H__

#include <vector>
#include "Common.h"
#include "Math.h"
#include "GameSetting.h"

using namespace std;

const int AI_MAX_CUBE_COUNT = 50;

// AI 상태
enum class AIState
{
	E_IDLE,		// 대기
	E_THINK,	// 계산중
	E_AIM,		// 조준중
	E_THROW,	// 발사
	E_WAIT		// 결과 대기
};

enum class AITargetType
{
	E_SCORE,
	E_FINISH,
	E_SAFE,
	E_RISKY,
	E_SETUP,
	E_ITEM,
	E_CUBE
};

// AI 조준 결과
struct AIAimData
{
	Vec3 targetPos = { 0.0f, 0.0f, 60.0f };

	int targetScore = 0;
	int expectedScore = 0;

	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float accuracy = 1.0f;

	bool useItem = false;
	bool useCube = false;
	bool shouldThrow = false;

	int useItemSlot = -1;
	ItemType useItemType = ItemType::E_NONE;
};

struct AITargetCandidate
{
	int score = 0;
	Vec3 targetPos = { 0.0f, 0.0f, 60.0f };

	AITargetType type = AITargetType::E_SCORE;

	float scoreValue = 0.0f;
	float finishValue = 0.0f;
	float safetyValue = 0.0f;
	float riskValue = 0.0f;
	float finalValue = 0.0f;

	bool bust = false;
	bool finish = false;
};

// AI 플레이 데이터
struct AIPlayerData
{
	int score = 0;
	int chance = 0;

	ItemType itemSlot[3] =
	{
		ItemType::E_NONE,
		ItemType::E_NONE,
		ItemType::E_NONE
	};
};

class CThink
{
	private:
		Step m_Level;
		AIState m_State;

		AIPlayerData m_Self;
		AIPlayerData m_Enemy;

		vector<AITargetCandidate> m_Candidates;

		AITargetCandidate m_BestCandidate;
		AIAimData m_Aim;

		float m_ThinkTimer;
		float m_ThinkDelay;

		float m_Accuracy;
		float m_MistakeRate;
		float m_RiskWeight;
		float m_AggressiveWeight;
		float m_ItemWeight;
		float m_CubeWeight;

		// 목표를 보고 실제 반응하기까지의 인간 반응속도
		// Hard는 거의 즉시 반응
		// Easy는 늦게 반응
		float m_ReactionDelay;

		// 실시간 재판단 타이머
		float m_ReThinkTimer;

		// 몇 초마다 다시 생각할 것인가
		// 값이 작을수록 똑똑해진다.
		float m_ReThinkInterval;

		// 방향 보정 강도
		// 높을수록 목표를 빠르게 따라감
		float m_SteerSmooth;

		// 당황 확률
		// Easy는 갑자기 이상한 판단을 함
		float m_PanicRate;

		// 현재 목표를 포기하는 기준
		// 값이 높을수록 쉽게 포기
		float m_GiveUpThreshold;

		Vec3 m_TargetPos;			// 현재 과녁 위치
		Vec3 m_TargetDir;			// 현재 과녁 이동 방향
		float m_TargetSpeed;		// 현재 과녁 이동 속도
		float m_TargetRadius;		// 현재 과녁 크기
		bool m_TargetMoving;		// 현재 과녁이 움직이고 있는가
		Vec3 m_DartPos;				// 현재 다트 위치
		Vec3 m_DartDir;				// 현재 다트 방향
		float m_DartSpeed;			// 현재 다트 속도
		bool m_DartFlying;			// 현재 다트 비행 여부

		Instance m_Cubes[AI_MAX_CUBE_COUNT];
		int m_CubeCount;

		Vec3 m_CurrentPlanTarget;		// 현재 AI가 노리고 있는 목표 위치
		int m_CurrentPlanScore;			// 현재 AI가 노리고 있는 목표 점수
		// 목표를 유지한 시간
		// 일정 시간이 지나면 다시 판단
		float m_TargetHoldTimer;

		// CThink.cpp
		void Think();
		void ResetAim();

		void ThinkRealtime();
		Vec3 PredictTargetPosition();
		Vec3 SelectRealtimeTarget();
		Vec3 ChooseFallbackTarget();
		float EvaluateReachability(Vec3 targetPos);
		bool ShouldGiveUpCurrentTarget(Vec3 targetPos);
		float GetDifficultyErrorScale() const;
		float GetHumanJitterScale() const;

		// CThink_Target.cpp
		void BuildTargetCandidates();
		void BuildCountUpCandidates();
		void BuildZeroOneCandidates();
		void AddCandidate(int score, AITargetType type);
		void EvaluateCandidates();
		void SelectBestCandidate();
		Vec3 CalculateTargetPosition(int score);

		// CThink_Risk.cpp
		bool IsBust(int score);
		bool IsFinish(int score);
		bool CanFinishThisTurn();
		bool EnemyCanFinishNextTurn();
		bool IsDangerState();
		bool ShouldPlayAggressive();
		float CalculateBustRisk(int score);
		float CalculateSafetyValue(int score);
		float CalculateFinishValue(int score);
		float CalculateScoreValue(int score);

		// CThink_Item.cpp
		bool ShouldUseItem();
		int SelectBestItem();
		float EvaluateItemValue(ItemType item);
		bool HasItem(ItemType item);

		// CThink_Cube.cpp
		bool HasEmptyItemSlot() const;
		bool ShouldGetCube();
		float CalculateCubeValue();
		bool HasUsefulCubePath(Vec3& outCubePos);

		// CThink_Difficulty.cpp
		void ApplyDifficulty();
		void ApplyAimError();
		float Random01();
		float RandomRangeAI(float minValue, float maxValue);


	public:
		CThink();
		~CThink();

		void Init(Step level);
		void Update(float dt);

		void SetSelfData(int score, int chance);
		void SetEnemyData(int score, int chance);
		void SetSelfItem(int slot, ItemType type);

		void SetWorldData(Vec3 targetPos, Vec3 targetDir, float targetSpeed, float targetRadius, bool targetMoving, Vec3 dartPos, Vec3 dartDir, float dartSpeed, bool dartFlying);
		void SetCubeData(const Instance* cubes, int cubeCount);

		// 비행 중에도 계속 호출되는 실시간 판단
		void UpdateRealtime(float dt);

		void StartTurn();
		void EndTurn();

		const AIAimData& GetAimData() const { return m_Aim; }
		AIState GetState() const { return m_State; }
};


#endif

// 여기는 컴퓨터에 던지는 사고및 아이템 사용여부, 다트의 움직임및, 과녁의 위치정보, 아이템의 정보,
// 난이도예 따른 던져지는 다트의 정확도가 다를게 할것

/*
CThink.cpp              // Init, Update, 최종 판단 흐름
CThink_Target.cpp       // ZERO ONE / COUNT UP 목표 점수 선택
CThink_Risk.cpp         // 버스트, 승률, 위험도 계산
CThink_Item.cpp         // 아이템 사용 판단
CThink_Cube.cpp         // 큐브 먹을지 판단
CThink_Difficulty.cpp   // 난이도별 오차, 실수율
*/

// 지금의 AI의 대한 생각이 너무 느슨한것같음 좀더 적극적으로 행동하는 방식으로 다시 리빌딩
// 아이템사용에 주저하는 경향이 있는것같음