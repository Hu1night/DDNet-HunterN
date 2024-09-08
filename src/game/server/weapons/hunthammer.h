#ifndef GAME_SERVER_WEAPONS_HUNTHAMMER_H
#define GAME_SERVER_WEAPONS_HUNTHAMMER_H

#include <game/server/weapon.h>

class CHuntHammer : public CWeapon
{
public:
	CHuntHammer(CCharacter *pOwnerChar);
	~CHuntHammer() { if(m_SnapID)Server()->SnapFreeID(m_SnapID); }

	void Fire(vec2 Direction) override;
	void Snap(int SnappingClient, int OtherMode) override;
	int GetType() override { return WEAPON_HAMMER; }

protected:
	int m_SnapID;
};

#endif // GAME_SERVER_WEAPONS_HAMMER_H
