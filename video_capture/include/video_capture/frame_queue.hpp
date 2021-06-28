#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <deque>
#include <queue>
#include <algorithm>

namespace vc
{
	template <typename T>
	class frame_queue
	{
	public:
		using value_type = T;

	private:
		mutable std::mutex _lock;
		std::condition_variable notEmptyCond_;
		std::condition_variable notFullCond_;
		size_t _max_size;
		// std::optional<size_t> _max_size;
		std::deque<T> _queue;
		using guard = std::lock_guard<std::mutex>;
		using unique_guard = std::unique_lock<std::mutex>;

	public:
		frame_queue()
			: _max_size{std::numeric_limits<size_t>::max()}
		{
		}

		explicit frame_queue(size_t max_size)
			: _max_size{std::clamp<size_t>(max_size, 1, std::numeric_limits<size_t>::max())}
		{
		}

		void clear()
		{
			guard g(_lock);
			_queue = {};
		}

		bool is_empty() const
		{
			guard g(_lock);
			return _queue.empty();
		}

		size_t get_max_size() const
		{
			guard g(_lock);
			return _max_size;
		}

		void set_max_size(size_t cap)
		{
			guard g(_lock);
			_max_size = cap;
		}

		size_t size() const
		{
			guard g(_lock);
			return _queue.size();
		}

		void put(value_type val)
		{
			unique_guard g(_lock);
			if (_queue.size() >= _max_size)
				notFullCond_.wait(g, [=]
								  { return _queue.size() < _max_size; });
			bool wasEmpty = _queue.empty();
			_queue.emplace_back(std::move(val));
			if (wasEmpty)
			{
				g.unlock();
				notEmptyCond_.notify_one();
			}
		}

		/*

	bool try_put(value_type val) {
		unique_guard g(_lock);
		size_type n = _queue.size();
		if (n >= _max_size)
			return false;
		_queue.emplace(std::move(val));
		if (n == 0) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
    
	template <typename Rep, class Period>
	bool try_put_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(_lock);
		if (_queue.size() >= _max_size && !notFullCond_.wait_for(g, relTime, [=]{return _queue.size() < _max_size;}))
			return false;
        bool wasEmpty = _queue.empty();
		_queue.emplace(std::move(val));
		if (wasEmpty) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
    
	template <class Clock, class Duration>
	bool try_put_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(_lock);
		if (_queue.size() >= _max_size && !notFullCond_.wait_until(g, absTime, [=]{return _queue.size() < _max_size;}))
			return false;
        bool wasEmpty = _queue.empty();
		_queue.emplace(std::move(val));
		if (wasEmpty) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
*/

		void get(value_type *val)
		{
			unique_guard g(_lock);
			if (_queue.empty())
				notEmptyCond_.wait(g, [=]
								   { return !_queue.empty(); });
			*val = std::move(_queue.front());
			_queue.pop_front();
			if (_queue.size() == _max_size - 1)
			{
				g.unlock();
				notFullCond_.notify_one();
			}
		}

		auto get()
		{
			unique_guard g(_lock);
			if (_queue.empty())
				notEmptyCond_.wait(g, [=]
								   { return !_queue.empty(); });
			auto val = std::move(_queue.front());
			_queue.pop();
			if (_queue.size() == _max_size - 1)
			{
				g.unlock();
				notFullCond_.notify_one();
			}
			return val;
		}

		bool try_get(value_type *val)
		{
			unique_guard g(_lock);
			if (_queue.empty())
				return false;
			*val = std::move(_queue.front());
			_queue.pop_front();
			if (_queue.size() == _max_size - 1)
			{
				g.unlock();
				notFullCond_.notify_one();
			}
			return true;
		}

		/*    
    
	template <typename Rep, class Period>
	bool try_get_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(_lock);
		if (_queue.empty() && !notEmptyCond_.wait_for(g, relTime, [=]{return !_queue.empty();}))
			return false;
		*val = std::move(_queue.front());
		_queue.pop();
		if (_queue.size() == _max_size-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return true;
	}
    
	template <class Clock, class Duration>
	bool try_get_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(_lock);
		if (_queue.empty() && !notEmptyCond_.wait_until(g, absTime, [=]{return !_queue.empty();}))
			return false;
		*val = std::move(_queue.front());
		_queue.pop();
		if (_queue.size() == _max_size-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return true;
	}
*/
	};
}