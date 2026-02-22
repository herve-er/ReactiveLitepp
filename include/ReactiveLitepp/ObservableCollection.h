#pragma once
#include "ObservableObject.h"
#include <vector>

namespace ReactiveLitepp
{
	template<typename T>
	class ObservableCollection : public ObservableObject
	{
	public:
		using value_type = T;
		using container_type = std::vector<T>;
		using size_type = typename container_type::size_type;
		using difference_type = typename container_type::difference_type;
		using reference = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
		using pointer = typename container_type::pointer;
		using const_pointer = typename container_type::const_pointer;
		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;
		using reverse_iterator = typename container_type::reverse_iterator;
		using const_reverse_iterator = typename container_type::const_reverse_iterator;

		/** Simple description of the change kind for the event args. */
		enum class ChangeType
		{
			Add,
			Remove,
			Clear,
			Insert,
			Unknown
		};

		/** Args raised just before the collection changes. */
		struct CollectionChangingArgs
		{
			ChangeType Change;
			size_type Index;        // index at which the change happens (if any)
			size_type OldCount;     // collection size before the change
			size_type NewCount;     // expected size after the change (may be same as OldCount for unknown)
		};

		/** Args raised after the collection changed. */
		struct CollectionChangedArgs
		{
			ChangeType Change;
			size_type Index;        // index at which the change happened (if any)
			size_type OldCount;     // collection size before the change
			size_type NewCount;     // collection size after the change
		};

		Event<ObservableCollection<T>&, CollectionChangingArgs> CollectionChanging;
		Event<ObservableCollection<T>&, CollectionChangedArgs> CollectionChanged;

		Property<size_type> Count;

		ObservableCollection()
			: _items()
			, Count(
				[this]() { return _items.size(); },
				[](size_type&) {})
		{
		}

		// capacity helpers
		bool empty() const noexcept { return _items.empty(); }
		size_type size() const noexcept { return _items.size(); }
		size_type capacity() const noexcept { return _items.capacity(); }

		void reserve(size_type new_cap) { _items.reserve(new_cap); }
		void shrink_to_fit() { _items.shrink_to_fit(); }

		// element access
		reference operator[](size_type pos) { return _items[pos]; }
		const_reference operator[](size_type pos) const { return _items[pos]; }

		reference at(size_type pos) { return _items.at(pos); }
		const_reference at(size_type pos) const { return _items.at(pos); }

		reference front() { return _items.front(); }
		const_reference front() const { return _items.front(); }

		reference back() { return _items.back(); }
		const_reference back() const { return _items.back(); }

		// iterators
		iterator begin() noexcept { return _items.begin(); }
		const_iterator begin() const noexcept { return _items.begin(); }
		const_iterator cbegin() const noexcept { return _items.cbegin(); }

		iterator end() noexcept { return _items.end(); }
		const_iterator end() const noexcept { return _items.end(); }
		const_iterator cend() const noexcept { return _items.cend(); }

		reverse_iterator rbegin() noexcept { return _items.rbegin(); }
		const_reverse_iterator rbegin() const noexcept { return _items.rbegin(); }
		const_reverse_iterator crbegin() const noexcept { return _items.crbegin(); }

		reverse_iterator rend() noexcept { return _items.rend(); }
		const_reverse_iterator rend() const noexcept { return _items.rend(); }
		const_reverse_iterator crend() const noexcept { return _items.crend(); }

		// modifiers with change notifications
		void clear() noexcept
		{
			if (_items.empty())
				return;
			auto oldCount = _items.size();
			auto newCount = size_type{ 0 };
			NotifyCollectionChanging(ChangeType::Clear, 0, oldCount, newCount);
			_items.clear();
			NotifyCollectionChanged(ChangeType::Clear, 0, oldCount, newCount);
		}

		void push_back(const T& value)
		{
			auto index = _items.size();
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Add, index, oldCount, newCount);
			_items.push_back(value);
			NotifyCollectionChanged(ChangeType::Add, index, oldCount, newCount);
		}

		void push_back(T&& value)
		{
			auto index = _items.size();
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Add, index, oldCount, newCount);
			_items.push_back(std::move(value));
			NotifyCollectionChanged(ChangeType::Add, index, oldCount, newCount);
		}

		template<class... Args>
		reference emplace_back(Args&&... args)
		{
			auto index = _items.size();
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Add, index, oldCount, newCount);
			auto& ref = _items.emplace_back(std::forward<Args>(args)...);
			NotifyCollectionChanged(ChangeType::Add, index, oldCount, newCount);
			return ref;
		}

		iterator insert(const_iterator pos, const T& value)
		{
			auto index = static_cast<size_type>(pos - _items.cbegin());
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Insert, index, oldCount, newCount);
			auto it = _items.insert(pos, value);
			NotifyCollectionChanged(ChangeType::Insert, index, oldCount, newCount);
			return it;
		}

		iterator insert(const_iterator pos, T&& value)
		{
			auto index = static_cast<size_type>(pos - _items.cbegin());
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Insert, index, oldCount, newCount);
			auto it = _items.insert(pos, std::move(value));
			NotifyCollectionChanged(ChangeType::Insert, index, oldCount, newCount);
			return it;
		}

		template<class... Args>
		iterator emplace(const_iterator pos, Args&&... args)
		{
			auto index = static_cast<size_type>(pos - _items.cbegin());
			auto oldCount = _items.size();
			auto newCount = oldCount + 1;
			NotifyCollectionChanging(ChangeType::Insert, index, oldCount, newCount);
			auto it = _items.emplace(pos, std::forward<Args>(args)...);
			NotifyCollectionChanged(ChangeType::Insert, index, oldCount, newCount);
			return it;
		}

		iterator erase(const_iterator pos)
		{
			auto index = static_cast<size_type>(pos - _items.cbegin());
			auto oldCount = _items.size();
			auto newCount = oldCount > 0 ? oldCount - 1 : 0;
			NotifyCollectionChanging(ChangeType::Remove, index, oldCount, newCount);
			auto it = _items.erase(pos);
			NotifyCollectionChanged(ChangeType::Remove, index, oldCount, newCount);
			return it;
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			if (first == last)
				return _items.erase(first, last);
			auto index = static_cast<size_type>(first - _items.cbegin());
			auto oldCount = _items.size();
			auto removed = static_cast<size_type>(last - first);
			auto newCount = oldCount - removed;
			NotifyCollectionChanging(ChangeType::Remove, index, oldCount, newCount);
			auto it = _items.erase(first, last);
			NotifyCollectionChanged(ChangeType::Remove, index, oldCount, newCount);
			return it;
		}

	private:
		void NotifyCollectionChanging(ChangeType change, size_type index, size_type oldCount, size_type newCount)
		{
			CollectionChangingArgs args{ change, index, oldCount, newCount };
			CollectionChanging.Notify(*this, args);
		}

		void NotifyCollectionChanged(ChangeType change, size_type index, size_type oldCount, size_type newCount)
		{
			CollectionChangedArgs args{ change, index, oldCount, newCount };
			CollectionChanged.Notify(*this, args);
		}

		container_type _items;
	};

	template<typename T>
	class ReadonlyObservableCollection
	{
	public:
		using collection_type = ObservableCollection<T>;
		using value_type = typename collection_type::value_type;
		using container_type = typename collection_type::container_type;
		using size_type = typename collection_type::size_type;
		using difference_type = typename collection_type::difference_type;
		using reference = typename collection_type::reference;
		using const_reference = typename collection_type::const_reference;
		using pointer = typename collection_type::pointer;
		using const_pointer = typename collection_type::const_pointer;
		using iterator = typename collection_type::iterator;
		using const_iterator = typename collection_type::const_iterator;
		using reverse_iterator = typename collection_type::reverse_iterator;
		using const_reverse_iterator = typename collection_type::const_reverse_iterator;
		using ChangeType = typename collection_type::ChangeType;
		using CollectionChangingArgs = typename collection_type::CollectionChangingArgs;
		using CollectionChangedArgs = typename collection_type::CollectionChangedArgs;

		ReadonlyProperty<size_type> Count;

		explicit ReadonlyObservableCollection(collection_type& collection)
			: _collection(&collection)
			, Count([this]() { return _collection->size(); })
		{
		}

		// capacity helpers
		bool empty() const noexcept { return _collection->empty(); }
		size_type size() const noexcept { return _collection->size(); }
		size_type capacity() const noexcept { return _collection->capacity(); }

		// element access
		const_reference operator[](size_type pos) const { return (*_collection)[pos]; }
		const_reference at(size_type pos) const { return _collection->at(pos); }
		const_reference front() const { return _collection->front(); }
		const_reference back() const { return _collection->back(); }

		// iterators
		const_iterator begin() const noexcept { return _collection->begin(); }
		const_iterator cbegin() const noexcept { return _collection->cbegin(); }
		const_iterator end() const noexcept { return _collection->end(); }
		const_iterator cend() const noexcept { return _collection->cend(); }
		const_reverse_iterator rbegin() const noexcept { return _collection->rbegin(); }
		const_reverse_iterator crbegin() const noexcept { return _collection->crbegin(); }
		const_reverse_iterator rend() const noexcept { return _collection->rend(); }
		const_reverse_iterator crend() const noexcept { return _collection->crend(); }

		Event<collection_type&, CollectionChangingArgs>& CollectionChanging() { return _collection->CollectionChanging; }
		Event<collection_type&, CollectionChangedArgs>& CollectionChanged() { return _collection->CollectionChanged; }

	private:
		collection_type* _collection;
	};
}
