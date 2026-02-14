#include <ReactiveLitepp/ObservableObject.h>

namespace ReactiveLitepp
{
	void ObservableObject::NotifyPropertyChanging(std::string_view propName)
	{
		PropertyChangingArgs args = PropertyChangingArgs(std::string(propName));
		PropertyChanging.Notify(*this, args);
	}

	void ObservableObject::NotifyPropertyChanged(std::string_view propName)
	{
		PropertyChangedArgs args = PropertyChangedArgs(std::string(propName));
		PropertyChanged.Notify(*this, args);
	}
}
