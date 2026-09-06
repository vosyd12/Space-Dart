#include "Math.h"

float Length(const Vec3& v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

void Normalize(Vec3& v)
{
    float len = Length(v);

    if (len != 0.0f)
    {
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
}

Vec3 Direction(const Vec3& from, const Vec3& to)
{
    Vec3 dir;
    dir.x = to.x - from.x;
    dir.y = to.y - from.y;
    dir.z = to.z - from.z;
    Normalize(dir);
    return dir;
}

float RandomRange(float min, float max)
{
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

float Clamp(float value, float min, float max)
{
    if (value < min) { return min; }
    if (value > max) { return max; }

    return value;
}

int ClampInt(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;

    return value;
}

float Distance(const Vec3& a, const Vec3& b)
{
    Vec3 d;

    d.x = b.x - a.x;
    d.y = b.y - a.y;
    d.z = b.z - a.z;

    return Length(d);
}

float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y +  a.z * b.z;
}

Vec3 Cross(const Vec3& a, const Vec3& b)
{
    Vec3 v;

    v.x = a.y * b.z - a.z * b.y;
    v.y = a.z * b.x - a.x * b.z;
    v.z = a.x * b.y - a.y * b.x;

    return v;
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

Vec3 LerpVec3(const Vec3& a, const Vec3& b, float t)
{
    Vec3 v;

    v.x = Lerp(a.x, b.x, t);
    v.y = Lerp(a.y, b.y, t);
    v.z = Lerp(a.z, b.z, t);

    return v;
}

float Saturate(float value)
{
    return Clamp(value, 0.0f, 1.0f);
}

float SmoothStep(float edge0, float edge1, float x)
{
    x = Saturate((x - edge0) / (edge1 - edge0));

    return x * x * (3.0f - 2.0f * x);
}

float ToRadian(float degree)
{
    return degree * (PI / 180.0f);
}

float ToDegree(float radian)
{
    return radian * (180.0f / PI);
}

Vec3 RotateY(const Vec3& pos, float angle)
{
    Vec3 r;

    float c = cosf(angle);
    float s = sinf(angle);

    r.x = pos.x * c - pos.z * s;
    r.y = pos.y;
    r.z = pos.x * s + pos.z * c;

    return r;
}

float DistanceFade(float dist, float minDist, float maxDist)
{
    float t = (dist - minDist) / (maxDist - minDist);

    t = Saturate(t);

    return 1.0f - t;
}

float WarpStretch(float speed, float maxStretch)
{
    return Clamp(speed * 0.05f, 1.0f, maxStretch);
}

float Gaussian(float x, float sigma)
{
    return expf(-(x * x) / (2.0f * sigma * sigma));
}

Vec3 BillboardRight(const Vec3& cameraPos, const Vec3& objectPos)
{
    Vec3 look = Direction(objectPos, cameraPos);

    Vec3 up = { 0,1,0 };

    Vec3 right = Cross(up, look);

    Normalize(right);

    return right;
}

Vec3 BillboardUp(const Vec3& cameraPos, const Vec3& objectPos)
{
    Vec3 look = Direction(objectPos, cameraPos);

    Vec3 right = BillboardRight(cameraPos, objectPos);

    Vec3 up = Cross(look, right);

    Normalize(up);

    return up;
}

// 충돌 / 판정 공용 계산
bool IsCircleHit2D(const Vec3& a, const Vec3& b, float radius)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    float distSq = (dx * dx) + (dy * dy);

    return distSq <= (radius * radius);
}

bool IsSphereHit3D(const Vec3& a, const Vec3& b, float radius)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;

    float distSq =
        (dx * dx) +
        (dy * dy) +
        (dz * dz);

    return distSq <= (radius * radius);
}

// 과녁 거리 비율
float DistanceRate2D(const Vec3& pos, const Vec3& center, float radius)
{
    if (radius <= 0.0f) { return 0.0f; }

    float dx = pos.x - center.x;
    float dy = pos.y - center.y;

    float dist = sqrtf((dx * dx) + (dy * dy));

    float rate = dist / radius;

    return Clamp(rate, 0.0f, 1.0f);
}

bool IsInsideCircle2D(const Vec3& pos, const Vec3& center, float radius)
{
    return DistanceRate2D(pos, center, radius) <= 1.0f;
}

// 각도 계산
float GetAngle2D( const Vec3& pos, const Vec3& center)
{
    float dx = pos.x - center.x;
    float dy = pos.y - center.y;

    float angle = atan2f(dy, dx);

    angle = ToDegree(angle);

    if (angle < 0.0f) { angle += 360.0f; }

    return angle;
}

int GetDartSectorByAngle(float angleDegree)
{
    static const int sectorTable[20] =
    {
        20, 1, 18, 4, 13,
        6, 10, 15, 2, 17,
        3, 19, 7, 16, 8,
        11, 14, 9, 12, 5
    };

    while (angleDegree < 0.0f) { angleDegree += 360.0f; }
    while (angleDegree >= 360.0f) { angleDegree -= 360.0f; }

    float sectorSize = 360.0f / 20.0f;

    int index =
        (int)(angleDegree / sectorSize);

    index = Clamp(index, 0, 19);

    return sectorTable[index];
}

float SafeDivide(float a, float b, float defaultValue)
{
    if (fabsf(b) < 0.00001f) { return defaultValue; }
    return a / b;
}

Vec3 NormalizeCopy(Vec3 v)
{
    float len = Length(v);

    if (len <= 0.00001f) { return Vec3(0, 0, 0); }

    return Vec3( v.x / len, v.y / len, v.z / len);
}

int ScaleAlpha(int alpha, float scale)
{
    alpha = (int)(alpha * scale);
    return Clamp(alpha, 0, 255);
}

D3DCOLOR MakeColor( int a, int r, int g, int b)
{
    a = Clamp(a, 0, 255);
    r = Clamp(r, 0, 255);
    g = Clamp(g, 0, 255);
    b = Clamp(b, 0, 255);

    return D3DCOLOR_ARGB(a, r, g, b);
}