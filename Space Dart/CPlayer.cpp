#include "CPlayer.h"

CPlayer::CPlayer()
{
	m_Owner = E_PLAYER;
}

CPlayer::~CPlayer() {}

void CPlayer::Init()
{
	Init(E_PLAYER);
}

void CPlayer::Init(DartOwner owner)
{
	m_Owner = owner;

	CStatus::Reset();
	SetAlive(true);

	m_Item.Init();

	m_Dart.Init(
		0.0f,
		0.0f,
		1.0f,
		{ 0.0f, 0.0f, 1.0f },
		m_Owner);
}

void CPlayer::ThrowDart(Vec3 targetPos)
{
	if (!HasChance()) { return; }
	if (m_Dart.GetMoving()) { return; }

	m_Dart.Fly(targetPos);

	DecreaseChance();
}

void CPlayer::MoveDart(float x, float y)
{
	if (m_Dart.GetMoving()){ return; }
	m_Dart.Move(x, y);
}

void CPlayer::UpdateDart()
{
	m_Dart.Update();
}

void CPlayer::ResetDart()
{
	m_Dart.Init(
		0.0f,
		0.0f,
		1.0f,
		{ 0.0f, 0.0f, 1.0f },
		m_Owner);
}

void CPlayer::Reset()
{
	CStatus::Reset();

	SetAlive(true);

	m_Item.Reset();

	ResetDart();
}