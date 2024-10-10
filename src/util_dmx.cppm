/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include "dmx_types.hpp"
#include "definitions.hpp"

export module source_engine.dmx;

export import :keyvalues2;

export namespace source_engine::dmx {
	enum class AttrType : uint32_t {
		None = 0,
		SingleFirst,
		Element = SingleFirst,
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
	using ElementRef = std::weak_ptr<Element>;
	using ElementRefArray = std::vector<ElementRef>;
	struct Attribute : public std::enable_shared_from_this<Attribute> {
		AttrType type = AttrType::Invalid;
		std::shared_ptr<void> data = nullptr;

		std::shared_ptr<Element> Get(const std::string &name) const;
		std::string DataToString() const;
		void DebugPrint(std::stringstream &ss);
		void DebugPrint(std::stringstream &ss, std::unordered_set<void *> &iteratedObjects, const std::string &t0 = "", const std::string &t = "");

		template<typename T>
		T *GetValue(AttrType type)
		{
			return (this->type == type) ? static_cast<T *>(data.get()) : nullptr;
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
		void RemoveArrayValue(uint32_t idx);
		void RemoveArrayValue(dmx::Attribute &attr);
		void AddArrayValue(dmx::Attribute &attr);
	};
	struct Element : public std::enable_shared_from_this<Element> {
		std::string type;
		std::string name;
		util::GUID GUID;
		std::unordered_map<std::string, std::shared_ptr<dmx::Attribute>> attributes;
		std::unordered_map<std::string, std::weak_ptr<Element>> nameToChildElement;

		std::string GetGUIDAsString() const;
		std::shared_ptr<Element> Get(const std::string &name) const;
		std::shared_ptr<Attribute> GetAttr(const std::string &name) const;
		void DebugPrint(std::stringstream &ss);
		void DebugPrint(std::stringstream &ss, std::unordered_set<void *> &iteratedObjects, const std::string &t = "");
	};
	class FileData {
	  public:
		static std::shared_ptr<FileData> Load(const std::shared_ptr<ufile::IFile> &f);

		const std::vector<std::shared_ptr<Element>> &GetElements() const;
		const std::shared_ptr<Attribute> &GetRootAttribute() const;
		void DebugPrint(std::stringstream &ss);
	  private:
		FileData() = default;
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
	AttrType get_array_type(AttrType type);

	Time get_time(const std::string &value);
	Time get_time(int32_t value);
	Quat get_quaternion(const std::string &value);
};
