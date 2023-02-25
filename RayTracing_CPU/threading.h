#pragma once

#include <thread>
#include <mutex>
#include "Camera.h"
#include <iostream>

constexpr int PIXEL_PER_THREAD = 20;

class ThreadPool {
public:
	ThreadPool() {
		current_work = 0;
		total_work = 0;
		samplePerPixel = 0;
		should_stop = false;
		debug_position = -1;
	};

	void setDebugPosition(long pos) { debug_position = pos; }
	long getDebugPosition() { return debug_position; }
	long getTotalWork() { return total_work; }
	int getSamplePerPixel() { return samplePerPixel; }
	bool shouldStop() { return should_stop; }

	void startWork(const Camera& cam, const Scene& scene, Buffer& buffer);
	long getWork(const Camera& cam);
	bool isDone();

	void updateCamera();

	void stopRender();

	friend void threadWork(ThreadPool& threadPool, const Camera& cam, const Scene& object, Buffer& buffer);
private:
	std::vector<std::thread> m_threadPool;
	std::mutex mux_work;
	std::mutex mux_wait;
	std::condition_variable cv_should_stop;
	uint32_t debug_position;
	uint32_t current_work, total_work;
	uint32_t samplePerPixel;
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point end;
	bool should_stop;
};

void threadWork(ThreadPool& threadPool, const Camera& cam, const Scene& object, Buffer& buffer);