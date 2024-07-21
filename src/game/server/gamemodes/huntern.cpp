/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "huntern.h"
#include <game/server/entities/character.h>
#include <game/server/entities/textentity.h>
#include <game/server/weapons.h>
//#include <game/server/entities/pickup.h>
#include <game/server/classes.h>

#include <game/server/hunterncommands.h> // HunterN commands

CGameControllerHunterN::CGameControllerHunterN() :
	IGameController()
{
	m_pGameType = "hunterN";
	m_GameFlags = HUNTERN_GAMEFLAGS;
	// 生存模式，回合模式，SUDDENDEATH，回合终局显示游戏结束，游戏结束/旁观Snap队伍模式

	m_MinimumPlayers = 2;

	m_DoWinchenkClassTick = -1;
	mem_zero(m_aMaprotation, sizeof(m_aMaprotation));

	INSTANCE_CONFIG_INT(&m_HunterRatio, "htn_hunt_ratio", 4, 2, MAX_CLIENTS, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "几个玩家里选取一个猎人（整数,默认4,限制2~64）");
	//INSTANCE_CONFIG_INT(&m_Broadcastm_HunterList, "htn_hunt_broadcast_list", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人列表（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterDeath, "htn_hunt_broadcast_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人死亡（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_EffectHunterDeath, "htn_hunt_effert_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人死亡是否使用出生烟（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_HuntFragNum, "htn_hunt_frag_num", 18, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人榴弹产生的破片数量（整数,默认18,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_HuntFragTrack, "htn_hunt_frag_track", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人榴弹破片是否追踪定向（开关,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_Wincheckdeley, "htn_wincheck_deley", 200, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "终局判断延时毫秒（整数,默认100,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_GameoverTime, "htn_gameover_time", 7, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "结算界面时长秒数（整数,默认0,限制0~268435455）");
	//INSTANCE_CONFIG_INT(&m_RoundMode, "htn_round_mode", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "回合模式 正常0 娱乐1（整数,默认0,限制0~1）");;

#define ALOTMAPS "?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps] ?s[maps]"
	InstanceConsole()->Register("htn_map_add", ALOTMAPS/* just 64 maps :P */, CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapAdd, this, "向地图循环添加地图");
#undef ALOTMAPS
	InstanceConsole()->Register("htn_map_clear", "", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapClear, this, "清除地图循环列表");
	InstanceConsole()->Register("htn_map_remove", "i[map-id]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapRemove, this, "在地图循环列表清除地图");
	InstanceConsole()->Register("htn_map", "", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConMapList, this, "显示地图循环列表");

	InstanceConsole()->Register("htn_setclass", "i[class-id] ?i[CID] ?i[team-id] ?i[hunt-weapon]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSetClass, this, "给玩家设置职业（1平民,2猎人,4剑圣）");
	InstanceConsole()->Register("htn_giveweapon", "i[weapon-id] i[slot] ?i[CID] ?i[ammo-num] ?i[is-powerup]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConGiveWeapon, this, "给玩家武器");
	InstanceConsole()->Register("htn_setheal", "i[health] ?i[armor] ?i[CID] ?i[max-health] ?i[max-armor]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSetHeal, this, "给玩家血量和盾");
	InstanceConsole()->Register("htn_revive", "?i[CID]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConRevive, this, "复活吧");
	InstanceConsole()->Register("htn_sign", "s[message]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSign, this, "复活吧");
}

// Functional
void CGameControllerHunterN::OnClassChange(CCharacter *pChr) // 更改职业
{

}

void CGameControllerHunterN::OnClassReset(CCharacter *pChr) // 职业重置（出生后）
{
	pChr->m_MaxHealth = 10;
	pChr->m_MaxArmor = 10;

	pChr->RemoveWeapon(WEAPON_HAMMER); // OnClassSpawn给武器
	pChr->RemoveWeapon(WEAPON_GUN); // OnClassSpawn给手枪
	pChr->SetPowerUpWeapon(WEAPON_ID_NONE);

	pChr->m_SpawnTick = pChr->Server()->Tick(); // for CGameControllerHunterN::OnEntitySnap()

	pChr->Controller()->OnCharacterSpawn(pChr);
}

bool CGameControllerHunterN::CycleMap() // 循环地图
{
	if(!m_aMaprotation[0]) // 列表没东西
		return false;

	bool CurrentMapfound = false; // 是否找到了当前地图

	for(int i = 0; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i) // 循环所有子地图
	{
		if(!m_aMaprotation[i]) // 列表走到了尽头 没找到可用地图
			break;
		else if(m_aMaprotation[i] == m_MapIndex) // 是否在列表里找到了当前地图
			CurrentMapfound = true; // 下次循环会试图寻找可用地图
		else if(CurrentMapfound)
		{
			m_MapIndex = m_aMaprotation[i]; // 切换地图
			GameServer()->Teams()->ReloadGameInstance(GameWorld()->Team());
			return true;
		}
	}

	m_MapIndex = m_aMaprotation[0]; // 重置到列表第一项
	GameServer()->Teams()->ReloadGameInstance(GameWorld()->Team());
	return true;
}

// Event
void CGameControllerHunterN::OnInit() // 使换图后终局以清除分数
{
	SetGameState(IGS_END_MATCH, 2); // EndMatch();
}

void CGameControllerHunterN::OnWorldReset() // 重置部分值和职业选择
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
			OnClassReset(pPlayer->GetCharacter()); // 在这里给他们Class提示和武器
	}

	InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", m_HunterList);
}

void CGameControllerHunterN::OnPreEntitySnap(int SnappingClient, int OtherMode) // 出生用旗子显示自己队伍
{
	CPlayer *pPlayer = GetPlayerIfInRoom(SnappingClient);
	if(!pPlayer || pPlayer->GetCID() != SnappingClient) // 只对自己显示
		return;

	CCharacter *pChar = pPlayer->GetCharacter();
	if(!pChar || pChar->m_SpawnTick + (Server()->TickSpeed() * 5) <= Server()->Tick()) // m_SpawnTick + 5 sec
		return;

	CNetObj_Flag *pFlag = (CNetObj_Flag *)Server()->SnapNewItem(NETOBJTYPE_FLAG, 0, sizeof(CNetObj_Flag)); // 显示一个旗子
	if(!pFlag)
		return;

	pFlag->m_X = round_to_int(pChar->m_Pos.x);
	pFlag->m_Y = round_to_int(pChar->m_Pos.y);
	pFlag->m_Team = pPlayer->m_AmongUsTeam; // 显示玩家的队伍
}

void CGameControllerHunterN::DoWincheckMatch() // Roundlimit 触发DoWincheckMatch
{
	if(m_GameInfo.m_MatchNum <= 0 || m_GameInfo.m_MatchCurrent < m_GameInfo.m_MatchNum)
		return;

	int PrevMapIndex = m_MapIndex; // 记录换图前的地图Index
	if(CycleMap()) // 换图！！！！
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "地图循环: '%s' -> '%s'", GameServer()->Teams()->GetMapName(PrevMapIndex), GameServer()->Teams()->GetMapName(m_MapIndex));
		SendChatTarget(-1, aBuf);
	}

	SetGameState(IGS_END_MATCH, m_GameoverTime); // EndMatch();
}

void CGameControllerHunterN::DoWincheckRound() // check for time based win
{
	bool IsTimeEnd = (!m_SuddenDeath && m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) >= m_GameInfo.m_TimeLimit * Server()->TickSpeed() * 60);

	if(m_DoWinchenkClassTick && !IsTimeEnd)  // 时间没有结束且延时终局
	{
		if(m_DoWinchenkClassTick >= 0)
			--m_DoWinchenkClassTick; // 计时
		return;
	}

	int AliveTeamPlayerCount[NUM_TEAMS] = {0};

	for(int i = 0; i < MAX_CLIENTS; ++i) // 计数玩家
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(!pPlayer || pPlayer->GetTeam() == TEAM_SPECTATORS
			|| (pPlayer->m_RespawnDisabled
			&& (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())))
			continue;

		AliveTeamPlayerCount[pPlayer->m_AmongUsTeam]++; // 计数活着的玩家
	}

	if(!IsTimeEnd && (AliveTeamPlayerCount[TEAM_RED] && AliveTeamPlayerCount[TEAM_BLUE])) // 如果不是回合限时结束则需某队死光
	{
		m_DoWinchenkClassTick = -1;
		return;
	}

	// 游戏结束
	SetGameState(IGS_END_ROUND, m_GameoverTime); // EndRound();

	m_GameFlags |= IGF_MARK_TEAMS; // 队伍形式显示结算界面
	m_aTeamscore[TEAM_RED] = AliveTeamPlayerCount[TEAM_RED]; // 队伍分数形式显示双方人数
	m_aTeamscore[TEAM_BLUE] = AliveTeamPlayerCount[TEAM_BLUE];

	for(int i = 0; i < MAX_CLIENTS; ++i) // 进行玩家分数和隐藏分操作
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(!pPlayer)
			continue;

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
			pPlayer->m_HiddenScore += 2; // 存活加分

		if(pPlayer->m_HiddenScore) // 玩家拥有隐藏分
			pPlayer->m_Score += pPlayer->m_HiddenScore; // 添加隐藏分
	}

	if(!AliveTeamPlayerCount[TEAM_RED] && !AliveTeamPlayerCount[TEAM_BLUE]) // 终局聊天消息
	{
		SendChatTarget(-1, "两人幸终！");
	}
	else if(!AliveTeamPlayerCount[TEAM_BLUE]) // no blue
	{
		SendChatTarget(-1, m_HunterList);
		SendChatTarget(-1, "平民胜利！");
		//GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE); // 猎人死的时候够吵了

		m_aTeamscore[TEAM_BLUE] = -m_aTeamscore[TEAM_BLUE]; // 反转蓝队分数 显示"红队胜利"
	}
	else if(!AliveTeamPlayerCount[TEAM_RED]) // no red
	{
		//SendChatTarget(-1, m_HunterList); // 猎人胜利不显示列表（因为平民被打死的时候已经显示过了）
		SendChatTarget(-1, "猎人胜利！");
		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);

		m_aTeamscore[TEAM_RED] = -m_aTeamscore[TEAM_RED]; // 反转红队分数 就会显示"蓝队胜利"
	}
	else
	{
		SendChatTarget(-1, m_HunterList);
		SendChatTarget(-1, "游戏结束！");
		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
		//m_aTeamscore[TEAM_BLUE] = -m_aTeamscore[TEAM_BLUE]; // 不反转分数 也会显示"红队胜利"
	}
}

bool CGameControllerHunterN::CanChangeTeam(CPlayer *pPlayer, int JoinTeam) const // 加入膀胱者则重置职业Flag
{
	if(JoinTeam == TEAM_SPECTATORS)
	{
		pPlayer->m_Class = CLASS_NONE; // 设置成没有职业
		pPlayer->m_AmongUsTeam = TEAM_SPECTATORS; // 设置成没有队伍
	}
	return true;
}

void CGameControllerHunterN::OnPlayerJoin(class CPlayer *pPlayer) // 使新进旁观者收到猎人列表
{
	if(m_GameState == IGS_GAME_RUNNING && pPlayer->m_RespawnDisabled) // 死人
		SendChatTarget(pPlayer->GetCID(), m_HunterList);
}

void CGameControllerHunterN::OnCharacterSpawn(CCharacter *pChr) // 出生给予生命值和武器 以及职业提示
{
	switch(IsGameRunning()
		? pChr->GetPlayer()->m_Class // 如果游戏在正常运行
		: CLASS_NONE) // 如果游戏未开始
	{
		default:{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器
		break;
	}case CLASS_CIVIC:{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast(m_apClassSpawnMsg[0], pChr->GetPlayer()->GetCID(), true);
		break;
	}case CLASS_HUNTER:{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->ForceSetWeapon(WEAPON_HAMMER, WEAPON_ID_HUNTHAMMER, -1);
		pChr->GetPlayer()->m_UseHunterWeapon = true; // 使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast(m_apClassSpawnMsg[1], pChr->GetPlayer()->GetCID(), true);
		break;
	}/*case CLASS_JUGGERNAUT:{
		pChr->m_MaxHealth = 114;
		pChr->IncreaseHealth(114);
		pChr->m_MaxArmor = 5;
		pChr->IncreaseArmor(5);
		pChr->SetPowerUpWeapon(WEAPON_ID_JUGNINJA, -1);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_NINJA_FIRE, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast(m_apClassSpawnMsg[2], pChr->GetPlayer()->GetCID(), true);
		break;
	}case CLASS_PUPPETEE:{
		pChr->IncreaseHealth(10);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);
		pChr->ForceSetWeapon(WEAPON_HAMMER, WEAPON_ID_PUPPETEEHAMMER, -1);
		pChr->GetPlayer()->m_UseHunterWeapon = false; // 不使用猎人武器

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast(m_apClassSpawnMsg[3], pChr->GetPlayer()->GetCID(), true);
		break;
	}*/}
}

int CGameControllerHunterN::OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) // 使Hunter不受到自己的伤害
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

int CGameControllerHunterN::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) // 杀手隐藏分增减 和受害人职业死亡消息 以及延时终局
{
	if(m_GameState != IGS_GAME_RUNNING) // 如果游戏在正常运行
		return DEATH_SKIP_SCORE; // 跳过内置分数逻辑

	int VictimCID = pVictim->GetPlayer()->GetCID();

	switch(pVictim->GetPlayer()->m_Class) // 猎人死亡 进行计数和猎人死亡报告
	{
	case CLASS_HUNTER:{
		--m_NumHunter; // 计数猎人死亡

		if(m_EffectHunterDeath)
			GameWorld()->CreatePlayerSpawn(pVictim->m_Pos); // 死亡给个出生烟

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Hunter '%s' was defeated! ", Server()->ClientName(VictimCID));

		if(m_BroadcastHunterDeath == 1 ||
			!m_NumHunter) // 如果是最后一个Hunter
		{
			SendChatTarget(-1, aBuf); // 直接全体广播
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
				if(!pPlayer)
					continue;
				
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
		break;
	}/*case CLASS_JUGGERNAUT:{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Juggernaut '%s' was defeated! ", Server()->ClientName(VictimCID));
		SendChatTarget(-1, aBuf);
		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
		break;
	}*/default:{//case CLASS_CIVIC: // 平民死亡
		GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP);
		break;
	}}

	if(pKiller != pVictim->GetPlayer()) // 不是自杀
	{
		pKiller->m_HiddenScore += // 给予杀手分数
				m_aKillScore[pVictim->GetPlayer()->m_Class] // Class
				[pKiller->m_Class == pVictim->GetPlayer()->m_Class]; // IsTeamKill

		if(Weapon >= WEAPON_WORLD)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "你被 '%s' 的%s所杀", Server()->ClientName(pKiller->GetCID()), m_apWeaponName[Weapon + 1]);

			SendChatTarget(VictimCID, aBuf); // 给被弄死的人发
		}

		new CTextEntity(GameWorld(), pVictim->m_Pos, CTextEntity::TYPE_GUN, 8, CTextEntity::ALIGN_MIDDLE, (char *)" R.I.P.");
	}

	if(m_NumHunter) // 如果没有猎人(当然是全死光啦) 就不要发猎人列表 等EndMatch
		SendChatTarget(VictimCID, m_HunterList); // 给被弄死的人发猎人列表

	m_DoWinchenkClassTick = ((Server()->TickSpeed() * m_Wincheckdeley) / 1000); // 延时终局

	return DEATH_NO_REASON | DEATH_SKIP_SCORE; // 隐藏死因并跳过内置分数逻辑
}
