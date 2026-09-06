#include "CGameManager.h"

#include "CTitleScene.h"
#include "CSettingScene.h"
#include "CModScene.h"
#include "CPlayScene.h"
#include "CLoadingScene.h"
#include "KeyProc.h"

GameScene g_SettingReturnScene = GameScene::E_TITLE;

CGameManager::CGameManager()
{
    m_hwnd = nullptr;
    m_CurrentScene = nullptr;
    m_SavedGameScene = nullptr;
    m_CurrentSceneType = GameScene::E_NONE;
    m_initialized = false;
}

CGameManager::~CGameManager()
{
    if (m_CurrentScene)
    {
        delete m_CurrentScene;
        m_CurrentScene = nullptr;
    }

    if (m_SavedGameScene)
    {
        delete m_SavedGameScene;
        m_SavedGameScene = nullptr;
    }
}

void CGameManager::Init(HWND hwnd)
{
    m_hwnd = hwnd;

    m_d3d.Init(hwnd);

    m_CurrentScene = new CTitleScene();
    m_CurrentScene->Init(hwnd, &m_d3d);

    m_CurrentSceneType = GameScene::E_TITLE;

    m_initialized = true;
}

void CGameManager::Update()
{
    // 이전 프레임 시간 저장
    // static이라 함수가 끝나도 값이 유지됨
    static DWORD prevTime = GetTickCount();

    // 현재 시간
    DWORD currentTime = GetTickCount();

    // 실제 지난 시간을 초 단위로 계산
    float dt = (currentTime - prevTime) / 1000.0f;

    // 다음 프레임 계산을 위해 현재 시간을 저장
    prevTime = currentTime;

    // 비정상적으로 큰 dt 방지
    // 창 이동, 디버깅 중단, 순간 렉이 생기면 dt가 너무 커질 수 있음
    if (dt > 0.1f) { dt = 0.1f; }

    if (!m_CurrentScene) { return; }

    // 실제 dt를 씬에 전달
    m_CurrentScene->Update(dt);

    // 일시정지 상태가 아닐 때만 D3D 업데이트
    if (!m_CurrentScene->IsPaused()) {  m_d3d.Update(dt); }

    GameScene next = m_CurrentScene->GetNextScene();

    if (next != GameScene::E_NONE)
    {
        m_CurrentScene->ResetNextScene();
        ChangeScene(next);
    }
}

void CGameManager::Render(HDC hdc)
{
    if (!m_CurrentScene) return;

    if (!m_d3d.BeginFrame()) { return; }

    m_d3d.DrawSpace();

    m_CurrentScene->Render(hdc);

    m_d3d.EndFrame();
}

void CGameManager::ChangeScene(GameScene scene)
{
    // 게임에서 설정창으로 들어갈 때는 플레이 씬을 삭제하지 않고 보관한다.
    if (scene == GameScene::E_SETTING)
    {
        g_SettingReturnScene = m_CurrentSceneType;

        if (m_CurrentSceneType == GameScene::E_GAME)
        {
            m_SavedGameScene = m_CurrentScene;
            m_CurrentScene = nullptr;
        }
        else
        {
            if (m_CurrentScene)
            {
                delete m_CurrentScene;
                m_CurrentScene = nullptr;
            }
        }

        m_CurrentScene = new CSattingScene();
        m_CurrentScene->Init(m_hwnd, &m_d3d);
        m_CurrentSceneType = GameScene::E_SETTING;

        return;
    }

    // 설정창에서 게임으로 돌아갈 때는 보관해둔 플레이 씬을 그대로 복구한다.
    // Init을 다시 호출하지 않으므로 점수, 찬스, 타이머, 아이템 상태가 유지된다.
    if (scene == GameScene::E_GAME && m_SavedGameScene)
    {
        if (m_CurrentScene)
        {
            delete m_CurrentScene;
            m_CurrentScene = nullptr;
        }

        m_CurrentScene = m_SavedGameScene;
        m_SavedGameScene = nullptr;

        m_CurrentScene->ResetNextScene();
        m_CurrentSceneType = GameScene::E_GAME;

        return;
    }

    if (m_CurrentScene)
    {
        delete m_CurrentScene;
        m_CurrentScene = nullptr;
    }

    if (m_SavedGameScene)
    {
        delete m_SavedGameScene;
        m_SavedGameScene = nullptr;
    }

    switch (scene)
    {
        case GameScene::E_TITLE:
        {
            m_CurrentScene = new CTitleScene();
            break;
        }

        case GameScene::E_SETTING:
        {
            m_CurrentScene = new CSattingScene();
            break;
        }

        case GameScene::E_MODE_SELECT:
        {
            m_CurrentScene = new CModScene();
            break;
        }

        case GameScene::E_LOADING:
        {
            m_CurrentScene = new CLoadingScene();
            break;
        }

        case GameScene::E_GAME:
        {
            m_CurrentScene = new CPlayScene();
            break;
        }

        default: { break; }
    }

    if (m_CurrentScene)
    {
        m_CurrentScene->Init(m_hwnd, &m_d3d);
        m_CurrentSceneType = scene;
    }

    ResetInputState(m_hwnd);
}

void CGameManager::OnResize()
{
    if (!m_initialized) return;

    m_d3d.ResetDevice();
}