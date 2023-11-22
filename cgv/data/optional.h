#pragma once

#include <map>

namespace cgv {
namespace data {

/// @brief A simple and naiive implementation of an optional value.
/// 
/// This class handles an optional contained value that may or may not be present and can
/// be used as the return type of a function that may fail. This specific implementation is
/// very simple and shall only be used with simple data types. The contained type T must be
/// default constructible and the implementation will always hold a valid instance of the
/// value, hence retrieving the value will always work and never throw an exception. The
/// actually contained value shall be considered valid only if has_value returns true. Due
/// to the fact that objects are always constructed, this implementation will be inefficient
/// for types that have complex constructors.
/// 
/// For more sophisticated implementations of an optional type see std::optional (since C++17)
/// or: https://github.com/akrzemi1/Optional.
/// 
/// @tparam T the value type.
template<typename T>
class optional {
private:
	/// The stored value.
	T _value;
	/// Whether the object holds a valid value.
	bool _has_value = false;

public:
	/// @brief Construct without a contained value.
	optional() {}

	/// @brief Construct with a contained value.
	/// @param v The contained value.
	optional(T v) : _value(v), _has_value(true) {}

	/// @brief Assign a non-optional value of type T.
	/// 
	/// After the assignment, has_value returns true.
	/// 
	/// @param v The value to assign.
	/// @return A reference to this object.
	optional<T>& operator=(const T& v) {
		_value = v;
		_has_value = true;
		return *this;
	}

	/// @brief Compare with another optional of the same type T.
	/// @param rhs The object to compare to.
	/// @return The result of operator== n both contained values, if both optionals have values, false otherwise.
	bool operator==(const optional<T>& rhs) const {
		return _has_value && rhs.has_value() ? (_value == rhs.value()) : false;
	}

	/// @brief Allow conversion to bool.
	explicit operator bool() const {
		return _has_value;
	}

	/// @brief Mark the optional as not containing a valid value.
	///
	/// This will not alter the contained value to prevent the additional overhead of constructing a new instance.
	void reset() {
		_has_value = false;
	}

	/// @brief Test if a value is contained.
	/// @return true if a valid value if contained, false otherwise.
	bool has_value() const {
		return _has_value;
	}
	
	/// @brief Access the contained value.
	/// 
	/// If has_value would return false, the object returned by this method is still valid but the exact value
	/// is undefined.
	/// 
	/// @return The contained value.
	T value() const {
		return _value;
	}
};

} // namespace data
} // namespace cgv
