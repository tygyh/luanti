// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "inventory.h"
#include "util/pointedthing.h"

#include "../gamerundata.h"
#include "../gameui.h"

class HandlePointingAtObject
{
public:
	static void handlePointingAtObject(const PointedThing& pointed, const ItemStack& playeritem,
	                                   const ItemStack& hand_item, const v3f& player_position, bool show_debug,
	                                   GameRunData runData, const std::unique_ptr<GameUI>& m_game_ui,
	                                   InputHandler* input, f32 m_repeat_dig_time, Client* client);
};
