// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "itemdef.h"
#include "nodemetadata.h"
#include "client/client.h"
#include "client/game_formspec.h"
#include "client/soundmaker.h"

class NodePlacement
{
public:
	static bool nodePlacement(const ItemDefinition& selected_def, const ItemStack& selected_item, const v3s16& nodepos,
	                   const v3s16& neighborpos, const PointedThing& pointed, const NodeMetadata* meta, Client* client,
	                   SoundMaker* soundmaker, InputHandler* input, const NodeDefManager* nodedef_manager,
	                   GameFormSpec* m_game_formspec);
};
