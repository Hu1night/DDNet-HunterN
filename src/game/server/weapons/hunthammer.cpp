#include "hunthammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CHuntHammer::CHuntHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Firedelay;

	IndicatorSnapID = Server()->SnapNewID();
}

void CHuntHammer::Snap(int SnappingClient, int OtherMode)
{
	if(Character()->GetPlayer()->GetCID() != SnappingClient)
		return;

	if(!Character()->m_IsFiring || m_ReloadTimer > 1)
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, IndicatorSnapID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	CEntity *IndCharacter = GameWorld()->ClosestEntity(Pos(), 8192, CGameWorld::ENTTYPE_CHARACTER, Character());
	if(!IndCharacter)
		return;

	vec2 IndicatorFrom = Pos();
	vec2 IndicatorTo = IndicatorFrom + normalize(IndCharacter->m_Pos - IndicatorFrom) * 120;

	pObj->m_X = (int)IndicatorTo.x;
	pObj->m_Y = (int)IndicatorTo.y;
	pObj->m_FromX = (int)IndicatorFrom.x;
	pObj->m_FromY = (int)IndicatorFrom.y;
	pObj->m_StartTick = Server()->Tick() - 3;
}

void CHuntHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);

	if(Character()->IsSolo() || Character()->m_Hit & CCharacter::DISABLE_HIT_HAMMER)
		return;

	vec2 HammerHitPos = Pos() + Direction * GetProximityRadius() * 0.75f;

	CCharacter *apEnts[MAX_CLIENTS];
	int Hits = 0;
	int Num = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 0.5f, (CEntity **)apEnts,
		MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for(int i = 0; i < Num; ++i)
	{
		CCharacter *pTarget = apEnts[i];

		//if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
		if((pTarget == Character() || (pTarget->IsAlive() && pTarget->IsSolo())))
			continue;

		// set his velocity to fast upward (for now)
		if(length(pTarget->m_Pos - HammerHitPos) > 0.0f)
			GameWorld()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - HammerHitPos) * GetProximityRadius() * 0.5f);
		else
			GameWorld()->CreateHammerHit(HammerHitPos);

		vec2 Dir;
		if(length(pTarget->m_Pos - Pos()) > 0.0f)
			Dir = normalize(pTarget->m_Pos - Pos());
		else
			Dir = vec2(0.f, -1.f);

		float Strength = Character()->CurrentTuning()->m_HammerStrength;

		vec2 Temp = pTarget->Core()->m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
		Temp = ClampVel(pTarget->m_MoveRestrictions, Temp);
		Temp -= pTarget->Core()->m_Vel;

		pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, 20, ClientID, WEAPON_HAMMER, GetWeaponID(), false);

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