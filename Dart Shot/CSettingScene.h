#ifndef CSATTINGSCENE_H
#define CSATTINGSCENE_H

#include "CScene.h"
#include "CSettingInfo.h"
#include <string>

using namespace std;

class CSattingScene : public CScene
{
	private:
        SettingTab m_currentTab;

        GameSetting m_value;
        SettingSlider m_slider;
        SettingButton m_button;
        KeyBindButton m_keyButton;

        DragType m_dragType;
        KeyBindTarget m_waitingKey;

        bool m_waitInputReady;
        bool m_bindPrevKey[256];

        // 슬라이더 내부에서 현재 마우스 위치가 어느 정도 비율인지 계산하는 함수
        float CalcRatio(const RECT& bar, POINT pt);
        // 슬라이더 계산 함수
        int CalcStepIndex(const RECT& bar, POINT pt, int stepCount);
        // 현재 마우스가 특정 RECT 영역 안에 있는지 검사
        bool IsMouseOver(const RECT& rc, POINT pt);

        // 키 설정 관련 함수
        // 현재 눌린 키를 얻는 함수
        int GetPressedKey();
        // VK값을 문자열로 반환
        wstring KeyToString(int vk);
        // 특정행동(키입력)에 실제 키를 적용
        void ApplyKeyBind(KeyBindTarget target, int vk);
        // 키 설정 버튼의 문자열을 새로고침
        void RefreshKeyButtonText();

        // Update 계열 함수
        // 상단 탭 버튼 업데이트
        void UpdateTabButtons(POINT pt, bool mouseDown);
        // 게임/ 시스템 탭 업데이트
        void UpdateGameTab(POINT pt, bool mouseDown);
        // 사운드 탭 업데이트
        void UpdateSoundTab(POINT pt, bool mouseDown);
        // 키 설정 탭 업데이트
        void UpdateKeyTab(POINT pt, bool mouseDown);

        // Render 계열 함수
        // 상단 탭 UI 랜더링
        void RenderTabs(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
        // 게임 / 시스템 탭 랜더링
        void RenderGameTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
        // 사운드 탭 랜더링
        void RenderSoundTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font);
        // 키 설정 탭 랜더링
        void RenderKeyTab(LPDIRECT3DDEVICE9 device, ID3DXFont* font);

        void DrawLabel(ID3DXFont* font, const wstring& text, int x, int y, int width, int height);
        void DrawKeyRow(ID3DXFont* font, const wstring& label, int x, int y, CButton& keyButton);

        void ClearBindInputState();
	public:
        void Init(HWND hwnd, CD3D* d3d) override;
        void Update(float dt) override;
        void Render(HDC hdc) override;
};

#endif

// 스토리 출력관련 영어또는 한국어 선택가능
// 마우스감도를 위한 감도칸은 전체적으로 마우스또는 키보드조작에 영향을 주는게 좋을것 같다.
// 마우스 반전을 위한 버튼도 키보드 또는 마우스를 조작에 영향을 준다

// 추가 기능 : AA기능(SSAA, MSAA, TAA)
// 지금의 게임tab과 시스템tab은 하나로 통합하여 최종적으로 게임tab을 살려서 쓴다.