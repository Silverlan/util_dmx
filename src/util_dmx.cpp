/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util_dmx.hpp"
#include "dmx_keyvalues2.hpp"
#include "util_dmx_types.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <mathutil/uvec.h>
#include <mathutil/uquat.h>
#include <unordered_set>

#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")

#pragma optimize("",off)
namespace dmx
{
#pragma pack(push,1)
	struct DmeHeader
	{
		int32_t type;
		int32_t name;
		std::array<char,16> GUID;

		std::vector<DmeHeader> subElements;
	};
	struct DmAttribute
	{
		int32_t Name; // string dictionary index
		char AttributeType; // see below
		void *Value; // see below
	};
	struct DmeBody
	{
		int32_t nAttributes;
		//DmeAttribute Attributes[];
	};
	struct BinaryDMX_v5
	{
		std::string header;
		//std::vector<std::string> dictionary;

		DmeHeader rootElement;
		std::vector<DmeBody> bodies;
	};
#pragma pack(pop)

	class StringDictionary
	{
	public:
		StringDictionary(VFilePtr &f,const std::string &encoding,uint32_t encodingVersion)
			: m_file(f)
		{
			if(encoding == "binary")
			{
				m_indexSize = m_lengthSize = sizeof(int32_t);
				if(encodingVersion == 4u)
					m_indexSize = sizeof(int16_t);
				else if(encodingVersion == 2u || encodingVersion == 3u)
					m_lengthSize = m_indexSize = sizeof(int16_t);
				else if(encodingVersion == 1u)
				{
					m_bDummy = true;
					return;
				}
			}
			else if(encoding == "binary_proto")
			{
				m_bDummy = true;
				return;
			}

			auto numStrings = (m_lengthSize == sizeof(int16_t)) ? static_cast<int32_t>(f->Read<int16_t>()) : f->Read<int32_t>();
			m_strings.reserve(numStrings);
			for(auto i=decltype(numStrings){0};i<numStrings;++i)
				m_strings.push_back(f->ReadString());
		}
		std::string ReadString() const {
			if(m_bDummy)
				return GetString();
			auto idx = (m_indexSize == sizeof(int16_t)) ? m_file->Read<int16_t>() : m_file->Read<int32_t>();
			return m_strings.at(idx);
		}
		std::string GetString() const {
			return m_file->ReadString();
		}
	private:
		VFilePtr m_file = nullptr;
		std::vector<std::string> m_strings;
		uint32_t m_indexSize = 0u;
		uint32_t m_lengthSize = 0u;
		bool m_bDummy = false;
	};

	AttrType get_id_type(const std::string &encoding,uint32_t encodingVersion,uint32_t id);
};

std::string dmx::type_to_string(AttrType type)
{
	switch(type)
	{
		case AttrType::None:
			return "None";
		case AttrType::Element:
			return "Element";
		case AttrType::Int:
			return "Int";
		case AttrType::Float:
			return "Float";
		case AttrType::Bool:
			return "Bool";
		case AttrType::String:
			return "String";
		case AttrType::Binary:
			return "Binary";
		case AttrType::Time:
			return "Time";
		case AttrType::ObjectId:
			return "ObjectId";
		case AttrType::Color:
			return "Color";
		case AttrType::Vector2:
			return "Vector2";
		case AttrType::Vector3:
			return "Vector3";
		case AttrType::Vector4:
			return "Vector4";
		case AttrType::Angle:
			return "Angle";
		case AttrType::Quaternion:
			return "Quaternion";
		case AttrType::Matrix:
			return "Matrix";
		case AttrType::UInt64:
			return "UInt64";
		case AttrType::UInt8:
			return "UInt8";
		case AttrType::ElementArray:
			return "ElementArray";
		case AttrType::IntArray:
			return "IntArray";
		case AttrType::FloatArray:
			return "FloatArray";
		case AttrType::BoolArray:
			return "BoolArray";
		case AttrType::StringArray:
			return "StringArray";
		case AttrType::BinaryArray:
			return "BinaryArray";
		case AttrType::TimeArray:
			return "TimeArray";
		case AttrType::ObjectIdArray:
			return "ObjectIdArray";
		case AttrType::ColorArray:
			return "ColorArray";
		case AttrType::Vector2Array:
			return "Vector2Array";
		case AttrType::Vector3Array:
			return "Vector3Array";
		case AttrType::Vector4Array:
			return "Vector4Array";
		case AttrType::AngleArray:
			return "AngleArray";
		case AttrType::QuaternionArray:
			return "QuaternionArray";
		case AttrType::MatrixArray:
			return "MatrixArray";
	}
	return "Invalid";
}
bool dmx::is_single_type(AttrType type) {return type >= AttrType::SingleFirst && type <= AttrType::SingleLast;}
bool dmx::is_array_type(AttrType type) {return type >= AttrType::ArrayFirst && type <= AttrType::ArrayLast;}
dmx::AttrType dmx::get_single_type(AttrType type)
{
	if(type <= AttrType::SingleLast)
		return type;
	switch(type)
	{
		case AttrType::ElementArray:
			return AttrType::Element;
		case AttrType::IntArray:
			return AttrType::Int;
		case AttrType::FloatArray:
			return AttrType::Float;
		case AttrType::BoolArray:
			return AttrType::Bool;
		case AttrType::StringArray:
			return AttrType::String;
		case AttrType::BinaryArray:
			return AttrType::Binary;
		case AttrType::TimeArray:
			return AttrType::Time;
		case AttrType::ObjectIdArray:
			return AttrType::ObjectId;
		case AttrType::ColorArray:
			return AttrType::Color;
		case AttrType::Vector2Array:
			return AttrType::Vector2;
		case AttrType::Vector3Array:
			return AttrType::Vector3;
		case AttrType::Vector4Array:
			return AttrType::Vector4;
		case AttrType::AngleArray:
			return AttrType::Angle;
		case AttrType::QuaternionArray:
			return AttrType::Quaternion;
		case AttrType::MatrixArray:
			return AttrType::Matrix;
	}
	return AttrType::None;
}

static std::vector<dmx::AttrType> s_v1Attributes = {
	dmx::AttrType::None,
	dmx::AttrType::Element,
	dmx::AttrType::Int,
	dmx::AttrType::Float,
	dmx::AttrType::Bool,
	dmx::AttrType::String,
	dmx::AttrType::Binary,
	dmx::AttrType::ObjectId,
	dmx::AttrType::Color,
	dmx::AttrType::Vector2,
	dmx::AttrType::Vector3,
	dmx::AttrType::Vector4,
	dmx::AttrType::Angle,
	dmx::AttrType::Quaternion,
	dmx::AttrType::Matrix,
	dmx::AttrType::ElementArray,
	dmx::AttrType::IntArray,
	dmx::AttrType::FloatArray,
	dmx::AttrType::BoolArray,
	dmx::AttrType::StringArray,
	dmx::AttrType::BinaryArray,
	dmx::AttrType::ObjectIdArray,
	dmx::AttrType::ColorArray,
	dmx::AttrType::Vector2Array,
	dmx::AttrType::Vector3Array,
	dmx::AttrType::Vector4Array,
	dmx::AttrType::AngleArray,
	dmx::AttrType::QuaternionArray,
	dmx::AttrType::MatrixArray
};

static std::vector<dmx::AttrType> s_v2Attributes = {
	dmx::AttrType::None,
	dmx::AttrType::Element,
	dmx::AttrType::Int,
	dmx::AttrType::Float,
	dmx::AttrType::Bool,
	dmx::AttrType::String,
	dmx::AttrType::Binary,
	dmx::AttrType::Time,
	dmx::AttrType::Color,
	dmx::AttrType::Vector2,
	dmx::AttrType::Vector3,
	dmx::AttrType::Vector4,
	dmx::AttrType::Angle,
	dmx::AttrType::Quaternion,
	dmx::AttrType::Matrix,
	dmx::AttrType::ElementArray,
	dmx::AttrType::IntArray,
	dmx::AttrType::FloatArray,
	dmx::AttrType::BoolArray,
	dmx::AttrType::StringArray,
	dmx::AttrType::BinaryArray,
	dmx::AttrType::TimeArray,
	dmx::AttrType::ColorArray,
	dmx::AttrType::Vector2Array,
	dmx::AttrType::Vector3Array,
	dmx::AttrType::Vector4Array,
	dmx::AttrType::AngleArray,
	dmx::AttrType::QuaternionArray,
	dmx::AttrType::MatrixArray
};

static std::vector<dmx::AttrType> s_v3Attributes = {
	dmx::AttrType::None,
	dmx::AttrType::Element,
	dmx::AttrType::Int,
	dmx::AttrType::Float,
	dmx::AttrType::Bool,
	dmx::AttrType::String,
	dmx::AttrType::Binary,
	dmx::AttrType::Time,
	dmx::AttrType::Color,
	dmx::AttrType::Vector2,
	dmx::AttrType::Vector3,
	dmx::AttrType::Vector4,
	dmx::AttrType::Angle,
	dmx::AttrType::Quaternion,
	dmx::AttrType::Matrix,
	dmx::AttrType::UInt64,
	dmx::AttrType::UInt8
};

dmx::AttrType dmx::get_id_type(const std::string &encoding,uint32_t encodingVersion,uint32_t id)
{
	if(encoding != "binary" && encoding != "binary_proto")
		throw std::runtime_error("Unsupported encoding.");
	if(encodingVersion == 1 || encodingVersion == 2)
		return s_v1Attributes.at(id);
	else if(encodingVersion >= 3 && encodingVersion <= 5)
		return s_v2Attributes.at(id);
	else if(encodingVersion == 9)
	{
		if(id >= 32)
			throw std::runtime_error("Unsupported encoding version.");
		return s_v3Attributes.at(id);
	}
	return dmx::AttrType::None;
}

static std::string attr_value_to_string(const void *data,dmx::AttrType type)
{
	if(data == nullptr)
		return "NULL";
	switch(type)
	{
		case dmx::AttrType::None:
			return "NoData";
		case dmx::AttrType::Element:
		{
			auto v = *static_cast<const dmx::ElementRef*>(data);
			return v.expired() ? "expired" : v.lock()->name;
		}
		case dmx::AttrType::Int:
			return std::to_string(*static_cast<const dmx::Int*>(data));
		case dmx::AttrType::Float:
			return std::to_string(*static_cast<const dmx::Float*>(data));
		case dmx::AttrType::Bool:
			return std::to_string(*static_cast<const dmx::Bool*>(data));
		case dmx::AttrType::String:
			return *static_cast<const dmx::String*>(data);
		case dmx::AttrType::Binary:
		{
			auto &v = *static_cast<const dmx::Binary*>(data);
			return util::get_pretty_bytes(v.size());
		}
		case dmx::AttrType::Time:
			return std::to_string(*static_cast<const dmx::Time*>(data));
		case dmx::AttrType::ObjectId:
			return "ObjectId?";
		case dmx::AttrType::Color:
		{
			auto &col = *static_cast<const dmx::Color*>(data);
			return std::to_string(col.at(0)) +' ' +std::to_string(col.at(1)) +' ' +std::to_string(col.at(2)) +' ' +std::to_string(col.at(3));
		}
		case dmx::AttrType::Vector2:
		{
			auto &v = *static_cast<const dmx::Vector2*>(data);
			return std::to_string(v.x) +' ' +std::to_string(v.y);
		}
		case dmx::AttrType::Vector3:
		{
			auto &v = *static_cast<const dmx::Vector3*>(data);
			return std::to_string(v.x) +' ' +std::to_string(v.y) +' ' +std::to_string(v.z);
		}
		case dmx::AttrType::Vector4:
		{
			auto &v = *static_cast<const dmx::Vector4*>(data);
			return std::to_string(v.x) +' ' +std::to_string(v.y) +' ' +std::to_string(v.z) +' ' +std::to_string(v.w);
		}
		case dmx::AttrType::Angle:
		{
			auto &v = *static_cast<const dmx::Angle*>(data);
			return std::to_string(v.p) +' ' +std::to_string(v.y) +' ' +std::to_string(v.r);
		}
		case dmx::AttrType::Quaternion:
		{
			auto &v = *static_cast<const dmx::Quaternion*>(data);
			return std::to_string(v.x) +' ' +std::to_string(v.y) +' ' +std::to_string(v.z) +' ' +std::to_string(v.w);
		}
		case dmx::AttrType::Matrix:
		{
			auto &m = *static_cast<const dmx::Matrix*>(data);
			return std::to_string(m[0][0]) +' ' +std::to_string(m[0][1]) +' ' +std::to_string(m[0][2]) +' ' +std::to_string(m[0][3]) +
				std::to_string(m[1][0]) +' ' +std::to_string(m[1][1]) +' ' +std::to_string(m[1][2]) +' ' +std::to_string(m[1][3]) +
				std::to_string(m[2][0]) +' ' +std::to_string(m[2][1]) +' ' +std::to_string(m[2][2]) +' ' +std::to_string(m[2][3]) +
				std::to_string(m[3][0]) +' ' +std::to_string(m[3][1]) +' ' +std::to_string(m[3][2]) +' ' +std::to_string(m[3][3]);
		}
		case dmx::AttrType::UInt64:
			return std::to_string(*static_cast<const dmx::UInt64*>(data));
		case dmx::AttrType::UInt8:
			return std::to_string(*static_cast<const dmx::UInt8*>(data));
		case dmx::AttrType::Invalid:
			return "Invalid";
		default:
			return "Unknown";
	}
}
/*
template<class TArrayType,class TType>
	static std::string attr_array_to_string(void *data,dmx::AttrType type,dmx::AttrType singleType)
{
	std::string output = "";
	auto first = true;
	uint32_t limit = 4u;
	auto &aData = *static_cast<TArrayType*>(data);
	for(auto &v : aData)
	{
		if(first)
			first = false;
		else
			output += ", ";
		if(limit == 0)
		{
			output += "...";
			break;
		}
		output += attr_value_to_string(&v,singleType);
		--limit;
	}
	return output;
}
*/
std::shared_ptr<dmx::Element> dmx::Attribute::Get(const std::string &name) const
{
	static auto emptyElement = std::make_shared<dmx::Element>();
	if(type != AttrType::ElementArray)
		return emptyElement;
	auto &children = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(data.get());
	for(auto &child : children)
	{
		if(child->type != AttrType::Element)
			continue;
		auto &elRef = *static_cast<const dmx::ElementRef*>(child->data.get());
		if(elRef.expired())
			continue;
		auto el = elRef.lock();
		if(el->name != name)
			continue;
		return el;
	}
	return emptyElement;
}
std::string dmx::Attribute::DataToString() const
{
	if(is_array_type(type))
	{
		auto &attributes = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(data.get());
		std::string output = "";
		auto first = true;
		uint32_t limit = 4u;
		for(auto &attr : attributes)
		{
			if(first)
				first = false;
			else
				output += ", ";
			if(limit == 0)
			{
				output += "...";
				break;
			}
			output += attr->DataToString();
			--limit;
		}
		return output;
	}
	switch(type)
	{
		case AttrType::None:
		case AttrType::Element:
		case AttrType::Int:
		case AttrType::Float:
		case AttrType::Bool:
		case AttrType::String:
		case AttrType::Binary:
		case AttrType::Time:
		case AttrType::ObjectId:
		case AttrType::Color:
		case AttrType::Vector2:
		case AttrType::Vector3:
		case AttrType::Vector4:
		case AttrType::Angle:
		case AttrType::Quaternion:
		case AttrType::Matrix:
		case AttrType::UInt64:
		case AttrType::UInt8:
			return attr_value_to_string(data.get(),type);
		/*case AttrType::ElementArray:
			return attr_array_to_string<ElementRefArray,ElementRef>(data.get(),type,AttrType::Element);
		case AttrType::IntArray:
			return attr_array_to_string<IntArray,Int>(data.get(),type,AttrType::Int);
		case AttrType::FloatArray:
			return attr_array_to_string<FloatArray,Float>(data.get(),type,AttrType::Float);
		case AttrType::BoolArray:
			return attr_array_to_string<BoolArray,Bool>(data.get(),type,AttrType::Bool);
		case AttrType::StringArray:
			return attr_array_to_string<StringArray,String>(data.get(),type,AttrType::String);
		case AttrType::BinaryArray:
			return attr_array_to_string<BinaryArray,Binary>(data.get(),type,AttrType::Binary);
		case AttrType::TimeArray:
			return attr_array_to_string<TimeArray,Time>(data.get(),type,AttrType::Time);
		case AttrType::ObjectIdArray:
			return "ObjectId?"; //return attr_array_to_string<ObjectIdArray,ObjectId>(data.get(),type,AttrType::ObjectId);
		case AttrType::ColorArray:
			return attr_array_to_string<ColorArray,Color>(data.get(),type,AttrType::Color);
		case AttrType::Vector2Array:
			return attr_array_to_string<Vector2Array,Vector2>(data.get(),type,AttrType::Vector2);
		case AttrType::Vector3Array:
			return attr_array_to_string<Vector3Array,Vector3>(data.get(),type,AttrType::Vector3);
		case AttrType::Vector4Array:
			return attr_array_to_string<Vector4Array,Vector4>(data.get(),type,AttrType::Vector4);
		case AttrType::AngleArray:
			return attr_array_to_string<AngleArray,Angle>(data.get(),type,AttrType::Angle);
		case AttrType::QuaternionArray:
			return attr_array_to_string<QuaternionArray,Quaternion>(data.get(),type,AttrType::Quaternion);
		case AttrType::MatrixArray:
			return attr_array_to_string<MatrixArray,Matrix>(data.get(),type,AttrType::Matrix);*/
		case AttrType::Invalid:
			return "Invalid";
		default:
			return "Unknown";
	}
}
dmx::ElementRef *dmx::Attribute::GetElement() {return GetValue<ElementRef>(AttrType::Element);}
dmx::Int *dmx::Attribute::GetInt() {return GetValue<Int>(AttrType::Int);}
dmx::Float *dmx::Attribute::GetFloat() {return GetValue<Float>(AttrType::Float);}
dmx::Bool *dmx::Attribute::GetBoolean() {return GetValue<Bool>(AttrType::Bool);}
dmx::String *dmx::Attribute::GetString() {return GetValue<String>(AttrType::String);}
dmx::Binary *dmx::Attribute::GetBinary() {return GetValue<Binary>(AttrType::Binary);}
dmx::Time *dmx::Attribute::GetTime() {return GetValue<Time>(AttrType::Time);}
dmx::Color *dmx::Attribute::GetColor() {return GetValue<Color>(AttrType::Color);}
dmx::Vector2 *dmx::Attribute::GetVector2() {return GetValue<Vector2>(AttrType::Vector2);}
dmx::Vector3 *dmx::Attribute::GetVector3() {return GetValue<Vector3>(AttrType::Vector3);}
dmx::Vector4 *dmx::Attribute::GetVector4() {return GetValue<Vector4>(AttrType::Vector4);}
dmx::Angle *dmx::Attribute::GetAngle() {return GetValue<Angle>(AttrType::Angle);}
dmx::Quaternion *dmx::Attribute::GetQuaternion() {return GetValue<Quaternion>(AttrType::Quaternion);}
dmx::Matrix *dmx::Attribute::GetMatrix() {return GetValue<Matrix>(AttrType::Matrix);}
dmx::UInt64 *dmx::Attribute::GetUInt64() {return GetValue<UInt64>(AttrType::UInt64);}
dmx::UInt8 *dmx::Attribute::GetUInt8() {return GetValue<UInt8>(AttrType::UInt8);}
std::vector<std::shared_ptr<dmx::Attribute>> *dmx::Attribute::GetArray()
{
	return is_array_type(type) ? GetValue<std::vector<std::shared_ptr<dmx::Attribute>>>(AttrType::ElementArray) : nullptr;
}
std::vector<std::shared_ptr<dmx::Attribute>> *dmx::Attribute::GetArray(AttrType type)
{
	if(this->type != type)
		return nullptr;
	return is_array_type(type) ? GetValue<std::vector<std::shared_ptr<dmx::Attribute>>>(AttrType::ElementArray) : nullptr;
}
void dmx::Attribute::DebugPrint(std::stringstream &ss)
{
	std::unordered_set<void*> iteratedObjects {};
	DebugPrint(ss,iteratedObjects);
}
void dmx::Attribute::DebugPrint(std::stringstream &ss,std::unordered_set<void*> &iteratedObjects,const std::string &t0,const std::string &t)
{
	ss<<t0<<"Attr["<<type_to_string(type)<<"][" +DataToString() +']';
	if(iteratedObjects.find(this) != iteratedObjects.end())
		return;
	iteratedObjects.insert(this);
	if(data == nullptr)
		return;
	if(type == AttrType::ElementArray)
	{
		auto &childElements = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(data.get());
		auto tsub = t +'\t';
		auto tsubEl = tsub +'\t';
		for(auto &elAttr : childElements)
		{
			assert(elAttr->tpe == AttrType::Element);
			if(elAttr->type != AttrType::Element)
				continue; // This should never happen!
			ss<<'\n';
			elAttr->DebugPrint(ss,iteratedObjects,tsub,tsub);
			/*auto &elRef = *static_cast<dmx::ElementRef*>(elAttr->data.get());
			if(elRef.expired())
				continue;
			auto el = elRef.lock();
			ss<<'\n';
			el->DebugPrint(ss,iteratedObjects,tsubEl);*/
		}
		return;
	}
	if(type != AttrType::Element)
		return;
	auto &elRef = *static_cast<const dmx::ElementRef*>(data.get());
	if(elRef.expired())
		return;
	ss<<'\n';
	elRef.lock()->DebugPrint(ss,iteratedObjects,t +'\t');
}

std::string dmx::Element::GetGUIDAsString() const {return util::guid_to_string(GUID);}

std::shared_ptr<dmx::Element> dmx::Element::Get(const std::string &name) const
{
	auto it = nameToChildElement.find(name);
	if(it == nameToChildElement.end() || it->second.expired())
	{
		static auto emptyElement = std::make_shared<dmx::Element>();
		return emptyElement;
	}
	return it->second.lock();
}

std::shared_ptr<dmx::Attribute> dmx::Element::GetAttr(const std::string &name) const
{
	auto it = attributes.find(name);
	if(it == attributes.end())
		return nullptr;
	return it->second;
}

void dmx::Element::DebugPrint(std::stringstream &ss)
{
	std::unordered_set<void*> iteratedObjects {};
	DebugPrint(ss,iteratedObjects);
}
void dmx::Element::DebugPrint(std::stringstream &ss,std::unordered_set<void*> &iteratedObjects,const std::string &t)
{
	ss<<t<<"Element["<<name<<"]["<<type<<"]\n";
	auto first = true;
	auto tsub = t +'\t';
	for(auto &pair : attributes)
	{
		if(first)
			first = false;
		else
			ss<<'\n';
		ss<<t<<"\t["<<pair.first<<"] = ";
		pair.second->DebugPrint(ss,iteratedObjects,"",tsub);
	}
}

std::shared_ptr<dmx::FileData> dmx::FileData::Load(std::shared_ptr<VFilePtrInternal> &f)
{
	auto dmxHeader = dmx::BinaryDMX_v5{};
	const char *headerEnd = "-->";
	uint32_t headerMatch = 0;
	auto c = f->ReadChar();
	while(headerMatch < 3 && f->Eof() == false)
	{
		dmxHeader.header += c;
		if(dmxHeader.header.length() > 1'024) // DMX header should never be this long; Assume that something is wrong
			throw std::runtime_error("DMX header not found!");
		if(c == headerEnd[headerMatch])
			++headerMatch;
		else
			headerMatch = 0;
		c = f->ReadChar();
	}
	if(f->Eof())
		throw std::runtime_error("DMX header not found!");

	std::vector<std::string> headerData;
	ustring::split(dmxHeader.header,headerData);

	if(headerData.size() < 2 || headerData.at(1) != "dmx")
		throw std::runtime_error("Not a valid dmx file!");
	else if(headerData.at(3) == "keyvalues2")
	{
		// Not a DMX binary file, try loading KeyValues2 version
		std::shared_ptr<KeyValues2::Array> dmxRoot = nullptr;
		auto result = KeyValues2::Load(f,dmxRoot);
		if(result == KeyValues2::Result::Success)
		{
			auto result = CreateFromKeyValues2Data(dmxRoot.get());
			result->UpdateRootElement();
			result->UpdateChildElementLookupTables();

			// std::stringstream ss {};
			// result->DebugPrint(ss);
			// std::cout<<ss.str()<<std::endl;

			return result;
		}
		switch(result)
		{
			case KeyValues2::Result::InvalidFormat:
				throw std::runtime_error("Not a valid dmx file!");
				break;
			case KeyValues2::Result::SyntaxError:
				throw std::runtime_error("Unable to load dmx file: Syntax error!");
				break;
			default:
				break;
		}
		return nullptr;
	}
	else if(headerData.at(3) != "binary")
		throw std::runtime_error("Not a valid dmx file!");

	f->Seek(f->Tell() +1); // Skip '\0'-byte
	auto fGetHeaderData = [&headerData](const std::string &id,std::string &val,uint32_t &version) -> bool {
		auto it = std::find(headerData.begin(),headerData.end(),id);
		if((it -headerData.begin()) >= headerData.size() -2)
			return false;
		val = *(it +1);
		version = util::to_int(*(it +2));
		return true;
	};
	std::string encoding {};
	uint32_t encodingVersion = 0;
	std::string format;
	uint32_t formatVersion = 0;
	if(fGetHeaderData("encoding",encoding,encodingVersion) == false || fGetHeaderData("format",format,formatVersion) == false)
		throw std::runtime_error("Invalid dmx header: \"" +dmxHeader.header +"\"!");

	if(encodingVersion >= 9)
	{
		throw std::runtime_error("Unsupported dmx format version " +std::to_string(encodingVersion) +"!");

		// TODO: Read prefix attributes
	}

	dmx::StringDictionary dictionary(f,encoding,encodingVersion);
	auto fd = std::shared_ptr<FileData>(new FileData());

	auto numElements = f->Read<int32_t>();
	fd->m_elements.reserve(numElements *1.05); // Reserve 5% extra for potential missing elements, which will be added to the container dynamically
	std::vector<std::shared_ptr<dmx::Element>> elements {}; // Temporary container which owns all elements; Will be discarded once elements have been assigned to their attributes
	elements.reserve(numElements);
	for(auto i=decltype(numElements){0};i<numElements;++i)
	{
		elements.push_back(std::make_shared<Element>());
		auto &el = elements.back();
		el->type = dictionary.ReadString();
		el->name = (encodingVersion >= 4) ? dictionary.ReadString() : dictionary.GetString();
		el->GUID = f->Read<std::array<uint8_t,16>>();
		fd->m_elements.push_back(el);
	}

	auto fGetValue = [&f,&dictionary,encodingVersion,&fd](std::vector<std::shared_ptr<Element>> &elements,dmx::AttrType type,bool bFromArray=false) -> std::shared_ptr<dmx::Attribute> {
		auto attr = std::make_shared<dmx::Attribute>();
		attr->type = type;
		switch(type)
		{
			case dmx::AttrType::Element:
			{
				auto elIdx = f->Read<int32_t>();
				if(elIdx == -1)
					return attr;
				else if(elIdx == -2)
				{
					if(elements.capacity() == elements.size())
						elements.reserve(elements.size() +100); // Reserve for potential future missing elements
					elements.push_back(std::make_shared<Element>());
					auto &el = elements.back();
					auto id = f->ReadString();
					el->name = "Missing element";
					//el->id = id; // TODO
					attr->data = std::make_shared<ElementRef>(el);
				}
				else
					attr->data = std::make_shared<ElementRef>(elements.at(elIdx));
				break;
			}
			case dmx::AttrType::String:
			{
				attr->data = std::make_shared<String>((encodingVersion < 4 || bFromArray) ? dictionary.GetString() : dictionary.ReadString());
				break;
			}
			case dmx::AttrType::Int:
			{
				attr->data = std::make_shared<Int>(f->Read<Int>());
				break;
			}
			case dmx::AttrType::Float:
			{
				attr->data = std::make_shared<Float>(f->Read<Float>());
				break;
			}
			case dmx::AttrType::Bool:
			{
				attr->data = std::make_shared<Bool>(f->Read<Bool>());
				break;
			}
			case dmx::AttrType::Vector2:
			{
				attr->data = std::make_shared<Vector2>(f->Read<Vector2>());
				break;
			}
			case dmx::AttrType::Vector3:
			{
				attr->data = std::make_shared<Vector3>(f->Read<Vector3>());
				break;
			}
			case dmx::AttrType::Angle:
			{
				auto v = f->Read<Vector3>();
				attr->data = std::make_shared<EulerAngles>(EulerAngles(v.x,v.y,v.z));
				break;
			}
			case dmx::AttrType::Vector4:
			{
				attr->data = std::make_shared<Vector4>(f->Read<Vector4>());
				break;
			}
			case dmx::AttrType::Quaternion:
			{
				attr->data = std::make_shared<Quat>(f->Read<Quat>());
				break;
			}
			case dmx::AttrType::Matrix:
			{
				attr->data = std::make_shared<Mat4>(f->Read<Mat4>());
				break;
			}
			case dmx::AttrType::Color:
			{
				attr->data = std::make_shared<Color>(f->Read<Color>());
				break;
			}
			case dmx::AttrType::Time:
			{
				auto t = f->Read<int32_t>();
				attr->data = std::make_shared<Time>(dmx::get_time(t));
				break;
			}
			case dmx::AttrType::Binary:
			{
				attr->data = std::make_shared<Binary>();
				auto &data = *static_cast<Binary*>(attr->data.get());
				auto len = f->Read<int32_t>();
				data.resize(len);
				f->Read(data.data(),data.size() *sizeof(data.front()));
				break;
			}
			default:
				throw std::logic_error{"Unsupported DMX data type '" +std::to_string(umath::to_integral(type)) +"'"};
		}
		return attr;
	};

	// Note: We have to use numElements instead of fd->m_elements.size(), because the container size
	// can change due to missing elements that are added dynamically
	for(auto i=decltype(numElements){0};i<numElements;++i)
	{
		auto &el = *fd->m_elements.at(i);
		auto numAttributes = f->Read<int32_t>();
		for(auto j=decltype(numAttributes){0};j<numAttributes;++j)
		{
			auto name = dictionary.ReadString();
			auto attrType = dmx::get_id_type(encoding,encodingVersion,f->Read<uint8_t>());
			if(dmx::is_single_type(attrType))
				el.attributes[name] = fGetValue(fd->m_elements,attrType);
			else if(dmx::is_array_type(attrType))
			{
				auto singleType = dmx::get_single_type(attrType);
				auto &attr = el.attributes[name] = std::make_shared<dmx::Attribute>();
				attr->type = attrType;
				attr->data = std::make_shared<std::vector<std::shared_ptr<dmx::Attribute>>>();
				auto &attributes = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(attr->data.get());

				auto len = f->Read<int32_t>();
				attributes.reserve(len);
				for(auto k=decltype(len){0};k<len;++k)
					attributes.push_back(fGetValue(fd->m_elements,singleType,true));
			}
		}
	}

	fd->UpdateRootElement();
	fd->UpdateChildElementLookupTables();
	// std::stringstream ss {};
	// fd->DebugPrint(ss);
	// std::cout<<ss.str()<<std::endl;
	return fd;
}
void dmx::FileData::UpdateChildElementLookupTables()
{
	std::function<void(dmx::Element&)> fIterateChildren = nullptr;
	auto fIterateAttributeChildren = [&fIterateChildren](dmx::Attribute &attr) {
		auto &children = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(attr.data.get());
		for(auto &child : children)
		{
			if(child->data == nullptr)
				continue; // Child points to non-existing element? This shouldn't happen!
			if(child->type != dmx::AttrType::Element)
				throw std::logic_error{"Object of non-Element type is member of Element array!"};
			auto &elRef = *static_cast<const dmx::ElementRef*>(child->data.get());
			if(elRef.expired() == false)
				fIterateChildren(*static_cast<dmx::Element*>(elRef.lock().get()));
		}
	};

	fIterateChildren = [this,&fIterateChildren,&fIterateAttributeChildren](dmx::Element &el) {
		for(auto &pair : el.attributes)
		{
			auto &attr = *pair.second;
			if(attr.data == nullptr)
				continue; // This shouldn't happen?
			switch(attr.type)
			{
				case dmx::AttrType::Element:
				{
					auto &elRef = *static_cast<const dmx::ElementRef*>(attr.data.get());
					if(elRef.expired() == false)
					{
						auto elChild = elRef.lock();
						el.nameToChildElement[elChild->name] = elChild;
					}
					break;
				}
				case dmx::AttrType::ElementArray:
				{
					fIterateAttributeChildren(attr);
					break;
				}
			}
		}
	};
	auto elRoot = std::static_pointer_cast<dmx::ElementRef>(m_rootAttribute->data);
	if(elRoot && elRoot->expired() == false)
		fIterateChildren(*elRoot->lock());
}
void dmx::FileData::UpdateRootElement()
{
	std::unordered_set<dmx::Element*> nestedElements {};
	std::function<void(const dmx::Element&)> fIterateChildren = nullptr;
	fIterateChildren = [&fIterateChildren,&nestedElements](const dmx::Element &el) {
		for(auto &pair : el.attributes)
		{
			auto &attr = *pair.second;
			if(attr.data == nullptr)
				continue; // This shouldn't happen?
			switch(attr.type)
			{
				case dmx::AttrType::Element:
				{
					auto &elRef = *static_cast<const dmx::ElementRef*>(attr.data.get());
					if(elRef.expired() == false)
						nestedElements.insert(static_cast<dmx::Element*>(elRef.lock().get()));
					break;
				}
				case dmx::AttrType::ElementArray:
				{
					auto &children = *static_cast<std::vector<std::shared_ptr<dmx::Attribute>>*>(attr.data.get());
					for(auto &child : children)
					{
						if(child->data == nullptr)
							continue; // Child in array points to non-existing element? This shouldn't happen!
						if(child->type != dmx::AttrType::Element)
							throw std::logic_error{"Object of non-Element type is member of Element array!"};
						auto &elRef = *static_cast<const dmx::ElementRef*>(child->data.get());
						if(elRef.expired() == false)
							fIterateChildren(*static_cast<dmx::Element*>(elRef.lock().get()));
					}
					break;
				}
			}
		}
	};
	for(auto &el : m_elements)
		fIterateChildren(*el);

	auto attr = std::make_shared<Attribute>();
	attr->type = AttrType::Element;
	attr->data = (m_elements.empty() == false) ? std::make_shared<dmx::ElementRef>(m_elements.front()) : nullptr;
	m_rootAttribute = attr;
}
const std::vector<std::shared_ptr<dmx::Element>> &dmx::FileData::GetElements() const {return m_elements;}
const std::shared_ptr<dmx::Attribute> &dmx::FileData::GetRootAttribute() const {return m_rootAttribute;}

dmx::Time dmx::get_time(const std::string &value) {return get_time(util::to_int(value));}
dmx::Time dmx::get_time(int32_t value) {return value /10'000.0;}
Quat dmx::get_quaternion(const std::string &value)
{
	auto rot = uquat::create(value);
	rot = {rot.z,rot.w,rot.x,rot.y};
	return rot;
}

void dmx::FileData::DebugPrint(std::stringstream &ss)
{
	std::unordered_set<void*> iteratedObjects {};
	m_rootAttribute->DebugPrint(ss,iteratedObjects);
}
#pragma optimize("",on)
