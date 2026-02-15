/**
 * @file Property.h
 * @brief Defines the Property template class for reactive property binding
 *
 * This file provides a Property wrapper that allows custom getter/setter logic
 * or automatic value storage with a reactive programming pattern.
 */

#pragma once
#include "Event.h"
#include <optional>

namespace ReactiveLitepp
{
	/**
	 * @brief Function type for property getter operations
	 * @tparam PropType The type of the property value
	 */
	template<typename PropType>
	using Getter = std::function<PropType()>;

	/**
	 * @brief Function type for property setter operations
	 * @tparam PropType The type of the property value
	 */
	template<typename PropType>
	using Setter = std::function<void(PropType&)>;

	/**
	 * @brief A reactive property wrapper that supports custom getter/setter logic
	 *
	 * The Property class provides a flexible way to encapsulate values with custom
	 * get/set behavior. It supports implicit conversion and assignment operators for
	 * natural usage syntax.
	 *
	 * @tparam PropType The type of the property value
	 *
	 * Usage examples:
	 * @code
	 * // Custom getter/setter
	 * Property<int> prop([&](){ return _value; }, [&](int& v){ _value = v; });
	 *
	 *
	 * // Using the property
	 * prop = 42;              // Uses operator=
	 * int x = prop;           // Uses operator PropType()
	 * int y = prop.Get();     // Explicit get
	 * prop.Set(100);          // Explicit set
	 * @endcode
	 */
	template<typename PropType>
	class Property {
	public:
		using value_type = PropType;


		/**
		 * @brief Constructs a Property with custom getter and setter functions
		 * @param get Function to retrieve the property value
		 * @param set Function to update the property value
		 */
		Property(Getter<PropType> get, Setter<PropType> set);

		/**
		 * @brief Retrieves the current property value
		 * @return The current value of the property
		 */
		PropType Get() const;

		/**
		 * @brief Updates the property value
		 * @param value The new value to set
		 */
		void Set(PropType value);

		/**
		 * @brief Implicit conversion operator to the property type
		 *
		 * Allows the Property to be used directly in expressions as if it were
		 * the underlying type.
		 *
		 * @return The current value of the property
		 */
		operator PropType() const;

		/**
		 * @brief Assignment operator for setting the property value
		 *
		 * Provides natural assignment syntax for updating the property.
		 *
		 * @param value The new value to assign
		 * @return Reference to this Property
		 */
		Property& operator=(const PropType& value);

		/**
		 * @brief Equality operator for comparing the property value
		 *
		 * Allows natural comparison syntax to check whether the property
		 * currently holds the same value as the provided one.
		 *
		 * @param value The value to compare against the property.
		 * @return true if the property value is equal to @p value,
		 *         false otherwise.
		 */
		bool operator==(const PropType& value) const;

		/**
		 * @brief Stream output operator for Property
		 *
		 * Allows the property value to be printed via std::cout or similar.
		 *
		 * @param os Output stream
		 * @param prop The Property to output
		 * @return Reference to the output stream
		 */
		friend std::ostream& operator<<(std::ostream& os, const Property& prop) {
			os << prop.Get();
			return os;
		}

	private:
		Getter<PropType> _get;              ///< The getter function
		Setter<PropType> _set;              ///< The setter function
	};

	// ==========================================
	// Implementation
	// ==========================================

	template<typename PropType>
	Property<PropType>::Property(Getter<PropType> get, Setter<PropType> set)
		: _get(std::move(get)), _set(std::move(set))
	{
	}

	template<typename PropType>
	PropType Property<PropType>::Get() const
	{
		if (_get) return _get();
		else throw;
	}

	template<typename PropType>
	void Property<PropType>::Set(PropType value)
	{
		if (_set) _set(value);
		else throw;
	}

	template<typename PropType>
	Property<PropType>::operator PropType() const
	{
		return Get();
	}

	template<typename PropType>
	Property<PropType>& Property<PropType>::operator=(const PropType& value)
	{
		Set(value);
		return *this;
	}

	template<typename PropType>
	inline bool Property<PropType>::operator==(const PropType& value) const
	{
		return Get() == value;
	}
}