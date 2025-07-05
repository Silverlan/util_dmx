// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_DMX_TYPES_HPP__
#define __UTIL_DMX_TYPES_HPP__

#include <cinttypes>
#include <memory>
#include <vector>
#include <mathutil/umath.h>
#include <mathutil/eulerangles.h>
#include <mathutil/uquat.h>
#include <mathutil/uvec.h>
#include <mathutil/umat.h>

namespace source_engine::dmx {
	using Int = int32_t;
	using Float = float;
	using Bool = bool;
	using String = std::string;
	using Binary = std::vector<uint8_t>;
	using Time = float;
	// using ObjectId = ;
	using Color = std::array<uint8_t, 4>;
	using Vector2 = ::Vector2;
	using Vector3 = ::Vector3;
	using Vector4 = ::Vector4;
	using Angle = ::EulerAngles;
	using Quaternion = ::Quat;
	using Matrix = ::Mat4;
	using UInt64 = uint64_t;
	using UInt8 = uint8_t;

	using IntArray = std::vector<Int>;
	using FloatArray = std::vector<Float>;
	using BoolArray = std::vector<Bool>;
	using StringArray = std::vector<String>;
	using BinaryArray = std::vector<Binary>;
	using TimeArray = std::vector<Time>;
	// using ObjectIdArray = ;
	using ColorArray = std::vector<Color>;
	using Vector2Array = std::vector<Vector2>;
	using Vector3Array = std::vector<Vector3>;
	using Vector4Array = std::vector<Vector4>;
	using AngleArray = std::vector<Angle>;
	using QuaternionArray = std::vector<Quaternion>;
	using MatrixArray = std::vector<Matrix>;
};

#endif
