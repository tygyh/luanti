// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once
#include "client/inputhandler.h"
#include "client/keys.h"

class keypress
{
public:
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
};
