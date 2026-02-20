#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/ObservableCollection.h>

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
