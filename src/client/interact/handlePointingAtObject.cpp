// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "handlePointingAtObject.h"

#include "keypress.h"
#include "log.h"
#include "network/networkprotocol.h"

#include "../clientobject.h"
#include "client/client.h"

static constexpr float object_hit_delay = 0.2;

void setInfoText(const bool show_debug, GameRunData runData, const std::unique_ptr<GameUI>& m_game_ui)
{
	ClientActiveObject* selected_object = runData.selected_object;
	std::wstring infotext = unescape_translate(utf8_to_wide(selected_object->infoText()));

	if (show_debug)
	{
		if (!infotext.empty())
		{
			infotext += L"\n";
		}
		infotext += utf8_to_wide(selected_object->debugInfoText());
	}

	m_game_ui->setInfoText(infotext);
}

void startDigging(const PointedThing& pointed, const ItemStack& tool_item, const ItemStack& hand_item,
                  const v3f& player_position, GameRunData& runData, Client* client)
{
	// Report direct punch
	v3f objpos = runData.selected_object->getPosition();
	v3f dir = (objpos - player_position).normalize();

	bool disable_send = runData.selected_object->directReportPunch(
		dir, &tool_item, &hand_item, runData.time_from_last_punch);
	runData.time_from_last_punch = 0;

	if (!disable_send)
		client->interact(INTERACT_START_DIGGING, pointed);
}

void punchObject(GameRunData& runData, const f32 m_repeat_dig_time)
{
	infostream << "Punched object" << std::endl;
	runData.punching = true;
	runData.nodig_delay_timer = std::max(0.15f, m_repeat_dig_time);
}

void place(const PointedThing& pointed, Client* client)
{
	infostream << "Pressed place button while pointing at object" << std::endl;
	client->interact(INTERACT_PLACE, pointed); // place
}

void handlePointingAtObject(const PointedThing& pointed, const ItemStack& tool_item, const ItemStack& hand_item,
                            const v3f& player_position, const bool show_debug, GameRunData runData,
                            const std::unique_ptr<GameUI>& m_game_ui, InputHandler* input, const f32 m_repeat_dig_time,
                            Client* client)
{
	setInfoText(show_debug, runData, m_game_ui);

	if (keypress::isKeyDown(KeyType::DIG, input))
	{
		if (runData.object_hit_delay_timer <= 0.0)
		{
			runData.object_hit_delay_timer = object_hit_delay;

			punchObject(runData, m_repeat_dig_time);

			startDigging(pointed, tool_item, hand_item, player_position, runData, client);
			return;
		}

		if (keypress::wasKeyPressed(KeyType::DIG, input))
			punchObject(runData, m_repeat_dig_time);
	}
	else if (keypress::wasKeyDown(KeyType::PLACE, input))
	{
		place(pointed, client);
	}
}
