#ifndef COMMON_H
#define COMMON_H

#include <math.h>
#include <d3d9.h>
#include <d3dx9.h>

static struct CUSTOMVERTEX
{
    FLOAT x, y, z;
    DWORD color;
    FLOAT u, v;
};

struct Vec3
{
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // 덧셈
    Vec3 operator+(const Vec3& v) const
    {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    // 뺄셈
    Vec3 operator-(const Vec3& v) const
    {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    // 스칼라 곱
    Vec3 operator*(float s) const
    {
        return Vec3(x * s, y * s, z * s);
    }

    // 누적 덧셈
    Vec3& operator+=(const Vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
};

struct Instance
{
	Vec3 pos;	// 위치
	Vec3 rot;	// 회전
	Vec3 scale;	// 스케일(크기)

	Instance()
	{
		pos = { 0,0,0 };
		rot = { 0,0,0 };
		scale = { 1,1,1 };
	}
};

enum DartOwner
{
	E_PLAYER,
	E_COMPUTER
};

struct DartState
{
    Vec3 pos;   // 위치
    Vec3 dir;   // 방향
};

enum class Step
{
    E_NONE,
    E_EASY,
    E_NOMAL,
    E_HARD
};

enum class GameType
{
    E_NONE,
    E_ZERO_ONE,
    E_COUNT_UP,
    // E_CRICKET		// 땅따먹기와 같음(과녁을 5x5 빙고게임)
};

enum class ItemType
{
    E_NONE,
    E_EXCHANGE,		// 상대와 점수를 바꿀수 있는 아이템
    E_POINT_UP,		// 맞춰야할 점수를 10 ~ 60 사이에 랜덤한 값이 추가된다     (화면중앙의 슬롯머신)
    E_POINT_DOWN,	// 맞춰야할 점수를 10 ~ 60 사이에 랜덤한 값이 빠진다.      (화면중앙의 슬롯머신)
    E_ITEMCHARGE,	// 아이템칸에 존재하는 모든 아이템을 다시 랜덤하게 뽑는다. (아이템칸의 슬롯머신)
    E_ITEM_STEAL,	// 상대방의 아이템을 하나 훔쳐온다                         (훔쳐오는 칸은 랜덤)
    E_TARGET_SLOW,	// 과녁의 속도가 절반으로 떨어진다.                        (과녁 주변으로 아래로 화살표 이펙트)
    E_TARGET_STOP,	// 과녁이 움직이지 않는다.                                 (과녁 주변으로 정지 정지 이펙트)
    E_EXTRA_CHANCE,	// 내턴의 찬스를 노카운팅한다.                             ()
    E_SNEAK_PEEK,	// 과녁이 움직일 방향을 미리 알려준다.                     (과녁의 움직일 방향으로 화살표로 알려준다.)
    E_TARGET_SIZEUP,// 과녁에 사이즈가 커진다.                                 (과녁이 커지는 이펙트 커질사이즈하고 기존사이즈를 교차로 몇번 보여주는 방식)

    E_SHIELD,       // 상대의 다음 아이템 1회 무효                             (다트 주변으로 옅은 파란색으로 보호막같이 씌여진다.)
    E_ITEM_COPY,    // 상대방 아이템 하나 랜덤 복사
    E_MAGNET,       // 다트 주변으로 큐브가 점점 끌려오는 효과
    E_ITEM_LOCK,    // 상대방의 아이템슬롯을 1칸 1턴동안 봉인
    E_MIRROR,       // 상대방의 아이템효과를 반사
    E_CURSE,        // 상대방의 점수를 절반으로 만든다.
    E_ITEM_BLOCK,   // 상대방의 턴에 아이템사용 금지                           (다트 주변으로 보라색 일렁거림이 보이는 방법)
    E_CONFUSE,      // 상대방의 조작 반전(사용시 1턴만 효과)
    E_SHAKE,        // 상대방의 조준 흔들림 발생(카메라의 흔들림, 조작쪽은 이상없음)
    E_HEAVY,        // 상대방의 다트 조준속도를 50퍼 감소
    E_DRAIN,        // 상대방의 찬스를 하나 뺏어서 내가 가진다.(전체중에 단 한번만 등장)
    E_CURSED_CUBE,  // 상대방의 큐브와 접촉시 꽝 아이템 흭득(단 1턴 동안)
    E_BLACK_HOLE,   // 과녁의 움직임 리미트 이후 랜덤한 좌표에 블랙홀을 소환해서 다트의 움직임에 방해함
    E_ITEM_BOMB,    // 상대방 아이템 슬롯중 랜덤한 아이템칸에 아이템을 삭제 - 폭발이펙트 필요
    E_PARASITE,     // 상대방의 점수중 30퍼만큼 흡수한다 (상대방은 70퍼 나는 30퍼) 종목에 따라서는 다른 효과를 발휘한다(제로원일시 +와 -가 반대로 작용한다.)
    E_LUCKY_SEVEN,  // 과녁에 7배수 점수에 적중시 보너스 점수 지급
    E_NOVA          // 상대의 모든 아이템 효과 제거
};

// 크리켓모드의경우는 아직 완전한 룰을 숙지하지 못하여 구현이 불가능
// 단 땅따먹기란 습성을 이용해서 자신이 맞춘 과녁에 자신의 색상으로 물드는게 가장 좋을것 같다.
// 그리고 크리켓모드 전용 아이템또한 존재해야할듯

#endif