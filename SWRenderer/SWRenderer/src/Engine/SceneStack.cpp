#include "SceneStack.h"

CSceneStack::CSceneStack()
{
}

CSceneStack::~CSceneStack()
{
}

void CSceneStack::Clear()
{
	for ( size_t i = 0; i < m_aScenes.size(); i++ )
	{
		SAFE_DELETE( m_aScenes[i] );
	}
}


void CSceneStack::PushScene( CSceneBase* pScene )
{
	m_aScenes.push_back( pScene );
}

void CSceneStack::PopScene()
{
	CSceneBase* pScene = m_aScenes.back();
	SAFE_DELETE( pScene );
	m_aScenes.pop_back();
}


void CSceneStack::Update()
{
	for ( size_t i = 0; i < m_aScenes.size(); i++ )
	{
		CSceneBase* pScene = m_aScenes[i];
		pScene->Update();
		if ( pScene->GetKillMe() )
		{
			SAFE_DELETE( pScene );
			m_aScenes.erase( m_aScenes.begin() + i );
			i--;
		}
	}
}

void CSceneStack::Render()
{
	for ( size_t i = 0; i < m_aScenes.size(); i++ )
	{
		m_aScenes[i]->Render();
	}
}


bool CSceneStack::On_KeyDown(uint32_t key)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_KeyDown( key ) )
		{
			return true;
		}
	}
	return false;
}

bool CSceneStack::On_KeyUp(uint32_t key)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_KeyUp( key ) )
		{
			return true;
		}
	}
	return false;
}

bool CSceneStack::On_MouseMove(int deltax, int deltay)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_MouseMove( deltax, deltay ) )
		{
			return true;
		}
	}
	return false;
}

bool CSceneStack::On_MouseButtonDown(uint32_t button)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_MouseButtonDown( button ) )
		{
			return true;
		}
	}
	return false;
}

bool CSceneStack::On_MouseButtonUp(uint32_t button)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_MouseButtonUp( button ) )
		{
			return true;
		}
	}
	return false;
}

bool CSceneStack::On_MouseWheel(int iDelta)
{
	for ( int i = (int)m_aScenes.size() - 1; i >= 0; i-- )
	{
		if ( m_aScenes[i]->On_MouseWheel( iDelta ) )
		{
			return true;
		}
	}
	return false;
}
