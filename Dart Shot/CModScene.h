#ifndef CMODSCENE_H
#define CMODSCENE_H

#include "CScene.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <cmath>

using namespace std;

enum class StoryEvent
{
	E_NORMAL,		// 스토리 출력중
	E_SELECT_STEP,	// 난이도 선택
	E_SELECT_TYPE,	// 종목 선택
	E_END			// 스토리 종료
};

enum class StoryDiver
{
	E_NOMAL = 0,
	E_ZERO_ONE = 200,
	E_COUNT_UP = 100
};

struct StoryLine
{
	wstring speaker;	// 이름
	wstring text1;		// 첫번째줄
	wstring text2;		// 두번째줄

	wstring imagePath;	// 이미지 출력
	bool isLeft;		// 왼쪽 출력

	int storydiver;		// 스토리의 분기
	StoryEvent event;	// 현재 줄의 이벤트 종류
};

class CModScene : public CScene
{
	private:
		vector<StoryLine> m_Story;
		// 실제로 지나온 스토리 인덱스 기록
		// 단순히 m_CurrentIndex--를 하면 분기 스토리에서 다른 루트로 꼬일 수 있으므로
		// 실제 지나온 위치만 저장해서 되돌아간다.
		vector<int> m_StoryHistory;
		// 현재 출력중인 스토리 위치
		int m_CurrentIndex;

		bool m_isStoryEnd;
		bool m_isSkip;

		RECT m_DialogBox;
		RECT m_SkipBtn;

		RECT m_EasyBtn;
		RECT m_NormalBtn;
		RECT m_HardBtn;

		RECT m_ZeroOneBtn;
		RECT m_CountUpBtn;

		RECT m_StartBtn;

		ID3DXFont* m_StoryNameFont;
		ID3DXFont* m_StoryTextFont;

		// txt 파일 읽기
		void LoadStory(const wchar_t* FileName);
		void DrawCharacterImage(LPDIRECT3DDEVICE9 device);
		void DrawChoiceButton(LPDIRECT3DDEVICE9 device, ID3DXFont* font, const RECT& rc, const wstring& text);
		// 현재 스토리 이벤트 처리
		void ProcessEvent();
		void NextStory();
		void SkipStory();

		void JumpToDiver(int diver);

		// 키 토큰을 현재 키 설정 문자열로 바꾸게 해주는 함수
		wstring GetKeyTextFromToken(const wstring& token);
		// 문장 안의 KEY_라는 부분만 찾아서 키 배지로 출력해주는 함수
		void DrawStoryTextRich(ID3DXFont* font, const wstring& text, const RECT& rc);
		// 노란색으로 반짝이는 키 박스를 그린다.
		void DrawKeyBadge(ID3DXFont* font, const wstring& keyText, int x, int y);

		//스토리창 좌하단/우하단에 이전/다음 안내 이미지를 출력한다.
		void DrawStoryControlGuide(LPDIRECT3DDEVICE9 device, ID3DXFont* font);

	public:
		CModScene();
		~CModScene();

		void Init(HWND hwnd, CD3D* d3d) override;
		void Update(float) override;
		void Render(HDC hdc) override;
};

#endif

// 화자 캐릭터들은 전부 화면에 띄어주고
// 대화에 해당하는 캐릭터 이미지만 조금더 사이즈를 키우는식으로 좀더 직관적으로 수정