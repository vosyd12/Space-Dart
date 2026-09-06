#ifndef CSETTINGINFO_H
#define CSETTINGINFO_H

#include <windows.h>
#include "CButton.h"
#include "GameSetting.h"

enum class SettingTab
{
    E_GAME,
    E_SYSTEM,
    E_SOUND,
    E_KEY
};

enum class DragType
{
    E_NONE,
    E_MOUSE,
    E_LUMI,
    E_FRAME,
    E_FOV,
    E_BGM,
    E_SFX
};

enum class KeyBindTarget
{
    E_NONE,
    E_UP,
    E_DOWN,
    E_RIGHT,
    E_LEFT,
    E_USEITEM,
    E_USINGITEMKEY0,
    E_USINGITEMKEY1,
    E_USINGITEMKEY2,
    E_USINGITEMKEY3,
    E_USINGITEMKEY4,
    E_PAUSE,
    E_FLY,
    E_VIEW,
    E_CONTROLCHANGE
};

// 슬라이더 위치 모음
struct SettingSlider
{
    RECT mouse;
    RECT lumi;
    RECT frame;
    RECT fov;
    RECT bgm;
    RECT sfx;
};

// 일반 버튼 모음
struct SettingButton
{
    CButton gameTab;            // 상단의 게임탭
    CButton systemTab;          // 상단의 시스템탭
    CButton soundTab;           // 상단의 사운드탭
    CButton keyTab;             // 상단의 키설정탭

    CButton fullScreen;         // 풀스크린 버튼
    CButton invertMouse;        // 마우스 반전 버튼
    CButton isEnglish;          // 
    CButton back;               // 뒤로가기 버튼
};

// 키 설정 버튼 모음
struct KeyBindButton
{
    CButton up;                 // 위로 이동
    CButton down;               // 아래로 이동
    CButton left;               // 왼쪽으로 이동
    CButton right;              // 오른쪽으로 이동
    CButton useitem;            // 아이템 사용키
    CButton usingitemkey[5];    // 아이템창(0~4)
    CButton pause;              // ESC키 - 일시정지
    CButton fly;                // 마우스왼클릭 - 다트 날림
    CButton view;               // 시점 변환 키 - z키
    CButton controlChange;      // 다트 조작키 - q키
};

#endif