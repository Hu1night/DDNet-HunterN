#include "puppeteehammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CPuppeteeHammer::CPuppeteeHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = 10;
	m_AmmoRegenTime = 1;
	m_FireDelay = 500;
	m_IsKeepFiring = false;
	m_PrevTickPuppets = 0;
	mem_zero(m_aPuppetCID, sizeof(m_aPuppetCID));
	mem_zero(m_aPuppetsDirPos, sizeof(m_aPuppetsDirPos));
}

void CPuppeteeHammer::Tick()
{
	CWeapon::Tick();

	int CharCount = 0;
	CCharacter *apChar[MAX_PUPPETS] = {0};

	for(int i = 0; i < MAX_PUPPETS; i++)
	{
		if(!m_aPuppetCID[i])
			continue;

		apChar[i] = GameServer()->GetPlayerChar(m_aPuppetCID[i] - 1);
		if(apChar[i])
			CharCount++;
		else
			m_aPuppetCID[i] = 0;
	}

	if(!CharCount) // Just do nothing
	{}
	else if((Character()->GetLatestPrevPrevInput().m_Fire & 1) && !(Character()->GetLatestInput().m_Fire & 1) && IsReloading()) // 松发让傀儡开火
	{
		vec2 AimPos = Pos() + Character()->GetAimPos();

		for(int i = 0; i < MAX_PUPPETS; i++)
		{
			CCharacter *pChr = apChar[i];
			if(!pChr)
				continue;

			CWeapon *pChrWeapon = pChr->CurrentWeapon();
			if(!pChrWeapon)
				continue;

			vec2 AimDir = normalize(AimPos - pChr->m_Pos);

			pChrWeapon->HandleFire(AimDir);
		}
	}
	else if((m_IsKeepFiring && !IsReloading()) // Pressing
		|| (m_PrevTickPuppets != CharCount)) // Update
	{
		vec2 OwnerDir = Character()->GetDirection();
		vec2 PuppetsDir = OwnerDir * clamp(length(Character()->GetAimPos()), 48.f, 128.f);
		int a = 0;

		for(int i = 0; i < MAX_PUPPETS; i++)
		{
			if(!m_aPuppetCID[i])
				continue;

			a++;
			m_aPuppetsDirPos[i] = PuppetsDir + (vec2(-OwnerDir.y, OwnerDir.x) * ((a / 2) * 32.f * (a % 2 ? 1 : -1) + ((CharCount % 2) * 16.f)));
		}
	}

	if(CharCount)
	{
		//SetPowerUpWeaponPointer(this);

		int OwnerQueuedWeaponSlot = Character()->GetLatestInput().m_WantedWeapon - 1;

		for(int i = 0; i < MAX_PUPPETS; i++)
		{
			CCharacter *pChr = apChar[i];
			if(!pChr)
				continue;

			//pChr->Freeze(5, true);

			pChr->SetQueuedWeaponSlot(OwnerQueuedWeaponSlot);

			// TODO: replace MoveBox
			pChr->Core()->m_Vel = Pos() + m_aPuppetsDirPos[i] - pChr->Core()->m_Pos;
			GameServer()->Collision()->MoveBox(&pChr->Core()->m_Pos, &pChr->Core()->m_Vel, vec2(GetProximityRadius(), GetProximityRadius()), 0.f);
			pChr->Core()->m_Vel = vec2(0.f, 0.f);
		}
	}

	m_IsKeepFiring = m_IsKeepFiring ? (Character()->GetLatestInput().m_Fire & 1) : false;
	m_PrevTickPuppets = CharCount;
}

void CPuppeteeHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);

	m_IsKeepFiring = true;

	if(Character()->IsSolo() || Character()->m_Hit & CCharacter::DISABLE_HIT_HAMMER)
		return;

	vec2 HammerHitPos = Pos() + Direction * GetProximityRadius() * 0.75f;

	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(HammerHitPos, GetProximityRadius() * 0.5f, (CEntity **)apEnts,
		MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for(int i = 0; i < Num; ++i)
	{
		CCharacter *pTarget = apEnts[i];

		//if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
		if((pTarget == Character() || (pTarget->IsAlive() && pTarget->IsSolo())))
			continue;

		bool IsPuppet = false;
		int TargetCID = pTarget->GetPlayer()->GetCID();
		for(int i = 0; i < MAX_PUPPETS; i++) // Check Puppet
		{
			if(TargetCID != m_aPuppetCID[i] - 1)
				continue;

			IsPuppet = true;
			break;
		}
		if(IsPuppet)
			continue;

		for(int i = 0; i < MAX_PUPPETS; i++) // Set Puppet
		{
			if(m_aPuppetCID[i])
				continue;

			m_aPuppetCID[i] = TargetCID + 1;
			break;
		}

		// set his velocity to fast upward (for now)
		if(length(pTarget->m_Pos - HammerHitPos) > 0.0f)
			GameWorld()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - HammerHitPos) * GetProximityRadius() * 0.5f);
		else
			GameWorld()->CreateHammerHit(HammerHitPos);

		pTarget->TakeDamage(vec2(0.f, -0.5f), 0, ClientID, WEAPON_HAMMER, GetWeaponID(), false); // 法师近战

		GameServer()->Antibot()->OnHammerHit(ClientID);
	}
}