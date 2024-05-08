#pragma once

#include <string>

namespace cgv {
namespace gui {

/// @brief A convenience class for compiling strings of delimited key-value pairs useful for defining GUI control options.
/// 
/// Both the key-value and the pair delimiters can be set separately. Instances of this class are implicitly convertible
/// to string.
///
/// Usage:
///		cgv::gui::property_string options; // using default delimiters
///		options.add("width", 100);
///		options.add("active", "true");
///		options.add_bracketed("label", "Example");
///
///		// use as string with implicit conversion
///		std::string str = options;
///	
/// Results in:	
///		"width=100;active=true;label='Example'"
class property_string {
private:
	/// The value (content) of this property_string
	std::string v_;
	/// The delimiter used between key and value
	std::string key_value_delimiter_;
	/// The delimiter used between key-value pairs (properties)
	std::string property_delimiter_;
	
	/// @brief Convert value type to std::string using std::to_string.
	/// 
	/// @tparam T The value type.
	/// @param value Teh value to convert.
	/// @return The resulting string.
	template<typename T>
	std::string convert_to_string(T value) const {
		return std::to_string(value);
	}

	/// @brief Convert value type to std::string using std::to_string.
	/// 
	/// @tparam T The value type.
	/// @param value Teh value to convert.
	/// @return The resulting string.
	std::string convert_to_string(bool value) const {
		return value ? "true" : "false";
	}

	/// @brief Convert value type to std::string using std::to_string.
	/// 
	/// @tparam T The value type.
	/// @param value Teh value to convert.
	/// @return The resulting string.
	std::string convert_to_string(const std::string& value) const {
		return value;
	}

	/// @brief Add a key-value pair to the end of the content
	/// 
	/// If key and value are empty this method has no effect. If the content is not
	/// empty a delimiter will be appended before the new key-value pair. If either key
	/// or value is empty only the non-empty argument will be added without the key-value
	/// delimiter. Otherwise, the pair is added with the key-value delimiter between key and
	/// value.
	/// 
	/// @param key The key string.
	/// @param value The value string.
	void add_void(const std::string& key, const std::string& value) {
		if(key.empty() && value.empty())
			return;

		if(!v_.empty())
			v_ += property_delimiter_;

		if(!key.empty() && !value.empty())
			v_ += key + key_value_delimiter_ + value;
		else
			v_ += key + value;
	}

public:
	/// @brief Construct a new empty property string using the given delimiters
	/// 
	/// @param key_value_delimiter The delimiter used between key and value.
	/// @param property_delimiter The delimiter used between key-value pairs (properties).
	property_string(const std::string& key_value_delimiter = "=", const std::string& property_delimiter = ";") :
		key_value_delimiter_(key_value_delimiter),
		property_delimiter_(property_delimiter) {}

	/// @brief Check if empty
	/// @return True if empty, false otherwise.
	bool empty() const {
		return v_.empty();
	}

	/// @brief Clear content.
	void clear() {
		v_.clear();
	}

	/// @brief Add key-value pair to end of content, converting value to string
	/// 
	/// @tparam T The value type.
	/// @param key The key string.
	/// @param value The value.
	template<typename T>
	void add(const std::string& key, T value) {
		add_void(key, convert_to_string(value));
	}

	/// @brief See add. Enclose value in given bracket.
	/// 
	/// @tparam T The value type.
	/// @param key The key string.
	/// @param value The value.
	/// @param bracket The bracket character used to enclose the value.
	template<typename T>
	void add_bracketed(const std::string& key, const T value, char bracket = '\'') {
		std::string str = bracket + convert_to_string(value) + bracket;
		add_void(key, str);
	}

	/// @brief See add. Enclose value in given bracket.
	/// 
	/// @tparam T The value type.
	/// @param key The key string.
	/// @param value The value.
	/// @param bracket The bracket character used to enclose the value.
	template<typename T>
	void add_bracketed(const std::string& key, const T value, char opening_bracket, char closing_bracket) {
		std::string str = opening_bracket + convert_to_string(value) + closing_bracket;
		add_void(key, str);
	}

	/// Implicit conversion to std::string
	operator std::string() const {
		return v_;
	}
};

}
}
