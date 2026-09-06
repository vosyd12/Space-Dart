#ifndef GAMESETTING_H
#define GAMESETTING_H

#include "Common.h"

struct GameSetting
{
	bool fullScreen = false;
	bool invertMouse = false;
	bool isEnglish = false;

	float mouseSensitivity = 0.5f;
	float luminosity = 0.5f;
	float fov = 60.0f;

	int frameLimit = 60;
	int frameIndex = 1;
	int frameValues[4] = { 30, 60, 120, 240 };

	int BGM = 50;
	int SFX = 50;

	Step step;
	GameType type;
};

extern GameSetting g_GameSetting;

#endif