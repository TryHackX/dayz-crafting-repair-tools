Crafting Repair Tools 1.1 - server setup (DayZ 1.30 Badlands)

1. Copy keys\TryHackX_New.bikey to your server's "keys" folder.
   (Older versions were signed with TryHackX.bikey - that key is no longer used.)
2. Start the server once - the config is created automatically:
   <server profile>\CraftingRepairTools\config.json
   The server sends it to every player on connect - restart the server after editing it.

Default recipes (combine the two items, one of them in hands):
   Sewing Kit (max 50 %)   = Bone Hook + Rope
   Sewing Kit              = Hook + Rope
   Leather Sewing Kit      = 2 Tanned Leather + Screwdriver
   Whetstone               = Stone + Hammer (the hammer loses 35 % condition)
   Tire Repair Kit         = Duct Tape + Epoxy Putty
   Epoxy Putty             = 10 Bones + Disinfectant Alcohol
   Weapon Cleaning Kit     = 6 Rags + Lard
   Duct Tape               = 3 Rags + Epoxy Putty (uses half of the putty)
   Duct Tape               = 5 Paper in the inventory + Epoxy Putty (uses half of the putty)
   Electronic Repair Kit   = Screwdriver + Pliers
Extra recipes in the config, switched off by default ("Enabled": 0):
   Weapon Cleaning Kit     = 6 Rags + 250 ml Engine Oil
   Electronic Repair Kit   = Metal Wire + Pliers (pliers lose 25 % condition)
   Leather Sewing Kit      = 2 Tanned Leather + Bone Hook
   Duct Tape               = 3 Burlap Strips + Epoxy Putty
   Sewing Kit (max 50 %)   = Bone Hook + 2 Burlap Strips
   Lockpick                = Metal Wire + Pliers (pliers lose 10 % condition)

Recipe settings (config.json, "Recipes" list - you can edit, add or remove recipes, max 40 enabled):
   "Name"             text in the crafting menu
   "Enabled"          1 = on, 0 = off
   "Result"           class name of the crafted item
   "ResultMaxPercent" quantity of the crafted item with perfect ingredients
   "Ingredient1/2"
      "Types"          accepted item class names, e.g. ["Rope", "BurlapStrip"]
      "Amount"         pieces / ml / % taken; items without quantity (paper): number of items
                       (the rest is taken from the player's inventory)
      "Destroy"        1 = the whole item is used up, 0 = only Amount is taken, tools are kept
      "DamagePercent"  condition lost by a kept item (e.g. the hammer)
      "QualityFactor"  1 = condition of this ingredient affects the crafted quantity
      "QuantityFactor" 1 = remaining quantity of this ingredient affects the crafted quantity
Broken entries (unknown result, no ingredient types) are switched off with a line in the server script log.
