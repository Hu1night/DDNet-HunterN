/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_LASERINDICATOR_H
#define GAME_SERVER_ENTITIES_LASERINDICATOR_H

#include <game/server/entity.h>

class CLaserIndicator : public CEntity
{
public:
	CLaserIndicator(
		CGameWorld *pGameWorld,
		int Owner,
		vec2 From,
		vec2 To,
		int Fineness);
	~CLaserIndicator();

	virtual void Reset() override;
	virtual void Tick() override;
	virtual bool NetworkClipped(int SnappingClient) override;
	virtual void Snap(int SnappingClient, int OtherMode) override;
	virtual void Destroy() override;

	vec2 m_From;

private:
	int m_Owner;
	int m_ID;
	int m_Fineness;
};

#endif
