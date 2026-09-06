#ifndef MATH_H
#define MATH_H

#include <math.h>
#include <windows.h>
#include "Common.h"

#define PI 3.1415926535f

// 벡터의 크기 계산
// 피타고라스 공식 (sqrt(x² + y² + z²))
float Length(const Vec3& v);

// 길이를 1로 만드는 작업
// 방향만 남긴다.
void Normalize(Vec3& v);

// 두점 방향 구하기
Vec3 Direction(const Vec3& from, const Vec3& to);

float RandomRange(float min, float max);

float Clamp(float value, float min, float max);
int ClampInt(int value, int min, int max);

// 두 벡터 거리
float Distance(const Vec3& a, const Vec3& b);

// 내적(Dot Product)
// 방향이 얼마나 같은지 계산
// 1 = 완전히 같은 방향
// 0 = 수직
// -1 반대 방향
float Dot(const Vec3& a, const Vec3& b);

// 외적(Cross Product)
// 두 벡터의 수직 방향 계산
// 카메라 회전, 빛의 벡터 계산에 사용
Vec3 Cross(const Vec3& a, const Vec3& b);

// 선형 보간(Lerp)
// 부드럽게 이동
// t = 0.0f -> 시작값
// t = 1.0f -> 목표값
float Lerp(float a, float b, float t);

// 벡터 선형 보간
Vec3 LerpVec3(const Vec3& a, const Vec3& b, float t);

float Saturate(float value);

// 일반 보간보다 더 부드럽다.
// 천천히 시작 -> 빠르게 -> 천천히 끝
// 자연스러운 애니메이션을 표현하기 위해서나 빛 변환에 사용된다.
float SmoothStep(float edge0, float edge1, float x);

// 각도 -> 라디안 변환
// DirectX는 라디안 사용한다.
float ToRadian(float degree);

// 라디안 -> 각도 변환
float ToDegree(float radian);

// y축 회전
Vec3 RotateY(const Vec3& pos, float angle);

// 길이 기반 밝기 감소
// 멀리 있을수록 어두워지는 계산
float DistanceFade(float dist, float minDist, float maxDist);

// 워프 스트레치 계산
// 워프 별 길이 계산
// 속도가 빠를수록 별이 길어지는 계산
float WarpStretch(float speed, float maxStretch);

// 가우시안 함수
// 블룸 / 블러 핵심 수학
// 빛이 부드럽게 퍼지는 계산
// sigma 올라갈수록 더 넓게 퍼짐
float Gaussian(float x, float sigma);

// 빌보드 Right 벡터 계산
// 카메라 기준 오른쪽 방향
Vec3 BillboardRight(const Vec3& cameraPos, const Vec3& objectPos);

// 빌보드 Up 벡터 계산
// 카메라 기준 위쪽 방향
Vec3 BillboardUp(const Vec3& cameraPos, const Vec3& objectPos);

// 충돌 / 판정 공용 계산
bool IsCircleHit2D(const Vec3& a, const Vec3& b, float radius);
bool IsSphereHit3D(const Vec3& a, const Vec3& b, float radius);

// 과녁 중심에서 얼마나 떨어졌는지 0~1 비율
float DistanceRate2D(const Vec3& pos, const Vec3& center, float radius);

// 원형 과녁 안쪽인지
bool IsInsideCircle2D(const Vec3& pos, const Vec3& center, float radius);

// 과녁 점수 계산 보조
float GetAngle2D(const Vec3& pos, const Vec3& center);
int GetDartSectorByAngle(float angleDegree);

// 안전 계산
float SafeDivide(float a, float b, float defaultValue = 0.0f);
Vec3 NormalizeCopy(Vec3 v);

// 색상 / 알파 보조
int ScaleAlpha(int alpha, float scale);
D3DCOLOR MakeColor(int a, int r, int g, int b);

#endif