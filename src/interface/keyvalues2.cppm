// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <sstream>
#include <fsys/filesystem.h>
#include "definitions.hpp"

export module source_engine.dmx:keyvalues2;

export namespace source_engine::dmx {
	class KeyValues2 {
	  public:
		enum class Result { Success, SyntaxError, InvalidFormat };
		struct Array;
		static Result Load(const std::shared_ptr<ufile::IFile> &f, std::shared_ptr<Array> &outArray);
		struct BaseElement {
			enum class Type : uint32_t { Invalid = 0, String, ElementItem, Element, ArrayItem, Array };
			void ToString(std::stringstream &outStream);
			virtual void ToString(std::stringstream &outStream, const std::string &t) = 0;
			virtual Type GetType() const;
		};
		struct StringValue : public BaseElement {
			StringValue(const std::string &value);
			virtual void ToString(std::stringstream &outStream, const std::string &t) override;
			virtual Type GetType() const override;

			std::string value;
		};
		struct ElementItem : public BaseElement {
			virtual void ToString(std::stringstream &outStream, const std::string &t) override;
			virtual Type GetType() const override;

			std::string type;
			std::shared_ptr<BaseElement> value;
		};
		struct Element : public BaseElement {
			virtual void ToString(std::stringstream &outStream, const std::string &t) override;
			virtual Type GetType() const override;

			std::unordered_map<std::string, std::shared_ptr<ElementItem>> children {};
		};
		struct ArrayItem : public BaseElement {
			virtual void ToString(std::stringstream &outStream, const std::string &t) override;
			virtual Type GetType() const override;

			std::optional<std::string> type;
			std::shared_ptr<BaseElement> value;
		};
		struct Array : public BaseElement {
			virtual void ToString(std::stringstream &outStream, const std::string &t) override;
			virtual Type GetType() const override;

			std::vector<std::shared_ptr<ArrayItem>> items {};
		};

		uint32_t GetErrorLine() const;
	  private:
		KeyValues2(const std::shared_ptr<ufile::IFile> &f);
		Result Read(std::shared_ptr<Array> &outArray);
		constexpr bool IsWhitespace(char c) const;
		constexpr bool IsControlCharacter(char c) const;
		char ReadChar();
		std::optional<char> ReadToken(bool includeWhitespace = false);
		std::optional<std::string> ReadString();
		bool ReadUntil(char c);
		bool ReadUntilAfter(char c);

		Result ReadArrayItem(Array &a);
		Result ReadArrayBody(Array &a, bool root = false);
		Result ReadElementItem(Element &e);
		Result ReadElementBody(Element &e);
		std::shared_ptr<ufile::IFile> m_file;
		uint32_t m_curLine = 0;
	};
};
