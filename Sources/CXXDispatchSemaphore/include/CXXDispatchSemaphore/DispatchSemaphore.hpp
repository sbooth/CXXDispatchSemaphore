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
#import <utility>

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

// MARK: SemaphoreGuard

/// Tag indicating that a semaphore has already been acquired and that the constructor should not wait.
struct already_acquired_t {
	explicit already_acquired_t() noexcept = default;
};

/// The semaphore has already been acquired and the constructor should not wait.
constexpr already_acquired_t already_acquired;

/// A flexible scoped semaphore guard.
class SemaphoreGuard final {
public:
	/// Constructs a semaphore guard and waits on the semaphore.
	/// @param semaphore A semaphore.
	explicit SemaphoreGuard(DispatchSemaphore& semaphore) noexcept;

	/// Constructs a semaphore guard and waits on the semaphore.
	///
	/// If the semaphore is not acquired before the timeout expires, the guard is
	/// constructed in a non-acquired state. In this case `operator bool()` will
	/// return false and the destructor will not signal the semaphore.
	/// @param semaphore A semaphore.
	/// @param timeout The earliest time at which the function will stop waiting.
	SemaphoreGuard(DispatchSemaphore& semaphore, dispatch_time_t timeout) noexcept;

	/// Constructs a semaphore guard with an already-acquired semaphore.
	/// @param semaphore A semaphore.
	SemaphoreGuard(DispatchSemaphore& semaphore, already_acquired_t) noexcept;

	SemaphoreGuard(const SemaphoreGuard&) = delete;
	SemaphoreGuard& operator=(const SemaphoreGuard&) = delete;

	SemaphoreGuard(SemaphoreGuard&&) = delete;
	SemaphoreGuard& operator=(SemaphoreGuard&&) = delete;

	/// Signals the semaphore if it has been acquired.
	~SemaphoreGuard() noexcept;

	/// true if the semaphore has been acquired.
	[[nodiscard]] explicit operator bool() const noexcept;

	/// Returns true if the semaphore has been acquired.
	[[nodiscard]] bool acquired() const noexcept;

	/// Dismisses the guard by marking the semaphore as not acquired without signaling.
	/// @return true if the semaphore was previously acquired, false otherwise
	bool dismiss() noexcept;

private:
	/// A reference to the semaphore.
	DispatchSemaphore& semaphore_;
	/// Whether the guard has acquired the semaphore.
	bool acquired_{false};
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
	if(rel_time <= std::chrono::duration<Rep, Period>::zero())
		return wait(DISPATCH_TIME_NOW);
	const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(rel_time);
	const auto timeout = dispatch_time(DISPATCH_TIME_NOW, nsec.count());
	return wait(timeout);
}

template<class Clock, class Duration>
inline bool DispatchSemaphore::try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time)
{
	const auto now = Clock::now();
	if(abs_time <= now)
		return wait(DISPATCH_TIME_NOW);
	const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(abs_time - now);
	const auto timeout = dispatch_time(DISPATCH_TIME_NOW, nsec.count());
	return wait(timeout);
}

// MARK: - SemaphoreGuard

inline SemaphoreGuard::SemaphoreGuard(DispatchSemaphore& semaphore) noexcept
: SemaphoreGuard{semaphore, DISPATCH_TIME_FOREVER}
{}

inline SemaphoreGuard::SemaphoreGuard(DispatchSemaphore& semaphore, dispatch_time_t timeout) noexcept
: semaphore_{semaphore}
{
	acquired_ = semaphore_.wait(timeout);
}

inline SemaphoreGuard::SemaphoreGuard(DispatchSemaphore& semaphore, already_acquired_t) noexcept
: semaphore_{semaphore}, acquired_{true}
{}

inline SemaphoreGuard::~SemaphoreGuard() noexcept
{
	if(acquired_)
		semaphore_.signal();
}

inline SemaphoreGuard::operator bool() const noexcept
{
	return acquired_;
}

inline bool SemaphoreGuard::acquired() const noexcept
{
	return acquired_;
}

inline bool SemaphoreGuard::dismiss() noexcept
{
	return std::exchange(acquired_, false);
}

} /* namespace CXXDispatchSemaphore */
