#pragma once

#include <atomic>
#include <cstdint>

//thread-safe ring buffer

template<typename T, uint32_t SIZE>
class CRingBuffer
{
public:

	static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be a power of two.");

	bool Push(const T& item)
	{
		uint32_t write = m_Write.load(std::memory_order_relaxed);
		uint32_t next = Increment(write);

		if (next == m_Read.load(std::memory_order_acquire))
		{
			return false; // Queue full
		}

		m_Buffer[write] = item;

		m_Write.store(next, std::memory_order_release);
		{
			return true;
		}
	}

	bool Pop(T& item)
	{
		uint32_t read = m_Read.load(std::memory_order_relaxed);

		if (read == m_Write.load(std::memory_order_acquire))
		{
			return false; // Queue empty
		}

		item = m_Buffer[read];

		m_Read.store(Increment(read), std::memory_order_release);
		return true;
	}

	bool IsEmpty() const
	{
		return m_Read.load(std::memory_order_acquire) == m_Write.load(std::memory_order_acquire);
	}

	bool IsFull() const
	{
		return Increment(m_Write.load(std::memory_order_acquire)) == m_Read.load(std::memory_order_acquire);
	}

	uint32_t Capacity() const
	{
		return SIZE;
	}
private:

	static constexpr uint32_t Increment(uint32_t index)
	{
		return (index + 1) & (SIZE - 1);
	}

	T m_Buffer[SIZE];

	std::atomic<uint32_t> m_Read{0};
	std::atomic<uint32_t> m_Write{0};
};