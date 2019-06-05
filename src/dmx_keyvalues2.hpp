/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __DMX_KEYVALUES2_HPP__
#define __DMX_KEYVALUES2_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <sstream>
#include <fsys/filesystem.h>

namespace dmx
{
	class KeyValues2
	{
	public:
		enum class Result
		{
			Success,
			SyntaxError,
			InvalidFormat
		};
		struct Array;
		static Result Load(const VFilePtr &f,std::shared_ptr<Array> &outArray);
		struct BaseElement
		{
			enum class Type : uint32_t
			{
				Invalid = 0,
				String,
				ElementItem,
				Element,
				ArrayItem,
				Array
			};
			void ToString(std::stringstream &outStream);
			virtual void ToString(std::stringstream &outStream,const std::string &t)=0;
			virtual Type GetType() const;
		};
		struct StringValue
			: public BaseElement
		{
			StringValue(const std::string &value);
			virtual void ToString(std::stringstream &outStream,const std::string &t) override;
			virtual Type GetType() const override;

			std::string value;
		};
		struct ElementItem
			: public BaseElement
		{
			virtual void ToString(std::stringstream &outStream,const std::string &t) override;
			virtual Type GetType() const override;

			std::string type;
			std::shared_ptr<BaseElement> value;
		};
		struct Element
			: public BaseElement
		{
			virtual void ToString(std::stringstream &outStream,const std::string &t) override;
			virtual Type GetType() const override;

			std::unordered_map<std::string,std::shared_ptr<ElementItem>> children {};
		};
		struct ArrayItem
			: public BaseElement
		{
			virtual void ToString(std::stringstream &outStream,const std::string &t) override;
			virtual Type GetType() const override;

			std::optional<std::string> type;
			std::shared_ptr<BaseElement> value;
		};
		struct Array
			: public BaseElement
		{
			virtual void ToString(std::stringstream &outStream,const std::string &t) override;
			virtual Type GetType() const override;

			std::vector<std::shared_ptr<ArrayItem>> items {};
		};

		uint32_t GetErrorLine() const;
	private:
		KeyValues2(const VFilePtr &f);
		Result Read(std::shared_ptr<Array> &outArray);
		constexpr bool IsWhitespace(char c) const;
		constexpr bool IsControlCharacter(char c) const;
		char ReadChar();
		std::optional<char> ReadToken(bool includeWhitespace=false);
		std::optional<std::string> ReadString();
		bool ReadUntil(char c);
		bool ReadUntilAfter(char c);

		Result ReadArrayItem(Array &a);
		Result ReadArrayBody(Array &a,bool root=false);
		Result ReadElementItem(Element &e);
		Result ReadElementBody(Element &e);
		VFilePtr m_file;
		uint32_t m_curLine = 0;
	};
};

#endif
