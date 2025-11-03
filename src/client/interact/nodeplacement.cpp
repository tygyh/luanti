// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "nodeplacement.h"

#include "keypress.h"
#include "client/clientmap.h"
#include "util/directiontables.h"

enum WallmountedDir : u8
{
	WALLMOUNTED_DOWN = 0,
	WALLMOUNTED_UP = 1,
	WALLMOUNTED_EAST = 2,
	WALLMOUNTED_WEST = 3,
	WALLMOUNTED_NORTH = 4,
	WALLMOUNTED_SOUTH = 5,
	WALLMOUNTED_UP_ROTATED = 6,
	WALLMOUNTED_DOWN_ROTATED = 7,
};

enum FacedirRotation : u8
{
	FACEDIR_NORTH = 0,
	FACEDIR_EAST = 1,
	FACEDIR_SOUTH = 2,
	FACEDIR_WEST = 3,
};

bool shouldRotateVerticalWallmount(const ContentFeatures& predicted_f, const v3s16 dir, const v3f pdir)
{
	switch (predicted_f.drawtype)
	{
	case NDT_TORCHLIKE:
		{
			const bool isFacingDiagonal = (pdir.X < 0 && pdir.Z > 0) || (pdir.X > 0 && pdir.Z < 0);
			return dir.Y > 0 ? isFacingDiagonal : !isFacingDiagonal;
		}
	case NDT_SIGNLIKE:
		return std::abs(pdir.X) < std::abs(pdir.Z);
	default:
		return std::abs(pdir.X) > std::abs(pdir.Z);
	}
}

WallmountedDir getVerticalDirectionRotated(const v3s16 dir)
{
	return dir.Y < 0 ? WALLMOUNTED_DOWN_ROTATED : WALLMOUNTED_UP_ROTATED;
}

WallmountedDir getVerticalDirection(const v3s16 dir)
{
	return dir.Y < 0 ? WALLMOUNTED_DOWN : WALLMOUNTED_UP;
}

u8 getVerticalNodeDirection(const ItemDefinition& selected_def, const v3s16& neighborpos,
                            Client* client, const ContentFeatures& predicted_f, const v3s16 dir)
{
	if (!selected_def.wallmounted_rotate_vertical)
		return getVerticalDirection(dir);

	const v3f ppos = client->getEnv().getLocalPlayer()->getPosition() / BS;
	const v3f pdir = v3f::from(neighborpos) - ppos;

	return shouldRotateVerticalWallmount(predicted_f, dir, pdir)
		       ? getVerticalDirectionRotated(dir)
		       : getVerticalDirection(dir);
}

u8 getWallMountFacingDirection(const v3s16 dir)
{
	return abs(dir.X) > abs(dir.Z)
		       ? dir.X < 0
			         ? WALLMOUNTED_WEST
			         : WALLMOUNTED_EAST
		       : dir.Z < 0
		       ? WALLMOUNTED_SOUTH
		       : WALLMOUNTED_NORTH;
}

u8 getDirection(const ItemDefinition& selected_def, const v3s16& nodepos, const v3s16& neighborpos, Client* client,
                const ContentFeatures& predicted_f)
{
	const v3s16 dir = nodepos - neighborpos;
	return abs(dir.Y) > MYMAX(abs(dir.X), abs(dir.Z))
		       ? getVerticalNodeDirection(selected_def, neighborpos, client, predicted_f, dir)
		       : getWallMountFacingDirection(dir);
}

u8 getBlockFacingDirection(const v3s16 dir)
{
	return abs(dir.X) > abs(dir.Z)
		       ? dir.X < 0
			         ? FACEDIR_WEST
			         : FACEDIR_EAST
		       : dir.Z < 0
		       ? FACEDIR_SOUTH
		       : FACEDIR_NORTH;
}

v3s16 getAttachmentCheckOffset(const NodeDefManager* nodedef, const ContentFeatures& predicted_f,
                               const MapNode predicted_node,
                               const int an)
{
	switch (an)
	{
	case 2: return predicted_f.param_type_2 == CPT2_FACEDIR ||
	               predicted_f.param_type_2 == CPT2_COLORED_FACEDIR ||
	               predicted_f.param_type_2 == CPT2_4DIR ||
	               predicted_f.param_type_2 == CPT2_COLORED_4DIR
		               ? facedir_dirs[predicted_node.getFaceDir(nodedef)]
		               : v3s16();
	case 3: return v3s16(0, -1, 0);
	case 4: return v3s16(0, 1, 0);
	default: return predicted_f.param_type_2 == CPT2_WALLMOUNTED || predicted_f.param_type_2 == CPT2_COLORED_WALLMOUNTED
		                ? predicted_node.getWallMountedDir(nodedef)
		                : v3s16(0, -1, 0);
	}
}

u8 getPaletteParam2(const ContentFeatures& predicted_f, const MapNode predicted_node, const s32 index)
{
	switch (predicted_f.param_type_2)
	{
	case CPT2_COLOR: return index;
	case CPT2_COLORED_WALLMOUNTED: return index & 0xf8 | predicted_node.getParam2() & 0x07;
	case CPT2_COLORED_FACEDIR: return index & 0xe0 | predicted_node.getParam2() & 0x1f;
	case CPT2_COLORED_4DIR: return index & 0xfc | predicted_node.getParam2() & 0x03;
	default: return 0; // Logically unreachable
	}
}

void applyColor(const ItemStack& selected_item, const ContentFeatures& predicted_f, MapNode predicted_node,
                const std::optional<u8> place_param2)
{
	if (!place_param2 && (predicted_f.param_type_2 == CPT2_COLOR
		|| predicted_f.param_type_2 == CPT2_COLORED_FACEDIR
		|| predicted_f.param_type_2 == CPT2_COLORED_4DIR
		|| predicted_f.param_type_2 == CPT2_COLORED_WALLMOUNTED))
	{
		const auto& indexstr = selected_item.metadata.getString("palette_index", 0);
		if (indexstr.empty())
			return;

		const s32 index = mystoi(indexstr);
		predicted_node.setParam2(getPaletteParam2(predicted_f, predicted_node, index));
	}
}

bool addNode(const ItemDefinition& selected_def, const v3s16& neighborpos, const PointedThing& pointed,
             Client* client, SoundMaker* soundmaker, const v3s16 p, const ContentFeatures& predicted_f,
             const MapNode predicted_node)
{
	// Don't place node when player would be inside new node
	// NOTE: This is to be eventually implemented by a mod as client-side Lua
	if (LocalPlayer* player = client->getEnv().getLocalPlayer();
		!predicted_f.walkable ||
		g_settings->getBool("enable_build_where_you_stand") ||
		(client->checkPrivilege("noclip") && g_settings->getBool("noclip")) ||
		(predicted_f.walkable &&
			neighborpos != player->getStandingNodePos() + v3s16(0, 1, 0) &&
			neighborpos != player->getStandingNodePos() + v3s16(0, 2, 0)))
	{
		// This triggers the required mesh update too
		client->addNode(p, predicted_node);
		// Report to server
		client->interact(INTERACT_PLACE, pointed);
		// A node is predicted, also play a sound
		soundmaker->m_player_rightpunch_sound = selected_def.sound_place;
		return true;
	}
	else
	{
		soundmaker->m_player_rightpunch_sound = selected_def.sound_place_failed;
		return false;
	}
}

bool addNodeToClientMap(const ItemDefinition& selected_def, const v3s16& neighborpos, const PointedThing& pointed,
                        Client* client, SoundMaker* soundmaker, const std::string& prediction, const v3s16 p,
                        const ContentFeatures& predicted_f, const MapNode predicted_node)
{
	try
	{
		return addNode(selected_def, neighborpos, pointed, client, soundmaker, p, predicted_f, predicted_node);
	}
	catch (const InvalidPositionException& _)
	{
		errorstream << "Node placement prediction failed for "
			<< selected_def.name << " (places "
			<< prediction << ") - Position not loaded" << std::endl;
		soundmaker->m_player_rightpunch_sound = selected_def.sound_place_failed;
		return false;
	}
}

bool nodePlacement(const ItemDefinition& selected_def, const ItemStack& selected_item, const v3s16& nodepos,
                   const v3s16& neighborpos, const PointedThing& pointed, const NodeMetadata* meta, Client* client,
                   SoundMaker* soundmaker, InputHandler* input, const NodeDefManager* nodedef_manager,
                   GameFormSpec m_game_formspec)
{
	const auto& prediction = selected_def.node_placement_prediction;

	const NodeDefManager* nodedef = client->ndef();
	ClientMap& map = client->getEnv().getClientMap();
	bool is_valid_position;

	MapNode node = map.getNode(nodepos, &is_valid_position);
	if (!is_valid_position)
	{
		soundmaker->m_player_rightpunch_sound = selected_def.sound_place_failed;
		return false;
	}

	// formspec in meta
	if (meta && !meta->getString("formspec").empty() && !input->isRandom()
		&& !keypress::isKeyDown(KeyType::SNEAK, input))
	{
		// on_rightclick callbacks are called anyway
		if (nodedef_manager->get(map.getNode(nodepos)).rightclickable)
			client->interact(INTERACT_PLACE, pointed);

		m_game_formspec.showNodeFormspec(meta->getString("formspec"), nodepos);
		return false;
	}

	// on_rightclick callback
	if (prediction.empty() || (nodedef->get(node).rightclickable &&
		!keypress::isKeyDown(KeyType::SNEAK, input)))
	{
		// Report to server
		client->interact(INTERACT_PLACE, pointed);
		return false;
	}

	verbosestream << "Node placement prediction for "
		<< selected_def.name << " is " << prediction << std::endl;
	v3s16 p = neighborpos;

	// Place inside node itself if buildable_to
	const MapNode n_under = map.getNode(nodepos, &is_valid_position);
	if (is_valid_position)
	{
		if (nodedef->get(n_under).buildable_to)
		{
			p = nodepos;
		}
		else
		{
			node = map.getNode(p, &is_valid_position);
			if (is_valid_position && !nodedef->get(node).buildable_to)
			{
				soundmaker->m_player_rightpunch_sound = selected_def.sound_place_failed;
				// Report to server
				client->interact(INTERACT_PLACE, pointed);
				return false;
			}
		}
	}

	// Find id of predicted node
	content_t id;

	if (nodedef->getId(prediction, id))
	{
		errorstream << "Node placement prediction failed for "
			<< selected_def.name << " (places " << prediction
			<< ") - Name not known" << std::endl;
		// Handle this as if prediction was empty
		// Report to server
		client->interact(INTERACT_PLACE, pointed);
		return false;
	}

	const ContentFeatures& predicted_f = nodedef->get(id);

	// Compare core.item_place_node() for what the server does with param2
	MapNode predicted_node(id, 0, 0);

	const auto place_param2 = selected_def.place_param2;
	u8 param2;
	if (place_param2)
	{
		param2 = *place_param2;
	}
	else
	{
		switch (predicted_f.param_type_2)
		{
		case CPT2_WALLMOUNTED:
		case CPT2_COLORED_WALLMOUNTED:
			{
				param2 = getDirection(selected_def, nodepos, neighborpos, client, predicted_f);
				break;
			}
		case CPT2_FACEDIR:
		case CPT2_COLORED_FACEDIR:
		case CPT2_4DIR:
		case CPT2_COLORED_4DIR:
			{
				const v3s16 dir = nodepos - floatToInt(client->getEnv().getLocalPlayer()->getPosition(), BS);
				param2 = getBlockFacingDirection(dir);
				break;
			}
		}
	}
	predicted_node.setParam2(param2);

	// Check attachment if node is in group attached_node
	if (const int an = itemgroup_get(predicted_f.groups, "attached_node"); an != 0)
	{
		v3s16 pp = getAttachmentCheckOffset(nodedef, predicted_f, predicted_node, an) + p;
		if (!nodedef->get(map.getNode(pp)).walkable)
		{
			soundmaker->m_player_rightpunch_sound = selected_def.sound_place_failed;
			// Report to server
			client->interact(INTERACT_PLACE, pointed);
			return false;
		}
	}

	applyColor(selected_item, predicted_f, predicted_node, place_param2);

	return addNodeToClientMap(selected_def, neighborpos, pointed, client, soundmaker, prediction, p, predicted_f,
	                          predicted_node);
}
