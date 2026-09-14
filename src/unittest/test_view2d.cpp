// Copyright (C) 2025 Luanti Contributors
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "unittest/test.h"
#include "client/Array2d.h"
#include "client/ColorFormats.h"
#include "client/View2d.h"

class TestView2d : public TestBase
{
public:
	TestView2d()
	{
		TestManager::registerTestModule(this);
	}

	const char *getName() override
	{
		return "TestView2d";
	}

	void runTests(IGameDef *gamedef) override
	{
		TEST(testBasicAccess);
		TEST(testSlicing);
		TEST(testIteration);
		TEST(testArray2d);
	}

	void testBasicAccess();
	void testSlicing();
	void testIteration();
	void testArray2d();
};

static TestView2d g_test_view2d;

void TestView2d::testBasicAccess()
{
	video::RGBA8 data[9];
	video::View2d<video::RGBA8> view(data, 3, 3);

	UASSERT(view.getWidth() == 3);
	UASSERT(view.getHeight() == 3);

	view.at(1, 1) = video::RGBA8(255, 0, 0, 255);
	UASSERT(view.at(1, 1) == video::RGBA8(255, 0, 0, 255));
}

void TestView2d::testSlicing()
{
	video::RGBA8 data[100];
	video::View2d<video::RGBA8> view(data, 10, 10);

	video::View2d<video::RGBA8> dropped = view.drop(2, 2);
	UASSERT(dropped.getWidth() == 8);
	UASSERT(dropped.getHeight() == 8);

	video::View2d<video::RGBA8> taken = view.take(5, 5);
	UASSERT(taken.getWidth() == 5);
	UASSERT(taken.getHeight() == 5);

	video::View2d<video::RGBA8> sliced = view.slice(2, 2, 5, 5);
	UASSERT(sliced.getWidth() == 5);
	UASSERT(sliced.getHeight() == 5);
}

void TestView2d::testIteration()
{
	video::Array2d<video::RGBA8> arr(10, 10);
	video::View2d<video::RGBA8> &view = arr.getView();

	for (u32 y = 0; y < view.getHeight(); ++y) {
		video::RGBA8 *row_ptr = view.row(y);
		for (u32 x = 0; x < view.getWidth(); ++x)
			row_ptr[x] = video::RGBA8(x * 10, y * 10, 0, 255);
	}

	for (u32 y = 0; y < view.getHeight(); ++y) {
		for (u32 x = 0; x < view.getWidth(); ++x) {
			video::RGBA8 pixel = view.at(x, y);
			UASSERT(pixel.r == x * 10);
			UASSERT(pixel.g == y * 10);
		}
	}
}

void TestView2d::testArray2d()
{
	video::Array2d<video::RGB8> arr(5, 5);
	UASSERT(arr.getWidth() == 5);
	UASSERT(arr.getHeight() == 5);

	arr.at(2, 2) = video::RGB8(100, 150, 200);
	UASSERT(arr.at(2, 2) == video::RGB8(100, 150, 200));

	video::Array2d<video::RGB8> arr2(std::move(arr));
	UASSERT(arr2.getWidth() == 5);
	UASSERT(arr2.at(2, 2) == video::RGB8(100, 150, 200));
}
