/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "huntern.h"
#include <game/server/entities/character.h>
#include <game/server/weapons.h>
#include <game/server/entities/pickup.h>
#include <game/server/classes.h>

// HunterN commands
static void ConSetClass(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom(pResult->GetInteger(0));
	if(pPlayer)
	{
		pPlayer->SetClass(pResult->GetInteger(1)); // 0 = CIVIC, 1 = HUNTER, 2 = JUGGERNAUT
		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
		{
			pSelf->ResetPlayerClass(pPlayer->GetCharacter());
			pSelf->OnClassSpawn(pPlayer->GetCharacter());
		}
	}
}

CGameControllerHunterN::CGameControllerHunterN() :
	IGameController()
{
	m_pGameType = "HunterN";
	m_GameFlags = IGF_SURVIVAL | IGF_SUDDENDEATH;
	m_ResetScoreOnEndMatch = false;
	MatchFlag = -1;

	INSTANCE_CONFIG_INT(&m_HunterRatio, "htn_hunt_ratio", 4, 2, MAX_CLIENTS, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "几个玩家里选取一个猎人（整数,默认4,限制2~64）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterList, "htn_hunt_broadcast_list", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人列表（整数,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterDeath, "htn_hunt_broadcast_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人死亡（整数,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_EffectHunterDeath, "htn_hunt_effert_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人死亡是否使用出生烟（整数,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_HuntFragsNum, "htn_hunt_frags_num", 18, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "猎人榴弹产生的破片数量（整数,默认18,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_Wincheckdeley, "htn_wincheck_deley", 100, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "终局判断延时毫秒（整数,默认0,限制0~268435455）");
	INSTANCE_CONFIG_INT(&m_GameoverTime, "htn_gameover_time", 7, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "结算界面时长秒数（整数,默认0,限制0~268435455）");
	//INSTANCE_CONFIG_INT(&m_RoundMode, "htn_round_mode", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "回合模式 正常0 娱乐1（整数,默认0,限制0~1）");

	INSTANCE_CONFIG_INT(&MatchFlag, "htn_matchflag", -1, -1, MAX_CLIENTS, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "Jug（整数,默认-1,限制-1~64）");

	InstanceConsole()->Register("htn_setclass", "i[CID] i[class-id]", CFGFLAG_CHAT | CFGFLAG_INSTANCE, ConSetClass, this, "给玩家设置职业");
}

void CGameControllerHunterN::OnClassSpawn(CCharacter *pChr) // 给予武器和职业提示
{
	if(pChr->GetPlayer()->GetClass() == CLASS_CIVIC)
	{
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("这局你是平民Civic！噶了所有猎人胜利!     \n猎人双倍伤害 有瞬杀锤子和破片榴弹", pChr->GetPlayer()->GetCID(), true);
	}
	else if(pChr->GetPlayer()->GetClass() == CLASS_HUNTER)
	{
		pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);

		pChr->GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("     这局你是猎人Hunter！噶了所有平民胜利!\n     猎人双倍伤害 有瞬杀锤子和破片榴弹", pChr->GetPlayer()->GetCID(), true);
	}
	else if(pChr->GetPlayer()->GetClass() == CLASS_JUG)
	{
		pChr->SetPowerUpWeapon(WEAPON_ID_JUGHAMMER, -1);

		pChr->m_MaxHealth = 40;
		pChr->IncreaseHealth(40);
		pChr->m_MaxArmor = 20;
		pChr->IncreaseArmor(20);

		pChr->GameWorld()->CreateSoundGlobal(SOUND_NINJA_FIRE, CmaskOne(pChr->GetPlayer()->GetCID()));
		pChr->GameServer()->SendBroadcast("     这局你是剑圣Juggernaut！噶了所有人胜利!\n     剑圣40心20盾 有盾反锤子且能斩杀", pChr->GetPlayer()->GetCID(), true);
	}
}

void CGameControllerHunterN::ResetPlayerClass(CCharacter *pChr)
{
	pChr->m_MaxHealth = 10;
	pChr->IncreaseHealth(10);
	pChr->m_MaxArmor = 10;
	pChr->SetArmor(0);

	pChr->RemoveWeapons(); // Remove All Weapons!
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

void CGameControllerHunterN::SelectClass() // 选择职业
{
	int PlayerCount = 0; // 玩家计数
	int CanHunterPlayerCount = 0; // 最近没当过猎人的玩家的计数
	int rHunter = 0; // 猎人选择随机数

	// nHunter = 0; // 需要选择多少个猎人

	for(int i = 0; i < MAX_CLIENTS; ++i) // 计数有PlayerCount个玩家 以及有CanHunterPlayerCount个玩家 我们要在m_CanHunter的玩家里面选择猎人
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			pPlayer->SetTeamDiractly(TEAM_RED);
			pPlayer->m_HiddenScore = 0; // 重置隐藏分
			pPlayer->SetClass(CLASS_CIVIC); // 重置玩家为平民
			++PlayerCount;
			if(pPlayer->m_CanHunter)
				++CanHunterPlayerCount;
		}
	}

	if(PlayerCount < 2)
		return;

	nHunter = (PlayerCount - 2) / m_HunterRatio + 1;// 我们要多少个猎人
	str_format(HunterList, sizeof(HunterList), "本回合的 %d 个Hunter是: ", nHunter);// Generate Hunter info message 生成猎人列表消息头

	SendChatRoom("——————欢迎来到HunterN猎人杀——————");

	CPlayer *pPlayer = GetPlayerIfInRoom(MatchFlag);
	if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
		(!pPlayer->m_RespawnDisabled ||
			(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()))) // 选择Jug
	{
		pPlayer->m_Class = CLASS_JUG;
		pPlayer->SetTeamDiractly(TEAM_BLUE);
		if(pPlayer->m_CanHunter)
			--CanHunterPlayerCount;

		m_GameFlags |= IGF_MARK_TEAMS;

		char aBuf[96];
		str_format(aBuf, sizeof(aBuf), "本回合 %s 成为了剑圣Juggernaut并与所有人为敌！", Server()->ClientName(pPlayer->GetCID()));
		SendChatRoom(aBuf);
		SendChatRoom("规则：剑圣40心20盾 其锤子能不反弹子弹的前提下斩杀玩家");
	}
	else
	{
		MatchFlag = -1; // 要成为Jug的玩家不存在 撤了

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "本回合有 %d 个猎人Hunter has been selected.", nHunter);
		SendChatRoom(aBuf);
		SendChatRoom("规则：每回合秘密抽选猎人 猎人对战平民 活人看不到死人消息");
		SendChatRoom("      猎人双倍伤害 有瞬杀锤子(平民无锤)和破片榴弹(对自己无伤)");
		SendChatRoom("分辨队友并消灭敌人取得胜利！Be warned! Sudden Death.");
	}

	for(int iHunter = nHunter; iHunter > 0; --iHunter)// 需要选择nHunter个猎人
	{
		if(CanHunterPlayerCount <= 0) // 先检查m_CanHunter的玩家够不够（即所有玩家是不是最近都当过猎人了） 如果不够就重置所有玩家的m_CanHunter
		{
			for(int i = 0; i < MAX_CLIENTS; ++i) // 重置所有玩家的m_CanHunter
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
					(!pPlayer->m_RespawnDisabled ||
						(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
				{
					pPlayer->SetCanHunter(true); // 设置m_CanHunter为真（就是让他们"最近没当过"猎人）（包括猎人 主要是为了随机性）
					if(pPlayer->m_Class == CLASS_CIVIC) // 只有平民才可以被计数
						++CanHunterPlayerCount;
				}
			}
		}

		rHunter = rand() % CanHunterPlayerCount; // 在CanHunterPlayerCount个玩家里选择第rHunter个猎人

		for(int i = 0; i < MAX_CLIENTS; ++i) // 在CanHunterPlayerCount个玩家里选择第rHunter个玩家为猎人
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer)
			{
				if(pPlayer->GetTeam() != TEAM_SPECTATORS &&
				(!pPlayer->m_RespawnDisabled ||
					(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())) &&
						pPlayer->m_CanHunter && (pPlayer->m_Class == CLASS_CIVIC)) // 找到不是观察者、能重生 或 活着的玩家 且 m_CanHunter为真、平民职业的玩家
				{
					if(rHunter == 0) // 找到了第rHunter个玩家 选择为猎人
					{
						pPlayer->SetClass(CLASS_HUNTER);

						pPlayer->SetCanHunter(false); // 把m_CanHunter设为否 即最近当过猎人
						--CanHunterPlayerCount;

						// Generate Hunter info message 生成猎人列表消息
						str_append(HunterList, Server()->ClientName(i), sizeof(HunterList));
						str_append(HunterList, ", ", sizeof(HunterList));

						if(iHunter != 1) // 最后一次循环 要循环全体玩家
							break;
					}
					--rHunter;
				}

				if(iHunter == 1) // 最后一次循环 所有玩家都选择了职业
					if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()) // 这个玩家出生了 所以OnCharacterSpawn已经过了 在这里给他们Class提示和武器
					{
						ResetPlayerClass(pPlayer->GetCharacter());
						OnClassSpawn(pPlayer->GetCharacter());
					}
			}
		}
	}

	if(MatchFlag == -1)
	{
		if(!m_BroadcastHunterList)
			for(int i = 0; i < MAX_CLIENTS; ++i) // 循环所有旁观者 把猎人列表告诉他们
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(pPlayer)
				{
						if(pPlayer->GetTeam() == TEAM_SPECTATORS &&
							!(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()))
						{
							GameServer()->SendChatTarget(pPlayer->GetCID(), HunterList);
					}
				}
			}
		else
		{
			SendChatRoom(HunterList);
		}
	}
}

void CGameControllerHunterN::OnWorldReset() // 重置部分值和触发职业选择
{
	m_GameFlags = IGF_SURVIVAL | IGF_SUDDENDEATH;
	DoWinchenkClassTick = -1;

	SelectClass();
}

void CGameControllerHunterN::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);

	if(m_GameState == IGS_GAME_RUNNING) // 如果游戏开始
	{
		OnClassSpawn(pChr);
	}
}

void CGameControllerHunterN::OnPlayerJoin(class CPlayer *pPlayer) // 使新进旁观者收到猎人列表
{
	if((m_GameState == IGS_GAME_RUNNING) && (pPlayer->GetTeam() == TEAM_SPECTATORS))
	{
		GameServer()->SendChatTarget(pPlayer->GetCID(), HunterList);
	}
}

int CGameControllerHunterN::OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) // 使Hunter不受到自己的伤害
{
	if((pChr->GetPlayer()->GetCID() == From && pChr->GetPlayer()->GetClass() == CLASS_HUNTER) || // Hunter不能受到来自自己的伤害（这样就不会被逆天榴弹自爆）
		(MatchFlag != -1 && From != MatchFlag && pChr->GetPlayer()->GetCID() != MatchFlag)) // 如果打Jug 则去除友伤
			return DAMAGE_NO_DAMAGE | DAMAGE_NO_INDICATOR;
	return DAMAGE_NORMAL;
}

int CGameControllerHunterN::OnPickup(CPickup *pPickup, CCharacter *pChar, SPickupSound *pSound) // Juggernaut不能捡东西
{
	if(pChar->GetPlayer()->m_Class != CLASS_JUG || ((pPickup->GetType() == POWERUP_HEALTH) || (pPickup->GetType() == POWERUP_ARMOR)))
		return IGameController::OnPickup(pPickup, pChar, pSound);
	return -1;
}

void CGameControllerHunterN::ClassWin(int Flag) // 游戏结束
{
	if(Flag == (FLAG_WIN_CIVIC | FLAG_WIN_HUNTER))
	{
		SendChatRoom("两人幸终！");
	}
	else if(Flag == FLAG_WIN_HUNTER)
	{
		SendChatRoom("Hunter猎人胜利！");
		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
	}
	else if(Flag == FLAG_WIN_CIVIC)
	{
		SendChatRoom(HunterList);
		SendChatRoom("Civic平民胜利！");
	}
	else if(Flag == (FLAG_WIN_JUG | FLAG_WIN_JUG_DEFEAT))
	{
		SendChatRoom("两人幸终！");
	}
	else if(Flag == FLAG_WIN_JUG)
	{
		SendChatRoom("Juggernaut剑圣胜利！");

		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
	}
	else if(Flag == FLAG_WIN_JUG_DEFEAT)
	{
		SendChatRoom("平民和猎人胜利！");
	}
	else if(Flag == FLAG_WIN_NONE)
	{
		SendChatRoom(HunterList);
		SendChatRoom("游戏结束！");

		GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
	}

	int JugCID = MatchFlag;
	MatchFlag = -1;

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer)
		{
			if(JugCID == -1)
			{
				if(pPlayer->m_Class & CLASS_HUNTER)
				{
					pPlayer->SetTeamDiractly(TEAM_BLUE); // 结算界面左边红队为民 右边蓝队为猎 玩家默认红队
					m_aTeamscore[TEAM_BLUE] += (Flag == FLAG_WIN_HUNTER) ? 1 : -1; // 用队伍分数显示有几个猎 正数表示猎人胜利
				}
				else if(pPlayer->m_Class & CLASS_CIVIC)
					m_aTeamscore[TEAM_RED] += (Flag == FLAG_WIN_CIVIC) ? 1 : -1; // 用队伍分数显示有几个民 负数表示猎人胜利
			}
			else if(pPlayer->GetCID() == JugCID)
			{
				pPlayer->m_Score = 0;
				pPlayer->m_HiddenScore = 0;
			}
			
			if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())
				pPlayer->m_HiddenScore += 2; // 存活加分
			if(pPlayer->m_Score > 24 && pPlayer->m_HiddenScore > 5)
				MatchFlag = pPlayer->GetCID(); // Jug判断
			if(pPlayer->m_HiddenScore)
				pPlayer->m_Score += pPlayer->m_HiddenScore; // 添加隐藏分
		}
	}

	m_GameFlags |= IGF_MARK_TEAMS;

	SetGameState(IGS_END_MATCH, m_GameoverTime); // EndMatch();
}

void CGameControllerHunterN::DoWincheckClass() // check for class based win
{
	int CivicCount = 0;
	int HunterCount = 0;
	int Flag = 0;

	for(int i = 0; i < MAX_CLIENTS; ++i) // Count Player
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			if(pPlayer->m_Class == CLASS_CIVIC)
				++CivicCount;
			else if(pPlayer->m_Class == CLASS_HUNTER)
				++HunterCount;
		}
	}

	CPlayer *pPlayer = (GetPlayerIfInRoom(MatchFlag));
	if(MatchFlag == -1 || (pPlayer && pPlayer->m_Class != CLASS_JUG))
	{
		if(!CivicCount)
			Flag |= FLAG_WIN_HUNTER;
		if(!HunterCount)
			Flag |= FLAG_WIN_CIVIC;
	}
	else
	{
		if(!CivicCount && !HunterCount)
			Flag |= FLAG_WIN_JUG;
		if(!(pPlayer && pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive()))
			Flag |= FLAG_WIN_JUG_DEFEAT;
	}

	if(Flag)
		ClassWin(Flag);
}

void CGameControllerHunterN::DoWincheckMatch() // check for time based win
{
	if(!m_SuddenDeath && m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) >= m_GameInfo.m_TimeLimit * Server()->TickSpeed() * 60)
	{
		ClassWin(FLAG_WIN_NONE);
	}
	else if(DoWinchenkClassTick != -1)
	{
		--DoWinchenkClassTick;

		if(DoWinchenkClassTick == 0)
			DoWincheckClass();
	}
}

int CGameControllerHunterN::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) // 杀手隐藏分增减 和受害人职业死亡消息 以及延时终局
{
	if(m_GameState == IGS_GAME_RUNNING)
	{
		if(pVictim->GetPlayer()->GetClass() == CLASS_HUNTER) // 猎人死亡
		{
			if(pKiller && pKiller != pVictim->GetPlayer() && (MatchFlag == -1))
			{
				if(pKiller->m_Class != CLASS_HUNTER) // 隐藏分添加
					pKiller->m_HiddenScore += 4;
				else
					pKiller->m_HiddenScore -= 2; // Teamkill
			}

			if(m_EffectHunterDeath)
				GameWorld()->CreatePlayerSpawn(pVictim->m_Pos); // 死亡给个出生烟

			char aBuf[48];
			str_format(aBuf, sizeof(aBuf), "Hunter '%s' was defeated!", Server()->ClientName(pVictim->GetPlayer()->GetCID()));

			--nHunter;

			if(m_BroadcastHunterDeath == 1 ||
				!nHunter) // 如果是最后一个Hunter
			{
				SendChatRoom(aBuf); // 直接全体广播
				GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE);
			}
			else
			{
				for(int i = 0; i < MAX_CLIENTS; ++i) // 逐个给所有人根据职业发送死亡消息
				{
					CPlayer *pPlayer = GetPlayerIfInRoom(i);
					if(pPlayer)
					{
						if((m_BroadcastHunterDeath != -1 && pPlayer->m_Class == CLASS_HUNTER) ||
							pPlayer->GetTeam() == TEAM_SPECTATORS)
						{
							GameServer()->SendChatTarget(pPlayer->GetCID(), aBuf); // 给所有猎人广播他们"队友"的死亡消息
							GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE, CmaskOne(pPlayer->GetCID()));
						}
						else
						{
							GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP, CmaskOne(pPlayer->GetCID()));
						}
					}
				}
			}
		}
		else if(pVictim->GetPlayer()->GetClass() == CLASS_CIVIC) // 平民死亡
		{
			if(pKiller && pKiller != pVictim->GetPlayer() && (MatchFlag == -1))
			{
				if(pKiller->m_Class != CLASS_CIVIC) // 隐藏分添加
					pKiller->m_HiddenScore += 1;
				else
					pKiller->m_HiddenScore -= 1; // Teamkill
			}

			GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP);
		}
		else if(pVictim->GetPlayer()->GetClass() == CLASS_JUG)
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
		}

		if(nHunter && (MatchFlag == -1))
			GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), HunterList); // 如果没有猎人 就不要发猎人列表 等EndMatch

		DoWinchenkClassTick = (Server()->TickSpeed() * m_Wincheckdeley / 1000);
	}

	return DEATH_NO_REASON | DEATH_SKIP_SCORE;
}
