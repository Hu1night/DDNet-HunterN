#ifndef GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H
#define GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H

#include <game/server/weapon.h>

class CPuppeteeHammer : public CWeapon
{
public:
	CPuppeteeHammer(CCharacter *pOwnerChar);
	~CPuppeteeHammer();

	void Tick() override;
	void Fire(vec2 Direction) override;
	int GetType() override { return WEAPON_HAMMER; }
	bool IsPowerupOver() { return false; }
private:
	enum { MAX_PUPPETS = 4, };
	int m_aPuppetCID[MAX_PUPPETS];
	vec2 m_aPuppetDirPos[MAX_PUPPETS];
	bool m_aPuppetWeapon[MAX_PUPPETS];
	bool m_IsKeepFiring;
	int m_PrevPuppetCount;
	int m_WeaponSlot;
};

#endif // GAME_SERVER_WEAPONS_PUPPETEEHAMMER_H
