// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "nodedef.h"
#include "../camera.h"
#include "../gamerundata.h"
#include "../soundmaker.h"

class Dig
{
public:
	static void handleDigging(const PointedThing& pointed, const v3s16& nodepos, const ItemStack& selected_item,
	                          const ItemStack& hand_item, f32 dtime, Client* client,
	                          const NodeDefManager* nodedef_manager,
	                          const IItemDefManager* itemdef_manager, GameRunData runData, f32 crack_animation_length,
	                          SoundMaker* soundmaker, f32 m_repeat_dig_time, Camera* camera);
};
