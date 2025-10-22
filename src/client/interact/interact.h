// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "dig.h"
#include "../gamerundata.h"
#include "../inputhandler.h"
#include "../soundmaker.h"
#include "tool.h"
#include "updatePointedThing.h"

#include "client/hud.h"
#include "gui/touchcontrols.h"

class Interact
{
public:
	static void handlePointingAtNothing(Client* client);

	static bool isKeyDown(const GameKeyType k, InputHandler* input)
	{
		return input->isKeyDown(k);
	}

	static bool wasKeyDown(const GameKeyType k, InputHandler* input)
	{
		return input->wasKeyDown(k);
	}

	static bool wasKeyPressed(const GameKeyType k, InputHandler* input)
	{
		return input->wasKeyPressed(k);
	}

	static bool wasKeyReleased(const GameKeyType k, InputHandler* input)
	{
		return input->wasKeyReleased(k);
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
};
