#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "BMLoadout.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR);// "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class RandomBMPreset: public BakkesMod::Plugin::BakkesModPlugin
{
	virtual void onLoad();
	virtual void onUnload();

	void onCreatePreset(std::vector<std::string> params);
	void setBMCode(std::string code);

private:
	void assignRandomItem(std::map<uint8_t, BMLoadout::Item>& loadout, enum BMLoadout::EQUIPSLOT slot, std::map<int, std::vector<int> > slotIdxToProductId);

	bool debugMode = false;
};

