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
	int m_Duration;
	class CCharacter *m_apHitObjects[MAX_CLIENTS];

public:
	CJugNinja(CCharacter *pOwnerChar);

	void Tick() override;
	void Fire(vec2 Direction) override;
	bool IsPowerupOver() override { return false; };
	void OnUnequip() override;
	int GetType() override { return WEAPON_NINJA; }
	void OnGiven(bool IsAmmoFillUp) override;
};

#endif // GAME_SERVER_WEAPONS_NINJA_H
