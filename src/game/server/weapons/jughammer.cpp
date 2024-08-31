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
	m_BounceMode = 0;
	m_AttackEnergyTick = 0;

	m_AttackEnergyLimit = 800;

	for(int i = 0; i < (int)sizeof(m_SnapID); i++)
		m_SnapID[i] = Server()->SnapNewID();
}

CJugHammer::~CJugHammer()
{
	for(int i = 0; i < (int)sizeof(m_SnapID); i++)
		Server()->SnapFreeID(m_SnapID[i]);
}

void CJugHammer::Snap(int SnappingClient, int OtherMode)
{
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

	CNetObj_Laser *pAttackEnergyIndObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[8], sizeof(CNetObj_Laser)));
	if(!pAttackEnergyIndObj)
		return;

	vec2 AttackEnergyInd = CirclePos - (Dir * (minimum(m_AttackEnergyTick * Server()->TickSpeed(), m_AttackEnergyLimit) / 8));

	pAttackEnergyIndObj->m_X = (int)CirclePos.x;
	pAttackEnergyIndObj->m_Y = (int)CirclePos.y;
	pAttackEnergyIndObj->m_FromX = (int)AttackEnergyInd.x;
	pAttackEnergyIndObj->m_FromY = (int)AttackEnergyInd.y;
	pAttackEnergyIndObj->m_StartTick = Server()->Tick() - 3;

	CNetObj_Laser *pBounceTempIndObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_SnapID[9], sizeof(CNetObj_Laser)));
	if(!pBounceTempIndObj)
		return;

	vec2 BounceTempInd = CirclePos + (Dir * 128.f * (maximum(m_BounceTempTick - m_BounceCooldownTick, 0) * 1.f / maximum(m_BounceTempTick, 1)));

	pBounceTempIndObj->m_X = (int)BounceTempInd.x;
	pBounceTempIndObj->m_Y = (int)BounceTempInd.y;
	pBounceTempIndObj->m_FromX = (int)CirclePos.x;
	pBounceTempIndObj->m_FromY = (int)CirclePos.y;
	pBounceTempIndObj->m_StartTick = Server()->Tick() - 3;
}

void CJugHammer::Tick()
{
	CWeapon::Tick();

	CNetObj_PlayerInput *PlayerInput = Character()->GetInput();
	vec2 Dir = normalize(vec2(Character()->GetInput()->m_TargetX, Character()->GetInput()->m_TargetY));
	vec2 CirclePos = Pos() + Dir * 96.f;

	CProjectile *apProj[64];
	int Num = GameWorld()->FindEntities(CirclePos, 96.f, (CEntity **)apProj, 64, CGameWorld::ENTTYPE_PROJECTILE);

	if(PlayerInput->m_Fire & 1)
	{
		m_AttackEnergyTick++;
	}
	else if(Character()->GetPrevInput()->m_Fire & 1)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d, %d", m_AttackEnergyTick * Server()->TickSpeed(), m_AttackEnergyLimit);
		GameServer()->SendChatTarget(-1, aBuf);

		int AttackEnergyTime = minimum(m_AttackEnergyTick * Server()->TickSpeed(), m_AttackEnergyLimit);

		CCharacter *pChr = (CCharacter *)GameWorld()->ClosestEntity(CirclePos, 960.f, CGameWorld::ENTTYPE_CHARACTER, Character());
		if(pChr)
		{
			if(length(pChr->m_Pos - CirclePos) < 96.f) // Hit the Character
			{
				GameWorld()->CreateHammerHit(pChr->m_Pos);
				pChr->TakeDamage(normalize(pChr->m_Pos - Pos()) * 4.f, (AttackEnergyTime * 12.5f) / 1000, Character()->GetPlayer()->GetCID(), WEAPON_HAMMER, GetWeaponID(), false);
			}
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
			pProj->SetDir(normalize(pChr ? pChr->m_Pos - pProj->m_Pos : pProj->m_Pos - Character()->m_Pos) * (AttackEnergyTime * 2.5f / 1000.f));
			pProj->m_LifeSpan = 2 * Server()->TickSpeed();
			pProj->m_IsFreeze = false;
		}

		m_AttackEnergyTick = 0;
	}

	bool IsBounced = false;

	if(!(Character()->GetInput()->m_PlayerFlags & PLAYERFLAG_AIM) && m_BounceCooldownTick >= m_BounceTempTick / 4)
	{
		for(int i = 0; i < Num; ++i)
		{
			CProjectile *pProj = apProj[i];
			
			if(!pProj->m_IsFreeze && pProj->GetOwner() != Character()->GetPlayer()->GetCID())
			{
				GameWorld()->CreateHammerHit(pProj->m_Pos);

				pProj->m_IsFreeze = true;
				IsBounced = true;
				break;
			}
		}
	}

	if(IsBounced)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d, %d", m_BounceCooldownTick, m_BounceTempTick);
		GameServer()->SendChatTarget(-1, aBuf);

		m_BounceTempTick = clamp((m_BounceTempTick - m_BounceCooldownTick - m_BounceTempTick / 4) * 4 + Server()->TickSpeed() / 10, Server()->TickSpeed() / 5, Server()->TickSpeed() * 3);
		m_BounceCooldownTick = 0;
	}
	else
		m_BounceCooldownTick++;
}

void CJugHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);
}