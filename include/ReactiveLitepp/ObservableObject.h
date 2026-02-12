#pragma once
#include "Event.h"


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
	};
}