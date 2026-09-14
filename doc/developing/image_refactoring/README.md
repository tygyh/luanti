# Image Refactoring Proposal - Issue #16364

This directory contains comprehensive documentation for refactoring Luanti's image abstractions as described in [issue #16364](https://github.com/luanti-org/luanti/issues/16364).

## 📚 Quick Navigation

| Document                                          | Purpose                                    | Read Time |
|---------------------------------------------------|--------------------------------------------|-----------|
| [Solution Overview](solution_overview.md)         | High-level summary and quick reference     | 5 min     |
| [Color Format Analysis](color_format_analysis.md) | Analysis of which formats to support       | 10 min    |
| [Technical Solution](technical_solution.md)       | Complete technical design and architecture | 30 min    |
| [Architecture Diagrams](architecture_diagrams.md) | Visual comparison and diagrams             | 15 min    |
| [Migration Guide](migration_guide.md)             | 8-phase migration plan (10-14 weeks)       | 20 min    |
| [Implementation Guide](implementation_guide.md)   | Concrete first steps with code             | 20 min    |
| [Proof of Concept](proof_of_concept.h)            | Working header-only implementation         | Reference |
| [Usage Examples](usage_examples.cpp)              | Practical code examples                    | Reference |

## 🎯 Problem Statement

The current Luanti image abstractions (`IImage` interface and `CImage` implementation) have several issues:

1. **Poor abstractions** - Code must manually handle basic tasks like iterating image regions, which is error-prone
2. **No type safety** - No compile-time way to express that an image is RGBA8 vs RGB8
3. **Unnecessary complexity** - The IImage/CImage interface/implementation split adds overhead
4. **No efficient sub-regions** - No way to work with image regions without copying data

## 💡 Solution Overview

Implement a modern, type-safe image library based on:

1. **View2d\<T>** - Lightweight, non-owning 2D view into data
2. **Array2d\<T>** - Owning 2D array with automatic memory management
3. **Type-safe color formats** - RGBA8, RGB8, R8 structs
4. **Image** - Unified class using std::variant for different formats

## 📊 Code Comparison

### Before (Current - Error-prone)
```cpp
IImage* img = createImage();
for (u32 y = 10; y < 110; ++y) {
    for (u32 x = 20; x < 120; ++x) {
        SColor pixel = img->getPixel(x, y);
        img->setPixel(x, y, processedPixel);
    }
}
```

### After (Proposed - Clean & Type-safe)
```cpp
Image img = Image::createRGBA8(100, 100);
auto region = img.getAsRGBA8().drop(10, 20).take(100, 100);

for (u32 y = 0; y < region.getHeight(); ++y) {
    RGBA8* row = region.row(y);
    for (u32 x = 0; x < region.getWidth(); ++x) {
        processPixel(row[x]);  // Type-safe!
    }
}
```

**Result**: 50-70% less code, type-safe, and faster

## ✨ Key Benefits

- **Type Safety** - Compile-time checking of color format operations
- **Performance** - Zero-cost abstractions, no virtual overhead
- **Clean API** - Declarative slicing: `view.drop(10, 20).take(100, 200)`
- **Maintainability** - Single implementation, clear ownership semantics

## 🗺️ Implementation Roadmap

8-phase migration plan totaling **10-14 weeks**:

1. Implement View2d, Array2d, color formats (2 weeks)
2. Add compatibility layer (1 week)
3. Migrate image loaders (1-2 weeks)
4. Migrate internal operations (2-3 weeks)
5. Update public APIs (1-2 weeks)
6. Analyze format usage (1 week)
7. Remove legacy code (1-2 weeks)
8. Documentation (1 week)

## 🚀 Getting Started

**For understanding the proposal:**
1. Read [Solution Overview](solution_overview.md) for a quick introduction
2. Read [Color Format Analysis](color_format_analysis.md) to understand which formats to support
3. Review [Architecture Diagrams](architecture_diagrams.md) for visual comparison
4. Study [Technical Solution](technical_solution.md) for complete details

**For implementing the proposal:**
1. Review [Migration Guide](migration_guide.md) for the overall plan
2. Follow [Implementation Guide](implementation_guide.md) for Phase 1
3. Reference [Proof of Concept](proof_of_concept.h) for working code

## 📝 Status

- **Status**: Proposal stage - awaiting review and approval
- **Issue**: [#16364](https://github.com/luanti-org/luanti/issues/16364)
- **Timeline**: 10-14 weeks for complete implementation
- **Breaking Changes**: Managed through 7-phase gradual migration

## 🤝 Contributing

This proposal is open for discussion and feedback. To contribute:

1. Review the documentation
2. Provide feedback on the approach
3. Suggest improvements
4. Help with implementation (if approved)

---

**Next Steps**: Review documentation → Discuss with maintainers → Get approval → Begin Phase 1
