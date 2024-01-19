/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "multibomb.h"

#include <game/server/entities/character.h>
#include <game/server/weapons.h>

CGameControllerMultiBomb::CGameControllerMultiBomb() :
	IGameController()
{
	m_pGameType = "bombS"; // len limited :(
	m_GameFlags = IGF_SURVIVAL | IGF_SUDDENDEATH;
}

void CGameControllerMultiBomb::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->IncreaseHealth(10);

	pChr->SetPowerUpWeapon(WEAPON_ID_BOMBHAMMER, 0);

	if(!IsGameRunning() && !m_BoomerNum)
		return;

	for(int i = 0; i < m_BoomerNum; i++)
	{
		if(m_BoomerCID[i] == pChr->GetPlayer()->GetCID())
		{
			((CBombHammer *)pChr->GetPowerupWeapon())->m_nextRoundtick = (rand() % Server()->TickSpeed()) + 1;
			((CBombHammer *)pChr->GetPowerupWeapon())->m_IsActive = true; // 启动炸弹
		}
	}
}

bool CGameControllerMultiBomb::OnEntity(int Index, vec2 Pos, int Layer, int Flags, int Number)
{
	// bypass pickups
	if(Index >= ENTITY_ARMOR_1 && Index <= ENTITY_WEAPON_LASER)
		return true;
	return false;
}

void CGameControllerMultiBomb::OnWorldReset()
{
	m_BoomerNum = (m_aTeamSize[TEAM_RED] - 2) / 5 + 1;
	m_BoomerCID[m_BoomerNum - 1] = {0};

	if(!m_BoomerNum)
		return;

	for(int j = 0; j < m_BoomerNum; ++j)
	{
		int rBoomer = (rand() % m_BoomerNum) + 1;

		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
				(!pPlayer->m_RespawnDisabled ||
					(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
			{
				if(!rBoomer--)
					m_BoomerCID[j] = pPlayer->GetCID(); // 缓存CID 只用在OnCharaSpawn上
			}
		}
	}
}

void CGameControllerMultiBomb::DoWincheckMatch()
{
	if(!m_SuddenDeath && m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) >= m_GameInfo.m_TimeLimit * Server()->TickSpeed() * 60)
		EndMatch();
}

int CGameControllerMultiBomb::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	CWeapon *pWeapon = pVictim->GetPowerupWeapon();
	if(!pWeapon || pWeapon->GetWeaponID() != WEAPON_ID_BOMBHAMMER || !((CBombHammer *)pWeapon)->m_IsActive) // 是炸弹人爆了吗
		return DEATH_SKIP_SCORE; // 跳过内置分数逻辑

	int AlivePlayerCount = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			++AlivePlayerCount;
			pPlayer->m_Score++; // 顺手加个分
		}
	}

	if(AlivePlayerCount <= m_BoomerNum * 2) // 人数不够的话 就不选炸弹人了
	{
		if(AlivePlayerCount <= m_BoomerNum) // 炸弹人炸光了
			EndMatch();
		return DEATH_SKIP_SCORE;
	}

	int rBoomer = (rand() % AlivePlayerCount) + 1; // 选择新的炸弹人
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			if(!rBoomer--)
			{
				((CBombHammer *)pPlayer->GetCharacter()->GetPowerupWeapon())->m_Roundleft = 21; // 重置
				((CBombHammer *)pPlayer->GetCharacter()->GetPowerupWeapon())->m_nextRoundtick = (rand() % Server()->TickSpeed()) + 1;
				((CBombHammer *)pPlayer->GetCharacter()->GetPowerupWeapon())->m_IsActive = true; // 启动他的炸弹
			}
		}
	}
	return DEATH_SKIP_SCORE;
}
