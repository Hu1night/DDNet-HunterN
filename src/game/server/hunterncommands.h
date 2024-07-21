#ifndef GAME_SERVER_HUNTERN_COMMANDS_H
#define GAME_SERVER_HUNTERN_COMMANDS_H

//#include <game/server/entities/textentity.h>

static void ConMapAdd(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	char aBuf[256];

	const char *pMapName;
	int MapIndex;
	int IsError = 1; // 1 = 地图循环没有空位
	int i = 0;

	for(int j = 0; j < pResult->NumArguments(); ++j) // 逐个加入地图到列表
	{
		pMapName = pResult->GetString(j);

		for(; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i) // 遍历地图循环列表
		{
			if(pSelf->m_aMaprotation[i]) // 循环 直到在列表里找到了可用空位
				continue;

			MapIndex = pSelf->GameServer()->Teams()->GetMapIndex(pMapName);
			if(MapIndex == 0)
			{
				IsError = 0; // 0 = 未找到地图
				break; // 跳出到错误提示
			}

			pSelf->m_aMaprotation[i] = MapIndex; // 加入循环列表

			str_format(aBuf, sizeof(aBuf), "Added map%d '%s' to slot %d", j, pMapName, i);
			pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);

			goto NextInputedMap; // 试图加入下一个地图
		}
		str_format(aBuf, sizeof(aBuf), "Cannot add map%d '%s' to slot %d (%s)", j, pMapName, i, (IsError ? "Out of Range" : "No Map Found"));
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);

		NextInputedMap:;
	}
}

static void ConMapList(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", "Value: ");

	char aBuf[256];

	for(int i = 0; i < CGameControllerHunterN::MAX_MAPROTATIONS; ++i) // 遍历地图循环列表
	{
		if(!pSelf->m_aMaprotation[i]) // 列表走到了尽头
			break;

		const char *pMapName = pSelf->GameServer()->Teams()->GetMapName(pSelf->m_aMaprotation[i]);
		if(!pMapName)
			continue;

		str_format(aBuf, sizeof(aBuf), "Slot%d | ID%d - %s, ", i, pSelf->m_aMaprotation[i], pMapName);
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf); // 输出
	}
}

static void ConMapRemove(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	int TargetMap = pResult->GetInteger(0);

	if(!pSelf->m_aMaprotation[TargetMap])
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Cannot remove map in empty slot %d", TargetMap);
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);
		return;
	}

	mem_copy(&pSelf->m_aMaprotation[TargetMap], &pSelf->m_aMaprotation[TargetMap + 1], CGameControllerHunterN::MAX_MAPROTATIONS - 1 - TargetMap);
	pSelf->m_aMaprotation[CGameControllerHunterN::MAX_MAPROTATIONS] = 0;

	char aBuf[32];
	str_format(aBuf, sizeof(aBuf), "Removed map in slot %d", TargetMap);
	pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "instance", aBuf);
}

static void ConMapClear(IConsole::IResult *pResult, void *pUserData)
{
	CGameControllerHunterN *pSelf = (CGameControllerHunterN *)pUserData;

	mem_zero(pSelf->m_aMaprotation, sizeof(pSelf->m_aMaprotation)); // 清空地图循环列表
}

static void ConSetClass(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom(pResult->NumArguments() > 1 ? pResult->GetInteger(1) : pResult->m_ClientID);
	if(!pPlayer) // If the player does not exist
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else
	{	
		int PrevPlayerClass = pPlayer->m_Class;
		pPlayer->m_Class = pResult->GetInteger(0); // Set Class
		if(pResult->NumArguments() > 2)
			pPlayer->m_AmongUsTeam = pResult->GetInteger(2); // Team

		if(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive() && PrevPlayerClass != pPlayer->m_Class)
		{
			CGameControllerHunterN::OnClassReset(pPlayer->GetCharacter()); // reset class after OnCharacterSpawn
			pSelf->GameWorld()->CreatePlayerSpawn(pPlayer->GetCharacter()->m_Pos);
		}
		if(pResult->NumArguments() > 3)
			pPlayer->m_UseHunterWeapon = pResult->GetInteger(3); // Override m_UseHunterWeapon after OnResetClass/OnCharacterSpawn
	}
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
		if(!pResult->GetInteger(4)) // Give Weapon
			pPlayer->GetCharacter()->GiveWeapon((pResult->GetInteger(1) < NUM_WEAPONS && pResult->GetInteger(1) >= 0) ? pResult->GetInteger(1) : 0, // Slot
				pResult->GetInteger(0), // Type
					(pResult->NumArguments() > 3) ? pResult->GetInteger(3) : -1); // ammo
		else // Powerup Weapon
			pPlayer->GetCharacter()->SetPowerUpWeapon(pResult->GetInteger(0), // Type
					(pResult->NumArguments() > 3) ? pResult->GetInteger(3) : -1); // ammo
	}
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

static void ConSign(IConsole::IResult *pResult, void *pUserData)
{
	IGameController *pSelf = (IGameController *)pUserData;

	CPlayer *pPlayer = pSelf->GetPlayerIfInRoom(pResult->m_ClientID);
	if(!pPlayer) // If the player does not exist
		pSelf->InstanceConsole()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "huntern", "invalid client id");
	else
		new CTextEntity(pSelf->GameWorld(), pPlayer->m_ViewPos, CTextEntity::TYPE_GUN, 8, CTextEntity::ALIGN_MIDDLE, (char *)pResult->GetString(0));
}

#endif