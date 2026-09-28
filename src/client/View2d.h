// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "irrTypes.h"
#include <cassert>

namespace video
{

template <typename T>
class View2d
{
public:
	View2d(T *data, u32 width, u32 height, u32 y_stride = 0) :
			m_data(data),
			m_width(width),
			m_height(height),
			m_x_stride(sizeof(T)),
			m_y_stride(y_stride > 0 ? y_stride : width * sizeof(T))
	{
		assert(data != nullptr || (width == 0 && height == 0));
	}

	T &at(u32 x, u32 y)
	{
		assert(x < m_width && y < m_height);
		u8 *ptr = reinterpret_cast<u8 *>(m_data) + y * m_y_stride + x * m_x_stride;
		return *reinterpret_cast<T *>(ptr);
	}

	const T &at(u32 x, u32 y) const
	{
		assert(x < m_width && y < m_height);
		const u8 *ptr = reinterpret_cast<const u8 *>(m_data) + y * m_y_stride + x * m_x_stride;
		return *reinterpret_cast<const T *>(ptr);
	}

	T *row(u32 y)
	{
		assert(y < m_height);
		u8 *ptr = reinterpret_cast<u8 *>(m_data) + y * m_y_stride;
		return reinterpret_cast<T *>(ptr);
	}

	const T *row(u32 y) const
	{
		assert(y < m_height);
		const u8 *ptr = reinterpret_cast<const u8 *>(m_data) + y * m_y_stride;
		return reinterpret_cast<const T *>(ptr);
	}

	View2d drop(u32 left, u32 top) const
	{
		assert(left <= m_width && top <= m_height);
		u8 *new_data = reinterpret_cast<u8 *>(m_data) + top * m_y_stride + left * m_x_stride;
		return View2d(reinterpret_cast<T *>(new_data), m_width - left, m_height - top, m_y_stride);
	}

	View2d take(u32 width, u32 height) const
	{
		assert(width <= m_width && height <= m_height);
		return View2d(m_data, width, height, m_y_stride);
	}

	View2d slice(u32 x, u32 y, u32 width, u32 height) const
	{
		return drop(x, y).take(width, height);
	}

	u32 getWidth() const { return m_width; }
	u32 getHeight() const { return m_height; }
	u32 getXStride() const { return m_x_stride; }
	u32 getYStride() const { return m_y_stride; }

	T *getData() { return m_data; }
	const T *getData() const { return m_data; }

private:
	T *m_data = nullptr;
	u32 m_width = 0;
	u32 m_height = 0;
	u32 m_x_stride = sizeof(T);
	u32 m_y_stride = 0;
};

} // end namespace video
