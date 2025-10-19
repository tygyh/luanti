// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#pragma once

#include "irr_v3d.h"
#include "mapnode.h"
#include "mtevent.h"

class NodeDugEvent final : public MtEvent {
public:
	v3s16 p;
	MapNode n;

	NodeDugEvent(const v3s16 p, const MapNode n) : p(p), n(n) {}

	Type getType() const override { return NODE_DUG; }
};
