// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "handlePointingAtNode.h"

#include "dig.h"
#include "keypress.h"
#include "nodeplacement.h"
#include "scripting_client.h"
#include "../gameui.h"
#include "../game_formspec.h"
#include "../inputhandler.h"
#include "client/client.h"
#include "client/clientmap.h"

bool shouldDig(const Client* client, const GameRunData& runData, InputHandler* input)
{
	return runData.nodig_delay_timer <= 0.0 && keypress::isKeyDown(KeyType::DIG, input)
		&& !runData.digging_blocked && client->checkPrivilege("interact");
}

bool shouldPlace(const Client* client, const GameRunData& runData, InputHandler* input, const f32 m_repeat_place_time)
{
	return (keypress::wasKeyPressed(KeyType::PLACE, input) || runData.repeat_place_timer >= m_repeat_place_time) &&
		client->checkPrivilege("interact");
}

void handlePlacing(const PointedThing& pointed, const ItemStack& selected_item, Client* client, GameRunData& runData,
                   InputHandler* input, const NodeDefManager* nodedef_manager, const IItemDefManager* itemdef_manager,
                   SoundMaker* soundmaker, Camera* camera, GameFormSpec* m_game_formspec, const v3s16 nodepos,
                   const NodeMetadata* meta)
{
	const v3s16 neighborpos = pointed.node_abovesurface;

	runData.repeat_place_timer = 0;
	infostream << "Place button pressed while looking at ground" << std::endl;

	// Placing animation (always shown for feedback)
	camera->setDigging(1);

	soundmaker->m_player_rightpunch_sound = SoundSpec();

	// If the wielded item has node placement prediction,
	// make that happen
	// And also set the sound and send the interact
	// But first check for meta formspec and rightclickable
	auto& def = selected_item.getDefinition(itemdef_manager);
	const bool placed =
		NodePlacement::nodePlacement(def, selected_item, nodepos, neighborpos, pointed, meta, client,
		                             soundmaker, input, nodedef_manager, m_game_formspec);

	if (placed && client->modsLoaded())
		client->getScript()->on_placenode(pointed, def);
}

void handlePointingAtNode(const PointedThing& pointed, const ItemStack& selected_item, const ItemStack& hand_item,
                          const f32 dtime, Client* client, GameRunData runData, InputHandler* input,
                          const NodeDefManager* nodedef_manager, const IItemDefManager* itemdef_manager,
                          const f32 crack_animation_length, SoundMaker* soundmaker, const f32 m_repeat_dig_time,
                          Camera* camera, const std::unique_ptr<GameUI>& m_game_ui, const f32 m_repeat_place_time,
                          GameFormSpec* m_game_formspec)
{
	const v3s16 nodepos = pointed.node_undersurface;

	/*
		Check information text of node
	*/

	if (shouldDig(client, runData, input))
	{
		Dig::handleDigging(pointed, nodepos, selected_item, hand_item, dtime, client, nodedef_manager,
		                   itemdef_manager, runData, crack_animation_length, soundmaker, m_repeat_dig_time,
		                   camera);
	}

	ClientMap& map = client->getEnv().getClientMap();

	// This should be done after digging handling
	const NodeMetadata* meta = map.getNodeMetadata(nodepos);
	const ContentFeatures node = nodedef_manager->get(map.getNode(nodepos));

	if (meta)
	{
		m_game_ui->setInfoText(unescape_translate(utf8_to_wide(meta->getString("infotext"))));
	}
	else if (node.name == "unknown")
	{
		m_game_ui->setInfoText(L"Unknown node");
	}

	if (shouldPlace(client, runData, input, m_repeat_place_time))
	{
		handlePlacing(pointed, selected_item, client, runData, input, nodedef_manager, itemdef_manager, soundmaker,
		              camera, m_game_formspec, nodepos, meta);
	}
}
