#include "pch.h"
#include "RandomBMPreset.h"

BAKKESMOD_PLUGIN(RandomBMPreset, "Randomize BakkesMod loadout", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RandomBMPreset::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerNotifier("randomize_bm_loadout", [this](std::vector<std::string> params) { onCreatePreset(params); },
		"Generates a random car preset with all items in BakkesMod and applies it via item mod code", PERMISSION_MENU);

	/*cvarManager->registerCvar("random_bm_set_colors", "1", "Enables color randomization when generating random preset",
		true, true, 0, true, 1, true);*/
	cvarManager->registerCvar("randomize_bm_same_teams", "0", "Set to make randomize_bm_loadout create one loadout for both teams",
		true, true, 0, true, 1, true);

	std::srand(std::time(nullptr));

	debugMode = false;
}

void RandomBMPreset::onUnload()
{
}

void RandomBMPreset::onCreatePreset(std::vector<std::string> params)
{
	auto itemsWrapper = gameWrapper->GetItemsWrapper();
	if (itemsWrapper.IsNull()) {
		cvarManager->log("ItemsWrapper is NULL. Cannot proceed.");
		return;
	}
	
	auto products = itemsWrapper.GetAllProducts();
	std::map<int, std::vector<int> > slotIdxToProductId;
	for (auto product : products) {
		if (!product.IsNull()) {
			auto slotIdx = product.GetSlot().GetSlotIndex();
			auto it = slotIdxToProductId.find(slotIdx);
			if (it == slotIdxToProductId.end()) {
				if (debugMode) {
					cvarManager->log("Adding slot index: " + std::to_string(slotIdx) + ", Named: " + product.GetSlot().GetLabel().ToString());
				}
				slotIdxToProductId[slotIdx] = std::vector<int>();
			}
			it = slotIdxToProductId.find(slotIdx);
			it->second.push_back(product.GetID());
		}
	}

	bool bSetColors = true;
	/*auto cvarSetColors = cvarManager->getCvar("random_bm_set_colors");
	if (!cvarSetColors.IsNull()) {
		bSetColors = cvarSetColors.getBoolValue();
	}*/

	bool bBlueIsOrange = false;
	auto cvarBlueIsOrange = cvarManager->getCvar("randomize_bm_same_teams");
	if (!cvarBlueIsOrange.IsNull()) {
		bBlueIsOrange = cvarBlueIsOrange.getBoolValue();
	}

	// Create the loadout
	BMLoadout::BMLoadout loadout;
	loadout.body.blue_is_orange = bBlueIsOrange;
	
	// Randomize colors
	if (bSetColors) {
		loadout.body.blueColor.should_override = true;
		loadout.body.blueColor.primary_colors.randomize();
		loadout.body.blueColor.secondary_colors.randomize();

		if (!bBlueIsOrange) {
			loadout.body.orangeColor.should_override = true;
			loadout.body.orangeColor.primary_colors.randomize();
			loadout.body.orangeColor.secondary_colors.randomize();
		}
	}
	else {
		loadout.body.blueColor.should_override = false;
		loadout.body.orangeColor.should_override = false;
	}

    // Blue items
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_BODY, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_SKIN, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_WHEELS, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_BOOST, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_ANTENNA, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_HAT, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_PAINTFINISH, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_PAINTFINISH_SECONDARY, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_ENGINE_AUDIO, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_SUPERSONIC_TRAIL, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_GOALEXPLOSION, slotIdxToProductId);
	assignRandomItem(loadout.body.blue_loadout, BMLoadout::SLOT_PLAYERANTHEM, slotIdxToProductId);

	// Orange items
	if (!bBlueIsOrange) {
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_SKIN, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_WHEELS, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_BOOST, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_ANTENNA, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_HAT, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_PAINTFINISH, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_PAINTFINISH_SECONDARY, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_ENGINE_AUDIO, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_SUPERSONIC_TRAIL, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_GOALEXPLOSION, slotIdxToProductId);
		assignRandomItem(loadout.body.orange_loadout, BMLoadout::SLOT_PLAYERANTHEM, slotIdxToProductId);
	}

	// Generate code and apply
	std::string code = BMLoadout::save(loadout);
	setBMCode(code);
}

void RandomBMPreset::setBMCode(std::string code)
{
	cvarManager->executeCommand("cl_itemmod_enabled 1; cl_itemmod_code \"" + code + "\"");
}

void RandomBMPreset::assignRandomItem(std::map<uint8_t, BMLoadout::Item>& loadout, enum BMLoadout::EQUIPSLOT slot, std::map<int, std::vector<int> > slotIdxToProductId)
{
	BMLoadout::Item item;
	item.slot_index = (uint8_t) slot;

	if (slot != BMLoadout::SLOT_PAINTFINISH && slot != BMLoadout::SLOT_PAINTFINISH_SECONDARY && slot != BMLoadout::SLOT_PLAYERANTHEM) {
		item.paint_index = (uint8_t)(rand() % 19);
	}

	if (slot != BMLoadout::SLOT_BODY) {
		auto it = slotIdxToProductId.find(slot == BMLoadout::SLOT_PAINTFINISH_SECONDARY ? BMLoadout::SLOT_PAINTFINISH : slot);
		if (it == slotIdxToProductId.end() || it->second.size() == 0) {
			cvarManager->log("Could not find any items for equip slot: " + std::to_string(slot));
			return;
		}
		else {
			int r = rand() % (it->second.size() + 1);
			if (r >= it->second.size()) {
				return;
			}
			item.product_id = (uint16_t)it->second[r];
		}
	}

	if (debugMode) {
		if (slot != BMLoadout::SLOT_BODY) {
			auto p = gameWrapper->GetItemsWrapper().GetProduct((int)item.product_id);
			cvarManager->log("Item equipped to slot " + std::to_string(slot) + ": " + p.GetAsciiLabel().ToString() + ", PaintID: " + std::to_string(item.paint_index));
		}
		else {
			cvarManager->log("Item equipped to slot " + std::to_string(slot) + ": PaintID: " + std::to_string(item.paint_index));
		}
	}
	
	loadout[slot] = item;
}
