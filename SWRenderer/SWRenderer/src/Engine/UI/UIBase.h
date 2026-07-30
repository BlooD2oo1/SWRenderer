#pragma once
#include <vector>
#include "Engine/Graphics.h"

class CUIBase
{
public:
	CUIBase() {}
	virtual ~CUIBase()
	{
		for ( size_t i = 0; i < m_aChildren.size(); i++ )
		{
			SAFE_DELETE( m_aChildren[i] );
		}
		m_aChildren.clear();
	}
	
	virtual void Update()
	{
		for ( size_t i = 0; i < m_aChildren.size(); i++ )
		{
			m_aChildren[i]->Update();
		}
	}
	virtual void Render()
	{
		for ( size_t i = 0; i < m_aChildren.size(); i++ )
		{
			m_aChildren[i]->Render();
		}
	}
	virtual bool On_KeyDown( uint32_t key ) { return false; }
	virtual bool On_KeyUp( uint32_t key ) { return false; }
	virtual bool On_MouseMove( int deltax, int deltay ) { return false; }
	virtual bool On_MouseButtonDown( uint32_t button ) { return false; }
	virtual bool On_MouseButtonUp( uint32_t button ) { return false; }
	virtual bool On_MouseWheel( int iDelta ) { return false; }

	void	AddChild( CUIBase* pChild )
	{
		pChild->m_pParent = this;
		m_aChildren.push_back( pChild );
	}

	void	SetPosition( int x, int y ) { m_iPosX = x; m_iPosY = y; }
	void	SetSize( int width, int height ) { m_iWidth = width; m_iHeight = height; }

	void	GetAbsolutePosition( int& x, int& y ) const
	{
		x = m_iPosX;
		y = m_iPosY;
		if ( m_pParent )
		{
			int px, py;
			m_pParent->GetAbsolutePosition( px, py );
			x += px;
			y += py;
		}
	}

protected:
	virtual void OnCreate() {}
	virtual void OnClear() {}

protected:
	CUIBase*	m_pParent;
	std::vector<CUIBase*>	m_aChildren;

	int			m_iPosX;
	int			m_iPosY;
	int			m_iWidth;
	int			m_iHeight;
};

class CUIPane : public CUIBase
{
};

class CUIButton : public CUIBase
{
public:
	CUIButton() {}
	virtual ~CUIButton() {}
	virtual void Update()
	{
	}

	virtual void Render()
	{
		BGRA8 sColor( (uint8_t)255, 255, 255, 100 );
		int x = m_iPosX;
		int y = m_iPosY;
		CGraphics::GetInstance().DrawLineH( x, y, m_iWidth, sColor );
		x += m_iWidth-1;
		y += 1;
		CGraphics::GetInstance().DrawLineV( x, y, m_iHeight-1, sColor );
		x -= 1;
		y += m_iHeight-2;
		CGraphics::GetInstance().DrawLineH( x, y, -(m_iWidth-1), sColor );
		x -= m_iWidth - 2;
		y -= 1;
		CGraphics::GetInstance().DrawLineV( x, y, -(m_iHeight-2), sColor );	
	}
	virtual bool On_KeyDown( uint32_t key ) { return false; }
	virtual bool On_KeyUp( uint32_t key ) { return false; }
	virtual bool On_MouseMove( int deltax, int deltay ) { return false; }
	virtual bool On_MouseButtonDown( uint32_t button ) { return false; }
	virtual bool On_MouseButtonUp( uint32_t button ) { return false; }
	virtual bool On_MouseWheel( int iDelta ) { return false; }
};