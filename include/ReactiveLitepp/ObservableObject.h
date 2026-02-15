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
		void NotifyPropertyChanging(std::string_view propertyName);
		void NotifyPropertyChanged(std::string_view propertyName);

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

		template <auto Member, typename T>
		bool SetPropertyValue(Property<T>& field, const T& value)
		{
			if (field.Get() == value)
				return false;

			NotifyPropertyChanging<Member>();
			field = value;
			NotifyPropertyChanged<Member>();
			return true;
		}
	};
}