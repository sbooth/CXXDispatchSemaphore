//
// SPDX-FileCopyrightText: 2010 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXDispatchSemaphore
//

#pragma once

#import <cassert>
#import <chrono>
#import <stdexcept>

#import <dispatch/dispatch.h>

namespace CXXDispatchSemaphore {

/// A dispatch_semaphore_t wrapper.
class DispatchSemaphore final {
public:
	// MARK: Creation and Destruction

	/// Creates a new semaphore.
	/// @param value The starting value for the semaphore.
	/// @throw std::runtime_error if the semaphore could not be created.
	explicit DispatchSemaphore(intptr_t value);

	/// Creates a semaphore wrapping an existing dispatch semaphore.
	/// @note The results of passing a null dispatch semaphore are undefined.
	/// @param semaphore A dispatch semaphore.
	explicit DispatchSemaphore(dispatch_semaphore_t _Nonnull semaphore) noexcept;

	/// Creates a semaphore from an existing semaphore.
	/// @param other The semaphore to copy.
	DispatchSemaphore(const DispatchSemaphore& other) noexcept;

	/// Replaces this semaphore with an existing semaphore.
	/// @param other The semaphore to copy.
	/// @return A reference to this.
	DispatchSemaphore& operator=(const DispatchSemaphore& other) noexcept;

//	DispatchSemaphore(DispatchSemaphore&&) = delete;
//	DispatchSemaphore& operator=(DispatchSemaphore&&) = delete;

	/// Releases the underlying dispatch semaphore.
	~DispatchSemaphore() noexcept;

	// MARK: Primitives

	/// Waits for (decrements) the semaphore.
	///
	/// If the resulting value is less than zero this function waits for a signal to occur before returning.
	/// @param timeout The earliest time at which the function will stop waiting.
	/// @return true if the semaphore was decremented, false otherwise.
	bool wait(dispatch_time_t timeout) noexcept;

	/// Signals (increments) the semaphore.
	///
	/// If the previous value was less than zero, this function wakes a waiting thread.
	/// @return true if a thread was woken, false otherwise
	bool signal() noexcept;

	/// Waits for (decrements) the semaphore.
	///
	/// If the resulting value is less than zero this function waits for a signal to occur before returning.
	void wait() noexcept;

	// MARK: std::counting_semaphore Compatibility

	void acquire() noexcept;
	void release() noexcept;
	bool try_acquire() noexcept;

	template<class Rep, class Period>
	bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time);

	template<class Clock, class Duration>
	bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time);

private:
	/// The underlying dispatch semaphore.
	dispatch_semaphore_t _Nonnull semaphore_{nullptr};
};

/// A scoped semaphore guard that waits in the constructor and signals in the destructor.
class SemaphoreGuard final {
public:
	/// Constructs a semaphore guard and waits on the semaphore.
	/// @param semaphore A semaphore.
	explicit ScopeGuard(DispatchSemaphore& semaphore) noexcept;

	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator=(const ScopeGuard&) = delete;

	ScopeGuard(ScopeGuard&&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;

	/// Signals the semaphore.
	~ScopeGuard() noexcept;

private:
	/// A reference to the semaphore.
	DispatchSemaphore& semaphore_;
};

// MARK: - Implementation -

// MARK: Creation and Destruction

inline DispatchSemaphore::DispatchSemaphore(intptr_t value)
: semaphore_{dispatch_semaphore_create(value)}
{
	if(!semaphore_)
		throw std::runtime_error("Unable to create dispatch semaphore");
}

inline DispatchSemaphore::DispatchSemaphore(dispatch_semaphore_t _Nonnull semaphore) noexcept
: semaphore_{semaphore}
{
	assert(semaphore_ != nullptr);
#if !__has_feature(objc_arc)
	dispatch_retain(semaphore_);
#endif /* !__has_feature(objc_arc) */
}

inline DispatchSemaphore::DispatchSemaphore(const DispatchSemaphore& other) noexcept
: DispatchSemaphore{other.semaphore_}
{}

inline DispatchSemaphore& DispatchSemaphore::operator=(const DispatchSemaphore& other) noexcept
{
	if(this != &other) {
#if !__has_feature(objc_arc)
		dispatch_release(semaphore_);
#endif /* !__has_feature(objc_arc) */
		semaphore_ = other.semaphore_;
#if !__has_feature(objc_arc)
		dispatch_retain(semaphore_);
#endif /* !__has_feature(objc_arc) */
	}
	return *this;
}

inline DispatchSemaphore::~DispatchSemaphore() noexcept
{
#if !__has_feature(objc_arc)
	dispatch_release(semaphore_);
#endif /* !__has_feature(objc_arc) */
}

// MARK: Primitives

inline bool DispatchSemaphore::wait(dispatch_time_t timeout) noexcept
{
	return dispatch_semaphore_wait(semaphore_, timeout) == 0;
}

inline bool DispatchSemaphore::signal() noexcept
{
	return dispatch_semaphore_signal(semaphore_) != 0;
}

inline void DispatchSemaphore::wait() noexcept
{
	wait(DISPATCH_TIME_FOREVER);
}

// MARK: std::counting_semaphore Compatibility

inline void DispatchSemaphore::acquire() noexcept
{
	wait();
}

inline void DispatchSemaphore::release() noexcept
{
	signal();
}

inline bool DispatchSemaphore::try_acquire() noexcept
{
	return wait(DISPATCH_TIME_NOW);
}

template<class Rep, class Period>
inline bool DispatchSemaphore::try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time)
{
	const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(rel_time);
	const auto timeout = dispatch_time(DISPATCH_TIME_NOW, nsec.count());
	return wait(timeout);
}

template<class Clock, class Duration>
inline bool DispatchSemaphore::try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
{
	const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(abs_time - Clock::now());
	const auto timeout = dispatch_time(DISPATCH_TIME_NOW, nsec.count());
	return wait(timeout);
}

inline SemaphoreGuard::SemaphoreGuard(DispatchSemaphore& semaphore) noexcept
: semaphore_{semaphore}
{
	semaphore_.wait();
}

inline SemaphoreGuard::~SemaphoreGuard() noexcept
{
	semaphore_.signal();
}

} /* namespace CXXDispatchSemaphore */
