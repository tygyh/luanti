// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "inventory.h"
#include "util/pointedthing.h"

#include "../camera.h"
#include "../gamerundata.h"
#include "../soundmaker.h"
#include "../gameui.h"
#include "../game_formspec.h"

class HandlePointingAtNode
{
public:
	static void handlePointingAtNode(const PointedThing& pointed, const ItemStack& selected_item,
	                                 const ItemStack& hand_item, f32 dtime, Client* client, GameRunData runData,
	                                 InputHandler* input, const NodeDefManager* nodedef_manager,
	                                 const IItemDefManager* itemdef_manager, f32 crack_animation_length,
	                                 SoundMaker* soundmaker, f32 m_repeat_dig_time, Camera* camera,
	                                 const std::unique_ptr<GameUI>& m_game_ui, f32 m_repeat_place_time,
	                                 GameFormSpec* m_game_formspec);
};
