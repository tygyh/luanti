// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "dig.h"

#include "../client.h"
#include "../clientmap.h"
#include "client/particles.h"
#include "scripting_client.h"

f32 setDigIndex(const GameRunData& runData, const f32 crack_animation_length)
{
	return runData.dig_instantly
		       ? crack_animation_length // This is for e.g. torches
		       : static_cast<float>(crack_animation_length) * runData.dig_time /
		       runData.dig_time_complete;
}

float setNoDigDelayTimer(const GameRunData& runData,
                         const f32 crack_animation_length,
                         const f32 m_repeat_dig_time)
{
	float nodig_delay_timer = runData.dig_time_complete / crack_animation_length;

	// We don't want a corresponding delay to very time-consuming nodes
	// and nodes without digging time (e.g. torches) get a fixed delay.
	if (nodig_delay_timer > 0.3f)
		nodig_delay_timer = 0.3f;
	else if (runData.dig_instantly)
		nodig_delay_timer = 0.15f;

	// Ensure that the delay between breaking nodes
	// (dig_time_complete + nodig_delay_timer) is at least the
	// value of the repeat_dig_time setting.
	return std::max(nodig_delay_timer,
	                m_repeat_dig_time - runData.dig_time_complete);
}

void playDigSound(SoundMaker* soundmaker, const std::string& main_group,
                  const SoundSpec& sound_dig)
{
	if (!sound_dig.exists())
		return;

	if (sound_dig.name == "__group")
	{
		if (main_group.empty())
			return;

		soundmaker->m_player_leftpunch_sound.gain = 0.5;
		soundmaker->m_player_leftpunch_sound.name =
			std::string("default_dig_") + main_group;
	}
	else
	{
		soundmaker->m_player_leftpunch_sound = sound_dig;
	}
}

void startDigging(const PointedThing& pointed, const v3s16& nodepos,
                  Client* client, GameRunData runData,
                  const f32 crack_animation_length, const MapNode n,
                  const bool dig_instantly)
{
	if (!runData.digging)
	{
		infostream << "Started digging" << std::endl;
		runData.dig_instantly = dig_instantly;
		if (client->modsLoaded() && client->getScript()->on_punchnode(nodepos, n))
			return;

		client->interact(INTERACT_START_DIGGING, pointed);
		runData.digging = true;
		runData.btn_down_for_dig = true;
	}

	runData.dig_index = setDigIndex(runData, crack_animation_length);
}

void predict_node(const v3s16& nodepos, Client* client,
                  const NodeDefManager* nodedef_manager,
                  const std::string& node_dig_prediction)
{
	if (node_dig_prediction == "air")
	{
		client->removeNode(nodepos);
	}
	else if (!node_dig_prediction.empty())
	{
		if (const content_t id =
			nodedef_manager->getId(node_dig_prediction) != CONTENT_IGNORE)
			client->addNode(nodepos, id, true);
	}
	// implicit else: no prediction
}

void finish_digging(Client* client, GameRunData& runData, const f32 crack_animation_length, const f32 m_repeat_dig_time)
{
	// Don't show cracks if not diggable
	infostream << "Digging completed" << std::endl;
	client->setCrack(-1, v3s16(0, 0, 0));

	runData.dig_time = 0;
	runData.digging = false;
	// we successfully dug, now block it from repeating if we want to be safe
	if (g_settings->getBool("safe_dig_and_place"))
		runData.digging_blocked = true;

	runData.nodig_delay_timer =
		setNoDigDelayTimer(runData, crack_animation_length, m_repeat_dig_time);
}

void finish_digging_no_mod(const PointedThing& pointed, const v3s16& nodepos, Client* client,
                           const NodeDefManager* nodedef_manager, LocalPlayer* player, const MapNode n,
                           const std::string& node_dig_prediction)
{
	predict_node(nodepos, client, nodedef_manager, node_dig_prediction);

	client->interact(INTERACT_DIGGING_COMPLETED, pointed);

	client->getParticleManager()->addDiggingParticles(player, nodepos, n);

	// Send event to trigger sound
	client->getEventManager()->put(new NodeDugEvent(nodepos, n));
}

void Dig::handleDigging(const PointedThing& pointed, const v3s16& nodepos,
                        const ItemStack& selected_item, const ItemStack& hand_item,
                        const f32 dtime, Client* client,
                        const NodeDefManager* nodedef_manager,
                        const IItemDefManager* itemdef_manager, GameRunData runData,
                        const f32 crack_animation_length, SoundMaker* soundmaker,
                        const f32 m_repeat_dig_time, Camera* camera)
{
	// See also: serverpackethandle.cpp, action == 2
	LocalPlayer* player = client->getEnv().getLocalPlayer();
	const MapNode n = client->getEnv().getClientMap().getNode(nodepos);
	const auto& features = nodedef_manager->get(n);
	const ItemStack& tool_item =
		selected_item.name.empty() ? hand_item : selected_item;

	// NOTE: Similar piece of code exists on the server side for cheat detection.
	// Get digging parameters
	const DigParams params =
		getDigParams(features.groups,
		             &tool_item.getToolCapabilities(itemdef_manager, &hand_item),
		             tool_item.wear);

	if (params.diggable)
	{
		runData.dig_time_complete = params.time;
		client->getParticleManager()->addNodeParticle(player, nodepos, n);
		startDigging(pointed, nodepos, client, runData, crack_animation_length, n,
		             params.time == 0);
		playDigSound(soundmaker, params.main_group, features.sound_dig);

		if (runData.dig_index < crack_animation_length)
		{
			client->setCrack(runData.dig_index, nodepos);
		}
		else
		{
			finish_digging(client, runData, crack_animation_length, m_repeat_dig_time);

			if (client->modsLoaded() && client->getScript()->on_dignode(nodepos, n))
				return;

			finish_digging_no_mod(pointed, nodepos, client, nodedef_manager, player, n, features.node_dig_prediction);
		}

		runData.dig_time += dtime;
	}
	else
	{
		// I guess nobody will wait for this long
		runData.dig_time_complete = 10000000.0;
		startDigging(pointed, nodepos, client, runData, crack_animation_length, n,
		             false);

		finish_digging(client, runData, crack_animation_length, m_repeat_dig_time);

		if (client->modsLoaded() && client->getScript()->on_dignode(nodepos, n))
			return;

		finish_digging_no_mod(pointed, nodepos, client, nodedef_manager, player, n, features.node_dig_prediction);

		runData.dig_time = 0;
		client->setCrack(-1, nodepos);
	}

	camera->setDigging(0); // Dig animation
}
