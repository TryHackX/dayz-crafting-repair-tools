// Server config: $profile:CraftingRepairTools/config.json - every recipe of the mod is defined there.
// The server sends it to each player, both sides then fill the recipe slots (CRT_ConfigRecipe).

class CRT_IngredientSettings
{
    ref array<string> Types;    // accepted item classes (base classes work too)
    int Amount;                 // quantity units (pieces, ml, %), for items without quantity: number of items
    bool Destroy;               // 1 = the whole item is used up, 0 = only Amount is taken (tools are kept)
    float DamagePercent;        // condition lost by a kept item (e.g. the hammer)
    bool QualityFactor;         // condition of this ingredient affects the crafted quantity
    bool QuantityFactor;        // remaining quantity of this ingredient affects the crafted quantity

    void CRT_IngredientSettings()
    {
        Types = new array<string>;
        Amount = 1;
        Destroy = true;
        DamagePercent = 0;
        QualityFactor = true;
        QuantityFactor = false;
    }
}

class CRT_RecipeSettings
{
    string Name;
    bool Enabled;
    string Result;
    float ResultMaxPercent;     // crafted quantity at perfect ingredients
    ref CRT_IngredientSettings Ingredient1;
    ref CRT_IngredientSettings Ingredient2;

    void CRT_RecipeSettings()
    {
        Enabled = true;
        ResultMaxPercent = 100;
        Ingredient1 = new CRT_IngredientSettings();
        Ingredient2 = new CRT_IngredientSettings();
    }
}

class CRT_Settings
{
    int ConfigVersion;
    ref array<ref CRT_RecipeSettings> Recipes;

    void CRT_Settings()
    {
        ConfigVersion = 1;
        Recipes = new array<ref CRT_RecipeSettings>;
    }
}

class CRT_Config
{
    static const int MAX_RECIPES = 40;
    static const int RPC_SYNC_CONFIG = 1129469233;
    static const string CONFIG_FOLDER = "$profile:CraftingRepairTools";
    static const string CONFIG_FILE = "$profile:CraftingRepairTools/config.json";

    protected static ref CRT_Settings m_Settings;
    protected static int m_ApplyRetries;

    static CRT_Settings GetSettings()
    {
        if (!m_Settings)
            m_Settings = CreateDefault();
        return m_Settings;
    }

    //! Server: read the config (or create it), validate it and fill the recipe slots
    static void Load()
    {
        if (!FileExist(CONFIG_FOLDER))
            MakeDirectory(CONFIG_FOLDER);

        if (FileExist(CONFIG_FILE))
        {
            CRT_Settings loaded = new CRT_Settings();
            JsonFileLoader<CRT_Settings>.JsonLoadFile(CONFIG_FILE, loaded);
            m_Settings = loaded;
            Print("[CraftingRepairTools] Config loaded from: " + CONFIG_FILE);
        }
        else
        {
            m_Settings = CreateDefault();
            JsonFileLoader<CRT_Settings>.JsonSaveFile(CONFIG_FILE, m_Settings);
            Print("[CraftingRepairTools] Default config created at: " + CONFIG_FILE);
        }

        Validate(m_Settings);
        Apply();
    }

    //! Server: the config goes to the player as JSON text
    static void SendTo(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        string json;
        JsonSerializer js = new JsonSerializer();
        js.WriteToString(GetSettings(), false, json);

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(json);
        rpc.Send(player, RPC_SYNC_CONFIG, true, player.GetIdentity());
    }

    //! Client: config received from the server
    static void OnReceive(ParamsReadContext ctx)
    {
        string json;
        if (!ctx.Read(json))
            return;

        CRT_Settings received = ParseJson(json);
        if (!received)
            return;

        m_Settings = received;
        Apply();
    }

    static CRT_Settings ParseJson(string json)
    {
        CRT_Settings s = new CRT_Settings();
        string error;
        JsonSerializer js = new JsonSerializer();
        if (!js.ReadFromString(s, json, error))
        {
            Print("[CraftingRepairTools] Could not read config from server: " + error);
            return null;
        }
        return s;
    }

    //! Fills the registered recipe slots with the enabled recipes and rebuilds the vanilla recipe cache
    static void Apply()
    {
        PluginRecipesManager rm = PluginRecipesManager.Cast(GetPlugin(PluginRecipesManager));
        if (!rm)
        {
            m_ApplyRetries++;
            if (m_ApplyRetries <= 10)
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CRT_Config.Apply, 1000, false);
            return;
        }

        array<CRT_RecipeSettings> enabled = new array<CRT_RecipeSettings>;
        foreach (CRT_RecipeSettings recipe : GetSettings().Recipes)
        {
            if (recipe && recipe.Enabled)
                enabled.Insert(recipe);
        }

        int slot = 0;
        foreach (RecipeBase r : rm.m_RecipeList)
        {
            CRT_ConfigRecipe configRecipe = CRT_ConfigRecipe.Cast(r);
            if (!configRecipe)
                continue;

            CRT_RecipeSettings settings = null;
            if (slot < enabled.Count())
                settings = enabled[slot];
            configRecipe.CRT_Configure(settings);
            slot++;
        }

        rm.CallbackGenerateCache();
        Print("[CraftingRepairTools] " + Math.Min(enabled.Count(), slot).ToString() + " recipes active");
    }

    //! Broken entries are switched off with a log line, values are kept in sane ranges
    static void Validate(CRT_Settings s)
    {
        if (!s.Recipes)
            s.Recipes = new array<ref CRT_RecipeSettings>;

        int enabledCount = 0;
        foreach (CRT_RecipeSettings r : s.Recipes)
        {
            if (!r || !r.Enabled)
                continue;

            if (!r.Ingredient1 || !r.Ingredient2 || !r.Ingredient1.Types || !r.Ingredient2.Types || r.Ingredient1.Types.Count() == 0 || r.Ingredient2.Types.Count() == 0)
            {
                Print("[CraftingRepairTools] Recipe '" + r.Name + "' disabled: both ingredients need at least one type");
                r.Enabled = false;
                continue;
            }

            if (r.Result == "" || !GetGame().ConfigIsExisting("CfgVehicles " + r.Result))
            {
                Print("[CraftingRepairTools] Recipe '" + r.Name + "' disabled: unknown result '" + r.Result + "'");
                r.Enabled = false;
                continue;
            }

            enabledCount++;
            if (enabledCount > MAX_RECIPES)
            {
                Print("[CraftingRepairTools] Recipe '" + r.Name + "' disabled: more than " + MAX_RECIPES.ToString() + " enabled recipes");
                r.Enabled = false;
                continue;
            }

            r.ResultMaxPercent = Math.Clamp(r.ResultMaxPercent, 1, 100);
            ValidateIngredient(r.Name, r.Ingredient1);
            ValidateIngredient(r.Name, r.Ingredient2);
        }
    }

    protected static void ValidateIngredient(string recipe, CRT_IngredientSettings ing)
    {
        if (ing.Amount < 1)
            ing.Amount = 1;
        ing.DamagePercent = Math.Clamp(ing.DamagePercent, 0, 100);
        foreach (string type : ing.Types)
        {
            if (!GetGame().ConfigIsExisting("CfgVehicles " + type) && !GetGame().ConfigIsExisting("CfgWeapons " + type) && !GetGame().ConfigIsExisting("CfgMagazines " + type))
                Print("[CraftingRepairTools] Recipe '" + recipe + "': unknown ingredient '" + type + "'");
        }
    }

    // ----------------------------------------------------------------------------------------------------------
    // default recipes - the original 10 are enabled, the extra ones are switched off
    // (objects are created inside their owner - a 'new' object passed around as a temporary can be freed early)

    static CRT_Settings CreateDefault()
    {
        CRT_Settings s = new CRT_Settings();
        CRT_RecipeSettings r;

        r = AddRecipe(s, "Craft Sewing Kit", true, "SewingKit", 50);
        SetIngredient(r.Ingredient1, "BoneHook", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "Rope", 1, false, 0, true, false);

        r = AddRecipe(s, "Craft Sewing Kit", true, "SewingKit", 100);
        SetIngredient(r.Ingredient1, "Hook", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "Rope", 1, false, 0, true, false);

        r = AddRecipe(s, "Craft Leather Sewing Kit", true, "LeatherSewingKit", 100);
        SetIngredient(r.Ingredient1, "TannedLeather", 2, false, 0, true, false);
        SetIngredient(r.Ingredient2, "Screwdriver", 1, true, 0, true, false);

        r = AddRecipe(s, "Craft Whetstone", true, "Whetstone", 100);
        SetIngredient(r.Ingredient1, "Hammer", 1, false, 35, false, false);
        SetIngredient(r.Ingredient2, "Stone", 1, false, 0, true, false);

        r = AddRecipe(s, "Craft Tire Repair Kit", true, "TireRepairKit", 100);
        SetIngredient(r.Ingredient1, "DuctTape", 1, true, 0, true, true);
        SetIngredient(r.Ingredient2, "EpoxyPutty", 1, true, 0, true, true);

        r = AddRecipe(s, "Craft Epoxy Putty", true, "EpoxyPutty", 100);
        SetIngredient(r.Ingredient1, "Bone", 10, false, 0, true, false);
        SetIngredient(r.Ingredient2, "DisinfectantAlcohol", 1, true, 0, true, true);

        r = AddRecipe(s, "Craft Weapon Cleaning Kit", true, "WeaponCleaningKit", 100);
        SetIngredient(r.Ingredient1, "Rag", 6, false, 0, true, false);
        SetIngredient(r.Ingredient2, "Lard", 1, true, 0, true, true);

        r = AddRecipe(s, "Craft Duct Tape", true, "DuctTape", 100);
        SetIngredient(r.Ingredient1, "Rag", 3, false, 0, true, false);
        SetIngredient(r.Ingredient2, "EpoxyPutty", 50, false, 0, true, false);

        r = AddRecipe(s, "Craft Electronic Repair Kit", true, "ElectronicRepairKit", 100);
        SetIngredient(r.Ingredient1, "Screwdriver", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "Pliers", 1, true, 0, true, false);

        r = AddRecipe(s, "Craft Duct Tape", true, "DuctTape", 100);
        SetIngredient(r.Ingredient1, "Paper", 5, true, 0, true, false);
        SetIngredient(r.Ingredient2, "EpoxyPutty", 50, false, 0, true, false);

        r = AddRecipe(s, "Craft Weapon Cleaning Kit", false, "WeaponCleaningKit", 100);
        SetIngredient(r.Ingredient1, "Rag", 6, false, 0, true, false);
        SetIngredient(r.Ingredient2, "EngineOil", 250, false, 0, true, false);

        r = AddRecipe(s, "Craft Electronic Repair Kit", false, "ElectronicRepairKit", 100);
        SetIngredient(r.Ingredient1, "MetalWire", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "Pliers", 1, false, 25, false, false);

        r = AddRecipe(s, "Craft Leather Sewing Kit", false, "LeatherSewingKit", 100);
        SetIngredient(r.Ingredient1, "TannedLeather", 2, false, 0, true, false);
        SetIngredient(r.Ingredient2, "BoneHook", 1, true, 0, true, false);

        r = AddRecipe(s, "Craft Duct Tape", false, "DuctTape", 100);
        SetIngredient(r.Ingredient1, "BurlapStrip", 3, false, 0, true, false);
        SetIngredient(r.Ingredient2, "EpoxyPutty", 50, false, 0, true, false);

        r = AddRecipe(s, "Craft Sewing Kit", false, "SewingKit", 50);
        SetIngredient(r.Ingredient1, "BoneHook", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "BurlapStrip", 2, false, 0, true, false);

        r = AddRecipe(s, "Craft Lockpick", false, "Lockpick", 100);
        SetIngredient(r.Ingredient1, "MetalWire", 1, true, 0, true, false);
        SetIngredient(r.Ingredient2, "Pliers", 1, false, 10, false, false);

        return s;
    }

    //! The recipe is stored in s.Recipes right away, its ingredient objects already exist
    protected static CRT_RecipeSettings AddRecipe(CRT_Settings s, string name, bool enabled, string result, float maxPercent)
    {
        CRT_RecipeSettings r = new CRT_RecipeSettings();
        s.Recipes.Insert(r);
        r.Name = name;
        r.Enabled = enabled;
        r.Result = result;
        r.ResultMaxPercent = maxPercent;
        return r;
    }

    protected static void SetIngredient(CRT_IngredientSettings ing, string type, int amount, bool destroy, float damagePercent, bool qualityFactor, bool quantityFactor)
    {
        ing.Types.Clear();
        ing.Types.Insert(type);
        ing.Amount = amount;
        ing.Destroy = destroy;
        ing.DamagePercent = damagePercent;
        ing.QualityFactor = qualityFactor;
        ing.QuantityFactor = quantityFactor;
    }
}
