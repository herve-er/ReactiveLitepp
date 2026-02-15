#pragma once
#include "Event.h"
#include "Property.h"
#include <nameof.hpp>

namespace ReactiveLitepp
{
	struct PropertyChangeArgs {
	public:
		PropertyChangeArgs(std::string propertyName) : _propertyName(propertyName) {}
		std::string PropertyName() { return _propertyName; }

	private:
		std::string _propertyName = "";
	};

	struct PropertyChangingArgs : public PropertyChangeArgs {
		PropertyChangingArgs(std::string propertyName) : PropertyChangeArgs(propertyName) {}
	};

	struct PropertyChangedArgs : public PropertyChangeArgs {
		PropertyChangedArgs(std::string propertyName) : PropertyChangeArgs(propertyName) {}
	};

	class ObservableObject {
	public:
		Event<ObservableObject&, PropertyChangingArgs> PropertyChanging;
		Event<ObservableObject&, PropertyChangedArgs> PropertyChanged;

	protected:
		void NotifyPropertyChanging(std::string_view propertyName) {
			PropertyChangingArgs args = PropertyChangingArgs(std::string(propertyName));
			PropertyChanging.Notify(*this, args);
		}

		void NotifyPropertyChanged(std::string_view propertyName) {
			PropertyChangedArgs args = PropertyChangedArgs(std::string(propertyName));
			PropertyChanged.Notify(*this, args);
		}

		template <auto V>
		auto NotifyPropertyChanging() -> std::enable_if_t<std::is_member_pointer_v<decltype(V)>>
		{
			PropertyChangingArgs args = PropertyChangingArgs(std::string(nameof::nameof_member<V>()));
			PropertyChanging.Notify(*this, args);
			return;
		}

		template <auto V>
		auto NotifyPropertyChanged() -> std::enable_if_t<std::is_member_pointer_v<decltype(V)>>
		{
			PropertyChangedArgs args = PropertyChangedArgs(std::string(nameof::nameof_member<V>()));
			PropertyChanged.Notify(*this, args);
			return;
		}

		template<typename T>
		struct member_pointer_traits;

		template<typename Class, typename MemberType>
		struct member_pointer_traits<MemberType Class::*> {
			using type = MemberType;
		};

		template<auto Member>
		using property_value_t =
			typename member_pointer_traits<decltype(Member)>::type::value_type;

		template<auto Member>
		bool SetPropertyValueAndNotify(property_value_t<Member>& field, const property_value_t<Member>& value)
		{
			if (field == value)
				return false;

			NotifyPropertyChanging<Member>();
			field = value;
			NotifyPropertyChanged<Member>();
			return true;
		}
	};
}