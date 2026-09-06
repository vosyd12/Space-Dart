#include "CCamera.h"
#include "GameSetting.h"
#include "Math.h"

void CCamera::Init()
{
    m_Location = Vec3(0.0f, 0.0f, -10.0f);
    m_Target = Vec3(0.0f, 0.0f, 50.0f);

	m_Yaw = 0.0f;
	m_Pitch = 0.0f;
    m_Distance = 10.0f;

	m_Fov = D3DX_PI / 4.0f;

	m_MoveSpeed = 8.0f;

    m_Forward = Vec3(0.0f, 0.0f, 1.0f);
    m_Right = Vec3(1.0f, 0.0f, 0.0f);
    m_Up = Vec3(0.0f, 1.0f, 0.0f);

	m_ShakePower = 0.0f;
	m_ShakeTime = 0.0f;

	D3DXMatrixIdentity(&m_View);
    m_ViewMode = CameraViewMode::E_SHOULDER;
}

void CCamera::Update(Vec3 targetPos, float deltaTime)
{
    m_Target = targetPos;

    // Pitch 제한
    m_Pitch = Clamp(m_Pitch, -0.7f, 0.7f);

    // Forward 계산
    m_Forward.x = cosf(m_Pitch) * sinf(m_Yaw);
    m_Forward.y = sinf(m_Pitch);
    m_Forward.z = cosf(m_Pitch) * cosf(m_Yaw);

    Normalize(m_Forward);

    // Right 계산
    Vec3 worldUp = Vec3(0.0f, 1.0f, 0.0f);

    // Right = worldUp * forward
    m_Right.x = worldUp.y * m_Forward.z - worldUp.z * m_Forward.y;
    m_Right.y = worldUp.z * m_Forward.x - worldUp.x * m_Forward.z;
    m_Right.z = worldUp.x * m_Forward.y - worldUp.y * m_Forward.x;

    Normalize(m_Right);

    // Up 계산
    m_Up.x = m_Forward.y * m_Right.z - m_Forward.z * m_Right.y;
    m_Up.y = m_Forward.z * m_Right.x - m_Forward.x * m_Right.z;
    m_Up.z = m_Forward.x * m_Right.y - m_Forward.y * m_Right.x;

    Normalize(m_Up);
    // 숄더뷰 시점
    if (m_ViewMode == CameraViewMode::E_SHOULDER)
    {    // 숄더뷰 오프셋  
        Vec3 shoulderOffset = (m_Right * -1.0f) + (m_Up * 2.0f);
        // 카메라 위치 계산
        m_Location = m_Target - (m_Forward * m_Distance) + shoulderOffset;
    }
    //1인칭 시점
    else if (m_ViewMode == CameraViewMode::E_FIRST_PERSON)
    {
        m_Location =
            m_Target
            + (m_Forward * 0.5f)    // 약간 앞으로
            + (m_Up * 0.08f);       // 살짝 위로
    }

    // 화면 흔들림
    if (m_ShakeTime > 0.0f)
    {
        m_ShakeTime -= deltaTime;

        m_Location.x += RandomRange(-m_ShakePower, m_ShakePower);
        m_Location.y += RandomRange(-m_ShakePower, m_ShakePower);
        m_Location.z += RandomRange(-m_ShakePower, m_ShakePower);

        if (m_ShakeTime <= 0.0f)
        {
            m_ShakePower = 0.0f;
            m_ShakeTime = 0.0f;
        }
    }

    // View Matrix 생성
    D3DXVECTOR3 eye(m_Location.x, m_Location.y, m_Location.z);
    Vec3 lookTarget = m_Target + (m_Forward * 8.0f);        // 전방을 바라봄

    D3DXVECTOR3 at(lookTarget.x, lookTarget.y, lookTarget.z);
    D3DXVECTOR3 up(m_Up.x, m_Up.y, m_Up.z);

    D3DXMatrixLookAtLH(&m_View, &eye, &at, &up);
}

void CCamera::Apply(LPDIRECT3DDEVICE9 device)
{
    device->SetTransform(D3DTS_VIEW, &m_View);
}

void CCamera::Rotate(float dx, float dy)
{
    float sensitivity = 0.05f + (g_GameSetting.mouseSensitivity * 0.5f);

    m_Yaw += dx * sensitivity;

    if (g_GameSetting.invertMouse)  {  m_Pitch += dy * sensitivity; }
    else                            { m_Pitch += -dy * sensitivity; }

    float limit = 0.7f;

    if (m_Pitch > limit) { m_Pitch = limit; }
    if (m_Pitch < -limit) { m_Pitch = -limit; }
}

void CCamera::Zoom(float delta)
{
    // 줌 속도
    float zoomSpeed = 1.0f;

    m_Distance += delta * zoomSpeed;

    // 최소 / 최대 거리 제한
    if (m_Distance < 2.0f) { m_Distance = 2.0f; }
    if (m_Distance > 60.0f) { m_Distance = 60.0f; }
}

void CCamera::ToggleViewMode()
{
    if (m_ViewMode == CameraViewMode::E_SHOULDER)   {  m_ViewMode = CameraViewMode::E_FIRST_PERSON; }
    else                                            { m_ViewMode = CameraViewMode::E_SHOULDER; }
}