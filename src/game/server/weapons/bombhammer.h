#ifndef GAME_SERVER_WEAPONS_BOMBHAMMER_H
#define GAME_SERVER_WEAPONS_BOMBHAMMER_H

#include <game/server/weapon.h>

class CBombHammer : public CWeapon
{
public:
	CBombHammer(CCharacter *pOwnerChar);

	void Fire(vec2 Direction) override;
	void Tick() override;
	int GetType() override { return WEAPON_HAMMER; }
	bool IsPowerupOver() override { return false; }
	int m_Roundleft;
	int m_nextRoundtick;
	bool m_IsActive;
private:
	const int m_PerRoundms = 1000;
};

#endif // GAME_SERVER_WEAPONS_HAMMER_H
