// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "View2d.h"
#include <memory>

namespace video
{

template <typename T>
class Array2d
{
public:
	Array2d(u32 width, u32 height) :
			m_owned_data(new T[width * height]),
			m_view(m_owned_data.get(), width, height)
	{
	}

	Array2d(Array2d &&other) noexcept = default;
	Array2d &operator=(Array2d &&other) noexcept = default;

	Array2d(const Array2d &) = delete;
	Array2d &operator=(const Array2d &) = delete;

	View2d<T> &getView() { return m_view; }
	const View2d<T> &getView() const { return m_view; }

	T &at(u32 x, u32 y) { return m_view.at(x, y); }
	const T &at(u32 x, u32 y) const { return m_view.at(x, y); }

	T *row(u32 y) { return m_view.row(y); }
	const T *row(u32 y) const { return m_view.row(y); }

	u32 getWidth() const { return m_view.getWidth(); }
	u32 getHeight() const { return m_view.getHeight(); }

	T *getData() { return m_view.getData(); }
	const T *getData() const { return m_view.getData(); }

private:
	std::unique_ptr<T[]> m_owned_data;
	View2d<T> m_view;
};

} // end namespace video
