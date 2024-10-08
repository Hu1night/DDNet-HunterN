/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>
#include <game/server/classes.h>
#include <game/server/weapons.h>

class CGameControllerHunterN : public IGameController
{
//protected: // used class & struct

protected: // config
	int m_HunterRatio;
	//int m_BroadcastHunterList;
	int m_BroadcastHunterDeath;
	int m_EffectHunterDeath;
	int m_Wincheckdeley;
	int m_GameoverTime;
	//int m_RoundMode;

public: // Maprotation
	enum { MAX_MAPROTATIONS = 32, };
	int m_aMaprotation[MAX_MAPROTATIONS]; // 存储MapIndex的数组

public:
	CGameControllerHunterN();

	// function
	void OnClassChange(CCharacter *pChr);	
	static void OnClassReset(CCharacter *pChr);
	bool CycleMap();

	// game event
	void OnInit() override;
	void OnWorldReset() override;
	void OnPreEntitySnap(int SnappingClient, int OtherMode) override; // per tick
	void DoWincheckMatch() override;
	void DoWincheckRound() override; // per tick
	// player event
	//bool CanDeadPlayerFreeView(const class CPlayer *pSpectator) override { return true; }
	//bool CanDeadPlayerFollow(const CPlayer *pSpectator, const CPlayer *pTarget) override { return true; }
	bool CanChangeTeam(CPlayer *pPlayer, int JoinTeam) const override;
	void OnPlayerJoin(class CPlayer *pPlayer) override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override;
	//int OnPickup(CPickup *pPickup, CCharacter *pChar, SPickupSound *pSound) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;

protected: // Intelnal function and value
	int m_NumHunter; // 有多少个猎人
	int m_DoWinchenkClassTick; // 终局判断延迟的Tick
	char m_HunterList[256]; // 猎人列表
	//int m_aNumTeamPlayer[2]; // 存储每个队伍有多少玩家
	//int TeamClass[1];
	//int MatchFlag = -1;

	enum { HUNTERN_GAMEFLAGS = IGF_SURVIVAL | IGF_ROUND_TIMER_ROUND | IGF_SUDDENDEATH | IGF_MARK_MATCH | IGF_MARK_AMONGUS, };
	enum HUNTERN_WINFLAG
	{
		FLAG_WIN_NONE = 0,
		FLAG_WIN_NO_ONE = 1,
		FLAG_WIN_TEAMRED = 2,
		FLAG_WIN_TEAMBLUE = 4,
	};

	const int m_aKillScore[NUM_CLASSES][2] = // Kill & TeamKill Score
	{
		{0, 0}, // CLASS_NONE
		{1, -1}, // CLASS_CIVIC
		{4, -2}, // CLASS_HUNTER
		{100, -114514}, // CLASS_JUGGERNAUT
		//{4, -2}, // CLASS_PUPPETEE
		//{8, -2}, // CLASS_JOJO
	};

	/*const int m_aaClassReplaceWeapons[NUM_CLASSES][NUM_WEAPON_SLOTS] =
	{
		{WEAPON_ID_HUNTHAMMER},
	};*/

	const char* m_apClassSpawnMsg[NUM_CLASSES] =
	{
		{"你是平民Civic! 找出并消灭猎人以胜利!     \n猎人双倍伤害 有瞬杀锤子和高爆榴弹"},
		{"     你是猎人Hunter! 合作消灭平民以胜利!\n     猎人双倍伤害 有瞬杀锤子和高爆榴弹\n     能长按锤子追踪最近玩家和无伤榴弹跳"},
		{"     你是剑圣Juggernaut! 剑圣7心30盾\n     jug盾截停子弹 再打就发射\n     左键长按禁用盾反并蓄力近战"},
		//{"     你是傀儡师Puppetee! 消灭敌方队伍胜利!\n     猎人双倍伤害 有瞬杀锤子和破片榴弹"},
	};

	const char* m_apWeaponName[7] =
	{
		{"地刺"},
		{"锤子"},
		{"手枪"},
		{"霰弹"},
		{"榴弹"},
		{"激光"},
		{"忍者刀"},
	};
};

#endif