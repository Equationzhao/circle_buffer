/**
 * @file circular_buffer.h
 * @author Equationzhao (equationzhao@foxmail.com)
 * @brief A simple concept-based C++ implementation of a circular buffer.
 * @version 0.1
 * @date 2021-12-24
 *
 * @copyright Copyright (c) 2021
 *
 */


#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER
#pragma once

#pragma region Includes

#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <new>
#include <stdexcept>
#include <utility>

#pragma endregion


// template<typename T, class Allocator = std::allocator<T>>
template <std::copyable T>
/*
	TODO(Equationzhao):
		* refactor size_t in operator +,-,+=,-= of iterator
		* Add concept Support
		* Add constexpr support
		* Add user-defined comparator support
		* Add user-defined deleter support
		* Use placement new instead of operator new
		? Module support
		? allocator support
		? ranges support
*/
class CircularBuffer
{
private:
	/**
	 * @brief Node
	 *		contains the data
	 *		contains the last and next Node*
	 */
	class Node
	{
	public:
#pragma region data and pointer
		T data{};
		Node* next{nullptr};
		Node* prev{nullptr};
		Node* head{nullptr};
		size_t distance{0};
		//TODO(Equationzhao) refactor :use distance,
		// ? is the variable head still necessary ?
#pragma endregion

#pragma region Constructors && Destructor

		explicit Node(const T& data) : data(data)
		{
		}

		explicit Node(T&& data) : data(std::move(data))
		{
		}

		Node() = default;

		Node(const Node& data) = delete;
		Node(const Node&& data) = delete;
		auto operator=(const Node& data) = delete;
		auto operator=(Node&& data) = delete;

		virtual ~Node() = default;

		// ? compare the T data it contains
		//		or
		//	 test `are they actually the same object`
		bool operator==(const Node& rhs) const
		{
			return this == std::addressof(rhs);
		}


#pragma endregion

#pragma region Modifiers
		/*
		 * @brief write data
		 */
		auto write(const T& dataToWrite)
		{
			this->data = dataToWrite;
		}

		/*
		 * @brief write data
		 */
		auto write(T&& dataToWrite)
		{
			this->data = std::move(dataToWrite);
		}

		/*
		 * @brief get a copy of the data
		 * @return value
		 */
		[[nodiscard]] T read() const
		{
			return this->data;
		}


		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] T& get()
		{
			return this->data;
		}

		/**
		 * @brief  get Reference of the data
		 * @return reference
		 */
		[[nodiscard]] T& get() const
		{
			return this->data;
		}


		/**
		 * @brief  get const-reference of the data
		 * @return  const reference
		 */
		[[nodiscard]] const T& const_get() const
		{
			return this->data;
		}


#pragma endregion
	};

	Node* buffer;
	size_t capacity_{0};
	size_t size_{0};

	Node* toWrite;
	Node* toRead;

	/**
	 * @brief initialize the circular buffer
	 *
	 * @param capacityToInit the capacity of the buffer
	 */
	void init(size_t capacityToInit)
	{
		this->capacity_ = capacityToInit;
		this->size_ = 0;


#pragma region initialize buffer
		// create buffer
		buffer = new Node();
		Node* iterator_ = buffer;

		iterator_->head = buffer;


		// create nodes and link them
		// update head and distance
		size_t distance{0};
		iterator_->distance = distance;
		for (size_t i = 0; i < capacityToInit - 1; ++i)
		{
			iterator_->next = new Node();
			iterator_->next->prev = iterator_;
			iterator_ = iterator_->next;
			iterator_->distance = ++distance;
			iterator_->head = buffer;
		}

		// make it circular
		iterator_->next = buffer;
		buffer->prev = iterator_;

		toWrite = buffer;
		toRead = buffer;

#pragma endregion
	}

	/**
	 * @brief destroy the circular buffer
	 *
	 */
	void destroy()
	{
		/*
		 *  Find the last node in the circular buffer
		 *  Then go back to the previous node and delete `next` until the head node is reached
		 */

		if (buffer == nullptr)
		{
			return;
		}

		auto iterator_ = buffer;

		//* Recursively delete
		//! need Test!
		//BUG: StackOverFlow
		const auto deleterBack = [this](Node* node)
		{
			auto head_ = this;
			static std::function<void(Node*)> deleter_;
			deleter_ = [&head_](Node* node)
			{
				if (node == head_->buffer)
				{
					delete node;
					return;
				}

				auto prev = node->prev;
				delete prev->next;
				return deleter_(prev);
			};

			deleter_(node);
		};


		// Find the last one
		if (iterator_->next != nullptr)
		{
			while (iterator_->next != buffer)
			{
				iterator_ = iterator_->next;
			}

			// Go back to the previous node and delete `next`
			deleterBack(iterator_);
		}
	}

#pragma region getters

	[[nodiscard]] auto getSize_() const
	{
		return size_;
	}

	[[nodiscard]] auto getCapacity_() const
	{
		return capacity_;
	}

	[[nodiscard]] auto getHeadElement()
	{
		return buffer->get();
	}

	[[nodiscard]] auto getTailElement()
	{
		return buffer->prev->get();
	}

	[[nodiscard]] auto getHeadElement() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] auto getTailElement() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] auto getPtrHead_()
	{
		return std::addressof(buffer->data);
	}

	[[nodiscard]] auto getPtrTail_()
	{
		return std::addressof(buffer->prev->data);
	}


#pragma endregion

public:
#pragma region Iterators Defination

	/*
	* @brief normal iterator
	*/
	class iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Node;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;
		using self = iterator;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;
		using container_ptr = CircularBuffer*;
		using const_container_ptr = const CircularBuffer*;
	private:
		value_type* ptr_;
		const_container_ptr proxy_;

		auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			proxy_ = other.proxy_;
		}

	protected:
		[[nodiscard]] auto getProxy_() const
		{
			return proxy_;
		}


	public:
		inline static Node* const end{nullptr};


		iterator() : ptr_(nullptr), proxy_(nullptr)
		{
		}

		explicit iterator(value_type* ptr, const_container_ptr proxy) : ptr_(ptr), proxy_(proxy)
		{
		}

		iterator(const iterator& other) : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		iterator(iterator&& other) noexcept : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		iterator& operator =(const iterator& other)
		{
			if (this == std::addressof(other))
			[[unlikely]]
			{
				return *this;
			}

			clone(other);
			return *this;
		}

		iterator& operator =(iterator&& other) noexcept
		{
			clone(other);
			return *this;
		}

		bool operator==(const iterator& other) const
		{
			// When compare `end` to `end`,
			// if they are the iterators of the some container
			// return true, else return false
			return ptr_ == other.ptr_ && proxy_ == other.proxy_;
		}

		reference operator*()
		{
			assert(this->ptr_ != end);

			return ptr_->get();
		}

		const_reference operator*() const
		{
			assert(this->ptr_ != end);

			return ptr_->const_get();
		}

		pointer operator->() const
		{
			assert(this->ptr_ != end);

			return std::addressof(ptr_->data);
		}

		self& operator++()
		{
			assert(ptr_ != end);

			if (ptr_ == ptr_->head->prev)
			[[unlikely]]
			{
				ptr_ = end;
			}
			else
			[[likely]]
			{
				ptr_ = ptr_->next;
			}

			return *this;
		}


		self operator++(int) &
		{
			self tmp = *this;

			++(*this);

			return tmp;
		}

		// ! need test
		// When it's end, continue to + will call abort
		self operator+(const size_t n)
		{
			if (n == 0)
			[[unlikely]]
			{
				return *this;
			}

			assert(ptr_ != end);

			const auto t = n;


			auto iterator_ = ptr_;

			for (size_t i = 0; i < t; ++i)
			{
				if (iterator_ == ptr_->head->prev)
				[[unlikely]]
				{
					iterator_ = end;
				}
				else
				{
					iterator_ = iterator_->next;
				}
			}

			return iterator(iterator_, this->proxy_);
		}

		// ! need test
		self& operator+=(const size_t n)
		{
			// assert(ptr_ != end);
			for (size_t i = 0; i < n; ++i)
			{
				++(*this);
			}

			return *this;
		}

		// When it's begin(), -- will call abort
		self& operator--()
		{
			assert(this->ptr_ == proxy_->buffer);

			if (this->ptr_ == end)
			[[unlikely]]
			{
				this->ptr_ = proxy_->buffer->prev;
			}


			ptr_ = ptr_->prev;

			return *this;
		}

		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		self operator--(int) &
		{
			self tmp = *this;
			--(*this);
			return tmp;
		}

		// ! need test
		self operator-(size_t n)
		{
			auto iterator_ = ptr_;

			if (iterator_ == end)
			{
				iterator_ = proxy_->buffer->prev;
				--n;
			}


			for (size_t i = 0; i < n; ++i)
			{
				assert(iterator_->ptr_ == proxy_->buffer);
				iterator_ = iterator_->prev;
			}

			return iterator(iterator_, this->proxy_);
		}

		// ! need test
		// May be BUG
		// pay attention to begin&end
		difference_type operator-(const iterator& rhs)
		{
			assert(this->proxy_ == rhs.proxy_);

			const auto iterator_ = this->ptr_;
			if (iterator_ == end)
			[[unlikely]]
			{
				if (rhs.ptr_ == end)
				[[unlikely]]
				{
					return 0;
				}
				else
				{
					return proxy_->capacity() - rhs.ptr_->distance;
				}
			}

			if (rhs.ptr_ == end)
			[[unlikely]]
			{
				return iterator_->distance;
			}

			return iterator_->distance - rhs.ptr_->distance;
		}


		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		self& operator-=(const size_t n)
		{
			// assert (ptr_!=end)
			for (size_t i = 0; i < n; ++i)
			{
				--(*this);
			}

			return *this;
		}

		// ! need test
		// access the circular buffer like a circle
		// when n>capacity it will start over from the beginning again
		reference operator[](const size_t n) noexcept
		{
			auto iterator_ = ptr_;

			for (size_t i = 0; i < n; ++i)
			{
				iterator_ = iterator_->next;
			}

			return *iterator_;
		}

		// ! need test
		auto operator<=>(const iterator& other) const
		{
			// iterator of different containers cannot be compared => abort
			assert(this->proxy_ == other.proxy_);
			return ptr_->distance <=> other.ptr_->distance;
		}

		virtual ~iterator() = default;
	};

	/**
	 * @brief circular iterator
	 *
	 */
	class circular_iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Node;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;
		using self = circular_iterator;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;
		using container_ptr = CircularBuffer*;
		using const_container_ptr = const CircularBuffer*;

	private:
		value_type* ptr_;
		const_container_ptr proxy_;

		auto clone(const iterator& other)
		{
			ptr_ = other.ptr_;
			proxy_ = other.proxy_;
		}

	protected:
		[[nodiscard]] auto getProxy_() const
		{
			return proxy_;
		}

	public:
		circular_iterator() : ptr_(nullptr), proxy_(nullptr)
		{
		}

		explicit circular_iterator(value_type* ptr, const_container_ptr proxy) : ptr_(ptr), proxy_(proxy)
		{
		}

		explicit circular_iterator(const circular_iterator& other) : ptr_(other.ptr_), proxy_(other.proxy_)
		{
		}

		explicit circular_iterator(circular_iterator&& other) noexcept : ptr_(std::move(other.ptr_)),
																		 proxy_(std::move(other.proxy_))
		{
		}

		circular_iterator& operator =(const circular_iterator& other)
		{
			if (this == std::addressof(other))
			[[unlikely]]
			{
				return *this;
			}

			this->clone(other);
			return *this;
		}

		circular_iterator& operator =(circular_iterator&& other) noexcept
		{
			this->clone(other);
			return *this;
		}

		bool operator==(const circular_iterator& other) const
		{
			// When compare `end` to `end`,
			// if they are the iterators of the some container
			// return true, else return false
			return this->ptr_ == other.ptr_ && this->proxy_ == other.proxy_;
		}

		reference operator*()
		{
			return this->ptr_->get();
		}

		const_reference operator*() const
		{
			return this->ptr_->const_get();
		}

		pointer operator->() const
		{
			return std::addressof(this->ptr_->data);
		}

		/*
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 */
		self& operator++()
		{
			this->ptr_ = this->ptr_->next;

			return *this;
		}

		/*
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 *	rely on the implementation of prefix++
		 */
		self operator++(int) &
		{
			self tmp = *this;

			++(*this);

			return tmp;
		}


		/*	! need test
		 * @ brief Different from iterator,
		 *		circular_iterator will iterate circular_buffer circularly
		 */
		self operator+(const size_t n)
		{
			if (n == 0)
			[[unlikely]]
			{
				return *this;
			}


			auto iterator_ = this->ptr_;


			iterator_ = iterator_->next;


			return circular_iterator(iterator_, this->proxy_);
		}

		// ! need test
		self& operator+=(const size_t n)
		{
			for (size_t i = 0; i < n; ++i)
			{
				++(*this);
			}

			return *this;
		}

		self& operator--()
		{
			this->ptr_ = this->ptr_->prev;

			return *this;
		}

		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		self operator--(int) &
		{
			self tmp = *this;
			--(*this);
			return tmp;
		}

		// ! need test
		self operator-(size_t n)
		{
			auto iterator_ = this->ptr_;

			for (size_t i = 0; i < n; ++i)
			{
				iterator_ = iterator_->prev;
			}

			return circular_iterator(iterator_, this->proxy_);
		}


		difference_type operator-(const circular_iterator& rhs)
		{
			assert(this->proxy_ == rhs.proxy_);

			const auto iterator_ = this->ptr_;

			return iterator_->distance - rhs.ptr_->distance;
		}


		// ! need test
		// rely on the implementation of prefix--
		// when n>distance it will continue self-sub from the beginning
		self& operator-=(const size_t n)
		{
			for (size_t i = 0; i < n; ++i)
			{
				--(*this);
			}

			return *this;
		}

		// ! need test
		// rely on the implementation of iterator[]
		reference operator[](const size_t n) noexcept
		{
			auto iterator_ = ptr_;

			for (size_t i = 0; i < n; ++i)
			{
				iterator_ = iterator_->next;
			}

			return *iterator_;
		}

		// ! need test
		auto operator<=>(const iterator& other) const
		{
			// iterator of different containers cannot be compared => abort
			assert(this->proxy_ == other.proxy_);
			return this->ptr_->distance <=> other.ptr_->distance;
		}

		virtual ~circular_iterator() = default;
	};
#pragma endregion

#pragma region Constructor && Descructor

	explicit CircularBuffer(size_t capacity_)
	{
		init(capacity_);
	}

	// TODO(Equationzhao) Support user-defined deleter
	virtual ~CircularBuffer()
	{
		destroy();
	}

#pragma endregion

#pragma region deleted functions
	/*
			* may implement them later
				or
			? just designed to keep them deleted
	*/
	CircularBuffer(CircularBuffer&& other) = delete;

	explicit CircularBuffer(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(const CircularBuffer& other) = delete;

	CircularBuffer& operator=(CircularBuffer&& other) = delete;

#pragma endregion

#pragma region element access

	[[nodiscard]] T& front()
	{
		return buffer->get();
	}

	[[nodiscard]] T& front() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] const T& cfront() const
	{
		return buffer->const_get();
	}

	[[nodiscard]] T& back()
	{
		return buffer->prev->get();
	}

	[[nodiscard]] T& back() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] const T& cback() const
	{
		return buffer->prev->const_get();
	}

	[[nodiscard]] T& operator[](const size_t index) noexcept
	{
		auto iterator_ = buffer;

		for (size_t i = 0; i < index; ++i)
		{
			iterator_ = iterator_->next;
		}

		return iterator_->get();
	}


	// throws runtime_error
	// not recommended
	[[nodiscard]] T& at(const size_t index)
	{
		if (index > size)
		{
			throw std::runtime_error("");
		}

		return this[index];
	}
#pragma endregion

#pragma region Modifiers
	// TODO(Equationzhao) implementation details
	// Perfect forwarding
	auto write(T&& data)
	{
		toWrite->write(std::move(data));
		toWrite = toWrite->next;
		// not Implement yet
	}


	auto write(const T& data)
	{
		toWrite->write(data);
		toWrite = toWrite->next;
	}

	auto insert()
	{
		// not Implement yet
	}

	auto emplace()
	{
		// not Implement yet
	}

	/*
	 * @brief gets a copy of the data
	 *
	 * @return T
	 */
	auto read()
	{
		const auto& d = toRead->data;
		toRead = toRead->next;
		return d;
		// not Implement yet
	}

	/*
	 * TODO(Equationzhao) clear the memory
	 */
	auto clear()
	{
		// not Implement yet
	}

	auto sort()
	{
		// not Implement yet
	}

	/*
	 * @brief swaps the pointer and data member
	 *	
	 */
	auto swap(CircularBuffer& rhs) noexcept
	{
		if (std::addressof(rhs) == this)
		{
			return;
		}

		{
			auto temp = this->buffer;
			this->buffer = rhs.buffer;
			rhs.buffer = temp;
		}

		{
			auto temp2 = this->capacity_;
			this->capacity_ = rhs.capacity_;
			rhs.capacity_ = temp2;
		}

		{
			auto temp3 = this->size_;
			this->size_ = rhs.size_;
			rhs.size_ = temp3;
		}

		{
			auto temp4 = this->toWrite;
			this->toWrite = rhs.toWrite;
			rhs.toWrite = temp4;
		}

		{
			auto temp5 = this->toRead;
			this->toRead = rhs.toRead;
			rhs.toRead = temp5;
		}
	}

	/*
	 * @brief reverse the circle buffer
	 */
	auto reverse()
	{
		auto iterator_ = buffer;

		//* exchange the next and prev pointer
		for (size_t i = 0; i < capacity_; ++i)
		{
			auto temp = iterator_->next;
			iterator_->next = iterator_->prev;
			iterator_->prev = temp;
			iterator_ = iterator_->next;
		}
	}

	auto erase()
	{
		// not Implement yet
	}

	auto erase_if()
	{
		// not Implement yet
	}


#pragma endregion

#pragma region iterator

	/*
	 *
		Since it's a circle, the begin() and the end() is actually the same element.
		But in order to support range-based-for, the end iterator is designed to be a nullptr
		to access the what the `end` contains will call abort
		operator like prefix++/--,suffix++/--,+=,-= is legal for the `end` 
	 *
	 */


	[[nodiscard]] iterator begin() const
	{
		return iterator(buffer, this);
	}

	[[nodiscard]] iterator end() const
	{
		return iterator(iterator::end, this);
	}

	using const_iterator = iterator;

	[[nodiscard]] const_iterator cbegin() const
	{
		return const_iterator(iterator::end, this);
	}

	[[nodiscard]] const_iterator cend() const
	{
		return const_iterator(iterator::end, this);
	}


	//////////////////////////////////////////////////////////////////////////////////


	[[nodiscard]] circular_iterator circular_begin() const
	{
		return circular_iterator(buffer, this);
	}

	using const_circular_iterator = circular_iterator;

	[[nodiscard]] const_circular_iterator circular_cbegin() const
	{
		return const_circular_iterator(buffer, this);
	}


#pragma endregion

#pragma region compare
	[[nodiscard]] auto operator<=>(const CircularBuffer&) const = default;

	[[nodiscard]] bool operator==(const CircularBuffer& rhs) const
	{
		if (this->size() != rhs.size())
		{
			return false;
		}

		else
		{
			for (auto i = this->begin(), j = rhs.begin(); i != this->end(); ++i, ++j)
			{
				if (*i != *j)
				{
					return false;
				}
			}


			return true;
		}
	}
#pragma endregion

#pragma region Capacity

	[[nodiscard]] size_t size() const
	{
		return getSize_();
	}

	[[nodiscard]] size_t capacity() const
	{
		return getCapacity_();
	}

	[[nodiscard]] bool empty() const
	{
		return size() == 0;
	}
};

#pragma endregion

#endif // !CIRCULAR_BUFFER
