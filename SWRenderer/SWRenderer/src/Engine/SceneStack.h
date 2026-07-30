#pragma once
#include <vector>
#include "Common/Globals.h"
#include "Engine/UI/UIBase.h"

class CSceneBase
{
public:
	CSceneBase()
	{
		m_pUIRoot = nullptr;
		m_bKillMe = false;
	}

	virtual ~CSceneBase()
	{
		SAFE_DELETE( m_pUIRoot );
	}

	virtual void Update()
	{
		if ( m_pUIRoot )
		{
			m_pUIRoot->Update();
		}
	}

	virtual void Render()
	{
		if ( m_pUIRoot )
		{
			m_pUIRoot->Render();
		}
	}

	virtual bool On_KeyDown(uint32_t key) { return false; }
	virtual bool On_KeyUp(uint32_t key) { return false; }
	virtual bool On_MouseMove(int deltax, int deltay) { return false; }
	virtual bool On_MouseButtonDown(uint32_t button) { return false; }
	virtual bool On_MouseButtonUp(uint32_t button) { return false; }
	virtual bool On_MouseWheel(int iDelta) { return false; }

	bool		GetKillMe() const { return m_bKillMe; }

protected:
	virtual void OnCreate() {}
	virtual void OnClear() {}

	void		SetKillMe( bool bKillMe ) { m_bKillMe = bKillMe; }

protected:
	CUIBase*	m_pUIRoot;

private:
	bool		m_bKillMe;
};

class CSceneMainMenu : public CSceneBase
{
public:
	CSceneMainMenu()
	{
		m_pUIRoot = new CUIPane();
		{
			CUIButton* pButton = new CUIButton();
			pButton->SetPosition( 10, 10 );
			pButton->SetSize( 3, 3 );
			m_pUIRoot->AddChild( pButton );
		}

		{
			CUIButton* pButton = new CUIButton();
			pButton->SetPosition( 10, 25 );
			pButton->SetSize( 4, 4 );
			m_pUIRoot->AddChild( pButton );
		}
	}

	virtual ~CSceneMainMenu()
	{
	}

	virtual void Update()
	{
		CSceneBase::Update();

	}

	virtual void Render()
	{
		CSceneBase::Render();

	}
};

class CSceneStack
{
public:
	CSceneStack();
	~CSceneStack();
	void Clear();

	void PushScene( CSceneBase* pScene );
	void PopScene();

	void Update();
	void Render();

	bool On_KeyDown(uint32_t key);
	bool On_KeyUp(uint32_t key);
	bool On_MouseMove(int deltax, int deltay);
	bool On_MouseButtonDown(uint32_t button);
	bool On_MouseButtonUp(uint32_t button);
	bool On_MouseWheel(int iDelta);

private:
	std::vector<CSceneBase*>	m_aScenes;
};

