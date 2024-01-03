#include "jughammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CJugHammer::CJugHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Ammoregentime;
	m_FireDelay = 250;
	m_FullAuto = true;

	pIndFirst = new CLaserIndicator(
		GameWorld(),
		Character()->GetPlayer()->GetCID(),
		Pos() + Character()->GetDirection() * GetProximityRadius() * 7.25f,
		Pos() + Character()->GetDirection() * GetProximityRadius(), /* 1.f*/
		3);

	pIndSecond = new CLaserIndicator(
		GameWorld(),
		Character()->GetPlayer()->GetCID(),
		Pos() - Character()->GetDirection() * GetProximityRadius() * 1.75f, // negative
		Pos() + Character()->GetDirection() * GetProximityRadius() * 4.5f,
		2);
}

void CJugHammer::Tick() // laser indicator snap
{
	CWeapon::Tick();

	pIndFirst->m_From = Pos() + Character()->GetDirection() * GetProximityRadius() * 6.75f;
	pIndFirst->m_Pos = Pos() + Character()->GetDirection() * GetProximityRadius() /* 1.f*/;

	pIndSecond->m_From = Pos() - Character()->GetDirection() * GetProximityRadius() * 1.25f; // negative
	pIndSecond->m_Pos = Pos() + Character()->GetDirection() * GetProximityRadius() * 4.5f;
}

void CJugHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);

	if(Character()->IsSolo() || Character()->m_Hit & CCharacter::DISABLE_HIT_HAMMER)
		return;

	vec2 HammerHitPos = Pos() + Direction * GetProximityRadius() * 2.75f;

	int Hits = 0;
	CProjectile *apEntsProj[16];
	int ProjNum = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 4.5f, (CEntity **)apEntsProj,
		16, CGameWorld::ENTTYPE_PROJECTILE);

	for(int i = 0; i < ProjNum; ++i)
	{
		CProjectile *pTargetProj = apEntsProj[i];

		if((pTargetProj->m_Type != WEAPON_GRENADE) ||
			(distance(pTargetProj->m_Pos, HammerHitPos) < GetProximityRadius() * 1.8f))
		{
			GameWorld()->CreateHammerHit(pTargetProj->m_Pos);

			pTargetProj->SetStartPos(pTargetProj->m_Pos);
			pTargetProj->SetStartTick(Server()->Tick());
			pTargetProj->SetOwner(Character()->GetPlayer()->GetCID());
			pTargetProj->SetDir(normalize(pTargetProj->m_Pos - Character()->m_Pos) * 0.6f);
			pTargetProj->m_LifeSpan = 2 * Server()->TickSpeed();

			Hits++;
		}
	}

	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 0.75f, (CEntity **)apEnts,
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

		pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, (!(ProjNum || Hits) && (Character()->GetCore().m_HookedPlayer == pTarget->GetPlayer()->GetCID())) ? 20 : 4,
			ClientID, WEAPON_HAMMER, GetWeaponID(), false);

		GameServer()->Antibot()->OnHammerHit(ClientID);

		Hits++;
	}

	// if we Hit anything, we have to wait for the reload
	if(Hits)
		m_ReloadTimer = Server()->TickSpeed() * 120 / 1000;
}