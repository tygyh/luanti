// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2025 Luanti contributors

#include "updatePointedThing.h"

#include "client/client.h"
#include "client/clientmap.h"
#include "client/content_cao.h"
#include "client/mapblock_mesh.h"
#include "util/directiontables.h"

u16 calculateLightLevel(ClientMap& map, const NodeDefManager* nodedef, const v3s16 p)
{
	// Get selection mesh light level
	MapNode n = map.getNode(p);
	u16 node_light = getInteriorLight(n, -1, nodedef);
	u16 light_level = node_light;

	for (const v3s16& dir : g_6dirs)
	{
		n = map.getNode(p + dir);
		node_light = getInteriorLight(n, -1, nodedef);
		if (node_light > light_level)
			light_level = node_light;
	}
	return light_level;
}

video::SColor calculateMeshColor(Client* client, const u16 light_level)
{
	const u32 daynight_ratio = client->getEnv().getDayNightRatio();
	auto color = final_color_blend(light_level, daynight_ratio);

	// Modify final color a bit with time
	const u32 timer = client->getEnv().getFrameTime() % 5000;
	const float timerf = static_cast<float>(core::PI * (timer / 2500.0 - 0.5));
	const float sin_r = 0.08f * std::sin(timerf);
	const float sin_g = 0.08f * std::sin(timerf + core::PI * 0.5f);
	const float sin_b = 0.08f * std::sin(timerf + core::PI);
	color.setRed(core::clamp(core::round32(color.getRed() * (0.8 + sin_r)), 0, 255));
	color.setGreen(core::clamp(core::round32(color.getGreen() * (0.8 + sin_g)), 0, 255));
	color.setBlue(core::clamp(core::round32(color.getBlue() * (0.8 + sin_b)), 0, 255));
	return color;
}

v3f calculateRotation(const GenericCAO* gcao)
{
	return gcao != nullptr && gcao->getProperties().rotate_selectionbox
		       ? gcao->getSceneNode()->getAbsoluteTransformation().getRotationRadians()
		       : v3f();
}

void calculatePointedNode(const v3s16& camera_offset, Hud* hud, std::vector<aabb3f>* selectionboxes, ClientMap& map, const NodeDefManager* nodedef, const PointedThing& result)
{
	// Update selection boxes
	const MapNode n = map.getNode(result.node_undersurface);
	std::vector<aabb3f> boxes;
	n.getSelectionBoxes(nodedef, &boxes, n.getNeighbors(result.node_undersurface, &map));

	constexpr f32 d = 0.002f * BS;
	for (aabb3f box : boxes)
	{
		box.MinEdge -= v3f(d, d, d);
		box.MaxEdge += v3f(d, d, d);
		selectionboxes->push_back(box);
	}
	hud->setSelectionPos(intToFloat(result.node_undersurface, BS), camera_offset);
	hud->setSelectionRotationRadians(v3f());
	hud->setSelectedFaceNormal(result.intersection_normal);
}

void calculatePointedObject(const v3s16& camera_offset, Hud* hud, Client* client, GameRunData& runData, std::vector<aabb3f>* selectionboxes, const PointedThing& result)
{
	hud->pointing_at_object = true;

	runData.selected_object = client->getEnv().getActiveObject(result.object_id);
	aabb3f selection_box{{0.0f, 0.0f, 0.0f}};
	if (g_settings->getBool("show_entity_selectionbox") && runData.selected_object->doShowSelectionBox() &&
		runData.selected_object->getSelectionBox(&selection_box))
	{
		const v3f pos = runData.selected_object->getPosition();
		selectionboxes->push_back(selection_box);
		hud->setSelectionPos(pos, camera_offset);
		const auto gcao = dynamic_cast<GenericCAO*>(runData.selected_object);
		hud->setSelectionRotationRadians(calculateRotation(gcao));
	}
	hud->setSelectedFaceNormal(result.raw_intersection_normal);
}

void updateSelectionBoxes(Hud* hud, Client* client, const std::vector<aabb3f>* selectionboxes, ClientMap& map,
                          const NodeDefManager* nodedef)
{
	// Update selection mesh light level and vertex colors
	if (selectionboxes->empty())
		return;

	const u16 light_level = calculateLightLevel(map, nodedef, floatToInt(hud->getSelectionPos(), BS));
	const video::SColor color = calculateMeshColor(client, light_level);

	// Set mesh final color
	hud->setSelectionMeshColor(color);
}

PointedThing UpdatePointedThing::updatePointedThing(const v3s16& camera_offset, Hud* hud, Client* client,
                                                    GameRunData runData, RaycastState* s)
{
	std::vector<aabb3f>* selectionboxes = hud->getSelectionBoxes();
	selectionboxes->clear();
	hud->setSelectedFaceNormal(v3f());

	ClientEnvironment& env = client->getEnv();
	ClientMap& map = env.getClientMap();
	const NodeDefManager* nodedef = map.getNodeDefManager();

	runData.selected_object = nullptr;
	hud->pointing_at_object = false;

	const PointedThing pointedThing = env.continueRaycast(s);
	if (pointedThing.type == POINTEDTHING_OBJECT)
	{
		calculatePointedObject(camera_offset, hud, client, runData, selectionboxes, pointedThing);
	}
	else if (pointedThing.type == POINTEDTHING_NODE)
	{
		calculatePointedNode(camera_offset, hud, selectionboxes, map, nodedef, pointedThing);
	}

	updateSelectionBoxes(hud, client, selectionboxes, map, nodedef);
	return pointedThing;
}
