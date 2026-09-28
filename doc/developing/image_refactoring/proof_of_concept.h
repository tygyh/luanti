// Proof-of-Concept Implementation for Issue #16364
// This demonstrates the complete architecture with working code

#pragma once

#include <memory>
#include <variant>
#include <cstring>
#include <cassert>

// Note: In actual implementation, these would use irr types
using u8 = unsigned char;
using u32 = unsigned int;

// Placeholder for SColor - in actual implementation, use irr/include/SColor.h
struct SColor {
	u32 color;

	SColor(u8 a, u8 r, u8 g, u8 b) : color((a << 24) | (r << 16) | (g << 8) | b) {
	}

	u8 getAlpha() const { return (color >> 24) & 0xFF; }
	u8 getRed() const { return (color >> 16) & 0xFF; }
	u8 getGreen() const { return (color >> 8) & 0xFF; }
	u8 getBlue() const { return color & 0xFF; }
};

namespace video {
	// Color Format Types
	struct RGBA8 {
		u8 r, g, b, a;

		RGBA8() = default;

		RGBA8(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {
		}

		static RGBA8 fromSColor(const SColor &c) {
			return RGBA8(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
		}

		SColor toSColor() const {
			return SColor(a, r, g, b);
		}

		bool operator==(const RGBA8 &other) const {
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}
	};

	struct RGB8 {
		u8 r, g, b;

		RGB8() = default;

		RGB8(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {
		}

		static RGB8 fromSColor(const SColor &c) {
			return RGB8(c.getRed(), c.getGreen(), c.getBlue());
		}

		SColor toSColor() const {
			return SColor(255, r, g, b);
		}

		bool operator==(const RGB8 &other) const {
			return r == other.r && g == other.g && b == other.b;
		}
	};

	struct R8 {
		u8 r;

		R8() = default;

		explicit R8(u8 r) : r(r) {
		}

		bool operator==(const R8 &other) const {
			return r == other.r;
		}
	};

	// View2d - Non-owning 2D view
	template<typename T>
	class View2d {
	private:
		T *data_;
		u32 width_, height_;
		u32 x_stride_, y_stride_;

	public:
		View2d(T *data, u32 width, u32 height, u32 y_stride = 0)
			: data_(data), width_(width), height_(height)
			  , x_stride_(sizeof(T))
			  , y_stride_(y_stride > 0 ? y_stride : width * sizeof(T)) {
			assert(data != nullptr || (width == 0 && height == 0));
		}

		T &at(u32 x, u32 y) {
			assert(x < width_ && y < height_);
			u8 *ptr = reinterpret_cast<u8 *>(data_) + y * y_stride_ + x * x_stride_;
			return *reinterpret_cast<T *>(ptr);
		}

		const T &at(u32 x, u32 y) const {
			assert(x < width_ && y < height_);
			const u8 *ptr = reinterpret_cast<const u8 *>(data_) + y * y_stride_ + x * x_stride_;
			return *reinterpret_cast<const T *>(ptr);
		}

		T *row(u32 y) {
			assert(y < height_);
			u8 *ptr = reinterpret_cast<u8 *>(data_) + y * y_stride_;
			return reinterpret_cast<T *>(ptr);
		}

		const T *row(u32 y) const {
			assert(y < height_);
			const u8 *ptr = reinterpret_cast<const u8 *>(data_) + y * y_stride_;
			return reinterpret_cast<const T *>(ptr);
		}

		View2d drop(u32 left, u32 top) const {
			assert(left <= width_ && top <= height_);
			u8 *new_data = reinterpret_cast<u8 *>(data_) + top * y_stride_ + left * x_stride_;
			return View2d(reinterpret_cast<T *>(new_data),
			              width_ - left, height_ - top, y_stride_);
		}

		View2d take(u32 width, u32 height) const {
			assert(width <= width_ && height <= height_);
			return View2d(data_, width, height, y_stride_);
		}

		View2d slice(u32 x, u32 y, u32 width, u32 height) const {
			return drop(x, y).take(width, height);
		}

		u32 getWidth() const { return width_; }
		u32 getHeight() const { return height_; }
		u32 getYStride() const { return y_stride_; }
		T *getData() { return data_; }
		const T *getData() const { return data_; }
	};

	// Array2d - Owning 2D array
	template<typename T>
	class Array2d {
	private:
		std::unique_ptr<T[]> owned_data_;
		View2d<T> view_;

	public:
		Array2d(u32 width, u32 height)
			: owned_data_(new T[width * height])
			  , view_(owned_data_.get(), width, height) {
		}

		Array2d(Array2d &&other) noexcept = default;

		Array2d &operator=(Array2d &&other) noexcept = default;

		Array2d(const Array2d &) = delete;

		Array2d &operator=(const Array2d &) = delete;

		View2d<T> &getView() { return view_; }
		const View2d<T> &getView() const { return view_; }

		T &at(u32 x, u32 y) { return view_.at(x, y); }
		const T &at(u32 x, u32 y) const { return view_.at(x, y); }
		T *row(u32 y) { return view_.row(y); }
		const T *row(u32 y) const { return view_.row(y); }
		u32 getWidth() const { return view_.getWidth(); }
		u32 getHeight() const { return view_.getHeight(); }
		T *getData() { return view_.getData(); }
		const T *getData() const { return view_.getData(); }
	};

	// Image - Variant-based image container
	class Image {
	public:
		using ImageData = std::variant<Array2d<RGBA8>, Array2d<RGB8>, Array2d<R8> >;

	private:
		ImageData data_;

		explicit Image(ImageData &&data) : data_(std::move(data)) {
		}

	public:
		static Image createRGBA8(u32 width, u32 height) {
			return Image(Array2d<RGBA8>(width, height));
		}

		static Image createRGB8(u32 width, u32 height) {
			return Image(Array2d<RGB8>(width, height));
		}

		static Image createR8(u32 width, u32 height) {
			return Image(Array2d<R8>(width, height));
		}

		bool isRGBA8() const { return std::holds_alternative<Array2d<RGBA8> >(data_); }
		bool isRGB8() const { return std::holds_alternative<Array2d<RGB8> >(data_); }
		bool isR8() const { return std::holds_alternative<Array2d<R8> >(data_); }

		View2d<RGBA8> &getAsRGBA8() {
			return std::get<Array2d<RGBA8> >(data_).getView();
		}

		const View2d<RGBA8> &getAsRGBA8() const {
			return std::get<Array2d<RGBA8> >(data_).getView();
		}

		View2d<RGB8> &getAsRGB8() {
			return std::get<Array2d<RGB8> >(data_).getView();
		}

		const View2d<RGB8> &getAsRGB8() const {
			return std::get<Array2d<RGB8> >(data_).getView();
		}

		View2d<R8> &getAsR8() {
			return std::get<Array2d<R8> >(data_).getView();
		}

		const View2d<R8> &getAsR8() const {
			return std::get<Array2d<R8> >(data_).getView();
		}

		u32 getWidth() const {
			return std::visit([](const auto &arr) { return arr.getWidth(); }, data_);
		}

		u32 getHeight() const {
			return std::visit([](const auto &arr) { return arr.getHeight(); }, data_);
		}

		void fill(const SColor &color) {
			if (isRGBA8()) {
				RGBA8 rgba = RGBA8::fromSColor(color);
				auto &view = getAsRGBA8();
				for (u32 y = 0; y < view.getHeight(); ++y) {
					RGBA8 *row_ptr = view.row(y);
					for (u32 x = 0; x < view.getWidth(); ++x) {
						row_ptr[x] = rgba;
					}
				}
			} else if (isRGB8()) {
				RGB8 rgb = RGB8::fromSColor(color);
				auto &view = getAsRGB8();
				for (u32 y = 0; y < view.getHeight(); ++y) {
					RGB8 *row_ptr = view.row(y);
					for (u32 x = 0; x < view.getWidth(); ++x) {
						row_ptr[x] = rgb;
					}
				}
			}
		}

		SColor getPixel(u32 x, u32 y) const {
			if (isRGBA8()) {
				return getAsRGBA8().at(x, y).toSColor();
			} else if (isRGB8()) {
				return getAsRGB8().at(x, y).toSColor();
			}
			return SColor(0, 0, 0, 0);
		}

		void setPixel(u32 x, u32 y, const SColor &color) {
			if (isRGBA8()) {
				getAsRGBA8().at(x, y) = RGBA8::fromSColor(color);
			} else if (isRGB8()) {
				getAsRGB8().at(x, y) = RGB8::fromSColor(color);
			}
		}
	};
} // namespace video
