class CfgPatches
{
	class task_force_radio
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 1.0;
		requiredAddons[] = { "CBA_Main", "cba_settings", "task_force_radio_items"};
		author = "[TF]Nkey";
		authorUrl = "https://github.com/michail-nikolaev/task-force-arma-3-radio";
		url="https://github.com/michail-nikolaev/task-force-arma-3-radio";
		version = "0.9.13";
		versionStr = "0.9.13";
		versionAr[] = {0,9,13};
	};
};

#include "CfgFunctions.h"
class task_force_radio_settings {
};
#include "description.h"
#include "RscTitles.hpp"

class CfgSounds
{
	class TFAR_rotatorPush
	{
		name = "TFAR - Rotator Switch (Push)";
		sound[] = {"\task_force_radio\sounds\hardPush.wss",0.5,1};
		titles[] = {};
	};
	class TFAR_rotatorClick
	{
		name = "TFAR - Rotator Switch (Click)";
		sound[] = {"\A3\ui_f\data\sound\RscButton\soundEscape.wss",0.5,1};
		titles[] = {};
	};
};