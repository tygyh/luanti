# Solution Overview - Image Refactoring

## Quick Summary

This document provides a high-level overview of the proposed image refactoring for issue #16364.

## The Problem

Current issues with `IImage`/`CImage`:

1. **Manual iteration is error-prone**
   ```cpp
   // Current: Manual bounds checking and offset calculations
   for (u32 y = startY; y < endY && y < img->getDimension().Height; ++y) {
       for (u32 x = startX; x < endX && x < img->getDimension().Width; ++x) {
           // Process pixel...
       }
   }
   ```

2. **No type safety**
   ```cpp
   // Runtime format checking
   if (img->getColorFormat() == ECF_A8R8G8B8) {
       // Process as RGBA8...
   }
   ```

3. **Interface/Implementation split adds complexity**
   - Virtual function overhead
   - Reference counting complexity
   - Harder to maintain

4. **No efficient sub-regions**
   - Must copy data to work with sub-regions
   - Or manually calculate offsets everywhere

## The Solution

### Core Components

#### 1. View2d\<T> - Non-owning 2D View
```cpp
template<typename T>
class View2d {
    T* data_;
    u32 width_, height_;
    u32 y_stride_;  // pitch
    
public:
    // Element access
    T& at(u32 x, u32 y);
    T* row(u32 y);
    
    // Slicing operations
    View2d drop(u32 left, u32 top) const;
    View2d take(u32 width, u32 height) const;
    View2d slice(u32 x, u32 y, u32 w, u32 h) const;
};
```

**Benefits:**
- Zero-cost abstraction
- Type-safe pixel access
- Efficient slicing without copying
- Clean, declarative API

#### 2. Array2d\<T> - Owning 2D Array
```cpp
template<typename T>
class Array2d {
    std::unique_ptr<T[]> owned_data_;
    View2d<T> view_;
    
public:
    Array2d(u32 width, u32 height);
    View2d<T>& getView();
};
```

**Benefits:**
- RAII memory management
- Move semantics for efficiency
- Provides View2d interface

#### 3. Type-Safe Color Formats
```cpp
struct RGBA8 { u8 r, g, b, a; };
struct RGB8 { u8 r, g, b; };
struct R8 { u8 r; };
```

**Benefits:**
- Compile-time type safety
- Clear memory layout
- No format confusion

#### 4. Unified Image Class
```cpp
class Image {
    std::variant<Array2d<RGBA8>, Array2d<RGB8>, Array2d<R8>> data_;
    
public:
    static Image createRGBA8(u32 w, u32 h);
    
    bool isRGBA8() const;
    View2d<RGBA8>& getAsRGBA8();
    
    u32 getWidth() const;
    u32 getHeight() const;
};
```

**Benefits:**
- Single implementation (no interface split)
- Type-safe format access
- std::variant for flexibility

## Example Usage

### Creating and Filling an Image
```cpp
Image img = Image::createRGBA8(100, 100);
img.fill(SColor(255, 255, 0, 0));  // Red
```

### Working with Sub-Regions
```cpp
auto& view = img.getAsRGBA8();
auto region = view.drop(10, 10).take(50, 50);

for (u32 y = 0; y < region.getHeight(); ++y) {
    RGBA8* row = region.row(y);
    for (u32 x = 0; x < region.getWidth(); ++x) {
        row[x].r = 255;  // Type-safe!
    }
}
```

### Texture Modifier (Motivation from Issue)
```cpp
void applyModifier(Image& dst, const Image& src) {
    // As described in issue: drop(10,20), take(100,200), then blit
    auto src_region = src.getAsRGBA8().drop(10, 20).take(100, 200);
    auto& dst_view = dst.getAsRGBA8();
    
    for (u32 y = 0; y < src_region.getHeight(); ++y) {
        memcpy(dst_view.row(y), src_region.row(y), 
               src_region.getWidth() * sizeof(RGBA8));
    }
}
```

## Key Benefits

### Type Safety
- **Before**: Runtime format checks with switch statements
- **After**: Compile-time type checking with templates
- **Impact**: Catch errors at compile time, not runtime

### Performance
- **Before**: Virtual function calls, pointer indirections
- **After**: Template inlining, direct pointer access
- **Impact**: Same or better performance, no regressions

### Code Clarity
- **Before**: 15+ lines for sub-region iteration with manual offsets
- **After**: 3 lines with declarative slicing
- **Impact**: 50-70% less code for common operations

### Maintainability
- **Before**: Interface/implementation split, reference counting
- **After**: Single implementation, clear ownership
- **Impact**: Easier to understand and modify

## Implementation Timeline

| Phase | Duration  | Description                              |
|-------|-----------|------------------------------------------|
| 1     | 2 weeks   | Implement View2d, Array2d, color formats |
| 2     | 1 week    | Add compatibility layer                  |
| 3     | 1-2 weeks | Migrate image loaders                    |
| 4     | 2-3 weeks | Migrate internal operations              |
| 5     | 1-2 weeks | Update public APIs                       |
| 6     | 1 week    | Analyze format usage                     |
| 7     | 1-2 weeks | Remove legacy code                       |
| 8     | 1 week    | Documentation                            |

**Total**: 10-14 weeks (2.5-3.5 months)

## Risk Mitigation

- **Gradual migration** with backwards compatibility
- **Comprehensive testing** (unit, integration, performance)
- **No performance regression** (benchmark critical paths)
- **Clear rollback plan** if issues discovered

## Success Criteria

- ✅ All tests pass
- ✅ No performance regression
- ✅ Cleaner, more maintainable code
- ✅ Type-safe API
- ✅ Easy migration path

## Next Steps

1. Review this proposal with maintainers
2. Discuss and approve approach
3. Audit color format usage in codebase
4. Begin Phase 1 implementation
5. Iterate based on feedback

## Further Reading

- [Technical Solution](technical_solution.md) - Complete technical details
- [Architecture Diagrams](architecture_diagrams.md) - Visual comparison
- [Migration Guide](migration_guide.md) - Detailed migration plan
- [Implementation Guide](implementation_guide.md) - First steps with code
