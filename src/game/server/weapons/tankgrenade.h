#ifndef GAME_SERVER_WEAPONS_TANKGRENATE_H
#define GAME_SERVER_WEAPONS_TANKGRENATE_H

#include <game/server/weapon.h>

class CTankGrenade : public CWeapon
{
public:
	CTankGrenade(CCharacter *pOwnerChar);
	~CTankGrenade();

	void Snap(int SnappingClient, int OtherMode) override;
	void Tick() override;
	void Fire(vec2 Direction) override;
	int GetType() override { return Character()->m_IsFiring ? WEAPON_GRENADE : WEAPON_GUN; }
	bool IsPowerupOver() { return false; }

	// callback
	static bool GrenadeCollide(class CProjectile *pProj, vec2 Pos, CCharacter *pHit, bool EndOfLife);
private:
	vec2 m_Pos;
	float m_Angle;

	int m_GunFireTick;
	int m_ForwardTick;

	// Snap
	enum { SNAP_POINTS = 6, };
	int m_aSnapID[SNAP_POINTS];
	vec2 m_aSnapPointsPos[SNAP_POINTS];
	inline void UpdateSnapPosOffset(vec2 Dir);

	float ClampAngle(float a) { return a > pi ? a - pi * 2 : (a < -pi ? a + pi * 2 : a); }
	float GetAngle(vec2 a)
	{
		if(a.x == 0 && a.y == 0)
			return 0.0f;
		else if(a.x == 0)
			return a.y < 0 ? -pi / 2 : pi / 2;
		float result = atanf(a.y / a.x);
		if(a.x < 0)
			result = a.y > 0 ? result - pi : result + pi;
		return result;
	}

	inline void GunFire(vec2 Direction);

	const float m_RotateSpeed = 0.34906585039886591538473814444444f; // 40/360 per sec
};

#endif // GAME_SERVER_WEAPONS_GRENATE_H
