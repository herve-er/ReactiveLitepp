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
	 * @brief Function type for property getter operations
	 * @tparam PropType The type of the property value
	 */
	template<typename PropType>
	using AutoGetter = std::function<PropType(PropType& /*internal value*/)>;

	/**
	 * @brief Function type for property setter operations
	 * @tparam PropType The type of the property value
	 */
	template<typename PropType>
	using AutoSetter = std::function<void(PropType& /*new value*/, PropType& /*internal value*/)>;

	/**
	 * @brief A reactive property wrapper that supports custom getter/setter logic
	 *
	 * The Property class provides a flexible way to encapsulate values with custom
	 * get/set behavior, or to store values internally with automatic getter/setter
	 * generation. It supports implicit conversion and assignment operators for
	 * natural usage syntax.
	 *
	 * @tparam PropType The type of the property value
	 * @tparam ConstructorArgs Variadic template parameters for constructing the internal value
	 *
	 * Usage examples:
	 * @code
	 * // Custom getter/setter
	 * Property<int> prop([&](){ return _value; }, [&](int& v){ _value = v; });
	 *
	 * // Internal value storage with custom constructor args
	 * Property<std::string> name("DefaultName");
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
		/**
		 * @brief Constructs a Property with custom getter and setter functions
		 * @param get Function to retrieve the property value
		 * @param set Function to update the property value
		 */
		Property(Getter<PropType> get, Setter<PropType> set);

		/**
		 * @brief Constructs a Property with custom getter and setter functions
		 * @param get Function to retrieve the property value
		 * @param set Function to update the property value
		 */
		Property(AutoGetter<PropType> get, AutoSetter<PropType> set);

		/**
		 * @brief Constructs a Property with internal value storage
		 *
		 * Creates an internal value initialized with the provided constructor arguments.
		 * Automatically generates getter and setter functions that work with the internal value.
		 *
		 * @param args Constructor arguments forwarded to PropType's constructor
		 */
		Property(const PropType& initialValue);

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

	private:
		Getter<PropType> _get;              ///< The getter function
		Setter<PropType> _set;              ///< The setter function
		AutoGetter<PropType> _autoGet;
		AutoSetter<PropType> _autoSet;
		std::unique_ptr<PropType> _value;   ///< Internal value storage (when not using custom getter/setter)
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
	Property<PropType>::Property(AutoGetter<PropType> get, AutoSetter<PropType> set)
		: _autoGet(std::move(get)), _autoSet(std::move(set))
	{
		_value = std::make_unique<PropType>();
	}

	template<typename PropType>
	inline Property<PropType>::Property(const PropType& initialValue)
	{
		_value = std::make_unique<PropType>(initialValue);
		_get = [this]() { return *_value; };
		_set = [this](PropType& val) { *_value = val; };
	}

	template<typename PropType>
	PropType Property<PropType>::Get() const
	{
		if (_get) return _get();
		if (_autoGet) return _autoGet(*_value);
		throw;
	}

	template<typename PropType>
	void Property<PropType>::Set(PropType value)
	{
		if (_set) _set(value);
		else if (_autoGet) _autoSet(value, *_value);
		throw;
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
}