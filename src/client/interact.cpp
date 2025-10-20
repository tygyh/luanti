// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "interact.h"

#include "clientmap.h"
#include "log.h"
#include "nodedef.h"
#include "nodedugevent.h"
#include "particles.h"
#include "scripting_client.h"
#include "network/networkprotocol.h"
#include "util/pointedthing.h"

void handlePointingAtNothing(Client *client)
{
	infostream << "Attempted to place item while pointing at nothing" << std::endl;
	PointedThing fauxPointed;
	fauxPointed.type = POINTEDTHING_NOTHING;
	client->interact(INTERACT_ACTIVATE, fauxPointed);
}

void handleDigging(const PointedThing &pointed, const v3s16 &nodepos, const ItemStack &selected_item,
                   const ItemStack &hand_item, f32 dtime, Client *client, const NodeDefManager *nodedef_manager,
                   const IItemDefManager *itemdef_manager, GameRunData runData, f32 crack_animation_length,
                   SoundMaker *soundmaker, f32 m_repeat_dig_time, Camera *camera)
{
	// See also: serverpackethandle.cpp, action == 2
	LocalPlayer *player = client->getEnv().getLocalPlayer();
	ClientMap &map = client->getEnv().getClientMap();
	MapNode n = map.getNode(nodepos);
	const auto &features = nodedef_manager->get(n);
	const ItemStack &tool_item = selected_item.name.empty() ? hand_item : selected_item;

	// NOTE: Similar piece of code exists on the server side for
	// cheat detection.
	// Get digging parameters
	DigParams params = getDigParams(features.groups,
	                                &tool_item.getToolCapabilities(itemdef_manager, &hand_item),
	                                tool_item.wear);

	// If can't dig, try hand
	if (!params.diggable)
	{
		params = getDigParams(features.groups,
		                      &hand_item.getToolCapabilities(itemdef_manager));
	}

	if (!params.diggable)
	{
		// I guess nobody will wait for this long
		runData.dig_time_complete = 10000000.0;
	}
 	else
	{
		runData.dig_time_complete = params.time;

		client->getParticleManager()->addNodeParticle(player, nodepos, n);
	}

	if (!runData.digging)
	{
		infostream << "Started digging" << std::endl;
		runData.dig_instantly = runData.dig_time_complete == 0;
		if (client->modsLoaded() && client->getScript()->on_punchnode(nodepos, n))
			return;

		client->interact(INTERACT_START_DIGGING, pointed);
		runData.digging = true;
		runData.btn_down_for_dig = true;
	}

	if (!runData.dig_instantly)
	{
		runData.dig_index = (float) crack_animation_length
		                    * runData.dig_time
		                    / runData.dig_time_complete;
	}
 	else
	{
		// This is for e.g. torches
		runData.dig_index = crack_animation_length;
	}

	const auto &sound_dig = features.sound_dig;

	if (sound_dig.exists() && params.diggable)
	{
		if (sound_dig.name == "__group")
		{
			if (!params.main_group.empty())
			{
				soundmaker->m_player_leftpunch_sound.gain = 0.5;
				soundmaker->m_player_leftpunch_sound.name =
						std::string("default_dig_") +
						params.main_group;
			}
		} else { soundmaker->m_player_leftpunch_sound = sound_dig; }
	}

	// Don't show cracks if not diggable
	if (runData.dig_time_complete >= 100000.0) {} else if (runData.dig_index < crack_animation_length)
	{
		client->setCrack(runData.dig_index, nodepos);
	}
 	else
	{
		infostream << "Digging completed" << std::endl;
		client->setCrack(-1, v3s16(0, 0, 0));

		runData.dig_time = 0;
		runData.digging = false;
		// we successfully dug, now block it from repeating if we want to be safe
		if (g_settings->getBool("safe_dig_and_place"))
			runData.digging_blocked = true;

		runData.nodig_delay_timer =
				runData.dig_time_complete / (float) crack_animation_length;

		// We don't want a corresponding delay to very time consuming nodes
		// and nodes without digging time (e.g. torches) get a fixed delay.
		if (runData.nodig_delay_timer > 0.3f)
			runData.nodig_delay_timer = 0.3f;
		else if (runData.dig_instantly)
			runData.nodig_delay_timer = 0.15f;

		// Ensure that the delay between breaking nodes
		// (dig_time_complete + nodig_delay_timer) is at least the
		// value of the repeat_dig_time setting.
		runData.nodig_delay_timer = std::max(runData.nodig_delay_timer,
		                                     m_repeat_dig_time - runData.dig_time_complete);

		if (client->modsLoaded() &&
		    client->getScript()->on_dignode(nodepos, n)) { return; }

		if (features.node_dig_prediction == "air") { client->removeNode(nodepos); } else if (!features.
			node_dig_prediction.empty())
		{
			content_t id;
			bool found = nodedef_manager->getId(features.node_dig_prediction, id);
			if (found)
				client->addNode(nodepos, id, true);
		}
		// implicit else: no prediction

		client->interact(INTERACT_DIGGING_COMPLETED, pointed);

		client->getParticleManager()->addDiggingParticles(player, nodepos, n);

		// Send event to trigger sound
		client->getEventManager()->put(new NodeDugEvent(nodepos, n));
	}

	if (runData.dig_time_complete < 100000.0) { runData.dig_time += dtime; }
 	else
	{
		runData.dig_time = 0;
		client->setCrack(-1, nodepos);
	}

	camera->setDigging(0); // Dig animation
}
