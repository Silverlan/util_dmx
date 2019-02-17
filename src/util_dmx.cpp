/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util_dmx.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <mathutil/uvec.h>
#include <mathutil/uquat.h>

#pragma comment(lib,"util.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")

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
					m_lengthSize = m_lengthSize = sizeof(int16_t);
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
		throw std::exception("");
	if(encodingVersion == 1 || encodingVersion == 2)
		return s_v1Attributes.at(id);
	else if(encodingVersion >= 3 && encodingVersion <= 5)
		return s_v2Attributes.at(id);
	else if(encodingVersion == 9)
	{
		if(id >= 32)
			throw std::exception("");
		return s_v3Attributes.at(id);
	}
	return dmx::AttrType::None;
}

std::shared_ptr<dmx::FileData> dmx::FileData::Load(std::shared_ptr<VFilePtrInternal> &f)
{
	auto dmxHeader = dmx::BinaryDMX_v5{};
	dmxHeader.header = f->ReadString();

	std::vector<std::string> headerData;
	ustring::split(dmxHeader.header,headerData);

	if(headerData.size() < 2 || headerData.at(1) != "dmx")
		throw std::runtime_error("Not a valid dmx file!");

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
	fd->m_elements.reserve(numElements);
	for(auto i=decltype(numElements){0};i<numElements;++i)
	{
		fd->m_elements.push_back(std::make_shared<Element>());
		auto &el = fd->m_elements.back();
		el->type = dictionary.ReadString();
		el->name = (encodingVersion >= 4) ? dictionary.ReadString() : dictionary.GetString();
		el->GUID = f->Read<std::array<char,16>>();
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
					elements.push_back(std::make_shared<Element>());
					auto &el = elements.back();
					auto id = f->ReadString();
					el->name = "Missing element";
					//el->id = id; // TODO
					attr->data = std::make_shared<std::weak_ptr<Element>>(el);
				}
				else
					attr->data = std::make_shared<std::weak_ptr<Element>>(elements.at(elIdx));
				break;
			}
			case dmx::AttrType::String:
			{
				attr->data = std::make_shared<std::string>((encodingVersion < 4 || bFromArray) ? dictionary.GetString() : dictionary.ReadString());
				break;
			}
			case dmx::AttrType::Int:
			{
				attr->data = std::make_shared<int32_t>(f->Read<int32_t>());
				break;
			}
			case dmx::AttrType::Float:
			{
				attr->data = std::make_shared<float>(f->Read<float>());
				break;
			}
			case dmx::AttrType::Bool:
			{
				attr->data = std::make_shared<bool>(f->Read<bool>());
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
				attr->data = std::make_shared<std::array<uint8_t,4>>(f->Read<std::array<uint8_t,4>>());
				break;
			}
			case dmx::AttrType::Time:
			{
				auto t = f->Read<int32_t>();
				attr->data = std::make_shared<float>(t /10'000.0);
				break;
			}
			case dmx::AttrType::Binary:
			{
				attr->data = std::make_shared<std::vector<uint8_t>>();
				auto &data = *static_cast<std::vector<uint8_t>*>(attr->data.get());
				auto len = f->Read<int32_t>();
				data.resize(len);
				f->Read(data.data(),data.size() *sizeof(data.front()));
				break;
			}
		}
		return attr;
	};

	for(auto i=decltype(fd->m_elements.size()){0};i<fd->m_elements.size();++i)
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
	return fd;
}

const std::vector<std::shared_ptr<dmx::Element>> &dmx::FileData::GetElements() const {return m_elements;}
