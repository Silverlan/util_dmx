// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "dmx_types.hpp"
#include <mathutil/uvec.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <cstring>

module source_engine.dmx;

import :keyvalues2;

class KV2ToDMXConverter {
  public:
	static KV2ToDMXConverter Convert(const source_engine::dmx::KeyValues2::Array &kv2Data);

	const std::vector<std::shared_ptr<source_engine::dmx::Element>> &GetElements() const;
  private:
	bool KV2StringToDMXAttribute(const source_engine::dmx::KeyValues2::StringValue &kvStrValue, const std::string &type, source_engine::dmx::Attribute &outAttribute, const std::string &elementName = "", const std::shared_ptr<source_engine::dmx::Element> &parentElement = nullptr);
	void KV2ElementToDMXAttribute(const source_engine::dmx::KeyValues2::Element &kvEl, const std::string &type, source_engine::dmx::Attribute &outAttribute);
	void KV2ArrayToDMXElement(const source_engine::dmx::KeyValues2::Array &kvEl, const source_engine::dmx::KeyValues2::ElementItem &kvChild, source_engine::dmx::Attribute &outAttribute);
	void InitializeDMXElement(const source_engine::dmx::KeyValues2::Element &kv2El, std::shared_ptr<source_engine::dmx::Element> &inOutElement);

	// Contains all references to elements that need to be updated once all
	// dmx elements and attributes have been created
	std::vector<std::pair<std::shared_ptr<source_engine::dmx::ElementRef>, std::string>> m_refsToUpdate = {};

	std::unordered_map<std::string, source_engine::dmx::ElementRef> m_idToElement = {};
	std::vector<std::shared_ptr<source_engine::dmx::Element>> m_elements = {};
};

const std::vector<std::shared_ptr<source_engine::dmx::Element>> &KV2ToDMXConverter::GetElements() const { return m_elements; }

KV2ToDMXConverter KV2ToDMXConverter::Convert(const source_engine::dmx::KeyValues2::Array &kv2Data)
{
	KV2ToDMXConverter conversionData {};
	for(auto &item : kv2Data.items) {
		auto el = std::make_shared<source_engine::dmx::Element>();
		el->type = item->type.has_value() ? *item->type : "";
		if(item->value->GetType() == source_engine::dmx::KeyValues2::BaseElement::Type::Element)
			conversionData.InitializeDMXElement(static_cast<source_engine::dmx::KeyValues2::Element &>(*item->value), el);
		else
			throw std::invalid_argument {"Object of type 'Element' expected as value for array, got type '" + std::to_string(umath::to_integral(item->value->GetType())) + "'!"};
		conversionData.m_elements.push_back(el);
	}

	for(auto &pair : conversionData.m_refsToUpdate) {
		auto &ref = pair.first;
		auto &elementId = pair.second;
		auto it = conversionData.m_idToElement.find(elementId);
		if(it != conversionData.m_idToElement.end())
			*ref = it->second;
		else
			throw std::invalid_argument {"Element id '" + elementId + "' refers to unknown element!"};
	}
	return conversionData;
}

bool KV2ToDMXConverter::KV2StringToDMXAttribute(const source_engine::dmx::KeyValues2::StringValue &kvStrValue, const std::string &type, source_engine::dmx::Attribute &outAttribute, const std::string &elementName, const std::shared_ptr<source_engine::dmx::Element> &parentElement)
{
	auto &value = kvStrValue.value;
	if(type == "string") {
		if(parentElement) {
			if(elementName == "name") {
				parentElement->name = value;
				return false;
			}
		}
		outAttribute.type = source_engine::dmx::AttrType::String;
		outAttribute.data = std::make_shared<source_engine::dmx::String>(value);
	}
	else if(type == "elementid") {
		if(elementName == "id") {
			if(parentElement)
				m_idToElement.insert(std::make_pair(value, parentElement));
			return false;
		}
		else
			throw std::invalid_argument {"Found item of type 'elementid', but item name is not 'id'!"};
	}
	else if(type == "vector3") {
		outAttribute.type = source_engine::dmx::AttrType::Vector3;
		outAttribute.data = std::make_shared<source_engine::dmx::Vector3>(uvec::create(value));
	}
	else if(type == "quaternion") {
		outAttribute.type = source_engine::dmx::AttrType::Quaternion;
		outAttribute.data = std::make_shared<source_engine::dmx::Quaternion>(source_engine::dmx::get_quaternion(value));
	}
	else if(type == "element") {
		auto ref = std::make_shared<source_engine::dmx::ElementRef>();
		outAttribute.type = source_engine::dmx::AttrType::Element;
		outAttribute.data = ref;
		if(value.empty() == false)
			m_refsToUpdate.push_back({ref, value});
	}
	else if(type == "int") {
		outAttribute.type = source_engine::dmx::AttrType::Int;
		outAttribute.data = std::make_shared<source_engine::dmx::Int>(util::to_int(value));
	}
	else if(type == "float") {
		outAttribute.type = source_engine::dmx::AttrType::Float;
		outAttribute.data = std::make_shared<source_engine::dmx::Float>(util::to_float(value));
	}
	else if(type == "bool") {
		outAttribute.type = source_engine::dmx::AttrType::Bool;
		outAttribute.data = std::make_shared<source_engine::dmx::Bool>(util::to_boolean(value));
	}
	else if(type == "time") {
		outAttribute.type = source_engine::dmx::AttrType::Time;
		outAttribute.data = std::make_shared<source_engine::dmx::Time>(source_engine::dmx::get_time(value));
	}
	else if(type == "color") {
		outAttribute.type = source_engine::dmx::AttrType::Color;
		source_engine::dmx::Color color {};
		ustring::string_to_array<uint8_t, int32_t>(value, color.data(), atoi, color.size());
		outAttribute.data = std::make_shared<source_engine::dmx::Color>(color);
	}
	else if(type == "binary") {
		outAttribute.type = source_engine::dmx::AttrType::Binary;
		auto binary = std::make_shared<source_engine::dmx::Binary>();
		binary->resize(value.size());
		memcpy(binary->data(), value.data(), value.size());
		outAttribute.data = binary;
	}
	else
		throw std::invalid_argument {"DMX type '" + type + "' is currently not supported for KeyValues2 format!"};
	return true;
}

void KV2ToDMXConverter::KV2ArrayToDMXElement(const source_engine::dmx::KeyValues2::Array &kvEl, const source_engine::dmx::KeyValues2::ElementItem &kvChild, source_engine::dmx::Attribute &outAttribute)
{
	auto arrayType = source_engine::dmx::AttrType::Invalid;
	std::string singleType = "";
	if(kvChild.type == "float_array") {
		arrayType = source_engine::dmx::AttrType::FloatArray;
		singleType = "float";
	}
	else if(kvChild.type == "int_array") {
		arrayType = source_engine::dmx::AttrType::IntArray;
		singleType = "int";
	}
	else if(kvChild.type == "string_array") {
		arrayType = source_engine::dmx::AttrType::StringArray;
		singleType = "string";
	}
	else if(kvChild.type == "time_array") {
		arrayType = source_engine::dmx::AttrType::TimeArray;
		singleType = "time";
	}
	else if(kvChild.type == "quaternion_array") {
		arrayType = source_engine::dmx::AttrType::QuaternionArray;
		singleType = "quaternion";
	}
	else if(kvChild.type == "vector3_array") {
		arrayType = source_engine::dmx::AttrType::Vector3Array;
		singleType = "vector3";
	}
	else if(kvChild.type == "element_array") {
		arrayType = source_engine::dmx::AttrType::ElementArray;
		singleType = "element";
	}
	else
		throw std::invalid_argument {"DMX array type '" + kvChild.type + "' is currently not supported for KeyValues2 format!"};
	auto values = std::make_shared<std::vector<std::shared_ptr<source_engine::dmx::Attribute>>>();
	outAttribute.data = values;
	outAttribute.type = arrayType;
	values->reserve(kvEl.items.size());
	for(auto &arrayItem : kvEl.items) {
		switch(arrayItem->value->GetType()) {
		case source_engine::dmx::KeyValues2::BaseElement::Type::String:
			{
				auto &kvStrValue = static_cast<source_engine::dmx::KeyValues2::StringValue &>(*arrayItem->value);
				auto attr = std::make_shared<source_engine::dmx::Attribute>();
				if(KV2StringToDMXAttribute(kvStrValue, singleType, *attr))
					values->push_back(attr);
				break;
			}
		case source_engine::dmx::KeyValues2::BaseElement::Type::Element:
			{
				auto &kvEl = static_cast<source_engine::dmx::KeyValues2::Element &>(*arrayItem->value);
				auto attr = std::make_shared<source_engine::dmx::Attribute>();
				KV2ElementToDMXAttribute(kvEl, singleType, *attr);
				values->push_back(attr);
				break;
			}
		default:
			throw std::invalid_argument {"Unexpected array item type " + std::to_string(umath::to_integral(arrayItem->GetType()))};
		}
	}
}

void KV2ToDMXConverter::KV2ElementToDMXAttribute(const source_engine::dmx::KeyValues2::Element &kvEl, const std::string &type, source_engine::dmx::Attribute &outAttribute)
{
	auto el = std::make_shared<source_engine::dmx::Element>(); // TODO: Cache elements by ID!
	el->type = type;
	InitializeDMXElement(kvEl, el);
	outAttribute.data = std::make_shared<source_engine::dmx::ElementRef>(el);
	outAttribute.type = source_engine::dmx::AttrType::Element;

	m_elements.push_back(el);
}

void KV2ToDMXConverter::InitializeDMXElement(const source_engine::dmx::KeyValues2::Element &kv2El, std::shared_ptr<source_engine::dmx::Element> &inOutElement)
{
	for(auto &pair : kv2El.children) {
		auto &kvChild = pair.second;
		auto attr = std::make_shared<source_engine::dmx::Attribute>();
		auto &kvValue = kvChild->value;
		auto type = kvValue->GetType();
		if(type == source_engine::dmx::KeyValues2::BaseElement::Type::String) {
			auto &kvStrValue = static_cast<source_engine::dmx::KeyValues2::StringValue &>(*kvValue);
			if(KV2StringToDMXAttribute(kvStrValue, kvChild->type, *attr, pair.first, inOutElement))
				inOutElement->attributes.insert(std::make_pair(pair.first, attr));
		}
		else if(type == source_engine::dmx::KeyValues2::BaseElement::Type::Element) {
			auto &kvEl = static_cast<source_engine::dmx::KeyValues2::Element &>(*kvValue);
			KV2ElementToDMXAttribute(kvEl, kvChild->type, *attr);
			inOutElement->attributes.insert(std::make_pair(pair.first, attr));
		}
		else if(type == source_engine::dmx::KeyValues2::BaseElement::Type::Array) {
			auto &kvEl = static_cast<source_engine::dmx::KeyValues2::Array &>(*kvValue);
			KV2ArrayToDMXElement(kvEl, *kvChild, *attr);
			inOutElement->attributes.insert(std::make_pair(pair.first, attr));
		}
		else
			throw std::invalid_argument {"DMX type '" + std::to_string(umath::to_integral(type)) + "' is currently not supported for KeyValues2 format!"};
	}
}

std::shared_ptr<source_engine::dmx::FileData> source_engine::dmx::FileData::CreateFromKeyValues2Data(const void *pKv2Data)
{
	auto data = KV2ToDMXConverter::Convert(*static_cast<const KeyValues2::Array *>(pKv2Data));
	auto fd = std::shared_ptr<FileData>(new FileData());
	fd->m_elements = data.GetElements();
	return fd;
}
#pragma optimize("", on)
