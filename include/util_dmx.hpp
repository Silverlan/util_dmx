/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __UTIL_DMX_HPP__
#define __UTIL_DMX_HPP__

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "util_dmx_types.hpp"

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

	struct Element;
	struct Attribute
	{
		AttrType type = AttrType::Invalid;
		std::shared_ptr<void> data = nullptr;

		std::shared_ptr<Element> Get(const std::string &name) const;
		std::string DataToString() const;
		void DebugPrint(std::stringstream &ss);
		void DebugPrint(std::stringstream &ss,std::unordered_set<void*> &iteratedObjects,const std::string &t0="",const std::string &t="");

		template<typename T>
			T *GetValue(AttrType type)
		{
			return (this->type == type) ? static_cast<T*>(data.get()) : nullptr;
		}
		ElementRef *GetElement();
		Int *GetInt();
		Float *GetFloat();
		Bool *GetBoolean();
		String *GetString();
		Binary *GetBinary();
		Time *GetTime();
		Color *GetColor();
		Vector2 *GetVector2();
		Vector3 *GetVector3();
		Vector4 *GetVector4();
		Angle *GetAngle();
		Quaternion *GetQuaternion();
		Matrix *GetMatrix();
		UInt64 *GetUInt64();
		UInt8 *GetUInt8();
		std::vector<std::shared_ptr<dmx::Attribute>> *GetArray();
		std::vector<std::shared_ptr<dmx::Attribute>> *GetArray(AttrType type);
	};
	struct Element
		: public std::enable_shared_from_this<Element>
	{
		std::string type;
		std::string name;
		std::array<uint8_t,16> GUID;
		std::unordered_map<std::string,std::shared_ptr<dmx::Attribute>> attributes;
		std::unordered_map<std::string,std::weak_ptr<Element>> nameToChildElement;

		std::string GetGUIDAsString() const;
		std::shared_ptr<Element> Get(const std::string &name) const;
		std::shared_ptr<Attribute> GetAttr(const std::string &name) const;
		void DebugPrint(std::stringstream &ss);
		void DebugPrint(std::stringstream &ss,std::unordered_set<void*> &iteratedObjects,const std::string &t="");
	};
	class FileData
	{
	public:
		static std::shared_ptr<FileData> Load(std::shared_ptr<VFilePtrInternal> &f);

		const std::vector<std::shared_ptr<Element>> &GetElements() const;
		const std::shared_ptr<Attribute> &GetRootAttribute() const;
		void DebugPrint(std::stringstream &ss);
	private:
		FileData()=default;
		static std::shared_ptr<FileData> CreateFromKeyValues2Data(const void *kv2Data);
		void UpdateRootElement();
		void UpdateChildElementLookupTables();

		std::shared_ptr<Attribute> m_rootAttribute = nullptr;
		std::vector<std::shared_ptr<Element>> m_elements = {};
	};
	std::string type_to_string(AttrType type);
	bool is_single_type(AttrType type);
	bool is_array_type(AttrType type);
	AttrType get_single_type(AttrType type);
};

#endif
