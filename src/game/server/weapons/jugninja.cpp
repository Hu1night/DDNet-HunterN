#include "jugninja.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CJugNinja::CJugNinja(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_NINJA].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_NINJA].m_Ammoregentime;
	m_FireDelay = 340;
	m_OldVelAmount = 0;
	m_CurrentMoveTime = -1;
	m_ActivationDir = vec2(0, 0);
	m_NumObjectsHit = 0;
}

void CJugNinja::Fire(vec2 Direction)
{
	CCharacter *pChr = ((CCharacter *)GameWorld()->ClosestEntity(Pos(), 768.f, CGameWorld::ENTTYPE_CHARACTER, Character()));
	if(!pChr)
		return;

	m_NumObjectsHit = 0;

	m_ActivationDir = Direction;
	m_CurrentMoveTime = 140 * Server()->TickSpeed() / 1000;
	m_OldVelAmount = length(Character()->Core()->m_Vel);

	GameWorld()->CreateSound(Pos(), SOUND_NINJA_FIRE);
}

void CJugNinja::Tick()
{
	CWeapon::Tick();

	if(!IsReloading)

	if(m_CurrentMoveTime >= 0)
		m_CurrentMoveTime--;

	if(m_CurrentMoveTime == 0)
	{
		// reset velocity
		Character()->Core()->m_Vel = m_ActivationDir * m_OldVelAmount;
	}

	if(m_CurrentMoveTime > 0)
	{
		// Set velocity
		Character()->Core()->m_Vel = m_ActivationDir * 60;
		vec2 OldPos = Pos();
		GameServer()->Collision()->MoveBox(&Character()->Core()->m_Pos, &Character()->Core()->m_Vel, vec2(GetProximityRadius(), GetProximityRadius()), 0.f);

		// reset velocity so the client doesn't predict stuff
		Character()->Core()->m_Vel = vec2(0.f, 0.f);

		// check if we Hit anything along the way
		{
			CCharacter *aEnts[MAX_CLIENTS];
			vec2 Dir = Pos() - OldPos;
			float Radius = GetProximityRadius() * 2.0f;
			vec2 Center = OldPos + Dir * 0.5f;
			int Num = GameWorld()->FindEntities(Center, Radius, (CEntity **)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for(int i = 0; i < Num; ++i)
			{
				if(aEnts[i] == Character())
					continue;

				// make sure we haven't Hit this object before
				bool bAlreadyHit = false;
				for(int j = 0; j < m_NumObjectsHit; j++)
				{
					if(m_apHitObjects[j] == aEnts[i])
						bAlreadyHit = true;
				}
				if(bAlreadyHit)
					continue;

				// check so we are sufficiently close
				if(distance(aEnts[i]->m_Pos, Pos()) > (GetProximityRadius() * 2.0f))
					continue;

				// Hit a player, give him damage and stuffs...
				GameWorld()->CreateSound(aEnts[i]->m_Pos, SOUND_NINJA_HIT);
				// set his velocity to fast upward (for now)
				if(m_NumObjectsHit < 10)
					m_apHitObjects[m_NumObjectsHit++] = aEnts[i];
				aEnts[i]->TakeDamage(vec2(0, -10.0f), g_pData->m_Weapons.m_Ninja.m_pBase->m_Damage,
					Character()->GetPlayer()->GetCID(), WEAPON_NINJA, GetWeaponID(), false);
			}
		}
		{
			CProjectile *aEnts[16];
			int Num = GameWorld()->FindEntities(Pos() + (Character()->GetDirection() * GetProximityRadius() * 2.5f), GetProximityRadius() * 2.0f, (CEntity **)aEnts, 16, CGameWorld::ENTTYPE_PROJECTILE);

			for(int i = 0; i < Num; ++i)
			{
				if(aEnts[i]->GetOwner() == Character()->GetPlayer()->GetCID())
					continue;

				CCharacter *pChr = ((CCharacter *)GameWorld()->ClosestEntity((Pos() * 2) - OldPos, 384.f, CGameWorld::ENTTYPE_CHARACTER, Character()));
				if(pChr)
					vec2 Dir = normalize(pChr->m_Pos - aEnts[i]->m_Pos) * 0.5f;
				else
					vec2 Dir = normalize(aEnts[i]->m_Pos - Character()->m_Pos) * 0.5f;

				aEnts[i]->SetDir(normalize(pChr->m_Pos - aEnts[i]->m_Pos) * 0.5f);
				aEnts[i]->SetStartPos(aEnts[i]->m_Pos);
				aEnts[i]->SetStartTick(Server()->Tick());
				aEnts[i]->SetOwner(Character()->GetPlayer()->GetCID());
				aEnts[i]->m_LifeSpan = 2 * Server()->TickSpeed();

				GameWorld()->CreateHammerHit(aEnts[i]->m_Pos);
			}
		}
	}
}

void CJugNinja::OnUnequip()
{
	if(m_CurrentMoveTime > 0)
		Character()->Core()->m_Vel = m_ActivationDir * m_OldVelAmount;
}
