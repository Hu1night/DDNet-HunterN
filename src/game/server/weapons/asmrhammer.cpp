#include "asmrhammer.h"

CASMRHammer::CASMRHammer(CCharacter *pOwnerChar) : CHammer(pOwnerChar)
{
	m_FireDelay = 0;
	m_FullAuto = true;
}

void CASMRHammer::Tick()
{
	CWeapon::Tick();
	if(Character()->GetLatestInput().m_Fire & 1)
		GameWorld()->CreateSoundGlobal(SOUND_MENU); // :D
}