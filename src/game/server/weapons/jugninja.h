#ifndef GAME_SERVER_WEAPONS_JUGNINJA_H
#define GAME_SERVER_WEAPONS_JUGNINJA_H

#include <game/server/weapon.h>

class CJugNinja : public CWeapon
{
private:
	vec2 m_ActivationDir;
	int m_CurrentMoveTime;
	float m_OldVelAmount;
	int m_NumObjectsHit;
	class CCharacter *m_apHitObjects[MAX_CLIENTS];

	int m_SnapID[2];
	bool IsIndicatorSnap;
	vec2 m_IndPos[3];

public:
	CJugNinja(CCharacter *pOwnerChar);
	~CJugNinja() { if(m_SnapID[0])Server()->SnapFreeID(m_SnapID[0]); if(m_SnapID[1])Server()->SnapFreeID(m_SnapID[1]); }

	void Tick() override;
	void Fire(vec2 Direction) override;
	bool IsPowerupOver() override { return false; };
	void OnUnequip() override;
	int GetType() override { return WEAPON_NINJA; }
	void Snap(int SnappingClient, int Othermode) override;
};

#endif // GAME_SERVER_WEAPONS_NINJA_H
