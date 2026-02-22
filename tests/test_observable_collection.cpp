#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/ObservableCollection.h>
#include <type_traits>

using namespace ReactiveLitepp;

TEST_CASE("ObservableCollection Count reflects size", "[observable_collection][count]") {
	ObservableCollection<int> coll;
	REQUIRE(coll.size() == 0u);
	REQUIRE(static_cast<std::size_t>(coll.Count) == 0u);

	coll.push_back(1);
	REQUIRE(coll.size() == 1u);
	REQUIRE(static_cast<std::size_t>(coll.Count) == 1u);

	coll.push_back(2);
	REQUIRE(coll.size() == 2u);
	REQUIRE(static_cast<std::size_t>(coll.Count) == 2u);

	coll.clear();
	REQUIRE(coll.size() == 0u);
	REQUIRE(static_cast<std::size_t>(coll.Count) == 0u);
}

TEST_CASE("ObservableCollection push_back raises events with correct args", "[observable_collection][push_back]") {
	ObservableCollection<int> coll;

	ObservableCollection<int>::CollectionChangingArgs lastChanging{};
	ObservableCollection<int>::CollectionChangedArgs lastChanged{};
	int changingCalls = 0;
	int changedCalls = 0;

	auto subChanging = coll.CollectionChanging.Subscribe(
		[&](auto&, const auto& args)
		{
			changingCalls++;
			lastChanging = args;
		});

	auto subChanged = coll.CollectionChanged.Subscribe(
		[&](auto&, const auto& args)
		{
			changedCalls++;
			lastChanged = args;
		});

	coll.push_back(42);

	REQUIRE(changingCalls == 1);
	REQUIRE(changedCalls == 1);
	REQUIRE(lastChanging.Change == ObservableCollection<int>::ChangeType::Add);
	REQUIRE(lastChanged.Change == ObservableCollection<int>::ChangeType::Add);
	REQUIRE(lastChanging.Index == 0u);
	REQUIRE(lastChanged.Index == 0u);
	REQUIRE(lastChanging.OldCount == 0u);
	REQUIRE(lastChanging.NewCount == 1u);
	REQUIRE(lastChanged.OldCount == 0u);
	REQUIRE(lastChanged.NewCount == 1u);
	REQUIRE(coll[0] == 42);
}

TEST_CASE("ObservableCollection clear raises events with correct args", "[observable_collection][clear]") {
	ObservableCollection<int> coll;
	coll.push_back(1);
	coll.push_back(2);

	ObservableCollection<int>::CollectionChangingArgs lastChanging{};
	ObservableCollection<int>::CollectionChangedArgs lastChanged{};

	auto subChanging = coll.CollectionChanging.Subscribe(
		[&](auto&, const auto& args) { lastChanging = args; });
	auto subChanged = coll.CollectionChanged.Subscribe(
		[&](auto&, const auto& args) { lastChanged = args; });

	coll.clear();

	REQUIRE(lastChanging.Change == ObservableCollection<int>::ChangeType::Clear);
	REQUIRE(lastChanged.Change == ObservableCollection<int>::ChangeType::Clear);
	REQUIRE(lastChanging.OldCount == 2u);
	REQUIRE(lastChanging.NewCount == 0u);
	REQUIRE(lastChanged.OldCount == 2u);
	REQUIRE(lastChanged.NewCount == 0u);
}

TEST_CASE("ObservableCollection insert and erase raise events with correct args", "[observable_collection][insert_erase]") {
	ObservableCollection<int> coll;
	coll.push_back(1); // index 0
	coll.push_back(3); // index 1

	ObservableCollection<int>::CollectionChangingArgs lastChanging{};
	ObservableCollection<int>::CollectionChangedArgs lastChanged{};

	auto subChanging = coll.CollectionChanging.Subscribe(
		[&](auto&, const auto& args) { lastChanging = args; });
	auto subChanged = coll.CollectionChanged.Subscribe(
		[&](auto&, const auto& args) { lastChanged = args; });

	// Insert in the middle
	auto it = coll.insert(coll.begin() + 1, 2);

	REQUIRE(*it == 2);
	REQUIRE(coll.size() == 3u);
	REQUIRE(coll[0] == 1);
	REQUIRE(coll[1] == 2);
	REQUIRE(coll[2] == 3);
	REQUIRE(lastChanging.Change == ObservableCollection<int>::ChangeType::Insert);
	REQUIRE(lastChanged.Change == ObservableCollection<int>::ChangeType::Insert);
	REQUIRE(lastChanging.Index == 1u);
	REQUIRE(lastChanged.Index == 1u);
	REQUIRE(lastChanging.OldCount == 2u);
	REQUIRE(lastChanging.NewCount == 3u);
	REQUIRE(lastChanged.OldCount == 2u);
	REQUIRE(lastChanged.NewCount == 3u);

	// Erase the inserted element
	it = coll.erase(coll.begin() + 1);
	REQUIRE(*it == 3);
	REQUIRE(coll.size() == 2u);
	REQUIRE(coll[0] == 1);
	REQUIRE(coll[1] == 3);
	REQUIRE(lastChanging.Change == ObservableCollection<int>::ChangeType::Remove);
	REQUIRE(lastChanged.Change == ObservableCollection<int>::ChangeType::Remove);
	REQUIRE(lastChanging.Index == 1u);
	REQUIRE(lastChanged.Index == 1u);
	REQUIRE(lastChanging.OldCount == 3u);
	REQUIRE(lastChanging.NewCount == 2u);
	REQUIRE(lastChanged.OldCount == 3u);
	REQUIRE(lastChanged.NewCount == 2u);
}

TEST_CASE("ObservableCollection emplace_back and emplace behave like vector", "[observable_collection][emplace]") {
	ObservableCollection<std::pair<int, std::string>> coll;

	auto& ref = coll.emplace_back(1, "one");
	REQUIRE(coll.size() == 1u);
	REQUIRE(ref.first == 1);
	REQUIRE(ref.second == "one");

	coll.emplace(coll.begin(), 0, "zero");
	REQUIRE(coll.size() == 2u);
	REQUIRE(coll[0].first == 0);
	REQUIRE(coll[0].second == "zero");
	REQUIRE(coll[1].first == 1);
	REQUIRE(coll[1].second == "one");
}

TEST_CASE("ObservableCollection iteration matches underlying vector", "[observable_collection][iteration]") {
	ObservableCollection<int> coll;
	coll.push_back(1);
	coll.push_back(2);
	coll.push_back(3);

	int sum = 0;
	for (auto v : coll)
	{
		sum += v;
	}
	REQUIRE(sum == 6);

	sum = 0;
	for (auto it = coll.rbegin(); it != coll.rend(); ++it)
	{
		sum += *it;
	}
	REQUIRE(sum == 6);
}

TEST_CASE("ReadonlyObservableCollection reflects collection state", "[observable_collection][readonly]") {
	ObservableCollection<int> coll;
	ReadonlyObservableCollection<int> view(coll);

	coll.push_back(10);
	coll.push_back(20);

	REQUIRE(view.size() == 2u);
	REQUIRE(static_cast<std::size_t>(view.Count) == 2u);
	REQUIRE(view[0] == 10);
	REQUIRE(view.at(1) == 20);

	REQUIRE(std::is_same_v<decltype(view[0]), const int&>);
	REQUIRE(std::is_same_v<decltype(view.begin()), ReadonlyObservableCollection<int>::const_iterator>);
}

TEST_CASE("ReadonlyObservableCollection forwards events", "[observable_collection][readonly_events]") {
	ObservableCollection<int> coll;
	ReadonlyObservableCollection<int> view(coll);

	int changedCalls = 0;
	auto sub = view.CollectionChanged().Subscribe([&](auto&, const auto&) { changedCalls++; });

	coll.push_back(1);
	coll.push_back(2);

	REQUIRE(changedCalls == 2);
}

TEST_CASE("ReadonlyObservableCollection accessors and iteration", "[observable_collection][readonly_access]") {
	ObservableCollection<int> coll;
	ReadonlyObservableCollection<int> view(coll);

	REQUIRE(view.empty());
	coll.push_back(5);
	coll.push_back(10);
	coll.push_back(15);

	REQUIRE_FALSE(view.empty());
	REQUIRE(view.size() == 3u);
	REQUIRE(view.front() == 5);
	REQUIRE(view.back() == 15);

	int sum = 0;
	for (auto v : view)
	{
		sum += v;
	}
	REQUIRE(sum == 30);

	sum = 0;
	for (auto it = view.rbegin(); it != view.rend(); ++it)
	{
		sum += *it;
	}
	REQUIRE(sum == 30);
}

TEST_CASE("ReadonlyObservableCollection reflects owner changes", "[observable_collection][readonly_owner]") {
	ObservableCollection<std::string> coll;
	ReadonlyObservableCollection<std::string> view(coll);

	coll.push_back("alpha");
	coll.push_back("beta");
	REQUIRE(view.size() == 2u);
	REQUIRE(view[1] == "beta");

	coll.erase(coll.begin());
	REQUIRE(view.size() == 1u);
	REQUIRE(view.front() == "beta");

	coll.clear();
	REQUIRE(view.empty());
}

TEST_CASE("ReadonlyObservableCollection type traits", "[observable_collection][readonly_traits]") {
	REQUIRE(std::is_same_v<ReadonlyObservableCollection<int>::const_iterator,
		decltype(std::declval<ReadonlyObservableCollection<int>>().begin())>);
	REQUIRE(std::is_same_v<ReadonlyObservableCollection<int>::const_reference,
		decltype(std::declval<ReadonlyObservableCollection<int>>()[0])>);
	REQUIRE_FALSE(std::is_assignable_v<ReadonlyObservableCollection<int>&, ObservableCollection<int>>);
}
