# Technical Solution - Image Refactoring

## Table of Contents

1. [Problem Analysis](#problem-analysis)
2. [Proposed Architecture](#proposed-architecture)
3. [Component Design](#component-design)
4. [Implementation Strategy](#implementation-strategy)
5. [Testing Strategy](#testing-strategy)
6. [Performance Considerations](#performance-considerations)

## Problem Analysis

### Current Architecture Issues

The existing `IImage`/`CImage` architecture has several fundamental problems:

#### 1. Poor Abstractions

Code working with images must implement basic operations manually:

```cpp
// Current: Manual sub-region iteration
void processRegion(IImage* img, u32 x, u32 y, u32 w, u32 h) {
    for (u32 j = y; j < y + h && j < img->getDimension().Height; ++j) {
        for (u32 i = x; i < x + w && i < img->getDimension().Width; ++i) {
            SColor pixel = img->getPixel(i, j);
            // Process pixel...
            img->setPixel(i, j, processedPixel);
        }
    }
}
```

This is error-prone because:
- Manual bounds checking required
- Offset calculations done repeatedly
- Easy to make off-by-one errors
- Performance suffers from repeated virtual calls

#### 2. No Type Safety

There's no way to express at compile-time that an image is a specific format:

```cpp
void processRGBA8Image(IImage* img) {
    // Runtime check required
    if (img->getColorFormat() != ECF_A8R8G8B8) {
        return; // Wrong format!
    }
    
    // Now we "know" it's RGBA8, but the type system doesn't
    // Still getting/setting SColor, not RGBA8 directly
}
```

#### 3. Interface/Implementation Split

The `IImage` interface and `CImage` implementation split adds:
- Virtual function call overhead
- Reference counting complexity
- More code to maintain
- Harder to understand ownership

#### 4. No Efficient Views

Working with sub-regions requires either:
- Copying the data (expensive)
- Manual offset calculations everywhere (error-prone)

## Proposed Architecture

### Overview

The new architecture consists of four main components:

```
View2d<T>      ← Non-owning 2D view (lightweight, sliceable)
    ↓
Array2d<T>     ← Owning wrapper (RAII memory management)
    ↓
  Image        ← Variant holding different format arrays
    ↓
ImageView      ← Variant holding different format views
```

### Design Principles

1. **Zero-cost abstraction** - Templates inline to same or better performance
2. **Type safety** - Compile-time format checking where possible
3. **Clear ownership** - Separate owning (Array2d) from non-owning (View2d)
4. **Modern C++** - Use RAII, move semantics, std::variant
5. **Gradual migration** - Support both old and new APIs during transition

## Component Design

### 1. View2d\<T> - 2D View Template

A lightweight, non-owning view into 2D data.

```cpp
template<typename T>
class View2d {
private:
    T* data_;
    u32 width_;
    u32 height_;
    u32 x_stride_;  // bytes to next column (usually sizeof(T))
    u32 y_stride_;  // bytes to next row (pitch)
    
public:
    // Constructor
    View2d(T* data, u32 width, u32 height, u32 y_stride = 0);
    
    // Element access with bounds checking (debug builds)
    T& at(u32 x, u32 y);
    const T& at(u32 x, u32 y) const;
    
    // Row access for efficient iteration
    T* row(u32 y);
    const T* row(u32 y) const;
    
    // Slicing operations - return new views
    View2d drop(u32 left, u32 top) const;
    View2d take(u32 width, u32 height) const;
    View2d slice(u32 x, u32 y, u32 width, u32 height) const;
    
    // Getters
    u32 getWidth() const { return width_; }
    u32 getHeight() const { return height_; }
    u32 getYStride() const { return y_stride_; }
    T* getData() { return data_; }
    const T* getData() const { return data_; }
};
```

**Key features:**
- **Lightweight**: Just a pointer and dimensions
- **Non-owning**: Doesn't manage memory
- **Sliceable**: `drop()` and `take()` create sub-views without copying
- **Efficient**: Direct pointer arithmetic, inlines well

**Slicing example:**
```cpp
// Original 100x100 view
View2d<RGBA8> view(data, 100, 100);

// Create 50x50 sub-view starting at (10, 10)
auto region = view.drop(10, 10).take(50, 50);

// No data copied! Just pointer arithmetic:
// new_ptr = old_ptr + (10 * y_stride) + (10 * x_stride)
// new_width = 50, new_height = 50, same y_stride
```

### 2. Array2d\<T> - Owning 2D Array

An owning wrapper around `View2d<T>` that manages memory allocation.

```cpp
template<typename T>
class Array2d {
private:
    std::unique_ptr<T[]> owned_data_;
    View2d<T> view_;
    
public:
    // Constructor - allocates memory
    Array2d(u32 width, u32 height);
    
    // Move semantics (no copying)
    Array2d(Array2d&& other) noexcept = default;
    Array2d& operator=(Array2d&& other) noexcept = default;
    
    // Deleted copy operations
    Array2d(const Array2d&) = delete;
    Array2d& operator=(const Array2d&) = delete;
    
    // Access as view
    View2d<T>& getView() { return view_; }
    const View2d<T>& getView() const { return view_; }
    
    // Convenience forwarding
    T& at(u32 x, u32 y) { return view_.at(x, y); }
    T* row(u32 y) { return view_.row(y); }
    u32 getWidth() const { return view_.getWidth(); }
    u32 getHeight() const { return view_.getHeight(); }
};
```

**Key features:**
- **RAII**: Automatic memory management
- **Move-only**: Use move semantics for efficiency
- **View interface**: Provides `View2d<T>` for operations
- **Clear ownership**: Owns the data, views don't

### 3. Color Format Types

Type-safe pixel representations:

```cpp
// RGBA with 8 bits per channel
struct RGBA8 {
    u8 r, g, b, a;
    
    RGBA8() = default;
    RGBA8(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}
    
    static RGBA8 fromSColor(const SColor& c);
    SColor toSColor() const;
    
    bool operator==(const RGBA8& other) const;
};

// RGB with 8 bits per channel
struct RGB8 {
    u8 r, g, b;
    // Similar interface...
};

// Single channel 8-bit
struct R8 {
    u8 r;
    // Similar interface...
};
```

**Benefits:**
- Explicit memory layout
- Type-safe pixel operations
- Conversion to/from SColor for compatibility

### 4. Image Class

Unified image class using std::variant:

```cpp
class Image {
public:
    using ImageData = std::variant<
        Array2d<RGBA8>,
        Array2d<RGB8>,
        Array2d<R8>
    >;
    
private:
    ImageData data_;
    
public:
    // Factory methods
    static Image createRGBA8(u32 width, u32 height);
    static Image createRGB8(u32 width, u32 height);
    static Image createR8(u32 width, u32 height);
    
    // Type queries
    bool isRGBA8() const;
    bool isRGB8() const;
    bool isR8() const;
    
    // Typed access (throws if wrong type)
    View2d<RGBA8>& getAsRGBA8();
    const View2d<RGBA8>& getAsRGBA8() const;
    
    // Generic operations using std::visit
    u32 getWidth() const;
    u32 getHeight() const;
    void fill(const SColor& color);
    
    // Compatibility operations
    SColor getPixel(u32 x, u32 y) const;
    void setPixel(u32 x, u32 y, const SColor& color);
};
```

**Key features:**
- **Single implementation**: No interface/implementation split
- **Type-safe variants**: std::variant for format flexibility
- **Factory methods**: Clean creation API
- **Compatibility**: Methods for gradual migration

## Implementation Strategy

### Phase 1: Core Abstractions (2 weeks)

Create the foundation:

1. Implement `View2d<T>` template class
2. Implement `Array2d<T>` template class
3. Define color format types (RGBA8, RGB8, R8)
4. Add comprehensive unit tests

**Deliverables:**
- `irr/include/View2d.h`
- `irr/include/Array2d.h`
- `irr/include/ColorFormats.h`
- `src/unittest/test_view2d.cpp`

### Phase 2: Image Class (1 week)

Build on the foundation:

1. Create `Image` class with variant
2. Implement factory methods
3. Implement basic operations
4. Add unit tests

**Deliverables:**
- `irr/include/Image.h`
- `irr/src/Image.cpp`
- `src/unittest/test_image.cpp`

### Phase 3: Compatibility Layer (1 week)

Enable gradual migration:

1. Modify `IImage` to optionally wrap `Image`
2. Add conversion functions
3. Test both implementations side-by-side

### Phase 4-8: Migration and Cleanup

See [Migration Guide](migration_guide.md) for detailed plans.

## Testing Strategy

### Unit Tests

Test each component in isolation:

```cpp
void testView2dBasicAccess() {
    RGBA8 data[9];  // 3x3
    View2d<RGBA8> view(data, 3, 3);
    
    view.at(1, 1) = RGBA8(255, 0, 0, 255);
    UASSERT(view.at(1, 1) == RGBA8(255, 0, 0, 255));
}

void testView2dSlicing() {
    RGBA8 data[100];  // 10x10
    View2d<RGBA8> view(data, 10, 10);
    
    auto sliced = view.drop(2, 2).take(5, 5);
    UASSERT(sliced.getWidth() == 5);
    UASSERT(sliced.getHeight() == 5);
}
```

### Integration Tests

Test with real image operations:
- Load PNG/JPG images
- Apply filters
- Scale/resize
- Save images

### Performance Tests

Benchmark critical operations:
- Pixel iteration
- Sub-region operations
- Blitting
- Scaling

Compare with old implementation to ensure no regressions.

## Performance Considerations

### Zero-Cost Abstraction

Templates inline to direct operations:

```cpp
// This code:
auto region = view.drop(10, 10).take(50, 50);
for (u32 y = 0; y < region.getHeight(); ++y) {
    RGBA8* row = region.row(y);
    for (u32 x = 0; x < region.getWidth(); ++x) {
        row[x].r = 255;
    }
}

// Inlines to approximately:
RGBA8* base = data + (10 * pitch) + (10 * sizeof(RGBA8));
for (u32 y = 0; y < 50; ++y) {
    RGBA8* row = base + (y * pitch);
    for (u32 x = 0; x < 50; ++x) {
        row[x].r = 255;
    }
}
```

No overhead compared to manual code!

### Memory Layout

Memory layout remains identical:
- Contiguous storage
- Row-major ordering
- Same alignment

### Virtual Call Elimination

Old code:
```cpp
for (...) {
    img->setPixel(x, y, color);  // Virtual call!
}
```

New code:
```cpp
for (...) {
    row[x] = pixel;  // Direct memory write!
}
```

**Result**: 10-100x faster for tight loops

## Conclusion

This refactoring provides:

- ✅ Better abstractions (View2d, Array2d)
- ✅ Type safety for color formats
- ✅ Cleaner, more maintainable code
- ✅ Same or better performance
- ✅ Foundation for advanced features (SSCSM)

The gradual migration approach ensures minimal disruption while delivering immediate benefits as each phase completes.

---

**Next**: See [Migration Guide](migration_guide.md) for detailed implementation plan.
