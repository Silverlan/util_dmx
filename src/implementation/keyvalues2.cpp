// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <array>
#include <sharedutils/util_ifile.hpp>

module source_engine.dmx;

import :keyvalues2;

using namespace source_engine::dmx;

void KeyValues2::BaseElement::ToString(std::stringstream &outStream) { ToString(outStream, ""); }
KeyValues2::BaseElement::Type KeyValues2::BaseElement::GetType() const { return Type::Invalid; }

KeyValues2::StringValue::StringValue(const std::string &value) : value {value} {}
void KeyValues2::StringValue::ToString(std::stringstream &outStream, const std::string &t) { outStream << t << value << "\n"; }
KeyValues2::BaseElement::Type KeyValues2::StringValue::GetType() const { return Type::String; }

void KeyValues2::ElementItem::ToString(std::stringstream &outStream, const std::string &t)
{
	outStream << t << "ElementItem[" << type << "]\n";
	value->ToString(outStream, t + '\t');
}
KeyValues2::BaseElement::Type KeyValues2::ElementItem::GetType() const { return Type::ElementItem; }

void KeyValues2::Element::ToString(std::stringstream &outStream, const std::string &t)
{
	outStream << t << "Element\n";
	auto tSub = t + '\t';
	for(auto &pair : children)
		pair.second->ToString(outStream, tSub);
}
KeyValues2::BaseElement::Type KeyValues2::Element::GetType() const { return Type::Element; }

void KeyValues2::ArrayItem::ToString(std::stringstream &outStream, const std::string &t)
{
	outStream << t << "ArrayItem[" << (type.has_value() ? *type : "NoType") << "]\n";
	value->ToString(outStream, t + '\t');
}
KeyValues2::BaseElement::Type KeyValues2::ArrayItem::GetType() const { return Type::ArrayItem; }
void KeyValues2::Array::ToString(std::stringstream &outStream, const std::string &t)
{
	outStream << t << "Array\n";
	auto tSub = t + '\t';
	for(auto &item : items)
		item->ToString(outStream, tSub);
}
KeyValues2::BaseElement::Type KeyValues2::Array::GetType() const { return Type::Array; }

KeyValues2::Result KeyValues2::Load(const std::shared_ptr<ufile::IFile> &f, std::shared_ptr<Array> &outArray)
{
	KeyValues2 dmxKv2 {f};
	return dmxKv2.Read(outArray);
}
KeyValues2::KeyValues2(const std::shared_ptr<ufile::IFile> &f) : m_file {f} {}
KeyValues2::Result KeyValues2::Read(std::shared_ptr<Array> &outArray)
{
	/*constexpr auto *identifier = "<!-- dmx encoding keyvalues2 1 format tex 1 -->";
	std::array<char,std::char_traits<char>::length(identifier)> fileIdentifier {};
	m_file->Read(fileIdentifier.data(),fileIdentifier.size() +sizeof(fileIdentifier.front()));
	if(strncmp(identifier,fileIdentifier.data(),fileIdentifier.size()) != 0)
		return Result::InvalidFormat;*/
	outArray = std::make_shared<Array>();
	return ReadArrayBody(*outArray, true);
}

uint32_t KeyValues2::GetErrorLine() const { return m_curLine; }

constexpr bool KeyValues2::IsWhitespace(char c) const { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
constexpr bool KeyValues2::IsControlCharacter(char c) const { return c == '{' || c == '}' || c == '[' || c == ']'; }
char KeyValues2::ReadChar()
{
	auto c = m_file->ReadChar();
	if(c == '\n')
		++m_curLine;
	return c;
}
std::optional<char> KeyValues2::ReadToken(bool includeWhitespace)
{
	if(m_file->Eof())
		return {};
	auto c = ReadChar();
	while((includeWhitespace == false && IsWhitespace(c)) || c == '\0') {
		c = ReadChar();
		if(m_file->Eof() || c == '\0')
			return {};
	}
	return c;
}
std::optional<std::string> KeyValues2::ReadString()
{
	std::string str = "";
	auto token = ReadToken();
	if(token.has_value() == false)
		return {};
	auto inQuotes = token == '\"';
	if(inQuotes)
		token = ReadToken(true);
	for(;;) {
		if(token.has_value() == false)
			return {};
		if(inQuotes == false && IsWhitespace(*token))
			return str; // Reached end of string
		switch(*token) {
		case '"':
			{
				if(inQuotes)
					return str;
			}
		default:
			str += *token;
			break;
		}
		token = ReadToken(true);
	}
	// unreachable
	return {};
}
bool KeyValues2::ReadUntil(char c)
{
	auto token = ReadToken();
	while(token != c) {
		if(token == '\0' || m_file->Eof())
			return false;
		token = ReadToken();
	}
	return true;
}
bool KeyValues2::ReadUntilAfter(char c)
{
	if(ReadUntil(c) == false)
		return false;
	m_file->Seek(m_file->Tell() + 1);
	return m_file->Eof() == false;
}

KeyValues2::Result KeyValues2::ReadArrayItem(Array &a)
{
	auto type = ReadString();
	auto token = ReadToken();
	if(type.has_value() == false || token.has_value() == false)
		return Result::SyntaxError;
	auto item = std::make_shared<ArrayItem>();
	if(token == ',' || token == ']') {
		// Item has no type
		item->value = std::make_shared<StringValue>(*type);
		a.items.push_back(item);
		if(token == ']')
			m_file->Seek(m_file->Tell() - 1);
		return Result::Success;
	}
	item->type = *type;
	if(IsControlCharacter(*token)) {
		// Value is either an element or an array
		switch(*token) {
		case '{':
			{
				auto eChild = std::make_shared<Element>();
				item->value = eChild;
				auto result = ReadElementBody(*eChild);
				if(result != Result::Success)
					return result;
				a.items.push_back(item);
				return result;
			}
		case '[':
			{
				auto aChild = std::make_shared<Array>();
				item->value = aChild;
				auto result = ReadArrayBody(*aChild);
				if(result != Result::Success)
					return result;
				a.items.push_back(item);
				return result;
			}
		}
		return Result::SyntaxError;
	}
	// Value is a string
	m_file->Seek(m_file->Tell() - 1);
	auto value = ReadString();
	if(value.has_value() == false)
		return Result::SyntaxError;
	item->value = std::make_shared<StringValue>(*value);
	a.items.push_back(item);
	return Result::Success;
}

KeyValues2::Result KeyValues2::ReadArrayBody(Array &a, bool root)
{
	// Each item in the array has the following structure:
	// [type] <value>
	// Where value can be either a string, an element, or an array(?).
	// The type is OPTIONAL
	auto token = ReadToken();
	while(token.has_value() && *token != ']') {
		m_file->Seek(m_file->Tell() - 1);
		auto result = ReadArrayItem(a);
		if(result != Result::Success)
			return result;
		token = ReadToken();
		while(token.has_value() && *token == ',')
			token = ReadToken();
	}
	if(token.has_value() == false)
		return root ? Result::Success : Result::SyntaxError;
	return Result::Success;
}

KeyValues2::Result KeyValues2::ReadElementItem(Element &e)
{
	auto name = ReadString();
	auto type = ReadString();
	auto token = ReadToken();
	if(name.has_value() == false || type.has_value() == false || token.has_value() == false)
		return Result::SyntaxError;
	auto item = std::make_shared<ElementItem>();
	item->type = *type;
	if(IsControlCharacter(*token)) {
		// Value is either an element or an array
		switch(*token) {
		case '{':
			{
				auto eChild = std::make_shared<Element>();
				item->value = eChild;
				auto result = ReadElementBody(*eChild);
				if(result != Result::Success)
					return result;
				e.children[*name] = item;
				return result;
			}
		case '[':
			{
				auto aChild = std::make_shared<Array>();
				item->value = aChild;
				auto result = ReadArrayBody(*aChild);
				if(result != Result::Success)
					return result;
				e.children[*name] = item;
				return result;
			}
		}
		return Result::SyntaxError;
	}
	// Value is a string
	m_file->Seek(m_file->Tell() - 1);
	auto value = ReadString();
	if(value.has_value() == false)
		return Result::SyntaxError;
	item->value = std::make_shared<StringValue>(*value);
	e.children[*name] = item;
	return Result::Success;
}

KeyValues2::Result KeyValues2::ReadElementBody(Element &e)
{
	// Each item in the element has the following structure:
	// <name> <type> <value>
	// Where value can be either a string, an element, or an array
	auto token = ReadToken();
	while(token.has_value() && *token != '}') {
		m_file->Seek(m_file->Tell() - 1);
		auto result = ReadElementItem(e);
		if(result != Result::Success)
			return result;
		token = ReadToken();
	}
	if(token.has_value() == false)
		return Result::SyntaxError;
	return Result::Success;
}
