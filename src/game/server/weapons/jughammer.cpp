#include "jughammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CJugHammer::CJugHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Firedelay;

	m_BounceTempTick = 5;
	m_BounceCooldownTick = 0;
	m_AttackEnergyTick = 0;

	m_aBounceTypeTemp[0] = Server()->TickSpeed() / 4;
	m_aBounceTypeTemp[1] = Server()->TickSpeed() / 5;
	m_aBounceTypeTemp[2] = Server()->TickSpeed() * 3;
	m_AttackEnergyLimit = 800;

	for(unsigned int i = 0; i < sizeof(m_SnapID) / sizeof(m_SnapID[0]); i++)
		m_SnapID[i] = Server()->SnapNewID();
}

CJugHammer::~CJugHammer()
{
	for(auto ID : m_SnapID)
		Server()->SnapFreeID(ID);
}

void CJugHammer::Snap(int SnappingClient, int OtherMode)
{
	// Do attack range ind
	vec2 Dir = normalize(vec2(Character()->GetInput()->m_TargetX, Character()->GetInput()->m_TargetY));
	vec2 CirclePos = Pos() + Dir * 96.f;
	vec2 CirclePoints[8] =
		{CirclePos + vec2(96.f, 0.f),
		CirclePos + vec2(67.9f, 67.9f),
		CirclePos + vec2(0.f, 96.f),
		CirclePos + vec2(-67.9f, 67.9f),
		CirclePos + vec2(-96.f, 0.f),
		CirclePos + vec2(-67.9f, -67.9f),
		CirclePos + vec2(0.f, -96.f),
		CirclePos + vec2(67.9f, -67.9f),};

	for(int i = 0; i < 8; i++)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[i], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_X = (int)CirclePoints[i].x;
		pObj->m_Y = (int)CirclePoints[i].y;
		pObj->m_FromX = (int)CirclePoints[(i + 1) % 8].x;
		pObj->m_FromY = (int)CirclePoints[(i + 1) % 8].y;
		pObj->m_StartTick = Server()->Tick() - 2;
	}

	{
		// Do attack energy ind
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[8], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		vec2 AttackEnergyInd = CirclePos - (Dir * (clamp(m_AttackEnergyTick * Server()->TickSpeed() - 200, 0, m_AttackEnergyLimit) / 8));

		pObj->m_X = (int)CirclePos.x;
		pObj->m_Y = (int)CirclePos.y;
		pObj->m_FromX = (int)AttackEnergyInd.x;
		pObj->m_FromY = (int)AttackEnergyInd.y;
		pObj->m_StartTick = Server()->Tick() - 3;
	}

	{
		// Do bounce temperature ind
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[9], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		vec2 BounceTempInd = CirclePos + (Dir * 128.f * (maximum(m_BounceTempTick - m_BounceCooldownTick, 0) * 1.f / maximum(m_BounceTempTick, 1)));

		pObj->m_X = (int)BounceTempInd.x;
		pObj->m_Y = (int)BounceTempInd.y;
		pObj->m_FromX = (int)CirclePos.x;
		pObj->m_FromY = (int)CirclePos.y;
		pObj->m_StartTick = Server()->Tick() - 3;
	}

	{
		// Do character track
		if(SnappingClient != Character()->GetPlayer()->GetCID())
			return;

		CEntity *IndCharacter = GameWorld()->ClosestEntity(Pos(), 8192.f, CGameWorld::ENTTYPE_CHARACTER, Character());
		if(!IndCharacter)
			return;

		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[10], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		vec2 PlayerTrackInd = Pos() + vec2(0.f, -16.f) + normalize(IndCharacter->m_Pos - Pos() + vec2(0.f, -16.f)) * 128.f;

		pObj->m_X = (int)PlayerTrackInd.x;
		pObj->m_Y = (int)PlayerTrackInd.y;
		pObj->m_FromX = (int)Pos().x;
		pObj->m_FromY = (int)(Pos().y - 16.f);
		pObj->m_StartTick = Server()->Tick() - 3;
	}
}

void CJugHammer::Tick()
{
	CWeapon::Tick();

	CNetObj_PlayerInput *PlayerInput = Character()->GetInput();
	vec2 Dir = normalize(vec2(Character()->GetInput()->m_TargetX, Character()->GetInput()->m_TargetY));
	vec2 CirclePos = Pos() + Dir * 96.f;

	CProjectile *apProj[64];
	int Num = GameWorld()->FindEntities(CirclePos, 128.f, (CEntity **)apProj, 64, CGameWorld::ENTTYPE_PROJECTILE);

	int AttackDmg = maximum(((minimum(m_AttackEnergyTick * Server()->TickSpeed(), m_AttackEnergyLimit) - 200) * 17) / 1000, 0);
	bool IsBounced = false;
	int HitProj;

	if(PlayerInput->m_Fire & 1)
	{
		m_AttackEnergyTick++;
	}
	else if(Character()->GetPrevInput()->m_Fire & 1)
	{
		CCharacter *pChr = (CCharacter *)GameWorld()->ClosestEntity(CirclePos, 1280.f, CGameWorld::ENTTYPE_CHARACTER, Character());

		if(pChr && length(pChr->m_Pos - CirclePos) < 96.f)
		{
			GameWorld()->CreateHammerHit(pChr->m_Pos);
			pChr->TakeDamage(normalize(pChr->m_Pos - Pos()) * 4.f, AttackDmg, Character()->GetPlayer()->GetCID(), WEAPON_HAMMER, GetWeaponID(), false);
		}

		for(int i = 0; i < Num; ++i)
		{
			CProjectile *pProj = apProj[i];
			
			if(!pProj->m_IsFreeze && pProj->GetOwner() != Character()->GetPlayer()->GetCID())
				continue;

			GameWorld()->CreateHammerHit(pProj->m_Pos);

			pProj->SetStartPos(pProj->m_Pos);
			pProj->SetStartTick(Server()->Tick());
			pProj->SetOwner(Character()->GetPlayer()->GetCID());
			pProj->SetDir(normalize(pChr ? pChr->m_Pos - pProj->m_Pos : pProj->m_Pos - Character()->m_Pos) * 2.f);
			pProj->m_LifeSpan = 2 * Server()->TickSpeed();
			pProj->m_IsFreeze = false;
		}

		m_AttackEnergyTick = 0;
	}
	else if(m_BounceCooldownTick >= m_BounceTempTick / 4)
	{
		for(HitProj = 0; HitProj < Num; ++HitProj)
		{
			CProjectile *pProj = apProj[HitProj];
			
			if(!pProj->m_IsFreeze && pProj->GetOwner() != Character()->GetPlayer()->GetCID())
			{
				GameWorld()->CreatePlayerSpawn(pProj->m_Pos);

				pProj->m_IsFreeze = true;
				IsBounced = true;
				break;
			}
		}
	}

	if(IsBounced)
	{
		m_BounceTempTick = m_aBounceTypeTemp[apProj[HitProj]->m_Type - 1] + clamp(
			(m_BounceTempTick - m_BounceCooldownTick) * 2,
			0,
			Server()->TickSpeed() * 2);
		m_BounceCooldownTick = 0;
	}
	else
	{
		m_BounceCooldownTick++;
	}

	char aBuf[96];
	str_format(aBuf, sizeof(aBuf), "子弹截停冷却: %d / %d\n近战蓄力伤害: %d\n辅助线榴弹跳: %d",
		maximum(m_BounceTempTick / 4 - m_BounceCooldownTick, 0),
		maximum(m_BounceTempTick - m_BounceCooldownTick, 0),
		AttackDmg,
		m_ReloadTimer);

	GameServer()->SendBroadcast(aBuf, Character()->GetPlayer()->GetCID(), false);
}

void CJugHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();

	if(Character()->GetInput()->m_PlayerFlags & PLAYERFLAG_AIM)
	{
		GameWorld()->CreateSound(Pos(), SOUND_GRENADE_EXPLODE);
		GameWorld()->CreateExplosion(Pos() + Direction * 24.f, Character()->GetPlayer()->GetCID(), WEAPON_GRENADE, GetWeaponID(), 0, false);

		m_ReloadTimer = Server()->TickSpeed();
	}
	else
		GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);
}