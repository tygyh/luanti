// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "handlePointingAtNothing.h"

#include "util/pointedthing.h"

void handlePointingAtNothing(Client* client)
{
	infostream << "Attempted to place item while pointing at nothing" << std::endl;
	client->interact(INTERACT_ACTIVATE, PointedThing());
}
