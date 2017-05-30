// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "AnimatedCharacterEventProxies.h"
#include "AnimatedCharacter.h"

#include "CryActionCVars.h"

#include <CryEntitySystem/IEntityComponent.h>

#include <CryExtension/ClassWeaver.h>
#include <CryExtension/CryCreateClassInstance.h>

//////////////////////////////////////////////////////////////////////////

CRYREGISTER_CLASS(CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate)
CRYREGISTER_CLASS(CAnimatedCharacterComponent_StartAnimProc)
CRYREGISTER_CLASS(CAnimatedCharacterComponent_GenerateMoveRequest)

CAnimatedCharacterComponent_Base::CAnimatedCharacterComponent_Base()
	: m_pAnimCharacter(nullptr)
{
}

void CAnimatedCharacterComponent_Base::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		OnPrePhysicsUpdate(event.fParam[0]);
		break;
	}
}

uint64 CAnimatedCharacterComponent_Base::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_PREPHYSICSUPDATE);
}

//////////////////////////////////////////////////////////////////////////

CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate()
	: m_queuedRotation(IDENTITY)
	, m_hasQueuedRotation(false)
{
}

void CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::OnPrePhysicsUpdate(float)
{
	CRY_ASSERT(m_pAnimCharacter);

	if (CAnimationGraphCVars::Get().m_useQueuedRotation && m_hasQueuedRotation)
	{
		m_pEntity->SetRotation(m_queuedRotation, ENTITY_XFORM_USER | ENTITY_XFORM_NOT_REREGISTER);
		ClearQueuedRotation();
	}

	m_pAnimCharacter->PrepareAnimatedCharacterForUpdate();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::GetEventPriority(const int eventID) const
{
	CRY_ASSERT(m_pAnimCharacter);

	switch (eventID)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_PrepareAnimatedCharacterForUpdate;

			if (m_pAnimCharacter->GetEntityId() == CCryAction::GetCryAction()->GetClientActorId())
			{
				// we want the client StartAnimProc to happen after both CActor and GenMoveRequest.
				priority += EEntityEventPriority_Client;
			}

			return priority;
		}
	}
	return(ENTITY_PROXY_LAST - ENTITY_PROXY_USER);
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_StartAnimProc::OnPrePhysicsUpdate(float elapsedTime)
{
	CRY_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->PrepareAndStartAnimProc();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_StartAnimProc::GetEventPriority(const int eventID) const
{
	CRY_ASSERT(m_pAnimCharacter);

	switch (eventID)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_StartAnimProc;

			if (m_pAnimCharacter->GetEntityId() == CCryAction::GetCryAction()->GetClientActorId())
			{
				// we want the client StartAnimProc to happen after both CActor and GenMoveRequest.
				priority += EEntityEventPriority_Client;
			}
			return priority;
		}
	}

	return(ENTITY_PROXY_LAST - ENTITY_PROXY_USER);
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_GenerateMoveRequest::OnPrePhysicsUpdate(float elapsedTime)
{
	CRY_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->GenerateMovementRequest();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_GenerateMoveRequest::GetEventPriority(const int eventID) const
{
	CRY_ASSERT(m_pAnimCharacter);

	switch (eventID)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_AnimatedCharacter;

			if (m_pAnimCharacter->GetEntityId() == CCryAction::GetCryAction()->GetClientActorId())
			{
				priority += EEntityEventPriority_Client;
			}
			return priority;
		}
	}

	return(ENTITY_PROXY_LAST - ENTITY_PROXY_USER);
}
