/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "huntern.h"
#include <game/server/entities/character.h>
#include <game/server/weapons.h>
//#include <game/server/entities/pickup.h>
#include <game/server/classes.h>

// HunterN commands
static void ConMapAdd(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	char aBuf[256];

	if(pResult->NumArguments() > 0) // 如果要加入地图
	{
		for(int j = 0; j < pResult->NumArguments(); ++j) // 逐个加入地图到列表
		{
			const char *pMapName = pResult->GetString(j);

			int IsError = 2; // 2 = 地图循环没有空位
			int i;
			for(i = 0; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i) // 遍历地图循环列表
			{
				if(!pSelf->m_aMaprotation[i]) // 在列表里找到了可用空位
				{
					int MapIndex = pSelf->GameServer()->Teams()->GetMapIndex(pMapName);
					if(MapIndex == 0)
					{
						IsError = 1; // 1 = 未找到地图
						break; // 跳出到错误提示
					}

					pSelf->m_aMaprotation[i] = MapIndex; // 加入循环列表
					str_format(aBuf, sizeof(aBuf), "Add map%d '%s' to slot %d", j, pMapName, i);
					pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);
					IsError = 0; // 0 = 没问题
				}
			}

			if(IsError)
			{
				str_format(aBuf, sizeof(aBuf), "Cannot add map%d '%s' to slot %d (%s)", j, pMapName, i, IsError ? "No Map Found" : "Out of Range");
				pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);
			}
		}
	}
	else // 显示列表
	{
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", "Value: ");

		for(int i = 0; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i) // 遍历地图循环列表
		{
			if(!pSelf->m_aMaprotation[i]) // 列表走到了尽头
				break;

			const char *pMapName = pSelf->GameServer()->Teams()->GetMapName(pSelf->m_aMaprotation[i]);
			if(!pMapName)
				continue;

			str_format(aBuf, sizeof(aBuf), "%d-%d | %s, ", i, pSelf->m_aMaprotation[i], pMapName);
			pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf); // 输出
		}
	}
}

static void ConMapClear(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	for(int i = 0; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i)
		pSelf->m_aMaprotation[i] = {0}; // 清空地图循环列表
}

static void ConSetClass(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom(pResult->GetInteger(1));
	if(!pPlayer) // If the player does not exist
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else
	{	pPlayer->m_Class = pResult->GetInteger(0); // Set Class 1 = CIVIC, 2 = HUNTER, 4 = JUGGERNAUT
		if(pResult->NumArguments() > 2)
			pPlayer->m_AmongUsTeam = pResult->GetInteger(2); // Team
		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
			CGameControllerHunterN::OnResetClass(pPlayer->GetCharacter()); // reset class after OnCharacterSpawn
		if(pResult->NumArguments() > 3)
			pPlayer->m_UseHunterWeapon = pResult->GetInteger(3);} // Override m_UseHunterWeapon after OnResetClass/OnCharacterSpawn
}

static void ConGiveWeapon(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom((pResult->NumArguments() > 2) ? pResult->GetInteger(2) : pResult->m_ClientID);
	if(!pPlayer) 
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else if(!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "character is dead");
	else
	{	pPlayer->GetCharacter()->RemoveWeapon((pResult->GetInteger(1) < NUM_WEAPONS && pResult->GetInteger(1) >= 0) ? pResult->GetInteger(1) : 0); // Slot
		pPlayer->GetCharacter()->GiveWeapon((pResult->GetInteger(1) < NUM_WEAPONS && pResult->GetInteger(1) >= 0) ? pResult->GetInteger(1) : 0, // Slot
			pResult->GetInteger(0), // Type
				(pResult->NumArguments() > 3) ? pResult->GetInteger(3) : -1);} // ammo
}

static void ConSetHeal(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom((pResult->NumArguments() > 2) ? pResult->GetInteger(2) : pResult->m_ClientID);
	if(!pPlayer)
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else if(!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "character is dead");
	else
	{	if(pResult->NumArguments() > 3) // Set m_MaxHealth
			pPlayer->GetCharacter()->m_MaxHealth = pResult->GetInteger(3) > 0 ? pResult->GetInteger(3) : 0; // math maximum(pResult->GetInteger(3), 0);
		if(pResult->NumArguments() > 4) // Set m_MaxArmor
			pPlayer->GetCharacter()->m_MaxArmor = pResult->GetInteger(4) > 0 ? pResult->GetInteger(4) : 0; // math maximum(pResult->GetInteger(4), 0);

		pPlayer->GetCharacter()->SetHealth(pResult->GetInteger(0)); // Set Health
		if(pResult->NumArguments() > 1)
			pPlayer->GetCharacter()->SetArmor(pResult->GetInteger(1));} // Set Armor
}

static void ConRevive(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;// CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData; // Use CGameControllerHunterN instead of IGameController

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom((pResult->NumArguments() > 0) ? pResult->GetInteger(0) : pResult->m_ClientID);
	if(!pPlayer) // If the player does not exist
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "character is alive");
	else
	{	if(pPlayer->m_Class == CLASS_NONE)
			pPlayer->m_Class = CLASS_CIVIC;
		pPlayer->TryRespawn();}
}

CGameControllerHunterN::CGameControllerHunterN() :
	IGameController()
{
	m_pGameType = "HunterN";
	m_GameFlags = HUNTERN_GAMEFLAGS;
	// 生存模式，回合模式，SUDDENDEATH，回合终局显示游戏结束，游戏结束/旁观Snap队伍模式

	INSTANCE_CONFIG_INT(&m_HunterRatio, "htn_hunt_ratio", 4, 2, MAX_CLIENTS, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "几个玩家里选取一个猎人（整数,默认4,限制2~64）");
	//INSTANCE_CONFIG_INT(&m_Broadcastm_HunterList, "htn_hunt_broadcast_list", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人列表（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterDeath, "htn_hunt_broadcast_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人死亡（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_EffectHunterDeath, "htn_hunt_effert_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人死亡是否使用出生烟（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_HuntFragNum, "htn_hunt_frag_num", 18, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人榴弹产生的破片数量（整数,默认18,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_HuntFragTrack, "htn_hunt_frag_track", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人榴弹破片是否追踪定向（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_Wincheckdeley, "htn_wincheck_deley", 100, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "终局判断延时毫秒（整数,默认100,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_GameoverTime, "htn_gameover_time", 7, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "结算界面时长秒数（整数,默认0,限制0~268435455）");
	//INSTANCE_CONFIG_INT(&m_RoundMode, "htn_round_mode", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "回合模式 正常0 娱乐1（整数,默认0,限制0~1）");;

	InstanceConsole()->Register("htn_map", "", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapAdd, this, "地图所循环的列表");
#define ALOTMAPS "?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps]"
	InstanceConsole()->Register("htn_map_add", ALOTMAPS/* just 64 maps :P */, CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapAdd, this, "向地图循环添加地图");
	InstanceConsole()->Register("htn_map_clear", "", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapClear, this, "清除地图循环列表");

	InstanceConsole()->Register("htn_setclass", "i[class-id] ?i[CID] ?i[team-id] ?i[hunt-weapon]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSetClass, this, "给玩家设置职业（1平民,2猎人,4剑圣）");
	InstanceConsole()->Register("htn_giveweapon", "i[weapon-id] i[slot] ?i[CID] ?i[ammo-num]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConGiveWeapon, this, "给玩家武器");
	InstanceConsole()->Register("htn_setheal", "i[health] ?i[armor] ?i[CID] ?i[max-health] ?i[max-armor]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSetHeal, this, "给玩家血量和盾");
	InstanceConsole()->Register("htn_revive", "?i[CID]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConRevive, this, "复活吧");
}

// Functional
void CGameControllerHunterN::OnResetClass(CCharacter *pChr) // 职业重置（出生后）
{
	pChr->m_MaxHealth = 10;
	pChr->m_MaxArmor = 10;

	pChr->RemoveWeapon(WEAPON_HAMMER); // OnClassSpawn给武器
	pChr->RemoveWeapon(WEAPON_GUN); // OnClassSpawn给手枪
	pChr->RemovePowerUpWeapon();

	pChr->Controller()->OnCharacterSpawn(pChr);
}

void CGameControllerHunterN::SendChatRoom(const char *pText, int Flags)
{
	CNetMsg_Sv_Chat Msg;
	Msg.m_Team = 0;
	Msg.m_ClientID = -1;
	Msg.m_pMessage = pText;

	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(GetPlayerIfInRoom(i))
		{
			if(!((Server()->IsSixup(i) && (Flags & CGameContext::CHAT_SIXUP)) ||
			   (!Server()->IsSixup(i) && (Flags & CGameContext::CHAT_SIX))))
			return;

			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, i);
		}
}

void CGameControllerHunterN::CycleMap() // 循环地图
{
	if(!m_aMaprotation[0]) // 列表没东西
		return;

	bool CurrentMapfound = false; // 是否找到了当前地图

	for(int i = 0; i < 64; ++i) // 循环所有子地图
	{
		if(!m_aMaprotation[i]) // 列表走到了尽头 没找到可用地图
			break;
		else if(m_aMaprotation[i] == m_MapIndex) // 是否在列表里找到了当前地图
			CurrentMapfound = true; // 下次循环会试图寻找可用地图
		else if(CurrentMapfound)
		{
			m_MapIndex = m_aMaprotation[i]; // 切换地图
			GameServer()->Teams()->ReloadGameInstance(GameWorld()->Team());
			return;
		}
	}

	m_MapIndex = m_aMaprotation[0]; // 重置到列表第一项
	GameServer()->Teams()->ReloadGameInstance(GameWorld()->Team());
}

// Event
void CGameControllerHunterN::OnGameStart(bool IsRound)
{
	m_GameFlags = HUNTERN_GAMEFLAGS;
	m_DoWinchenkClassTick = -1;

	if(!IsRound)
	{
		CycleMap();
	}
}

void CGameControllerHunterN::OnWorldReset() // 重置部分值和职业选择
{
	//int TeamClass[1];
	//TeamClass[0] = CLASS_CIVIC; // TEAM_RED
	//TeamClass[1] = CLASS_HUNTER; // TEAM_BLUE

	//int PlayerCount = m_aTeamSize[TEAM_RED]; // 玩家计数
	int PreselectPlayerCount = 0; // 最近没当过猎人的玩家的计数
	int rHunter = 0; // 猎人选择随机数
	// int m_NumHunter = 0; // 需要选择多少个猎人

	for(int i = 0; i < MAX_CLIENTS; ++i) // 重置并计数玩家
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			pPlayer->m_AmongUsTeam = TEAM_RED; // 重置队伍
			pPlayer->m_HiddenScore = 0; // 重置隐藏分
			pPlayer->m_UseHunterWeapon = false; // 默认武器
			pPlayer->m_Class = CLASS_CIVIC; // 重置玩家为平民
			if(pPlayer->m_Preselect) // 猎人选择伪随机！我们要在m_Preselect的玩家里面选择猎人
				++PreselectPlayerCount; // 计数有PreselectPlayerCount个玩家
		}
	}

	if(m_aTeamSize[TEAM_RED] < 2) // m_aTeamSize[TEAM_RED] = 非队伍模式的人数量
		return;

	m_NumHunter = (m_aTeamSize[TEAM_RED] - 2) / m_HunterRatio + 1;// 我们要多少个猎人
	str_format(m_HunterList, sizeof(m_HunterList), "本回合的 %d 个Hunter是: ", m_NumHunter); // Generate Hunter info message 生成猎人列表消息头

	SendChatRoom("——————欢迎来到HunterN猎人杀——————");
	//MatchFlag = -1; // 要成为Jug的玩家不存在 撤了
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "本回合有 %d 个猎人Hunter has been selected.", m_NumHunter);
	SendChatRoom(aBuf);
	SendChatRoom("规则：每回合秘密抽选猎人 猎人对战平民 活人看不到死人消息");
	SendChatRoom("      猎人双倍伤害 有瞬杀锤子(平民无锤)和破片榴弹(对自己无伤)");
	SendChatRoom("分辨队友并消灭敌人来取得胜利！Be warned! Sudden Death.");

	for(int iHunter = m_NumHunter; iHunter > 0; --iHunter) // 需要选择m_NumHunter个猎人
	{
		if(PreselectPlayerCount <= 0) // 先检查m_Preselect的玩家够不够（即所有玩家是不是最近都当过猎人了） 如果不够就重置所有玩家的m_Preselect
		{
			for(int i = 0; i < MAX_CLIENTS; ++i) // 重置所有玩家的m_Preselect
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
					(!pPlayer->m_RespawnDisabled ||
						(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
				{
					pPlayer->m_Preselect = true; // 设置m_Preselect为真（包括猎人 主要是为了随机性）
					if(pPlayer->m_Class == CLASS_CIVIC) // 只有平民才可以被计数
						++PreselectPlayerCount; // 重置PreselectPlayerCount
				}
			}
		}

		rHunter = rand() % PreselectPlayerCount; // 在PreselectPlayerCount个玩家里选择第rHunter个猎人

		for(int i = 0; i < MAX_CLIENTS; ++i) // 在PreselectPlayerCount个玩家里选择第rHunter个玩家为猎人
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer)
			{
				if(pPlayer->GetTeam() != TEAM_SPECTATORS &&
				(!pPlayer->m_RespawnDisabled ||
					(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())) &&
						pPlayer->m_Preselect && (pPlayer->m_Class == CLASS_CIVIC)) // 在平民职业的玩家里面选择猎人
				{
					if(rHunter == 0) // 找到了第rHunter个玩家 选择为猎人
					{
						pPlayer->m_Class = CLASS_HUNTER; // 设置猎人Flag
						//pPlayer->m_UseHunterWeapon = true; // 使用猎人武器 // 在OnCharachar里面设置
						pPlayer->m_AmongUsTeam = TEAM_BLUE; // 设置队伍
						pPlayer->m_Preselect = false; // 把m_Preselect设为否 即最近当过猎人
						--PreselectPlayerCount;

						// Generate Hunter info message 生成猎人列表消息
						str_append(m_HunterList, Server()->ClientName(i), sizeof(m_HunterList));
						str_append(m_HunterList, ", ", sizeof(m_HunterList));

						//if(iHunter != 1) // 最后一次循环 要循环全体玩家
							break;
					}
					--rHunter;
				}

				/*if(iHunter == 1) // 最后一次循环 所有玩家都选择了职业
					if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()) // 这个玩家出生了 所以OnCharacterSpawn已经过了
					{
						OnResetClass(pPlayer->GetCharacter()); // 在这里给他们Class提示和武器
					}*/
			}
		}
	}

	for(int i = 0; i < MAX_CLIENTS; ++i) // 循环所有旁观者 把猎人列表告诉他们
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer)
		{
			if(pPlayer->GetTeam() == TEAM_SPECTATORS)
			{
				SendChatTarget(pPlayer->GetCID(), m_HunterList); // 给膀胱者发
			}
			else if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()) // 这个玩家出生了 所以OnCharacterSpawn已经过了
			{
				OnResetClass(pPlayer->GetCharacter()); // 在这里给他们Class提示和武器
			}
		}
	}

	InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", m_HunterList);
}

void CGameControllerHunterN::OnCharacterSpawn(CCharacter *pChr) // 给予生命值和武器
{
	if(m_GameState != IGS_GAME_RUNNING) // 如果游戏未开始
	{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		return;
	}

	// 如果游戏在正常运行
	if(pChr->GetPlayer()->m_Class == CLASS_CIVIC)
	{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("这局你是平民Civic! 消灭敌方队伍胜利!     \n猎人双倍伤害 有瞬杀锤子和破片榴弹", pChr->GetPlayer()->GetCID(), true);
	}
	else if(pChr->GetPlayer()->m_Class == CLASS_HUNTER)
	{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HUNTHAMMER, -1);
		pChr->GetPlayer()->m_UseHunterWeapon = true; // 使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("     这回合你被选择为猎人Hunter!\n     猎人双倍伤害 有瞬杀锤子和破片榴弹\n     分辨出你的队友 消灭敌方队伍胜利!", pChr->GetPlayer()->GetCID(), true);
		
	}
	/*else if(pChr->GetPlayer()->m_Class == CLASS_JUGGERNAUT)
	{
		pChr->m_MaxHealth = 114;
		pChr->IncreaseHealth(114);
		pChr->m_MaxArmor = 5;
		pChr->IncreaseArmor(5);
		pChr->SetPowerUpWeapon(WEAPON_ID_JUGNINJA, -1);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_NINJA_FIRE, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("     这局你是剑圣Juggernaut！噶了所有人胜利!\n     剑圣40心20盾 有盾反锤子且能斩杀", pChr->GetPlayer()->GetCID(), true);
	}*/
}

void CGameControllerHunterN::OnPlayerJoin(class CPlayer *pPlayer) // 使新进旁观者收到猎人列表
{
	if(m_GameState == IGS_GAME_RUNNING && pPlayer->m_RespawnDisabled) // 死人
	{
		SendChatTarget(pPlayer->GetCID(), m_HunterList);
		SendChatTarget(pPlayer->GetCID(), "1. 每局开始时会秘密随机选择玩家成为猎人或平民 且玩家只知道自己身份 猎人的目标是消灭所有平民");
		SendChatTarget(pPlayer->GetCID(), "2. 猎人使用高伤武器、瞬杀追踪锤(20伤,长按追踪)和破片榴弹 而平民没有锤子且只能使用常规武器");
		SendChatTarget(pPlayer->GetCID(), "3. 当玩家死时如为猎人死亡则通知其他猎人 且死亡原因和死后聊天仅旁观/死人可见");
	}
}

int CGameControllerHunterN::OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion)
// 使Hunter不受到自己的伤害
{
	if(pChr->GetPlayer()->GetCID() == From && pChr->GetPlayer()->m_Class == CLASS_HUNTER) // Hunter不能受到来自自己的伤害（这样就不会被逆天榴弹自爆）
		return DAMAGE_NO_DAMAGE | DAMAGE_NO_INDICATOR;
	return DAMAGE_NORMAL;
}

/*int CGameControllerHunterN::OnPickup(CPickup *pPickup, CCharacter *pChar, SPickupSound *pSound) // Juggernaut不能捡东西
{
	if(pChar->GetPlayer()->m_Class != CLASS_JUGGERNAUT || ((pPickup->GetType() == POWERUP_ARMOR)))
		return IGameController::OnPickup(pPickup, pChar, pSound);
	return -1;
}*/

bool CGameControllerHunterN::CanChangeTeam(CPlayer *pPlayer, int JoinTeam) const // 加入膀胱者则重置职业Flag
{
	if(JoinTeam == TEAM_SPECTATORS)
	{
		pPlayer->m_Class = CLASS_NONE; // 设置成没有职业
		pPlayer->m_AmongUsTeam = TEAM_SPECTATORS; // 设置成没有队伍
	}
	return true;
}

void CGameControllerHunterN::DoWincheckRound() // check for time based win
{
	bool IsTimeEnd = (!m_SuddenDeath && m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) >= m_GameInfo.m_TimeLimit * Server()->TickSpeed() * 60);

	if(IsTimeEnd || !m_DoWinchenkClassTick) // 计时
	{
		int PlayerCount = 0;
		int TeamRedCount = 0;
		int TeamBlueCount = 0;

		for(int i = 0; i < MAX_CLIENTS; ++i) // Count Player
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
				(!pPlayer->m_RespawnDisabled ||
					(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
			{
				++PlayerCount;
				if(pPlayer->m_AmongUsTeam == TEAM_RED)
					++TeamRedCount;
				else if(pPlayer->m_AmongUsTeam == TEAM_BLUE)
					++TeamBlueCount;
			}
		}

		if(IsTimeEnd || !(TeamBlueCount && TeamRedCount)) // 如果不是回合限时结束则需某队死光
		{
			// 游戏结束
			m_aTeamscore[TEAM_RED] = 0; // 重置
			m_aTeamscore[TEAM_BLUE] = 0;

			for(int i = 0; i < MAX_CLIENTS; ++i) // 进行玩家分数和隐藏分操作 和选择Jug
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(pPlayer)
				{
					if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
						pPlayer->m_HiddenScore += 2; // 存活加分
					//if(pPlayer->m_Score + minimum(25, pPlayer->m_HiddenScore * 5) > 48)
						//MatchFlag = pPlayer->GetCID(); // Jug判断
					if(pPlayer->m_HiddenScore)
					{
						pPlayer->m_Score += pPlayer->m_HiddenScore; // 添加隐藏分
					}

					if(pPlayer->m_Class & CLASS_CIVIC)
					{
						m_aTeamscore[TEAM_RED] += 1; // 用队伍分数显示有几个民
					}
					else if(pPlayer->m_Class & CLASS_HUNTER)
					{
						//pPlayer->m_AmongUsTeam = TEAM_BLUE; // 结算界面左边红队为民 右边蓝队为猎 平民为红队
						m_aTeamscore[TEAM_BLUE] += 1; // 用队伍分数显示有几个猎
					}
				}
			}

			if(!PlayerCount)
			{
				SendChatRoom("两人幸终！");
			}
			else if(!TeamBlueCount) // no blue
			{
				SendChatRoom(m_HunterList);
				SendChatRoom("红队胜利！"); // 平民为红队
				//GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE); // 猎人死的时候够吵了

				m_aTeamscore[TEAM_BLUE] = -m_aTeamscore[TEAM_BLUE]; // 反转蓝队分数 显示"红队胜利"
			}
			else if(!TeamRedCount) // no red
			{
				//SendChatRoom(m_HunterList); // 猎人胜利不显示列表（因为平民被打死的时候已经显示过了）
				SendChatRoom("蓝队胜利！"); // 猎人为蓝队
				GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);

				m_aTeamscore[TEAM_RED] = -m_aTeamscore[TEAM_RED]; // 反转红队分数 就会显示"蓝队胜利"
			}
			else
			{
				SendChatRoom(m_HunterList);
				SendChatRoom("游戏结束！");
				GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
				//m_aTeamscore[TEAM_BLUE] = -m_aTeamscore[TEAM_BLUE]; // 不反转分数 也会显示"红队胜利"
			}

			m_GameFlags |= IGF_MARK_TEAMS; // 队伍形式显示

			SetGameState(IGS_END_ROUND, m_GameoverTime); // EndRound();
		}
	}

	if(m_DoWinchenkClassTick >= 0)
		--m_DoWinchenkClassTick;
}

void CGameControllerHunterN::DoWincheckMatch() // Roundlimit 触发DoWincheckMatch
{
	if(m_GameInfo.m_MatchNum > 0 && m_GameInfo.m_MatchCurrent >= m_GameInfo.m_MatchNum)
		SetGameState(IGS_END_MATCH, m_GameoverTime); // EndMatch();
}

int CGameControllerHunterN::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) // 杀手隐藏分增减 和受害人职业死亡消息 以及延时终局
{
	if(m_GameState == IGS_GAME_RUNNING) // 如果游戏在正常运行
	{
		if(pVictim->GetPlayer()->m_Class == CLASS_HUNTER) // 猎人死亡
		{
			--m_NumHunter; // 计数猎人死亡

			if(m_EffectHunterDeath)
				GameWorld()->CreatePlayerSpawn(pVictim->m_Pos); // 死亡给个出生烟

			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "Hunter '%s' was defeated! ", Server()->ClientName(pVictim->GetPlayer()->GetCID()));

			if(m_BroadcastHunterDeath == 1 ||
				!m_NumHunter) // 如果是最后一个Hunter
			{
				SendChatRoom(aBuf); // 直接全体广播
				GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
			}
			else
			{
				char aBufEx[16];
				str_format(aBufEx, sizeof(aBufEx), "%d Hunter left.", m_NumHunter);
				str_append(aBuf, aBufEx, sizeof(aBuf));
				for(int i = 0; i < MAX_CLIENTS; ++i) // 逐个给所有人根据职业发送死亡消息
				{
					CPlayer *pPlayer = GetPlayerIfInRoom(i);
					if(pPlayer)
					{
						if((m_BroadcastHunterDeath != -1 && pPlayer->m_Class == CLASS_HUNTER) || // 猎
							pPlayer->GetTeam() == TEAM_SPECTATORS || pPlayer->m_DeadSpecMode) // 观察者 和 死人
						{
							SendChatTarget(pPlayer->GetCID(), aBuf); // 给所有猎人广播他们"队友"的死亡消息
							GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE, CmaskOne(pPlayer->GetCID()));
						}
						else
							GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP, CmaskOne(pPlayer->GetCID()));
					}
				}	
			}

			if(pKiller && pKiller != pVictim->GetPlayer()) // 不是自杀
			{
				if(pKiller->m_Class != CLASS_HUNTER) // 隐藏分添加
					pKiller->m_HiddenScore += 4;
				else
					pKiller->m_HiddenScore -= 2; // Teamkill
			}
		}
		else if(pVictim->GetPlayer()->m_Class == CLASS_CIVIC) // 平民死亡
		{
			if(pKiller && pKiller != pVictim->GetPlayer()) // 不是自杀
			{
				if(pKiller->m_Class != CLASS_CIVIC) // 隐藏分添加
					pKiller->m_HiddenScore += 1;
				else
					pKiller->m_HiddenScore -= 1; // Teamkill
			}

			GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP);
		}
		/*else if(pVictim->GetPlayer()->m_Class == CLASS_JUGGERNAUT)
		{
			if(pKiller && pKiller != pVictim->GetPlayer())
			{
				if(pKiller->m_Class != CLASS_JUG) // 隐藏分添加
					pKiller->m_HiddenScore += 8;
				else // :P
					pKiller->m_HiddenScore -= 114514; // Teamkill
			}

			SendChatRoom("Juggernaut was defeated!");
			GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
		}*/

		if(m_NumHunter) // 如果没有猎人(当然是全死光啦) 就不要发猎人列表 等EndMatch
		{
			if(Weapon >= WEAPON_WORLD)
			{
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "你被 '%s' 的%s所杀", Server()->ClientName(pKiller->GetCID()),
					(Weapon == WEAPON_HAMMER ? "锤子"
					: (Weapon == WEAPON_GUN ? "手枪"
					: (Weapon == WEAPON_SHOTGUN ? "霰弹"
					: (Weapon == WEAPON_GRENADE ? "榴弹"
					: (Weapon == WEAPON_LASER ? "激光"
					: "忍者刀" ))))));
				SendChatTarget(pVictim->GetPlayer()->GetCID(), aBuf); // 给被弄死的人发
			}

			SendChatTarget(pVictim->GetPlayer()->GetCID(), m_HunterList); // 给被弄死的人发
		}

		m_DoWinchenkClassTick = ((Server()->TickSpeed() * m_Wincheckdeley) / 1000); // 延时终局
	}

	return DEATH_NO_REASON | DEATH_SKIP_SCORE; // 隐藏死因并跳过内置分数逻辑
}
