# Crafting Repair Tools

Craft every vanilla repair kit from basic items - every recipe is defined in the server config. For DayZ 1.30 (Badlands).

[Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3827409624) · DayZ 1.30 (Badlands) · author: TryHackX

## Support

If the mod is useful for you or your server, you can support further development:

| Currency | Network | Address |
|---|---|---|
| Monero (XMR) | Monero | `45hvee4Jv7qeAm6SrBzXb9YVjb8DkHtFtFh7qkDMxS9zYX3NRi1dV27MtSdVC5X8T1YVoiG8XFiJkh4p9UncqWGxHi4tiwk` |
| Bitcoin (BTC) | Bitcoin (on-chain) | `bc1qncavcek4kknpvykedxas8kxash9kdng990qed2` |
| Ethereum (ETH) | Ethereum (mainnet) | `0xa3d38d5Cf202598dd782C611e9F43f342C967cF5` |

## Default recipes

| Result | Ingredients |
|---|---|
| Sewing Kit (max 50 %) | Bone Hook + Rope |
| Sewing Kit | Hook + Rope |
| Leather Sewing Kit | 2 Tanned Leather + Screwdriver |
| Whetstone | Stone + Hammer (the hammer loses 35 % condition) |
| Tire Repair Kit | Duct Tape + Epoxy Putty |
| Epoxy Putty | 10 Bones + Disinfectant Alcohol |
| Weapon Cleaning Kit | 6 Rags + Lard |
| Duct Tape | 3 Rags + Epoxy Putty (half of the putty) |
| Duct Tape | 5 Paper + Epoxy Putty (half of the putty) |
| Electronic Repair Kit | Screwdriver + Pliers |

Extra recipes in the config, **off by default**: Weapon Cleaning Kit from 250 ml Engine Oil, Electronic Repair Kit from Metal Wire,
Leather Sewing Kit with a Bone Hook, Duct Tape and Sewing Kit from Burlap Strips, Lockpick.

## Configuration

`<server profile>/CraftingRepairTools/config.json` (created on the first start, synced to players). Every recipe can be switched on/off
and edited, and you can add your own (up to 40 enabled):

```json
{
  "Name": "Craft Sewing Kit",
  "Enabled": 1,
  "Result": "SewingKit",
  "ResultMaxPercent": 100.0,
  "Ingredient1": { "Types": ["Hook"], "Amount": 1, "Destroy": 1, "DamagePercent": 0.0, "QualityFactor": 1, "QuantityFactor": 0 },
  "Ingredient2": { "Types": ["Rope", "BurlapStrip"], "Amount": 1, "Destroy": 0, "DamagePercent": 0.0, "QualityFactor": 1, "QuantityFactor": 0 }
}
```

- `Types` - accepted item classes, `Amount` - pieces / ml / % taken, `Destroy` - 1 = the item is used up,
  `DamagePercent` - wear of a kept tool, `QualityFactor` / `QuantityFactor` - condition / quantity of the ingredient affects the result.

## Installation

- Add the mod to the `-mod=` parameter of the server **and** the client.
- Copy `keys/TryHackX_New.bikey` to the `keys` folder of the server.

## Building from source

1. DayZ Tools → **AddonBuilder**: source directory `soucre/CraftingRepairTools`, addon prefix `CraftingRepairTools`, output `addons/`.
2. Sign the PBO with your own key (DayZ Tools → DSUtils / DSCreateKey + DSSignFile) and give servers your `.bikey`.
   The release on Steam Workshop is signed with `TryHackX_New`.

## Compatibility

- DayZ 1.30 (Badlands), tested on a dedicated server.
- No other mods required. Works together with Repair To Pristine Updated.

Credits: TryHackX, Kooftt (help with scripts).

## Changelog

- **1.2** - Fixed the config sync: on dedicated servers players never received the server config (DayZ cannot read back
  a text of about 1 KB or more from one RPC, the config has about 5 KB), so no recipe of the mod showed up in their
  crafting menu. The config is now sent in parts of 800 bytes and joined on the client. The config file is unchanged.
- **1.1** - DayZ 1.30 (Badlands) update: recipes defined in the server config, 6 extra recipes (off by default),
  rag stack fix, logo in the mods menu.

## License

© TryHackX. You are free to use this mod on your server. Please do not repack or reupload it without permission.
