#ifndef GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H
#define GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H

#include <game/server/weapon.h>

class CPuppeteeHammer : public CWeapon
{
public:
	CPuppeteeHammer(CCharacter *pOwnerChar);

	void Tick() override;
	void Fire(vec2 Direction) override;
	int GetType() override { return WEAPON_HAMMER; }
	bool IsPowerupOver() { return false; }
private:
	enum { MAX_PUPPETS = 4, };
	int m_aPuppetCID[MAX_PUPPETS];
	vec2 m_aPuppetsDirPos[MAX_PUPPETS];
	bool m_IsKeepFiring;
	int m_PrevTickPuppets;
};

#endif // GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H
