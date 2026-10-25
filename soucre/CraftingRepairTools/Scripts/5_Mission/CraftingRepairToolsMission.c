modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		CRT_Config.Load();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CRT_SendConfig, 2000, false, player);
	}

	protected void CRT_SendConfig(PlayerBase player)
	{
		CRT_Config.SendTo(player);
	}
}

// offline / single player: no MissionServer, the client mission loads the config itself
modded class MissionGameplay
{
	override void OnInit()
	{
		super.OnInit();
		if (GetGame().IsServer())
			CRT_Config.Load();
	}
}
