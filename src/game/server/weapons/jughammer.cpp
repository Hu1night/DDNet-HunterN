#include "jughammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>
#include <game/server/entities/laser.h>

CJugHammer::CJugHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Firedelay;
	m_FullAuto = true;
}

void CJugHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);

	if(Character()->IsSolo() || Character()->m_Hit & CCharacter::DISABLE_HIT_HAMMER)
		return;

	vec2 HammerHitPos = Pos() + Direction * GetProximityRadius() * 2.75f;

	new CLaser(
		GameWorld(),
		WEAPON_GUN, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		Pos() + Direction * GetProximityRadius() * 4.5f, //Pos
		-(Direction), //Dir
		105); // StartEnergy

	new CLaser(
		GameWorld(),
		WEAPON_GUN, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		Pos() + Direction * GetProximityRadius() * 6.75f, //Pos
		-(Direction), //Dir
		67); // StartEnergy

	int Hits = 0;
	CProjectile *apEntsProj[16];
	int Num = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 4.0f, (CEntity **)apEntsProj,
		16, CGameWorld::ENTTYPE_PROJECTILE);

	for(int i = 0; i < Num; ++i)
	{
		CProjectile *pTargetProj = apEntsProj[i];

		if((pTargetProj->m_Type != WEAPON_GRENADE) || (distance(pTargetProj->m_Pos, HammerHitPos) < 52.5f))
		{
			GameWorld()->CreateHammerHit(pTargetProj->m_Pos);

			pTargetProj->SetStartPos(pTargetProj->m_Pos);
			pTargetProj->SetStartTick(Server()->Tick());
			pTargetProj->SetOwner(Character()->GetPlayer()->GetCID());
			pTargetProj->SetDir(normalize(pTargetProj->m_Pos - Character()->m_Pos) * 0.5f);
			pTargetProj->m_LifeSpan = 2 * Server()->TickSpeed();

			Hits++;
		}
	}

	CCharacter *apEnts[MAX_CLIENTS];
	Num = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 0.75f, (CEntity **)apEnts,
		MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for(int i = 0; i < Num; ++i)
	{
		CCharacter *pTarget = apEnts[i];

		//if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
		if((pTarget == Character() || (pTarget->IsAlive() && pTarget->IsSolo())))
			continue;

		vec2 Dir;
		if(length(pTarget->m_Pos - Pos()) > 0.0f)
			Dir = normalize(pTarget->m_Pos - Pos());
		else
			Dir = vec2(0.f, -1.f);

		float Strength = Character()->CurrentTuning()->m_HammerStrength;

		vec2 Temp = pTarget->Core()->m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
		Temp = ClampVel(pTarget->m_MoveRestrictions, Temp);
		Temp -= pTarget->Core()->m_Vel;

		if(!Hits)
		{
			GameWorld()->CreateExplosionParticle(pTarget->m_Pos - Direction);
			GameWorld()->CreateSound(pTarget->m_Pos, SOUND_GRENADE_EXPLODE);
		}
		else
			GameWorld()->CreateHammerHit(pTarget->m_Pos - Direction);

		pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, (!Hits) ? 20 : 6, // Hunter
			ClientID, WEAPON_HAMMER, GetWeaponID(), false);

		GameServer()->Antibot()->OnHammerHit(ClientID);

		Hits++;
	}

	// if we Hit anything, we have to wait for the reload
	if(Hits)
	{
		float FireDelay;
		int TuneZone = Character()->m_TuneZone;
		if(!TuneZone)
			FireDelay = GameServer()->Tuning()->m_HammerHitFireDelay;
		else
			FireDelay = GameServer()->TuningList()[TuneZone].m_HammerHitFireDelay;
		m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
	}
}