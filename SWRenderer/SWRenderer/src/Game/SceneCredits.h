#pragma once

#include "Common/Globals.h"
#include "Engine/Graphics.h"

class CSceneCredits
{
public:
	CSceneCredits();
	~CSceneCredits();
	void Clear();
	void Create();
	void Update();
	void Render();

	bool On_KeyDown( uint32_t key );
	bool On_KeyUp( uint32_t key );

};