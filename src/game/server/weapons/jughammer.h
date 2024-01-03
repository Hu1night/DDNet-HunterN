#ifndef GAME_SERVER_WEAPONS_JUGHAMMER_H
#define GAME_SERVER_WEAPONS_JUGHAMMER_H

#include <game/server/weapon.h>
#include <game/server/entities/laserindicator.h>

/* Hammer but For HunterN Jug */
class CJugHammer : public CWeapon
{
public:
	CJugHammer(CCharacter *pOwnerChar);

	virtual void Tick() override;
	void Fire(vec2 Direction) override;
	int GetType() override { return WEAPON_NINJA; }
	bool IsPowerupOver() override { return false; } // Never give you up

private:
	CLaserIndicator *pIndFirst;
	CLaserIndicator *pIndSecond;
};

#endif // GAME_SERVER_WEAPONS_JUGHAMMER_H
