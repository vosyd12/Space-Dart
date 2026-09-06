#ifndef CCAMERA_H
#define CCAMERA_H

#include "Common.h"
#include <d3d9.h>
#include <d3dx9.h>

enum class CameraViewMode
{
    E_SHOULDER,
    E_FIRST_PERSON
};

class CCamera
{
	private:
        // 카메라 실제 위치
        Vec3 m_Location;

        // 바라보는 위치
        Vec3 m_Target;

        // 회전값
        float m_Yaw;
        float m_Pitch;

        // 타겟과 거리
        float m_Distance;

        // 시야각
        float m_Fov;

        // 이동 속도
        float m_MoveSpeed;

        // 카메라 방향 벡터
        Vec3 m_Forward;
        Vec3 m_Right;
        Vec3 m_Up;

        // 화면 흔들림
        float m_ShakePower;
        float m_ShakeTime;

        // View Matrix 저장
        D3DXMATRIX m_View;
        CameraViewMode m_ViewMode;

	public:
        void Init();

        // 타겟 기준 카메라 업데이트
        void Update(Vec3 targetPos, float deltaTime);

        // View 행렬 적용
        void Apply(LPDIRECT3DDEVICE9 device);

        // 회전
        void Rotate(float dx, float dy);

        // 줌
        void Zoom(float delta);

        // 흔들림
        void AddShake(float power, float time) { m_ShakePower = power; m_ShakeTime = time; }

        // FOV 변경
        void SetFov(float fov) { m_Fov = fov; }

        void ToggleViewMode();

        CameraViewMode GetViewMode() const          { return m_ViewMode; }
        const Vec3& GetLocation() const             { return m_Location; }
        const Vec3& GetForward() const              { return m_Forward; }
        const Vec3& GetRight() const                { return m_Right; }
        const Vec3& GetUp() const                   { return m_Up; }
        const D3DXMATRIX& GetViewMatrix() const     { return m_View; }
        float GetFov() const                        { return m_Fov; }
        void SetViewMode(CameraViewMode mode)       { m_ViewMode = mode; }
};

#endif

// Forward  벡터
// Right 벡터
// up 벡터
// View Matrix 저장
// Camera Shake
// FOV
// Camera Speed