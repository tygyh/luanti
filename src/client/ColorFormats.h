// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "SColor.h"
#include "irrTypes.h"

namespace video
{

struct RGBA8
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;

	RGBA8() = default;
	RGBA8(u8 r_, u8 g_, u8 b_, u8 a_) : r(r_), g(g_), b(b_), a(a_) {}

	static RGBA8 fromSColor(const SColor &c)
	{
		return RGBA8(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
	}

	SColor toSColor() const
	{
		return SColor(a, r, g, b);
	}

	bool operator==(const RGBA8 &other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	bool operator!=(const RGBA8 &other) const
	{
		return !(*this == other);
	}
};

struct RGB8
{
	u8 r;
	u8 g;
	u8 b;

	RGB8() = default;
	RGB8(u8 r_, u8 g_, u8 b_) : r(r_), g(g_), b(b_) {}

	static RGB8 fromSColor(const SColor &c)
	{
		return RGB8(c.getRed(), c.getGreen(), c.getBlue());
	}

	SColor toSColor() const
	{
		return SColor(255, r, g, b);
	}

	bool operator==(const RGB8 &other) const
	{
		return r == other.r && g == other.g && b == other.b;
	}

	bool operator!=(const RGB8 &other) const
	{
		return !(*this == other);
	}
};

struct R8
{
	u8 r;

	R8() = default;
	explicit R8(u8 r_) : r(r_) {}

	bool operator==(const R8 &other) const
	{
		return r == other.r;
	}

	bool operator!=(const R8 &other) const
	{
		return !(*this == other);
	}
};

} // end namespace video
