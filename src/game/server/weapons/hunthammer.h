#ifndef GAME_SERVER_WEAPONS_HUNTHAMMER_H
#define GAME_SERVER_WEAPONS_HUNTHAMMER_H

#include <game/server/weapon.h>

class CHuntHammer : public CWeapon
{
public:
	CHuntHammer(CCharacter *pOwnerChar);
	~CHuntHammer(){ Server()->SnapFreeID(IndicatorSnapID); }

	void Fire(vec2 Direction) override;
	void Tick() override;
	void Snap(int SnappingClient, int OtherMode) override;
	int GetType() override { return WEAPON_HAMMER; }

private:
	int IndicatorSnapID;
};

#endif // GAME_SERVER_WEAPONS_HAMMER_H
