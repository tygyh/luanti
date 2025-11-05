// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "dig.h"
#include "handlePointingAtNode.h"
#include "handlePointingAtObject.h"
#include "gui/touchcontrols.h"
#include "keypress.h"
#include "nodeplacement.h"
#include "tool.h"
#include "updatePointedThing.h"

#include "../game_formspec.h"
#include "../gamerundata.h"
#include "../gameui.h"
#include "../hud.h"
#include "../soundmaker.h"

class Interact
{
public:
	static void handlePointingAtNothing(Client* client);

	static bool isKeyDown(const GameKeyType k, InputHandler* input)
	{
		return keypress::isKeyDown(k, input);
	}

	static bool wasKeyDown(const GameKeyType k, InputHandler* input)
	{
		return keypress::wasKeyDown(k, input);
	}

	static bool wasKeyPressed(const GameKeyType k, InputHandler* input)
	{
		return keypress::wasKeyPressed(k, input);
	}

	static bool wasKeyReleased(const GameKeyType k, InputHandler* input)
	{
		return keypress::wasKeyReleased(k, input);
	}

	static void handleDigging(const PointedThing& pointed, const v3s16& nodepos, const ItemStack& selected_item,
	                          const ItemStack& hand_item, const f32 dtime, Client* client,
	                          const NodeDefManager* nodedef_manager, const IItemDefManager* itemdef_manager,
	                          const GameRunData& runData, const f32 crack_animation_length, SoundMaker* soundmaker,
	                          const f32 m_repeat_dig_time, Camera* camera)
	{
		Dig::handleDigging(pointed, nodepos, selected_item, hand_item, dtime, client, nodedef_manager, itemdef_manager,
		                   runData, crack_animation_length, soundmaker, m_repeat_dig_time, camera);
	}

	static bool isTouchShootlineUsed(const Camera* camera)
	{
		return g_touchcontrols && g_touchcontrols->isShootlineAvailable() &&
			camera->getCameraMode() == CAMERA_MODE_FIRST;
	}

	/*!
	 * Returns the object or node the player is pointing at.
	 * Also updates the selected thing in the Hud.
	 *
	 * @param  camera_offset     offset of the camera
	 * @param hud                Heads Up Display
	 * @param client             client connected to server
	 * @param runData            data about the current run of the game
	 * @param vision_line        the player's vision line
	 * @return selected_object   the selected object or NULL if not found
	 */
	static PointedThing updatePointedThing(const v3s16& camera_offset, Hud* hud, Client* client,
	                                       const GameRunData& runData, RaycastState vision_line)
	{
		return UpdatePointedThing::updatePointedThing(camera_offset, hud, client, runData, &vision_line);
	}

	static bool nodePlacement(const ItemDefinition& selected_def, const ItemStack& selected_item, const v3s16& nodepos,
	                          const v3s16& neighborpos, const PointedThing& pointed, const NodeMetadata* meta,
	                          Client* client, SoundMaker* soundmaker, InputHandler* input,
	                          const NodeDefManager* nodedef_manager, GameFormSpec* m_game_formspec)
	{
		return NodePlacement::nodePlacement(selected_def, selected_item, nodepos, neighborpos, pointed, meta, client,
		                                    soundmaker, input, nodedef_manager, m_game_formspec);
	}

	static void handlePointingAtNode(const PointedThing& pointed, const ItemStack& selected_item,
	                                 const ItemStack& hand_item, const f32 dtime, Client* client,
	                                 const GameRunData& runData, InputHandler* input,
	                                 const NodeDefManager* nodedef_manager, const IItemDefManager* itemdef_manager,
	                                 const f32 crack_animation_length, SoundMaker* soundmaker,
	                                 const f32 m_repeat_dig_time, Camera* camera,
	                                 const std::unique_ptr<GameUI>& m_game_ui, const f32 m_repeat_place_time,
	                                 GameFormSpec* m_game_formspec)
	{
		HandlePointingAtNode::handlePointingAtNode(pointed, selected_item, hand_item, dtime, client, runData, input,
		                                           nodedef_manager, itemdef_manager, crack_animation_length, soundmaker,
		                                           m_repeat_dig_time, camera, m_game_ui, m_repeat_place_time,
		                                           m_game_formspec);
	}

	static void handlePointingAtObject(const PointedThing& pointed, const ItemStack& playeritem,
	                                   const ItemStack& hand_item, const v3f& player_position, const bool show_debug,
	                                   const GameRunData& runData, const std::unique_ptr<GameUI>& m_game_ui,
	                                   InputHandler* input, const f32 m_repeat_dig_time, Client* client)
	{
		HandlePointingAtObject::handlePointingAtObject(pointed, playeritem, hand_item, player_position, show_debug,
		                                               runData, m_game_ui, input, m_repeat_dig_time, client);
	}
};
