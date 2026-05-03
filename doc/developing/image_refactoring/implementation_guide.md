# Implementation Guide - Phase 1

This guide provides concrete steps to implement Phase 1 of the image refactoring: the core abstractions (View2d, Array2d, and color formats).

## Overview

Phase 1 creates the foundation for the entire refactoring. We'll implement:
1. `View2d<T>` - Template 2D view class
2. `Array2d<T>` - Template 2D array class
3. Color format types (RGBA8, RGB8, R8)
4. Comprehensive unit tests

**Duration**: 2 weeks

## Step 1: Create View2d.h

**Location**: `irr/include/View2d.h`

```cpp
// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "irrTypes.h"
#include <cassert>

namespace video
{

//! Lightweight 2D view into data
/** This is a non-owning view that provides safe access to 2D data.
    It supports efficient slicing operations without copying data.
    Template parameter T is the element type (e.g., RGBA8, RGB8, etc.)
*/
template<typename T>
class View2d
{
private:
    T* data_;
    u32 width_;
    u32 height_;
    u32 x_stride_;  // bytes to next column
    u32 y_stride_;  // bytes to next row (pitch)
    
public:
    //! Constructor
    /** \param data Pointer to 2D data
        \param width Width in elements
        \param height Height in elements
        \param y_stride Bytes per row (if 0, calculated as width * sizeof(T))
    */
    View2d(T* data, u32 width, u32 height, u32 y_stride = 0)
        : data_(data)
        , width_(width)
        , height_(height)
        , x_stride_(sizeof(T))
        , y_stride_(y_stride > 0 ? y_stride : width * sizeof(T))
    {
        assert(data != nullptr || (width == 0 && height == 0));
    }
    
    //! Element access with bounds checking (debug builds)
    T& at(u32 x, u32 y)
    {
        assert(x < width_ && y < height_);
        u8* ptr = reinterpret_cast<u8*>(data_) + y * y_stride_ + x * x_stride_;
        return *reinterpret_cast<T*>(ptr);
    }
    
    //! Element access with bounds checking (debug builds) - const version
    const T& at(u32 x, u32 y) const
    {
        assert(x < width_ && y < height_);
        const u8* ptr = reinterpret_cast<const u8*>(data_) + y * y_stride_ + x * x_stride_;
        return *reinterpret_cast<const T*>(ptr);
    }
    
    //! Row access for efficient iteration
    T* row(u32 y)
    {
        assert(y < height_);
        u8* ptr = reinterpret_cast<u8*>(data_) + y * y_stride_;
        return reinterpret_cast<T*>(ptr);
    }
    
    //! Row access for efficient iteration - const version
    const T* row(u32 y) const
    {
        assert(y < height_);
        const u8* ptr = reinterpret_cast<const u8*>(data_) + y * y_stride_;
        return reinterpret_cast<const T*>(ptr);
    }
    
    //! Drop pixels from left and top
    /** \param left Number of pixels to drop from left
        \param top Number of pixels to drop from top
        \return New view starting at (left, top)
    */
    View2d drop(u32 left, u32 top) const
    {
        assert(left <= width_ && top <= height_);
        u8* new_data = reinterpret_cast<u8*>(data_) + top * y_stride_ + left * x_stride_;
        return View2d(reinterpret_cast<T*>(new_data), 
                     width_ - left, 
                     height_ - top, 
                     y_stride_);
    }
    
    //! Take only specified dimensions
    /** \param width Width to take
        \param height Height to take
        \return New view with specified dimensions
    */
    View2d take(u32 width, u32 height) const
    {
        assert(width <= width_ && height <= height_);
        return View2d(data_, width, height, y_stride_);
    }
    
    //! Slice - combination of drop and take
    /** \param x X offset
        \param y Y offset
        \param width Width of slice
        \param height Height of slice
        \return New view representing the slice
    */
    View2d slice(u32 x, u32 y, u32 width, u32 height) const
    {
        return drop(x, y).take(width, height);
    }
    
    //! Get width
    u32 getWidth() const { return width_; }
    
    //! Get height
    u32 getHeight() const { return height_; }
    
    //! Get X stride in bytes
    u32 getXStride() const { return x_stride_; }
    
    //! Get Y stride in bytes (pitch)
    u32 getYStride() const { return y_stride_; }
    
    //! Get raw data pointer
    T* getData() { return data_; }
    
    //! Get raw data pointer - const version
    const T* getData() const { return data_; }
};

} // end namespace video
```

## Step 2: Create Array2d.h

**Location**: `irr/include/Array2d.h`

```cpp
// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "View2d.h"
#include <memory>

namespace video
{

//! Owning 2D array
/** This class owns the memory for a 2D array and provides a View2d interface.
    Template parameter T is the element type.
*/
template<typename T>
class Array2d
{
private:
    std::unique_ptr<T[]> owned_data_;
    View2d<T> view_;
    
public:
    //! Constructor - allocates memory
    /** \param width Width in elements
        \param height Height in elements
    */
    Array2d(u32 width, u32 height)
        : owned_data_(new T[width * height])
        , view_(owned_data_.get(), width, height)
    {
    }
    
    //! Move constructor
    Array2d(Array2d&& other) noexcept = default;
    
    //! Move assignment
    Array2d& operator=(Array2d&& other) noexcept = default;
    
    //! Copy constructor - deleted (use explicit copy if needed)
    Array2d(const Array2d&) = delete;
    
    //! Copy assignment - deleted (use explicit copy if needed)
    Array2d& operator=(const Array2d&) = delete;
    
    //! Get mutable view
    View2d<T>& getView() { return view_; }
    
    //! Get const view
    const View2d<T>& getView() const { return view_; }
    
    //! Convenience: element access
    T& at(u32 x, u32 y) { return view_.at(x, y); }
    const T& at(u32 x, u32 y) const { return view_.at(x, y); }
    
    //! Convenience: row access
    T* row(u32 y) { return view_.row(y); }
    const T* row(u32 y) const { return view_.row(y); }
    
    //! Convenience: get width
    u32 getWidth() const { return view_.getWidth(); }
    
    //! Convenience: get height
    u32 getHeight() const { return view_.getHeight(); }
    
    //! Get raw data pointer
    T* getData() { return view_.getData(); }
    const T* getData() const { return view_.getData(); }
};

} // end namespace video
```

## Step 3: Create ColorFormats.h

**Location**: `irr/include/ColorFormats.h`

```cpp
// Copyright (C) 2025 Luanti Contributors
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "irrTypes.h"
#include "SColor.h"

namespace video
{

//! RGBA with 8 bits per channel
struct RGBA8
{
    u8 r, g, b, a;
    
    RGBA8() = default;
    RGBA8(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}
    
    //! Convert from SColor
    static RGBA8 fromSColor(const SColor& c)
    {
        return RGBA8(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
    }
    
    //! Convert to SColor
    SColor toSColor() const
    {
        return SColor(a, r, g, b);
    }
    
    bool operator==(const RGBA8& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    
    bool operator!=(const RGBA8& other) const
    {
        return !(*this == other);
    }
};

//! RGB with 8 bits per channel
struct RGB8
{
    u8 r, g, b;
    
    RGB8() = default;
    RGB8(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {}
    
    //! Convert from SColor
    static RGB8 fromSColor(const SColor& c)
    {
        return RGB8(c.getRed(), c.getGreen(), c.getBlue());
    }
    
    //! Convert to SColor
    SColor toSColor() const
    {
        return SColor(255, r, g, b);
    }
    
    bool operator==(const RGB8& other) const
    {
        return r == other.r && g == other.g && b == other.b;
    }
    
    bool operator!=(const RGB8& other) const
    {
        return !(*this == other);
    }
};

//! Single channel 8-bit (grayscale)
struct R8
{
    u8 r;
    
    R8() = default;
    explicit R8(u8 r) : r(r) {}
    
    bool operator==(const R8& other) const
    {
        return r == other.r;
    }
    
    bool operator!=(const R8& other) const
    {
        return !(*this == other);
    }
};

} // end namespace video
```

## Step 4: Create Unit Tests

**Location**: `src/unittest/test_view2d.cpp`

```cpp
// Copyright (C) 2025 Luanti Contributors

#include "unittest/test.h"
#include "View2d.h"
#include "Array2d.h"
#include "ColorFormats.h"

using namespace video;

class TestView2d : public TestBase
{
public:
    TestView2d() { TestManager::registerTestModule(this); }
    const char *getName() override { return "TestView2d"; }
    
    void runTests(IGameDef *gamedef) override;
    
    void testBasicAccess();
    void testSlicing();
    void testIteration();
    void testArray2d();
};

static TestView2d g_test_view2d;

void TestView2d::runTests(IGameDef *gamedef)
{
    TEST(testBasicAccess);
    TEST(testSlicing);
    TEST(testIteration);
    TEST(testArray2d);
}

void TestView2d::testBasicAccess()
{
    // Create a simple 3x3 array
    RGBA8 data[9];
    View2d<RGBA8> view(data, 3, 3);
    
    UASSERT(view.getWidth() == 3);
    UASSERT(view.getHeight() == 3);
    
    // Set and get
    view.at(1, 1) = RGBA8(255, 0, 0, 255);
    UASSERT(view.at(1, 1) == RGBA8(255, 0, 0, 255));
}

void TestView2d::testSlicing()
{
    RGBA8 data[100];  // 10x10
    View2d<RGBA8> view(data, 10, 10);
    
    // Test drop
    auto dropped = view.drop(2, 2);
    UASSERT(dropped.getWidth() == 8);
    UASSERT(dropped.getHeight() == 8);
    
    // Test take
    auto taken = view.take(5, 5);
    UASSERT(taken.getWidth() == 5);
    UASSERT(taken.getHeight() == 5);
    
    // Test slice
    auto sliced = view.slice(2, 2, 5, 5);
    UASSERT(sliced.getWidth() == 5);
    UASSERT(sliced.getHeight() == 5);
}

void TestView2d::testIteration()
{
    Array2d<RGBA8> arr(10, 10);
    auto& view = arr.getView();
    
    // Fill with pattern
    for (u32 y = 0; y < view.getHeight(); ++y) {
        RGBA8* row_ptr = view.row(y);
        for (u32 x = 0; x < view.getWidth(); ++x) {
            row_ptr[x] = RGBA8(x * 10, y * 10, 0, 255);
        }
    }
    
    // Verify pattern
    for (u32 y = 0; y < view.getHeight(); ++y) {
        for (u32 x = 0; x < view.getWidth(); ++x) {
            RGBA8 pixel = view.at(x, y);
            UASSERT(pixel.r == x * 10);
            UASSERT(pixel.g == y * 10);
        }
    }
}

void TestView2d::testArray2d()
{
    // Test construction
    Array2d<RGB8> arr(5, 5);
    UASSERT(arr.getWidth() == 5);
    UASSERT(arr.getHeight() == 5);
    
    // Test access
    arr.at(2, 2) = RGB8(100, 150, 200);
    UASSERT(arr.at(2, 2) == RGB8(100, 150, 200));
    
    // Test move semantics
    Array2d<RGB8> arr2(std::move(arr));
    UASSERT(arr2.getWidth() == 5);
    UASSERT(arr2.at(2, 2) == RGB8(100, 150, 200));
}
```

## Step 5: Update Build System

### For CMake (irr/CMakeLists.txt)

Add the new headers:

```cmake
set(IRRLICHT_HEADERS
    # ... existing headers ...
    ${CMAKE_CURRENT_SOURCE_DIR}/include/View2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Array2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ColorFormats.h
)
```

### For Unit Tests (src/unittest/CMakeLists.txt)

Add the test file:

```cmake
set(UNITTEST_SRCS
    # ... existing test files ...
    ${CMAKE_CURRENT_SOURCE_DIR}/test_view2d.cpp
)
```

## Step 6: Build and Test

```bash
cd /home/runner/work/luanti/luanti
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)

# Run tests
./bin/luanti --run-unittests
```

## Step 7: Verify Implementation

### Checklist

- [ ] Code compiles without errors
- [ ] No compiler warnings
- [ ] All tests pass
- [ ] Templates instantiate correctly
- [ ] No memory leaks (run with valgrind)
- [ ] Performance is good (basic benchmarks)

### Common Issues and Solutions

**Issue**: Template instantiation errors
- **Solution**: Ensure all template code is in headers

**Issue**: Alignment issues
- **Solution**: Consider adding `alignas(4)` to color format structs

**Issue**: Tests failing
- **Solution**: Check assertions, verify pointer arithmetic

**Issue**: Memory leaks
- **Solution**: Verify unique_ptr usage, check for cycles

## Step 8: Documentation

Document your implementation:

```cpp
/** @example View2d usage
 * 
 * Creating and using a View2d:
 * @code
 * RGBA8 pixels[100]; // 10x10
 * View2d<RGBA8> view(pixels, 10, 10);
 * 
 * // Access pixel
 * view.at(5, 5) = RGBA8(255, 0, 0, 255);
 * 
 * // Iterate rows
 * for (u32 y = 0; y < view.getHeight(); ++y) {
 *     RGBA8* row = view.row(y);
 *     for (u32 x = 0; x < view.getWidth(); ++x) {
 *         row[x].r = 255;
 *     }
 * }
 * 
 * // Create sub-view
 * auto region = view.drop(2, 2).take(5, 5);
 * @endcode
 */
```

## Next Steps

After completing Phase 1:

1. Review code with maintainers
2. Get feedback and iterate
3. Begin Phase 2 (Image class)
4. Continue following the migration guide

## Performance Considerations

### Inlining

Ensure templates inline properly:
- Use `-O2` or `-O3` for optimization
- Check assembly output for hot paths
- Profile with `perf` or similar tools

### Memory

Monitor memory usage:
- Use valgrind for leak detection
- Check allocation patterns
- Ensure move semantics work

### Benchmarks

Create simple benchmarks:

```cpp
void benchmarkIteration() {
    Array2d<RGBA8> arr(1000, 1000);
    auto& view = arr.getView();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (u32 y = 0; y < view.getHeight(); ++y) {
        RGBA8* row = view.row(y);
        for (u32 x = 0; x < view.getWidth(); ++x) {
            row[x].r = 255;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Iteration took: " << duration.count() << " µs\n";
}
```

## Conclusion

Following these steps will give you a solid foundation for the image refactoring. The View2d and Array2d templates provide the building blocks for the more complex Image class in Phase 2.

Take your time to ensure Phase 1 is solid before moving forward. A strong foundation makes the rest of the migration much easier.
