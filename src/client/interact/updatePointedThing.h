// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "raycast.h"
#include "client/clientenvironment.h"
#include "client/gamerundata.h"
#include "client/hud.h"
#include "util/pointedthing.h"

class UpdatePointedThing
{
public:
	/*!
	 * Returns the object or node the player is pointing at.
	 * Also updates the selected thing in the Hud.
	 *
	 * @param  camera_offset     offset of the camera
	 * @param hud                Heads Up Display
	 * @param client             client connected to server
	 * @param runData            data about the current run of the game
	 * @param s
	 * @return selected_object   the selected object or NULL if not found
	 */
	static PointedThing updatePointedThing(
		const v3s16& camera_offset, Hud* hud, Client* client, GameRunData runData, RaycastState* s);
};
