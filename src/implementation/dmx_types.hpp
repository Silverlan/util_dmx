/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
	struct Element;
	using ElementRef = std::weak_ptr<Element>;
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

	using ElementRefArray = std::vector<ElementRef>;
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

	Time get_time(const std::string &value);
	Time get_time(int32_t value);
	Quat get_quaternion(const std::string &value);
};

#endif
