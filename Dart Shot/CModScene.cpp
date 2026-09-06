#include "CModScene.h"
#include "GameSetting.h"
#include "KeyProc.h"

extern POINT g_mousePos;

CModScene::CModScene()
{
	m_CurrentIndex = 0;

	m_isStoryEnd = false;
	m_isSkip = false;

	m_StoryNameFont = nullptr;
	m_StoryTextFont = nullptr;
}

CModScene::~CModScene() 
{
	if (m_StoryNameFont)
	{
		m_StoryNameFont->Release();
		m_StoryNameFont = nullptr;
	}

	if (m_StoryTextFont)
	{
		m_StoryTextFont->Release();
		m_StoryTextFont = nullptr;
	}
}

void CModScene::Init(HWND hwnd, CD3D* d3d)
{
	CScene::Init(hwnd, d3d);

	m_DialogBox = { 20, 700, 880, 985 };

	m_SkipBtn = { 780, 30, 860, 70 };

	m_EasyBtn = { 120, 180, 320, 250 };
	m_NormalBtn = { 350, 180, 550, 250 };
	m_HardBtn = { 580, 180, 780, 250 };

	m_ZeroOneBtn = { 120, 180, 320, 250 };
	m_CountUpBtn = { 350, 180, 550, 250 };

	m_StartBtn = { 340, 560, 580, 660 };

	m_Story.clear();
	m_StoryHistory.clear();

	m_CurrentIndex = 0;
	m_isStoryEnd = false;
	m_isSkip = false;

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();

	if (device)
	{
		D3DXCreateFont(
			device,
			36, 0, FW_BOLD, 1, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕", &m_StoryNameFont);

		D3DXCreateFont(
			device, 27, 0, FW_NORMAL, 1, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕", &m_StoryTextFont);
	}

	// 설정에 로컬라이징 언어 선택에 따른 스토리 파일 출력
	if (g_GameSetting.isEnglish)	{ LoadStory(L"story_EN.txt"); }
	else							{ LoadStory(L"story_KR.txt"); }
}

void CModScene::Update(float deltaTime)
{
	POINT pt = g_mousePos;

	if (m_CurrentIndex >= m_Story.size())
	{
		m_isStoryEnd = true;
		return;
	}

	ProcessEvent();

	// 마우스 클릭 처리
	if (IsActionTriggered(InputAction::E_MOUSE_RIGHT))
	{
		if (!m_StoryHistory.empty())
		{
			m_CurrentIndex = m_StoryHistory.back();
			m_StoryHistory.pop_back();
			return;
		}
	}

	if (IsActionTriggered(InputAction::E_MOUSE_LEFT))
	{
		// 스킵 버튼
		if (PtInRect(&m_SkipBtn, g_mousePos))
		{
			SkipStory();
			return;
		}

		StoryLine& line = m_Story[m_CurrentIndex];

		// 일반 스토리 진행
		if (line.event == StoryEvent::E_NORMAL)
		{
			NextStory();
		}

		// 난이도 선택
		else if (line.event == StoryEvent::E_SELECT_STEP)
		{
			if (PtInRect(&m_EasyBtn, g_mousePos))
			{
				g_GameSetting.step = Step::E_EASY;
				NextStory();
			}

			else if (PtInRect(&m_NormalBtn, g_mousePos))
			{
				g_GameSetting.step = Step::E_NOMAL;
				NextStory();
			}

			else if (PtInRect(&m_HardBtn, g_mousePos))
			{
				g_GameSetting.step = Step::E_HARD;
				NextStory();
			}
		}

		// 종목 선택
		else if (line.event == StoryEvent::E_SELECT_TYPE)
		{
			if (PtInRect(&m_ZeroOneBtn, g_mousePos))
			{
				g_GameSetting.type = GameType::E_ZERO_ONE;
				JumpToDiver((int)StoryDiver::E_ZERO_ONE);
			}

			else if (PtInRect(&m_CountUpBtn, g_mousePos))
			{
				g_GameSetting.type = GameType::E_COUNT_UP;
				JumpToDiver((int)StoryDiver::E_COUNT_UP);
			}
		}

		// 스토리 종료
		else if (line.event == StoryEvent::E_END)
		{
			if (PtInRect(&m_StartBtn, g_mousePos))
			{
				m_nextScene = GameScene::E_LOADING;
			}
		}
	}
}

void CModScene::Render(HDC hdc)
{
	if (!m_d3d) return;

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();
	ID3DXFont* font = m_d3d->GetUIFont();

	if (!device || !font) return;
	if (m_CurrentIndex >= (int)m_Story.size()) return;

	StoryLine& line = m_Story[m_CurrentIndex];

	DrawCharacterImage(device);

	RECT dialog = CButton::ScaleRect(m_DialogBox);
	CButton::DrawRect(device, dialog, D3DCOLOR_ARGB(210, 20, 20, 35));

	RECT speaker = { 70, 725, 830, 770 };
	speaker = CButton::ScaleRect(speaker);
	CButton::DrawTextUI(
		m_StoryNameFont ? m_StoryNameFont : font,
		line.speaker,
		speaker,
		D3DCOLOR_XRGB(255, 230, 120));

	RECT lineRc = { 70, 785, 830, 788 };
	lineRc = CButton::ScaleRect(lineRc);
	CButton::DrawRect(device, lineRc, D3DCOLOR_ARGB(180, 220, 190, 80));

	RECT text1 = { 75, 810, 830, 850 };
	text1 = CButton::ScaleRect(text1);

	// 일반 텍스트는 흰색으로 출력하고,
	// {KEY_...} 토큰만 노란색 키 배지로 따로 출력한다.
	DrawStoryTextRich(
		m_StoryTextFont ? m_StoryTextFont : font,
		line.text1,
		text1);

	RECT text2 = { 75, 850, 830, 890 };
	text2 = CButton::ScaleRect(text2);

	DrawStoryTextRich(
		m_StoryTextFont ? m_StoryTextFont : font,
		line.text2,
		text2);

	DrawStoryControlGuide(device, font);

	DrawChoiceButton(
		device,
		font,
		m_SkipBtn,
		g_GameSetting.isEnglish ? L"SKIP" : L"건너뛰기");

	if (line.event == StoryEvent::E_SELECT_STEP)
	{
		if (line.event == StoryEvent::E_SELECT_STEP)
		{
			DrawChoiceButton(device, font, m_EasyBtn,
				g_GameSetting.isEnglish ? L"EASY" : L"쉬움");

			DrawChoiceButton(device, font, m_NormalBtn,
				g_GameSetting.isEnglish ? L"NORMAL" : L"보통");

			DrawChoiceButton(device, font, m_HardBtn,
				g_GameSetting.isEnglish ? L"HARD" : L"어려움");
		}
	}

	else if (line.event == StoryEvent::E_SELECT_TYPE)
	{
		DrawChoiceButton(device, font, m_ZeroOneBtn,
			g_GameSetting.isEnglish ? L"ZERO ONE" : L"제로 원");

		DrawChoiceButton(device, font, m_CountUpBtn,
			g_GameSetting.isEnglish ? L"COUNT UP" : L"카운트 업");
	}

	else if (line.event == StoryEvent::E_END)
	{
		DrawChoiceButton(device, font, m_StartBtn,
			g_GameSetting.isEnglish ? L"START GAME" : L"게임 시작");
	}
}

void CModScene::DrawChoiceButton( LPDIRECT3DDEVICE9 device, ID3DXFont* font, const RECT& rc, const wstring& text)
{
	if (!device || !font) return;

	// 가상 해상도 기준 마우스 좌표
	POINT mouse = g_mousePos;

	bool hover = PtInRect(&rc, mouse);

	bool pressed = hover && ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);

	D3DCOLOR color;

	if (pressed)	{ color = D3DCOLOR_ARGB(240, 120, 140, 220); }
	else if (hover) { color = D3DCOLOR_ARGB(210, 60, 70, 110); }
	else			{ color = D3DCOLOR_ARGB(180, 25, 25, 40); }

	RECT drawRc = CButton::ScaleRect(rc);

	CButton::DrawRect(device, drawRc, color);

	CButton::DrawTextUI(font, text, drawRc, D3DCOLOR_XRGB(255, 255, 255));
}

void CModScene::DrawCharacterImage(LPDIRECT3DDEVICE9 device)
{
	if (!device) return;
	if (!m_d3d) return;
	if (m_CurrentIndex >= (int)m_Story.size()) return;

	StoryLine& line = m_Story[m_CurrentIndex];

	RECT rc;

	if (line.isLeft)	{ rc = { 70, 250, 330, 700 }; }
	else				{ rc = { 570, 250, 830, 700 }; }

	rc = CButton::ScaleRect(rc);

	if (line.imagePath.empty() || line.imagePath == L" ")
	{
		CButton::DrawRect(device, rc, D3DCOLOR_ARGB(130, 80, 80, 120));
		return;
	}

	LPDIRECT3DTEXTURE9 texture = m_d3d->GetTextureManager().LoadTexture(line.imagePath);

	if (texture)
	{
		CButton::DrawTexture(
			device,
			rc,
			texture,
			D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	else { CButton::DrawRect(device, rc, D3DCOLOR_ARGB(130, 80, 80, 120)); }
}

void CModScene::LoadStory(const wchar_t* FileName)
{
	// 파일 읽기용 스트림 객체 생성
	wifstream file(FileName);

	// UTF-8 파일 읽기 설정
	file.imbue( locale( file.getloc(), new std::codecvt_utf8<wchar_t> ) );

	// 파일 열기 실패시
	if (!file.is_open()) { return; }

	wstring line;

	// 한 줄씩 읽는다.
	while (getline(file, line))
	{
		// 문자열을 구분자로 분리하기 위한 스트림
		wstringstream ss(line);

		StoryLine story;

		wstring eventStr;
		wstring leftStr;
		wstring diverStr;

		// 구분자 '|'
		// getline(스트림, 저장변수, 구분문자)
		// 위에서 아래순서대로 스토리 구조체에 대한 정보를 읽으므로 순서가 매우 중요하다
		getline(ss, story.speaker, L'|');
		getline(ss, story.text1, L'|');
		getline(ss, story.text2, L'|');
		getline(ss, story.imagePath, L'|');

		getline(ss, leftStr, L'|');
		getline(ss, diverStr, L'|');
		getline(ss, eventStr, L'|');

		// 문자열 -> bool
		story.isLeft = (leftStr == L"1");

		// 문자열 -> 이벤트
		if		(eventStr == L"STEP")		{ story.event = StoryEvent::E_SELECT_STEP; }
		else if (eventStr == L"TYPE")		{ story.event = StoryEvent::E_SELECT_TYPE; }
		else if (eventStr == L"END")		{ story.event = StoryEvent::E_END; }
		else								{ story.event = StoryEvent::E_NORMAL; }

		if (!diverStr.empty())	{ story.storydiver = _wtoi(diverStr.c_str()); }
		else					{ story.storydiver = 0; }

		// vector에 저장
		m_Story.push_back(story);
	}

	file.close();
}

void CModScene::ProcessEvent()
{
	if (m_CurrentIndex >= m_Story.size()) return;
}

void CModScene::NextStory()
{
	if (m_Story.empty()) return;

	// 현재 위치를 기록한다.
	// 나중에 오른클릭하면 이 위치로 되돌아간다.
	m_StoryHistory.push_back(m_CurrentIndex);

	// 현재 줄의 분기 번호를 저장한다.
	int currentDiver = m_Story[m_CurrentIndex].storydiver;

	// 다음 줄로 이동
	m_CurrentIndex++;

	if (m_CurrentIndex >= (int)m_Story.size())
	{
		m_CurrentIndex = (int)m_Story.size(); // overflow 방지
		m_isStoryEnd = true;
		return;
	}

	// 현재 줄이 분기 스토리였다면
	// 같은 분기 번호가 아닌 줄을 만났을 때 처리해야 한다.
	if (currentDiver != (int)StoryDiver::E_NOMAL)
	{
		// 다음 줄이 같은 분기라면 그대로 진행한다.
		if (m_Story[m_CurrentIndex].storydiver == currentDiver) { return; }

		// 다음 줄이 다른 분기라면, 
		// 다른 분기 설명은 건너뛰고 공통 스토리로 이동한다.
		while (m_CurrentIndex < (int)m_Story.size())
		{
			if (m_Story[m_CurrentIndex].storydiver == (int)StoryDiver::E_NOMAL) { return; }
			m_CurrentIndex++;
		}

		m_isStoryEnd = true;
	}
}

void CModScene::SkipStory()
{
	if (m_Story.empty()) return;

	// 현재 위치부터 먼저 종목을 찾는다.
	// 종목 선택을 아직 안 했다면 여기로 이동해서 선택하게 한다.
	for (int i = m_CurrentIndex; i < (int)m_Story.size(); i++)
	{
		if (m_Story[i].event == StoryEvent::E_SELECT_TYPE)
		{
			m_CurrentIndex = i;
			return;
		}
	}

	// 종목선택이후 난이도를 선택안했으면 난이도 위치까지 이동한다.
	for (int i = m_CurrentIndex; i < (int)m_Story.size(); i++)
	{
		if (m_Story[i].event == StoryEvent::E_SELECT_STEP)
		{
			m_CurrentIndex = i;
			return;
		}
	}

	for (int i = m_CurrentIndex; i < m_Story.size(); i++)
	{
		if (m_Story[i].event == StoryEvent::E_END)
		{
			m_CurrentIndex = i;
			return;
		}
	}

	// END 없으면 그냥 끝으로 이동
	m_CurrentIndex = (int)m_Story.size() - 1;
}

// 스토리 분기 이동 함수
void CModScene::JumpToDiver(int diver)
{
	// TYPE 선택 위치를 기록한다.
	// 오른클릭하면 종목 선택 장면으로 돌아올 수 있다.
	m_StoryHistory.push_back(m_CurrentIndex);

	for (int i = 0; i < (int)m_Story.size(); i++)
	{
		if (m_Story[i].storydiver == diver)
		{
			m_CurrentIndex = i;
			return;
		}
	}
}

wstring CModScene::GetKeyTextFromToken(const wstring& token)
{
	// 스토리 파일에서 사용하는 토큰을 실제 현재 키 설정으로 변환한다.

	if (token == L"{KEY_MOVE_UP}")			return GetKeyBindingName(InputAction::E_MOVE_UP);
	if (token == L"{KEY_MOVE_DOWN}")		return GetKeyBindingName(InputAction::E_MOVE_DOWN);
	if (token == L"{KEY_MOVE_LEFT}")		return GetKeyBindingName(InputAction::E_MOVE_LEFT);
	if (token == L"{KEY_MOVE_RIGHT}")		return GetKeyBindingName(InputAction::E_MOVE_RIGHT);
	if (token == L"{KEY_FIRE}")				return GetKeyBindingName(InputAction::E_FLY);
	if (token == L"{KEY_USEITEM}")			return GetKeyBindingName(InputAction::E_USEITEM);
	if (token == L"{KEY_ITEM0}")			return GetKeyBindingName(InputAction::E_ITEM0);
	if (token == L"{KEY_ITEM1}")			return GetKeyBindingName(InputAction::E_ITEM1);
	if (token == L"{KEY_ITEM2}")			return GetKeyBindingName(InputAction::E_ITEM2);
	if (token == L"{KEY_ITEM3}")			return GetKeyBindingName(InputAction::E_ITEM3);
	if (token == L"{KEY_ITEM4}")			return GetKeyBindingName(InputAction::E_ITEM4);
	if (token == L"{KEY_VIEW}")				return GetKeyBindingName(InputAction::E_VIEWPOINTCONVERSION);
	if (token == L"{KEY_CONTROL_CHANGE}")	return GetKeyBindingName(InputAction::E_CONTROL_CHANGE);
	if (token == L"{KEY_PAUSE}")			return GetKeyBindingName(InputAction::E_PAUSE);

	// 등록되지 않은 토큰이면 원문 그대로 출력한다.
	// 오타가 있어도 화면에서 바로 확인 가능하다.
	return token;
}

void CModScene::DrawStoryTextRich(ID3DXFont* font, const wstring& text, const RECT& rc)
{
	if (!font) return;
	if (text.empty()) return;

	// 현재 출력 위치.
	// 문장을 왼쪽부터 조금씩 그리면서 x를 오른쪽으로 밀어낸다.
	int x = rc.left;
	int y = rc.top;

	size_t pos = 0;

	while (pos < text.length())
	{
		// {KEY_ 로 시작하는 키 토큰을 찾는다.
		size_t keyStart = text.find(L"{KEY_", pos);

		// 더 이상 키 토큰이 없으면 남은 문장은 일반 흰색 글자로 출력한다.
		if (keyStart == wstring::npos)
		{
			wstring normalText = text.substr(pos);

			RECT drawRc = { x, y, rc.right, rc.bottom };

			font->DrawTextW(
				nullptr,
				normalText.c_str(),
				-1,
				&drawRc,
				DT_LEFT | DT_VCENTER | DT_SINGLELINE,
				D3DCOLOR_XRGB(255, 255, 255));

			break;
		}

		// 키 토큰 앞쪽의 일반 문장.
		// 예: "발사는 {KEY_FIRE} 이야"
		// 여기서 "발사는 " 부분.
		wstring normalText = text.substr(pos, keyStart - pos);

		if (!normalText.empty())
		{
			RECT drawRc = { x, y, rc.right, rc.bottom };

			font->DrawTextW(
				nullptr,
				normalText.c_str(),
				-1,
				&drawRc,
				DT_LEFT | DT_VCENTER | DT_SINGLELINE,
				D3DCOLOR_XRGB(255, 255, 255));

			// 방금 그린 일반 문장의 가로 길이를 계산한다.
			// 이 길이만큼 x를 이동해야 키 배지가 문장 바로 뒤에 붙는다.
			RECT calcRc = { 0, 0, 0, 0 };

			font->DrawTextW(
				nullptr,
				normalText.c_str(),
				-1,
				&calcRc,
				DT_CALCRECT | DT_SINGLELINE,
				D3DCOLOR_XRGB(255, 255, 255));

			x += calcRc.right - calcRc.left;
		}

		// 토큰의 끝 '}' 위치를 찾는다.
		size_t keyEnd = text.find(L"}", keyStart);

		// 닫는 괄호가 없으면 잘못 작성된 토큰이므로 중단한다.
		if (keyEnd == wstring::npos)
			break;

		// 예: {KEY_FIRE}
		wstring token = text.substr(keyStart, keyEnd - keyStart + 1);

		// 예: {KEY_FIRE} -> Mouse Left
		wstring keyText = GetKeyTextFromToken(token);

		// 노란색 반짝임 키 박스 출력
		DrawKeyBadge(font, keyText, x, y);

		// 키 글자의 크기를 계산해서 다음 글자가 이어질 위치를 잡는다.
		RECT keyRc = { 0, 0, 0, 0 };

		font->DrawTextW(
			nullptr,
			keyText.c_str(),
			-1,
			&keyRc,
			DT_CALCRECT | DT_SINGLELINE,
			D3DCOLOR_XRGB(255, 255, 255));

		// DrawKeyBadge 내부에서 좌우 여백을 주기 때문에
		// 실제 글자 폭보다 조금 더 많이 이동한다.
		x += (keyRc.right - keyRc.left) + 34;

		// 다음 검색 시작 위치.
		pos = keyEnd + 1;
	}
}

void CModScene::DrawKeyBadge(ID3DXFont* font, const wstring& keyText, int x, int y)
{
	if (!font) return;
	if (keyText.empty()) return;
	if (!m_d3d) return;

	LPDIRECT3DDEVICE9 device = m_d3d->GetDevice();
	if (!device) return;

	// 시간값으로 반짝임을 만든다.
	// sinf는 -1 ~ 1 사이로 움직인다.
	// 여기에 +1을 해서 0 ~ 2로 만들고,
	// 35를 곱해서 0 ~ 70 사이 밝기 변화값으로 사용한다.
	float t = GetTickCount() * 0.006f;
	int glow = (int)((sinf(t) + 1.0f) * 35.0f);

	// 키 글자 크기 계산.
	// 박스 크기를 글자 길이에 맞추기 위해 필요하다.
	RECT textSize = { 0, 0, 0, 0 };

	font->DrawTextW(
		nullptr,
		keyText.c_str(),
		-1,
		&textSize,
		DT_CALCRECT | DT_SINGLELINE,
		D3DCOLOR_XRGB(255, 255, 255));

	int textWidth = textSize.right - textSize.left;

	// 키 배지의 바깥 영역.
	// x + 6은 일반 글자와 키 박스 사이의 작은 간격이다.
	RECT badge =
	{
		x + 6,
		y + 2,
		x + textWidth + 30,
		y + 38
	};

	// 바깥 노란 glow 박스.
	// alpha 값에 glow를 더해서 은은하게 깜빡인다.
	CButton::DrawRect(
		device,
		badge,
		D3DCOLOR_ARGB(120 + glow, 130, 95, 10));

	// 안쪽 어두운 박스.
	// 키 글자가 밝게 보이도록 배경을 어둡게 깔아준다.
	RECT inner =
	{
		badge.left + 2,
		badge.top + 2,
		badge.right - 2,
		badge.bottom - 2
	};

	CButton::DrawRect(
		device,
		inner,
		D3DCOLOR_ARGB(190, 35, 28, 10));

	// 실제 키 이름 출력.
	// 노란색 계열에 glow를 더해 반짝이는 느낌을 준다.
	font->DrawTextW(
		nullptr,
		keyText.c_str(),
		-1,
		&badge,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE,
		D3DCOLOR_XRGB(255, 220 + glow, 60));
}

void CModScene::DrawStoryControlGuide(LPDIRECT3DDEVICE9 device, ID3DXFont* font)
{
	if (!device || !font) return;
	if (!m_d3d) return;
	if (m_CurrentIndex >= (int)m_Story.size()) return;

	StoryLine& line = m_Story[m_CurrentIndex];

	// 현재 이전으로 갈 수 있는지 여부
	bool canPrev = !m_StoryHistory.empty();
	bool canNext = (line.event == StoryEvent::E_NORMAL);

	// 시간 기반 Pulse 값
	// sinf는 -1 ~ 1 사이로 움직인다.
	// +1 후 *0.5를 곱하면 0 ~ 1 사이 값이 된다.
	// 이 값을 알파에 섞어서 부드럽게 밝아졌다 어두워지는 효과를 만듬
	float time = GetTickCount() * 0.006f;
	float pulse = (sinf(time) + 1.0f) * 0.5f;

	// 클릭 중 눌림 느낌.
	// 오른클릭 중이면 이전 안내가 아래로 살짝 눌린다.
	// 왼클릭 중이면 다음 안내가 아래로 살짝 눌린다.
	int prevPressY = IsActionPressed(InputAction::E_MOUSE_RIGHT) && canPrev ? 2 : 0;
	int nextPressY = IsActionPressed(InputAction::E_MOUSE_LEFT) && canNext ? 2 : 0;

	// 아주 작은 흔들림.
	// 완전 고정 이미지보다 살아있는 UI처럼 보이게 한다.
	// 단, 클릭 불가능한 방향은 흔들지 않는다.
	int prevShakeX = canPrev ? (int)(sinf(GetTickCount() * 0.014f) * 1.2f) : 0;
	int nextShakeX = canNext ? (int)(sinf(GetTickCount() * 0.014f + 3.14f) * 1.2f) : 0;

	// 클릭 가능한 방향은 더 밝고,
	// 불가능한 방향은 어둡게 표시한다.
	int prevAlpha = canPrev ? (int)(150 + pulse * 80) : 55;
	int nextAlpha = canNext ? (int)(150 + pulse * 80) : 55;

	D3DCOLOR prevColor = D3DCOLOR_ARGB(prevAlpha, 255, 255, 255);
	D3DCOLOR nextColor = D3DCOLOR_ARGB(nextAlpha, 255, 255, 255);

	D3DCOLOR prevTextColor = canPrev ?
		D3DCOLOR_ARGB(215, 255, 235, 120) :
		D3DCOLOR_ARGB(80, 150, 150, 150);

	D3DCOLOR nextTextColor = canNext ?
		D3DCOLOR_ARGB(215, 255, 235, 120) :
		D3DCOLOR_ARGB(80, 150, 150, 150);

	// TextureManager는 이미 로드된 텍스처를 재사용한다.
	// 따라서 Render에서 LoadTexture를 호출해도 매번 새로 생성되지 않는다.
	LPDIRECT3DTEXTURE9 prevTex = m_d3d->GetTextureManager().LoadTexture(L"text_prev");
	LPDIRECT3DTEXTURE9 nextTex = m_d3d->GetTextureManager().LoadTexture(L"text_next");

	// 가상 해상도 기준 좌표.
	// CButton::ScaleRect를 거치므로 실제 화면 크기에 맞게 자동 스케일된다.
	RECT prevIcon =
	{
		58 + prevShakeX,
		943 + prevPressY,
		82 + prevShakeX,
		967 + prevPressY
	};

	RECT prevText =
	{
		88,
		931 + prevPressY,
		160,
		978 + prevPressY
	};

	RECT nextText =
	{
		725,
		931 + nextPressY,
		790,
		978 + nextPressY
	};

	RECT nextIcon =
	{
		810 + nextShakeX,
		943 + nextPressY,
		834 + nextShakeX,
		967 + nextPressY
	};

	prevIcon = CButton::ScaleRect(prevIcon);
	prevText = CButton::ScaleRect(prevText);
	nextText = CButton::ScaleRect(nextText);
	nextIcon = CButton::ScaleRect(nextIcon);

	// 이전 아이콘 출력.
	// 텍스처가 없으면 글자만 출력되므로 안전하다.
	if (prevTex)
	{
		CButton::DrawTexture(device, prevIcon, prevTex, prevColor);
	}

	const wchar_t* prevLabel = g_GameSetting.isEnglish ? L"Prev" : L"이전";
	const wchar_t* nextLabel = g_GameSetting.isEnglish ? L"Next" : L"다음";

	// 이전 안내 문구.
	// 버튼이 아니므로 hover 판정 없이 항상 안내로만 출력한다.
	font->DrawTextW(
		nullptr,
		prevLabel,
		-1,
		&prevText,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE,
		prevTextColor);

	// 다음 안내 문구.
	font->DrawTextW(
		nullptr,
		nextLabel,
		-1,
		&nextText,
		DT_RIGHT | DT_VCENTER | DT_SINGLELINE,
		nextTextColor);

	// 다음 아이콘 출력.
	if (nextTex)
	{
		CButton::DrawTexture(device, nextIcon, nextTex, nextColor);
	}
}