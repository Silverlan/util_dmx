/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __UTIL_DMX_HPP__
#define __UTIL_DMX_HPP__

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

class VFilePtrInternal;
namespace dmx
{
	enum class AttrType : uint32_t
	{
		SingleFirst = 0,

		None = SingleFirst,
		Element,
		Int,
		Float,
		Bool,
		String,
		Binary,
		Time,
		ObjectId,
		Color,
		Vector2,
		Vector3,
		Vector4,
		Angle,
		Quaternion,
		Matrix,
		UInt64,
		UInt8,
		SingleLast = UInt8,

		ArrayFirst,
		ElementArray = ArrayFirst,
		IntArray,
		FloatArray,
		BoolArray,
		StringArray,
		BinaryArray,
		TimeArray,
		ObjectIdArray,
		ColorArray,
		Vector2Array,
		Vector3Array,
		Vector4Array,
		AngleArray,
		QuaternionArray,
		MatrixArray,
		ArrayLast = MatrixArray,

		Invalid = std::numeric_limits<uint32_t>::max()
	};
	struct Attribute
	{
		AttrType type = AttrType::Invalid;
		std::shared_ptr<void> data = nullptr;
	};
	struct Element
	{
		std::string type;
		std::string name;
		std::array<char,16> GUID;
		std::unordered_map<std::string,std::shared_ptr<dmx::Attribute>> attributes;
	};
	class FileData
	{
	public:
		static std::shared_ptr<FileData> Load(std::shared_ptr<VFilePtrInternal> &f);

		const std::vector<std::shared_ptr<Element>> &GetElements() const;
	private:
		FileData()=default;

		std::vector<std::shared_ptr<Element>> m_elements;
	};
	std::string type_to_string(AttrType type);
	bool is_single_type(AttrType type);
	bool is_array_type(AttrType type);
	AttrType get_single_type(AttrType type);
};

#endif
