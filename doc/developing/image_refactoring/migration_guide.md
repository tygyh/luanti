# Migration Guide - 8-Phase Plan

This document details the complete migration strategy from the current IImage/CImage architecture to the new View2d/Array2d/Image architecture.

## Timeline Overview

**Total Duration**: 10-14 weeks (2.5-3.5 months)

| Phase                  | Duration  | Complexity | Risk   |
|------------------------|-----------|------------|--------|
| 1. Core Abstractions   | 2 weeks   | Medium     | Low    |
| 2. Image Class         | 1 week    | Low        | Low    |
| 3. Compatibility Layer | 1 week    | Medium     | Medium |
| 4. Image Loaders       | 1-2 weeks | Medium     | Low    |
| 5. Internal Operations | 2-3 weeks | High       | Medium |
| 6. Public APIs         | 1-2 weeks | Medium     | High   |
| 7. Format Analysis     | 1 week    | Low        | Low    |
| 8. Legacy Removal      | 1-2 weeks | High       | High   |

## Phase 1: Core Abstractions (2 weeks)

### Objective
Implement the foundation: View2d, Array2d, and color format types.

### Tasks

1. **Create View2d.h**
   - Template class for 2D views
   - Slicing operations (drop, take, slice)
   - Element and row access
   - Unit tests

2. **Create Array2d.h**
   - Owning wrapper around View2d
   - Memory allocation/deallocation
   - Move semantics
   - Unit tests

3. **Create ColorFormats.h**
   - RGBA8, RGB8, R8 structs
   - Conversion to/from SColor
   - Unit tests

### Deliverables
```
irr/include/View2d.h
irr/include/Array2d.h
irr/include/ColorFormats.h
src/unittest/test_view2d.cpp
src/unittest/test_array2d.cpp
src/unittest/test_colorformats.cpp
```

### Success Criteria
- ✅ All tests pass
- ✅ Templates compile without errors
- ✅ No warnings from compiler
- ✅ Performance benchmarks look good

### Risks & Mitigation
- **Risk**: Template compilation errors
- **Mitigation**: Start with simple cases, add complexity gradually
- **Risk**: Performance issues
- **Mitigation**: Benchmark early, use `-O2` optimization

## Phase 2: Image Class (1 week)

### Objective
Build the Image class on top of the core abstractions.

### Tasks

1. **Create Image class**
   - std::variant holding Array2d variants
   - Factory methods for each format
   - Type queries (isRGBA8, etc.)
   - Typed accessors (getAsRGBA8, etc.)

2. **Implement basic operations**
   - getWidth(), getHeight()
   - fill()
   - getPixel(), setPixel() for compatibility

3. **Add tests**
   - Image creation
   - Format checking
   - Basic operations
   - Format conversions

### Deliverables
```
irr/include/Image.h
irr/src/Image.cpp
src/unittest/test_image.cpp
```

### Success Criteria
- ✅ Image class works with all formats
- ✅ std::visit operations work correctly
- ✅ Tests cover all code paths
- ✅ No memory leaks (valgrind clean)

## Phase 3: Compatibility Layer (1 week)

### Objective
Enable gradual migration by making IImage wrap the new Image.

### Tasks

1. **Modify IImage.h**
   ```cpp
   class IImage : public virtual IReferenceCounted {
   private:
       std::unique_ptr<Image> modern_impl_;
       
   public:
       static IImage* createModern(Image&& img);
       Image* getModernImage();
       const Image* getModernImage() const;
       
       // Existing virtual methods...
   };
   ```

2. **Update CImage.cpp**
   - Forward operations to modern_impl_ if available
   - Fall back to old implementation otherwise

3. **Add conversion utilities**
   - IImage* → Image
   - Image → IImage*

### Deliverables
```
irr/include/IImage.h (modified)
irr/src/CImage.cpp (modified)
irr/include/ImageConversion.h (new)
```

### Success Criteria
- ✅ Old code continues to work
- ✅ New code can use modern API
- ✅ Both APIs work side-by-side
- ✅ Feature flag to toggle implementations

### Risks & Mitigation
- **Risk**: Breaking existing code
- **Mitigation**: Extensive testing, feature flag for rollback

## Phase 4: Image Loaders (1-2 weeks)

### Objective
Update PNG/JPG/TGA loaders to create modern Image objects.

### Tasks

1. **Update CImageLoaderPNG.cpp**
   ```cpp
   IImage* CImageLoaderPNG::loadImage(io::IReadFile* file) {
       // Decode PNG...
       
       Image modern_img = Image::createRGBA8(width, height);
       auto& view = modern_img.getAsRGBA8();
       std::memcpy(view.getData(), decoded_data, size);
       
       return IImage::createModern(std::move(modern_img));
   }
   ```

2. **Update other loaders similarly**
   - CImageLoaderJPG.cpp
   - CImageLoaderTGA.cpp

3. **Test all image formats**
   - Load various test images
   - Verify correct decoding
   - Check memory usage

### Deliverables
```
irr/src/CImageLoaderPNG.cpp (modified)
irr/src/CImageLoaderJPG.cpp (modified)
irr/src/CImageLoaderTGA.cpp (modified)
```

### Success Criteria
- ✅ All image formats load correctly
- ✅ Same visual output as before
- ✅ No memory leaks
- ✅ Performance unchanged or better

## Phase 5: Internal Operations (2-3 weeks)

### Objective
Migrate internal image processing code to use new API.

### Tasks

1. **Update image filters**
   ```cpp
   // Old
   void applyFilter(IImage* img) { ... }
   
   // New
   void applyFilter(Image& img) { ... }
   
   // Wrapper for compatibility
   void applyFilter(IImage* img) {
       if (Image* modern = img->getModernImage()) {
           applyFilter(*modern);
       } else {
           // Old implementation
       }
   }
   ```

2. **Update texture handling**
   - Texture generation
   - Mipmap generation
   - Scaling/filtering

3. **Update GUI image handling**
   - Button images
   - Background images
   - Animated images

### Files to Update
```
src/client/imagefilters.cpp
src/client/imagesource.cpp
src/gui/guiButtonImage.cpp
src/gui/guiBackgroundImage.cpp
src/gui/guiAnimatedImage.cpp
... and others
```

### Success Criteria
- ✅ All operations work correctly
- ✅ Visual output unchanged
- ✅ Performance same or better
- ✅ Code is cleaner and more maintainable

### Risks & Mitigation
- **Risk**: Performance regression
- **Mitigation**: Benchmark each operation, optimize hot paths
- **Risk**: Visual artifacts
- **Mitigation**: Extensive visual testing, screenshot comparisons

## Phase 6: Public APIs (1-2 weeks)

### Objective
Update public-facing APIs that mods/plugins use.

### Tasks

1. **Analyze public API surface**
   - Which functions are exposed to mods?
   - Which take/return IImage*?

2. **Add new overloads**
   ```cpp
   // Old - keep for compatibility
   IImage* applyFilter(IImage* input);
   
   // New - preferred API
   Image applyFilter(const Image& input);
   ```

3. **Document migration**
   - Create migration guide for modders
   - Provide examples
   - Deprecation timeline

4. **Add deprecation warnings**
   ```cpp
   [[deprecated("Use Image version instead")]]
   IImage* oldFunction(IImage* img);
   ```

### Success Criteria
- ✅ New APIs available
- ✅ Old APIs still work
- ✅ Clear migration path documented
- ✅ No breaking changes yet

### Risks & Mitigation
- **Risk**: Breaking mods
- **Mitigation**: Keep old APIs working, long deprecation period

## Phase 7: Format Analysis (1 week)

### Objective
Determine which color formats are actually used and simplify.

### Tasks

1. **Audit codebase**
   ```bash
   grep -r "ECF_" src/ irr/ | grep -oE "ECF_[A-Z0-9_]+" | sort | uniq -c
   ```

2. **Analyze results**
   - Which formats are commonly used?
   - Which are legacy/rarely used?
   - Can we remove some?

3. **Update Image variant**
   ```cpp
   // Before: Many formats
   std::variant<Array2d<RGBA8>, Array2d<RGB8>, Array2d<R8>, 
                Array2d<R5G6B5>, Array2d<A1R5G5B5>, ...>
   
   // After: Common formats only
   std::variant<Array2d<RGBA8>, Array2d<RGB8>, Array2d<R8>>
   ```

4. **Add conversion for legacy formats**
   - Convert R5G6B5 → RGBA8 on load
   - Document format support

### Success Criteria
- ✅ Format usage documented
- ✅ Image variant simplified
- ✅ Conversions added where needed
- ✅ Tests updated

## Phase 8: Legacy Removal (1-2 weeks)

### Objective
Remove IImage/CImage and make Image the primary implementation.

### Tasks

1. **Remove IImage interface**
   - Delete IImage.h
   - Delete CImage.h/.cpp
   - Update all call sites

2. **Update all code**
   - IImage* → Image&
   - image->method() → image.method()
   - Update ownership (pointers → values/references)

3. **Remove compatibility layer**
   - Remove wrapper code
   - Remove feature flags
   - Simplify codebase

4. **Final testing**
   - Full regression test
   - Performance validation
   - Memory leak check

### Success Criteria
- ✅ All code compiles
- ✅ All tests pass
- ✅ No performance regression
- ✅ Code is cleaner

### Risks & Mitigation
- **Risk**: Breaking changes
- **Mitigation**: This is a breaking change, plan for major version bump
- **Risk**: Hidden dependencies
- **Mitigation**: Thorough testing, beta release

## Testing Strategy

### Unit Tests
- Test each component in isolation
- Cover edge cases and error conditions
- Aim for >90% coverage

### Integration Tests
- Load/save images in all formats
- Apply all operations (filter, scale, etc.)
- Render in GUI
- Compare with baseline images

### Performance Tests
```cpp
// Benchmark critical operations
void benchmarkPixelIteration() {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform operation...
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Assert no regression
    ASSERT(duration.count() <= baseline * 1.1);  // Max 10% slower
}
```

### Visual Tests
- Screenshot comparison
- Manual visual inspection
- Automated pixel-perfect comparison

## Risk Management

### Performance Regression
- **Mitigation**: Benchmark every phase
- **Rollback**: Keep old implementation available
- **Monitoring**: Profile hot paths

### Memory Leaks
- **Mitigation**: Use RAII, smart pointers
- **Testing**: Valgrind on every commit
- **Review**: Code review for ownership

### Breaking Changes
- **Mitigation**: Gradual migration
- **Communication**: Announce changes early
- **Support**: Help modders migrate

### Scope Creep
- **Mitigation**: Stick to defined phases
- **Planning**: Each phase is self-contained
- **Review**: Regular progress reviews

## Communication Plan

### For Core Developers
- Weekly status updates
- Code reviews for each phase
- Performance metrics sharing

### For Mod Developers
- Early announcement of changes
- Beta releases for testing
- Migration guide and examples
- Support forum

### For Users
- Transparent changelog
- Performance improvements highlighted
- No visible changes (ideally)

## Rollback Plan

If issues are discovered:

1. **Before Phase 8**: Use feature flag to toggle implementations
2. **After Phase 8**: Revert commits (breaking change)
3. **Emergency**: Hot fix specific issues

## Success Metrics

| Metric         | Target         | Measurement |
|----------------|----------------|-------------|
| Test Pass Rate | 100%           | CI/CD       |
| Code Coverage  | \>90%          | gcov        |
| Performance    | ≤10% slower    | Benchmarks  |
| Memory Usage   | Same or less   | Valgrind    |
| Code Reduction | 20-30% less    | LOC count   |
| Compile Time   | Same or faster | Build time  |

## Timeline Summary

```
Week  1-2: Phase 1 - Core Abstractions
Week  3:   Phase 2 - Image Class
Week  4:   Phase 3 - Compatibility Layer
Week  5-6: Phase 4 - Image Loaders
Week  7-9: Phase 5 - Internal Operations
Week 10-11: Phase 6 - Public APIs
Week 12:    Phase 7 - Format Analysis
Week 13-14: Phase 8 - Legacy Removal
```

**Total**: 14 weeks maximum

**Aggressive timeline**: 10 weeks with parallel work

## Conclusion

This migration plan provides:
- Clear phases with defined goals
- Risk mitigation strategies
- Testing at every step
- Rollback capabilities
- Stakeholder communication

Following this plan ensures a smooth transition to the new image architecture with minimal disruption and maximum benefit.

---

**Next**: See [Implementation Guide](implementation_guide.md) for concrete first steps.
