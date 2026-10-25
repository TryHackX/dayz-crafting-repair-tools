
class CfgPatches {
	class CraftingRepairTools {
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.2;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
		author = "TryHackX";
		name = "CraftingRepairTools";
	};
};

class CfgMods {
	class CraftingRepairTools {
		name = "Crafting repair tools";
		credits = "TryHackX, Kooftt";
		author = "TryHackX";
		authorID = "76561199846115789";
		version = "1.1";
		extra = 0;
		type = "mod";
		dependencies[]=
		{
			"World",
			"Mission"
		};

		class defs
		{
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"CraftingRepairTools/Scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"CraftingRepairTools/Scripts/5_Mission"
				};
			};
		};
	};
};