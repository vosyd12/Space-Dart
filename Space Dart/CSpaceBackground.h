#ifndef CSPACEBACKGROUND_H
#define CSPACEBACKGROUND_H

#include <windows.h>
#include <vector>

#include <d3d9.h>
#include <d3dx9.h>

#include "Common.h"
#include "Math.h"

using namespace std;

// 별 하나의 정보
struct StarParticle
{
	Vec3 pos;
	float size;				// 별 크기
	float brightness;		// 반짝임용 밝기
	float twinkleSpeed;		// 밝기 변화 속도
	float depth;			// 패럴랙스 깊이 값이 작을수록 멀리있는 별
	float Stretch;			// 워프시 늘어나는 길이
	D3DCOLOR color;

	float baseBrightness;	// 기본 밝기
	float twinklePower;		// 반짝임 강도
};

// 유성 종류
enum class MeteorType
{
	E_FAR,			// 먼 배경 유성
	E_NORMAL,		// 일반 유성
	E_LONG,			// 긴 꼬리 유성
	E_CINEMATIC		// 큰 시네마틱 유성
};

// 유성 정보
struct Meteor
{
	Vec3 pos;
	Vec3 dir;

	float speed;
	float size;
	float width;
	float tailLength;

	float brightness;		// 현재 밝기
	float baseBrightness;	// 원래 밝기

	float nearRate;

	float tailScale;
	float glowScale;
	float alphaScale;

	D3DCOLOR color;
	MeteorType type;

	bool active;
};

// 성운 정보
struct Nebula
{
	Vec3 pos;
	float size;
	int texIndex;	// 사용될 텍스처 번호
	float alpha;	// 성운 투명도 
	float moveSpeed;
	D3DCOLOR color;
};

// 회전 은하 정보
struct GalaxyStar
{
	Vec3 pos;
	float rotation;
	float rotationSpeed;
	float size;
	float brightness;
	D3DCOLOR color;
};

struct SpaceDust
{
	Vec3 pos;
	Vec3 dir;
	float speed;
	float size;
	float brightness;
	float depth;
	D3DCOLOR color;
};

struct WarpEffect
{
	bool active = false;		
	float power = 0.0f;			// 워프 강도
	float starStrectch = 1.0f;	// 별 늘어나는 길이
	float moveSpeed = 1.0f;		// 이동 속도 배율
};

struct BloomData
{
	bool enable = true;
	float threshold = 180.0f;	// bloom 시작 밝기
	float intensity = 1.5f;		// bloom 강도
	float blurpower = 2.0f;		// blur 강도
};

class CSpaceBackground
{
	private:
		vector<StarParticle>	m_Stars;
		vector<Meteor>			m_Meteors;
		vector<Nebula>			m_Nebulas;
		vector<GalaxyStar>		m_Galaxy;
		vector<SpaceDust>		m_Dusts;

		int m_Width;
		int m_Height;

		float m_GalaxyRotation;		// 은하 전체 회전값

		float m_meteorSpawnTimer;
		float m_meteorSpawnRate;

		Vec3 m_CameraOffset;
		WarpEffect m_Warp;
		BloomData m_Bloom;

		// 생성 함수
		void CreateStars();
		void CreateNebulas();
		void CreateGalaxy();
		void CreateDust();

	public:
		void Init(int width, int height);
		void Update(float deltaTime, Vec3 cameraMove);	// 프레임 타임 + 카메라 이동
		void SpawnMeteor();
		void StarWarp(float power);
		void StopWarp();

		const vector<StarParticle>& GetStars() const	{ return m_Stars; }
		const vector<Meteor>& GetMeteors() const		{ return m_Meteors; }
		const vector<Nebula>& GetNebulas() const		{ return m_Nebulas; }
		const vector<GalaxyStar>& GetGalaxy() const		{ return m_Galaxy; }
		const vector<SpaceDust>& GetDusts() const		{ return m_Dusts; }

		const WarpEffect& GetWarp() const				{ return m_Warp; }
		const BloomData& GetBloom() const				{ return m_Bloom; }
		float GetGalaxyRotation() const					{ return m_GalaxyRotation; }
};

#endif

// 빌보드(Billboard) 효과
// 카메라의 각도가 어디든 객체가 카메라가 바라보는 방향으로 움직이므로
// 카메라는 언제나 객체의 정면방향만 보게되는 효과
// 예시 : 사람 얼굴 사진을 들고 있는데, 내가 어디서 보든 정면을 보는 느낌

// 패럴랙스(Parallax) 효과
// 멀리있는건 천천히 움직이고 가까운건 빠르게 움직이는 효과
// depth(깊이) : 깊이에 값에 따라서 작을수록 멀리 있음

// 블룸(Bloom) 효과
// 빛이 번지는 효과이며
// 현실의 강한 빛은 딱 끊겨 보이지 않고 빛 주변으로 퍼진다.
// 예시로는 자동차의 헤드라이트, 태양, 네온사인 등등이 있다

// Additive Blending
// 빛 + 빛 = 더 밝아지는 효과
// 일반 투명도와는 다르다
// 예시 : 손전등 하나만 비추면 밝지만, 손전동 5개를 한지점에 비추면 엄청 밝아지는것과 같다.

// 가우시안 블러(Gauuian Blur) 효과
// 빛이나 이미지를 부드럽게 퍼뜨리는 계산
// 현실의 빛은 딱 잘라보이지 않는다. 가장자리가 부드럽게 퍼진다.
// 카메라의 초점이 퍼지는 효과랑 같음

// 추가 예정 : 블랙홀, 행성