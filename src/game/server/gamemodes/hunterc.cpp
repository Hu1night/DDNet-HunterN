/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "hunterc.h"

#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/weapons.h>
//#include <game/server/entities/pickup.h>
#include <game/server/classes.h>

CGameControllerHunterC::CGameControllerHunterC() :
	CGameControllerHunterN()
{
	m_pGameType = "hunterC";

	m_apClassSpawnMsg[0] = "你是平民Civic! 守旗并消灭猎人以胜利!     \n猎人双倍伤害 有瞬杀锤子和高爆榴弹";
	m_apClassSpawnMsg[1] = "     你是猎人Hunter! 抢占旗帜以胜利!\n     猎人双倍伤害 有瞬杀锤子和高爆榴弹";

	m_pFlag = nullptr;
}

void CGameControllerHunterC::OnGameStart(bool IsRound)
{
	m_GameFlags = HUNTERN_GAMEFLAGS;
	m_DoWinchenkClassTick = -1;
	
	int PreselectPlayerCount = 0; // 最近没当过猎人的玩家的计数
	int rHunter = 0; // 猎人选择随机数

	for(int i = 0; i < MAX_CLIENTS; ++i) // 重置并计数玩家
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(!pPlayer || pPlayer->GetTeam() == TEAM_SPECTATORS
			|| (pPlayer->m_RespawnDisabled && (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())))
			continue;

		pPlayer->m_AmongUsTeam = TEAM_RED; // 重置队伍
		pPlayer->m_HiddenScore = 0; // 重置隐藏分
		pPlayer->m_UseHunterWeapon = false; // 默认武器
		pPlayer->m_Class = CLASS_CIVIC; // 重置玩家为平民
		if(pPlayer->m_Preselect) // 猎人选择伪随机！我们要在m_Preselect的玩家里面选择猎人
			++PreselectPlayerCount; // 计数有PreselectPlayerCount个玩家
	}

	if(m_aTeamSize[TEAM_RED] < m_MinimumPlayers) // m_aTeamSize[TEAM_RED] = 非队伍模式的人数量
		return;

	m_NumHunter = (m_aTeamSize[TEAM_RED] - 2) / m_HunterRatio + 1; // 我们要多少个猎人
	str_format(m_HunterList, sizeof(m_HunterList), "本回合的 %d 个Hunter是: ", m_NumHunter); // Generate Hunter info message 生成猎人列表消息头

	SendChatTarget(-1, "——————欢迎来到HunterN猎人杀——————");
	//MatchFlag = -1; // 要成为Jug的玩家不存在 撤了
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "本回合有 %d 个猎人Hunter has been selected.", m_NumHunter);
	SendChatTarget(-1, aBuf);
	SendChatTarget(-1, "规则：每回合秘密抽选猎人 猎人对战平民 活人看不到死人消息");
	SendChatTarget(-1, "      猎人双倍伤害 有瞬杀锤子(平民无锤)和破片榴弹(对自己无伤)");
	SendChatTarget(-1, "分辨队友并消灭敌人来取得胜利！Be warned! Sudden Death.");

	for(int iHunter = m_NumHunter; iHunter > 0; --iHunter) // 需要选择m_NumHunter个猎人
	{
		if(PreselectPlayerCount <= 0) // 先检查m_Preselect的玩家够不够（即所有玩家是不是最近都当过猎人了） 如果不够就重置所有玩家的m_Preselect
		{
			for(int i = 0; i < MAX_CLIENTS; ++i) // 重置所有玩家的m_Preselect
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(!pPlayer || pPlayer->GetTeam() == TEAM_SPECTATORS
					|| (pPlayer->m_RespawnDisabled && (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())))
					continue;

				pPlayer->m_Preselect = true; // 设置m_Preselect为真（包括猎人 主要是为了随机性）
				if(pPlayer->m_Class == CLASS_CIVIC) // 只有平民才可以被计数
					++PreselectPlayerCount; // 重置PreselectPlayerCount
			}
		}

		rHunter = rand() % PreselectPlayerCount; // 在PreselectPlayerCount个玩家里选择第rHunter个猎人

		for(int i = 0; i < MAX_CLIENTS; ++i) // 在PreselectPlayerCount个玩家里选择第rHunter个玩家为猎人
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(!pPlayer || pPlayer->GetTeam() == TEAM_SPECTATORS
				|| (pPlayer->m_RespawnDisabled && (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive()))
				|| !pPlayer->m_Preselect || pPlayer->m_Class != CLASS_CIVIC) // 在平民职业的玩家里面选择猎人
				continue; // 首先剔除不在选择队列里的玩家

			if(rHunter != 0) // 计数玩家
			{
				rHunter--;
				continue;
			}

			pPlayer->m_Class = CLASS_HUNTER; // 设置猎人Flag
			//pPlayer->m_UseHunterWeapon = true; // 使用猎人武器 // 在OnCharachar里面设置
			pPlayer->m_AmongUsTeam = TEAM_BLUE; // 设置队伍
			pPlayer->m_Preselect = false; // 把m_Preselect设为否 即最近当过猎人
			--PreselectPlayerCount;

			// Generate Hunter info message 生成猎人列表消息
			str_append(m_HunterList, Server()->ClientName(i), sizeof(m_HunterList));
			str_append(m_HunterList, ", ", sizeof(m_HunterList));

			break;
		}
	}

	for(int i = 0; i < MAX_CLIENTS; ++i) // 循环所有旁观者 把猎人列表告诉他们
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(!pPlayer)
			continue;

		if(pPlayer->GetTeam() == TEAM_SPECTATORS)
			SendChatTarget(pPlayer->GetCID(), m_HunterList); // 给膀胱者发
		else if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()) // 这个玩家出生了 所以OnCharacterSpawn已经过了
			OnResetClass(pPlayer->GetCharacter()); // 在这里给他们Class提示和武器
	}

	InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", m_HunterList);
}
