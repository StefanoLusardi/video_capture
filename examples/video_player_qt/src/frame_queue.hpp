#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <deque>
#include <queue>
#include <algorithm>

namespace qvp
{
template <typename T, class Container=std::deque<T>>
class frame_queue
{
public:
	using container_type = Container;
	using value_type = T;
	using size_type = typename Container::size_type;
	static constexpr size_type MAX_CAPACITY = std::numeric_limits<size_type>::max();

private:
	mutable std::mutex lock_;
	std::condition_variable notEmptyCond_;
	std::condition_variable notFullCond_;
	size_type cap_;
	std::queue<T,Container> que_;
	using guard = std::lock_guard<std::mutex>;
	using unique_guard = std::unique_lock<std::mutex>;

public:
	frame_queue() : cap_(MAX_CAPACITY) {}

	explicit frame_queue(size_t cap) : cap_(std::max<size_type>(cap, 1)) {}

	void clear() {
		guard g(lock_);
		que_ = {};
	}

	bool empty() const {
		guard g(lock_);
		return que_.empty();
	}
    
	size_type capacity() const {
		guard g(lock_);
		return cap_;
	}
    
	void capacity(size_type cap) {
		guard g(lock_);
		cap_ = cap;
	}
    
	size_type size() const {
		guard g(lock_);
		return que_.size();
	}
    
	void put(value_type val) {
		unique_guard g(lock_);
		if (que_.size() >= cap_)
			notFullCond_.wait(g, [=]{return que_.size() < cap_;});
        bool wasEmpty = que_.empty();
		que_.emplace(std::move(val));
		if (wasEmpty) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
	}
    
	bool try_put(value_type val) {
		unique_guard g(lock_);
		size_type n = que_.size();
		if (n >= cap_)
			return false;
		que_.emplace(std::move(val));
		if (n == 0) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
    
	template <typename Rep, class Period>
	bool try_put_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(lock_);
		if (que_.size() >= cap_ && !notFullCond_.wait_for(g, relTime, [=]{return que_.size() < cap_;}))
			return false;
        bool wasEmpty = que_.empty();
		que_.emplace(std::move(val));
		if (wasEmpty) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
    
	template <class Clock, class Duration>
	bool try_put_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(lock_);
		if (que_.size() >= cap_ && !notFullCond_.wait_until(g, absTime, [=]{return que_.size() < cap_;}))
			return false;
        bool wasEmpty = que_.empty();
		que_.emplace(std::move(val));
		if (wasEmpty) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
		return true;
	}
    
	void get(value_type* val) {
		unique_guard g(lock_);
		if (que_.empty())
			notEmptyCond_.wait(g, [=]{return !que_.empty();});
		*val = std::move(que_.front());
		que_.pop();
		if (que_.size() == cap_-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
	}
    
	value_type get() {
		unique_guard g(lock_);
		if (que_.empty())
			notEmptyCond_.wait(g, [=]{return !que_.empty();});
		value_type val = std::move(que_.front());
		que_.pop();
		if (que_.size() == cap_-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return val;
	}
    
	bool try_get(value_type* val) {
		unique_guard g(lock_);
		if (que_.empty())
			return false;
		*val = std::move(que_.front());
		que_.pop();
		if (que_.size() == cap_-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return true;
	}
    
	template <typename Rep, class Period>
	bool try_get_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(lock_);
		if (que_.empty() && !notEmptyCond_.wait_for(g, relTime, [=]{return !que_.empty();}))
			return false;
		*val = std::move(que_.front());
		que_.pop();
		if (que_.size() == cap_-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return true;
	}
    
	template <class Clock, class Duration>
	bool try_get_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(lock_);
		if (que_.empty() && !notEmptyCond_.wait_until(g, absTime, [=]{return !que_.empty();}))
			return false;
		*val = std::move(que_.front());
		que_.pop();
		if (que_.size() == cap_-1) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return true;
	}
};

}