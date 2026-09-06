#include "CStatus.h"

CStatus::CStatus() 
{
	m_Point = 0;
	m_Chance = 0;    
	m_isAlive = true;

	m_ItemEffect.Reset();
}
CStatus::~CStatus() {}

void CStatus::Reset()
{
	m_Point = 0;
	m_Chance = 0;
	m_isAlive = true;

	m_ItemEffect.Reset();
}