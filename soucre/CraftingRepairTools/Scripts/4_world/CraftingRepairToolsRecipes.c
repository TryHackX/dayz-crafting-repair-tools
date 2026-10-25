// One generic recipe, configured from CRT_Config. Client and server register the same number of slots
// at startup (recipe IDs must match), the config fills them later.
class CRT_ConfigRecipe extends RecipeBase
{
	protected ref CRT_RecipeSettings m_CRT;

	override void Init()
	{
		m_Name = "Crafting Repair Tools";
		m_IsInstaRecipe = false;
		m_AnimationLength = 2;
		m_Specialty = 0;
	}

	//! null = unused slot
	void CRT_Configure(CRT_RecipeSettings settings)
	{
		m_CRT = settings;
		for (int i = 0; i < MAX_NUMBER_OF_INGREDIENTS; i++)
		{
			m_Ingredients[i].Clear();
			m_SoundCategories[i].Clear();
		}
		m_NumberOfResults = 0;

		if (!settings)
			return;

		m_Name = settings.Name;
		CRT_SetIngredient(0, settings.Ingredient1);
		CRT_SetIngredient(1, settings.Ingredient2);

		AddResult(settings.Result);
		m_ResultSetFullQuantity[0] = true;
		m_ResultSetQuantity[0] = -1;
		m_ResultSetHealth[0] = -1;
		m_ResultInheritsHealth[0] = -2;
		m_ResultInheritsColor[0] = -1;
		m_ResultToInventory[0] = -2;
		m_ResultUseSoftSkills[0] = false;
		m_ResultReplacesIngredient[0] = -1;
	}

	protected void CRT_SetIngredient(int index, CRT_IngredientSettings ing)
	{
		foreach (string type : ing.Types)
			InsertIngredient(index, type);

		m_MinDamageIngredient[index] = -1;
		m_MaxDamageIngredient[index] = 3;
		m_MinQuantityIngredient[index] = ing.Amount;	// ignored by vanilla for items without quantity
		m_MaxQuantityIngredient[index] = -1;
		m_IngredientAddHealth[index] = 0;				// tool wear is applied in Do
		m_IngredientSetHealth[index] = -1;
		m_IngredientAddQuantity[index] = 0;
		if (!ing.Destroy)
			m_IngredientAddQuantity[index] = -ing.Amount;
		m_IngredientDestroy[index] = ing.Destroy;
		m_IngredientUseSoftSkills[index] = false;
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if (!m_CRT || !m_CRT.Enabled)
			return false;

		return CRT_HasEnough(ingredients[0], m_CRT.Ingredient1, player) && CRT_HasEnough(ingredients[1], m_CRT.Ingredient2, player);
	}

	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		// crafted quantity = average of the selected quality / quantity factors, like the original recipes
		float sum = 0;
		int terms = 0;
		array<EntityAI> extra1 = new array<EntityAI>;
		array<EntityAI> extra2 = new array<EntityAI>;
		CRT_CollectFactors(ingredients[0], m_CRT.Ingredient1, player, extra1, sum, terms);
		CRT_CollectFactors(ingredients[1], m_CRT.Ingredient2, player, extra2, sum, terms);

		ItemBase resultItem = results.Get(0);
		if (resultItem && resultItem.HasQuantity())
		{
			float factor = 1;
			if (terms > 0)
				factor = sum / terms;

			float maxQuantity = resultItem.GetQuantityMax();
			resultItem.SetQuantity(Math.Min(maxQuantity * factor * m_CRT.ResultMaxPercent / 100, maxQuantity));
		}

		CRT_Wear(ingredients[0], m_CRT.Ingredient1);
		CRT_Wear(ingredients[1], m_CRT.Ingredient2);

		foreach (EntityAI e1 : extra1)
			e1.Delete();
		foreach (EntityAI e2 : extra2)
			e2.Delete();
	}

	//! Items without quantity (e.g. paper) with Amount > 1: the other pieces come from the player's inventory
	protected bool CRT_HasEnough(ItemBase item, CRT_IngredientSettings ing, PlayerBase player)
	{
		if (ing.Amount <= 1 || item.HasQuantity())
			return true;

		return CRT_FindExtraItems(item, ing, player, null) >= ing.Amount - 1;
	}

	protected int CRT_FindExtraItems(ItemBase item, CRT_IngredientSettings ing, PlayerBase player, array<EntityAI> found)
	{
		int count = 0;
		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI e : items)
		{
			if (e == item || e.IsSetForDeletion() || !CRT_IsType(e, ing.Types))
				continue;

			count++;
			if (found)
				found.Insert(e);
			if (count >= ing.Amount - 1)
				break;
		}
		return count;
	}

	protected bool CRT_IsType(EntityAI e, array<string> types)
	{
		foreach (string type : types)
		{
			if (e.IsKindOf(type))
				return true;
		}
		return false;
	}

	protected void CRT_CollectFactors(ItemBase item, CRT_IngredientSettings ing, PlayerBase player, array<EntityAI> extra, inout float sum, inout int terms)
	{
		if (ing.Amount > 1 && !item.HasQuantity())
			CRT_FindExtraItems(item, ing, player, extra);

		if (ing.QualityFactor)
		{
			float health = item.GetHealth01("", "");
			foreach (EntityAI e : extra)
				health += e.GetHealth01("", "");

			sum += health / (extra.Count() + 1);
			terms++;
		}

		if (ing.QuantityFactor && item.HasQuantity() && item.GetQuantityMax() > 0)
		{
			sum += item.GetQuantity() / item.GetQuantityMax();
			terms++;
		}
	}

	protected void CRT_Wear(ItemBase item, CRT_IngredientSettings ing)
	{
		if (!ing.Destroy && ing.DamagePercent > 0)
			item.AddHealth("", "", -item.GetMaxHealth("", "") * ing.DamagePercent / 100);
	}
};

modded class PluginRecipesManagerBase
{
	override void RegisterRecipies()
	{
		super.RegisterRecipies();
		for (int i = 0; i < CRT_Config.MAX_RECIPES; i++)
			RegisterRecipe(new CRT_ConfigRecipe);
	}
}

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == CRT_Config.RPC_SYNC_CONFIG && GetGame().IsClient())
			CRT_Config.OnReceive(ctx);
	}
}
