#ifndef __TBK_UTILS_CIRCLEQUEUE_H__
#define __TBK_UTILS_CIRCLEQUEUE_H__
#include <mutex>
#include <array>
#include <cassert>
namespace tbk{
template <typename T, int SIZE>
class CircleQueue {
public:
	CircleQueue()=default;
    ~CircleQueue()=default;
	const T& operator[](int index);
	int capacity() { return SIZE; }
	int size() { return _size; }
	int cycle() { return _cycle; }
	void push(const T& t);
private:
	long long _cycle = 0;
	int _size = 0;
	long long _currentIndex = SIZE-1;
	std::array<T, SIZE> _data;
};

template<typename T, int SIZE>
const T& CircleQueue<T,SIZE>::operator[](int index) {
	assert(index <= 0);
	assert(index > -SIZE);
	return _data[((index % SIZE) + SIZE + _currentIndex) % SIZE];
}

template<typename T, int SIZE>
void CircleQueue<T,SIZE>::push(const T& t) {
	_currentIndex = (_currentIndex + 1) % SIZE;
	if (_size < SIZE) _size++;
	_cycle++;
	_data[_currentIndex] = t;
}
}
#endif // __TBK_UTILS_CIRCLEQUEUE_H__