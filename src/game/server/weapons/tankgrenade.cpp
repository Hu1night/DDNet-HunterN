#include "tankgrenade.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>
#include <game/server/weapons/pistol.h>

CTankGrenade::CTankGrenade(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = 8;
	m_AmmoRegenTime = 0;
	m_FireDelay = 2000;
	m_FullAuto = true;

	m_Pos = pOwnerChar->m_Pos;
	m_Angle = GetAngle(Character()->GetAimPos());
	m_GunFireTick = 0;
	m_ForwardTick = 0;

	for(int i = 0; i < SNAP_POINTS; i++)
		m_aSnapID[i] = Server()->SnapNewID();

	UpdateSnapPosOffset(Character()->GetDirection());
}

CTankGrenade::~CTankGrenade()
{
	for(int i = 0; i < SNAP_POINTS; i++)
		Server()->SnapFreeID(m_aSnapID[i]);
}

bool CTankGrenade::GrenadeCollide(CProjectile *pProj, vec2 Pos, CCharacter *pHit, bool EndOfLife)
{
	if(pHit && pHit->GetPlayer()->GetCID() == pProj->GetOwner())
		return false;

	pProj->GameWorld()->CreateSound(Pos, SOUND_GRENADE_EXPLODE);
	pProj->GameWorld()->CreateExplosion(Pos, pProj->GetOwner(), WEAPON_GRENADE, pProj->GetWeaponID(), 0, pProj->GetOwner() < 0);

	return true;
}

void CTankGrenade::UpdateSnapPosOffset(vec2 Dir)
{
	m_aSnapPointsPos[0] = (Dir * 52.f) + (vec2(Dir.y, -Dir.x) * 20.f);
	m_aSnapPointsPos[1] = (Dir * 16.f) + (vec2(Dir.y, -Dir.x) * 56.f);
	m_aSnapPointsPos[2] = (Dir * -56.f) + (vec2(Dir.y, -Dir.x) * 36.f);

	m_aSnapPointsPos[3] = (Dir * -56.f) + (vec2(-Dir.y, Dir.x) * 36.f);
	m_aSnapPointsPos[4] = (Dir * 16.f) + (vec2(-Dir.y, Dir.x) * 56.f);
	m_aSnapPointsPos[5] = (Dir * 52.f) + (vec2(-Dir.y, Dir.x) * 20.f);
}

void CTankGrenade::Snap(int SnappingClient, int OtherMode)
{
	for(int i = 0; i < SNAP_POINTS; i++)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapID[i], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		vec2 From = Pos() + m_aSnapPointsPos[i];
		vec2 To = Pos() + m_aSnapPointsPos[(i < SNAP_POINTS - 1) ? i + 1 : 0];

		pObj->m_X = (int)To.x;
		pObj->m_Y = (int)To.y;
		pObj->m_FromX = (int)From.x;
		pObj->m_FromY = (int)From.y;
		pObj->m_StartTick = Server()->Tick();
	}
}

void CTankGrenade::Tick()
{
	CWeapon::Tick();

	vec2 CharAimPos = Character()->GetAimPos();

	if(Character()->GetInput().m_Hook) // "Remove" hook
		Character()->ResetHook();

	if(Character()->GetInput().m_Hook // Handle move
		&& m_ForwardTick <= 0)
	{
		m_ForwardTick = Server()->TickSpeed() / 10;

		Character()->Core()->m_Vel = Character()->GetDirection() * 10;
		GameServer()->Collision()->MoveBox(&m_Pos, &Character()->Core()->m_Vel, vec2(28.f, 28.f), 0.f);
		Character()->Core()->m_Pos = m_Pos;

		GameWorld()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);

		Character()->Core()->m_Vel = vec2(0.f, -0.5f); // Let the client do predict stuff
	}
	else
	{
		m_ForwardTick--;

		Character()->Core()->m_Pos = m_Pos; // cant move
		Character()->Core()->m_Vel = vec2(0.f, -0.5f); // Reset m_Vel
	}

	if(Character()->GetInput().m_Jump) // Second weapon
	{
		if(m_GunFireTick <= 0)
		{
			m_GunFireTick = Server()->TickSpeed() / 10;
			GunFire(normalize(CharAimPos));
		}
		else
			m_GunFireTick--;
	}

	{
		float DiffAngle = ClampAngle(m_Angle - GetAngle(CharAimPos)); // Rotating Turret
		float RotateSpeedPerTick = m_RotateSpeed / Server()->TickSpeed();
		m_Angle = ClampAngle(clamp(m_Angle - DiffAngle, m_Angle - RotateSpeedPerTick, m_Angle + RotateSpeedPerTick)); // Rotating
		UpdateSnapPosOffset(direction(m_Angle));
	}
}

void CTankGrenade::GunFire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	int Lifetime = Character()->CurrentTuning()->m_GunLifetime * Server()->TickSpeed();

	vec2 ProjStartPos = Pos() + direction(m_Angle) * 48.f;

	CProjectile *pProj = new CProjectile(
		GameWorld(),
		WEAPON_GUN, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		ProjStartPos, //Pos
		Direction, //Dir
		6.0f, // Radius
		Lifetime, //Span
		CPistol::BulletCollide);

	// pack the Projectile and send it to the client Directly
	CNetObj_Projectile p;
	pProj->FillInfo(&p);

	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(1);
	for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
		Msg.AddInt(((int *)&p)[i]);

	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	GameWorld()->CreateSound(Character()->m_Pos, SOUND_GUN_FIRE);
}

void CTankGrenade::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	int Lifetime = Character()->CurrentTuning()->m_GrenadeLifetime * Server()->TickSpeed();

	vec2 Dir = direction(m_Angle);
	vec2 ProjStartPos = Pos() + direction(m_Angle) * 48.f;

	CProjectile *pProj = new CProjectile(
		GameWorld(),
		WEAPON_GRENADE, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		ProjStartPos, //Pos
		Dir * 5, //Dir
		6.0f, // Radius
		Lifetime, //Span
		GrenadeCollide);

	// pack the Projectile and send it to the client Directly
	CNetObj_Projectile p;
	pProj->FillInfo(&p);

	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(1);
	for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
		Msg.AddInt(((int *)&p)[i]);

	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	GameWorld()->CreateSound(Character()->m_Pos, SOUND_GRENADE_FIRE);
}
