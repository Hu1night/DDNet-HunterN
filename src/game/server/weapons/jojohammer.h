#ifndef GAME_SERVER_WEAPONS_JOJOHAMMER_H
#define GAME_SERVER_WEAPONS_JOJOHAMMER_H

#include <game/server/weapon.h>

class CJOJOHammer : public CWeapon
{
public:
	CJOJOHammer(CCharacter *pOwnerChar);

	void Fire(vec2 Direction) override;
	int GetType() override { return WEAPON_HAMMER; }
};

#endif // GAME_SERVER_WEAPONS_JOJOHAMMER_H
