#ifndef GAME_SERVER_WEAPONS_JUGHAMMER_H
#define GAME_SERVER_WEAPONS_JUGHAMMER_H

#include <game/server/weapon.h>

class CJugHammer : public CWeapon
{
public:
	CJugHammer(CCharacter *pOwnerChar);
	~CJugHammer();

	void Snap(int SnappingClient, int OtherMode);
	void Tick();
	void Fire(vec2 Direction) override;
	int GetType() override { return Character()->GetInput()->m_PlayerFlags & PLAYERFLAG_AIM ? WEAPON_NINJA : WEAPON_HAMMER; }

	int m_BounceTempTick;
	int m_BounceCooldownTick;
	int m_BounceMode;
	int m_AttackEnergyTick;

	int m_AttackEnergyLimit;

	int m_SnapID[8 + 2];
};

#endif // GAME_SERVER_WEAPONS_JUGHAMMER_H
