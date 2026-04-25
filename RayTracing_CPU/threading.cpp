#include "config.h"
#include "threading.h"

#include "integrater/Renderer.h"

#include <chrono>
#include <iostream>

std::mutex mux_console;

void threadWork(ThreadPool& threadPool, const Scene& object, Buffer& buffer) {
	while (!threadPool.shouldStop()) {
		while (!threadPool.isDone()) {
			long current = threadPool.getWork();
			int samples = threadPool.getSamplePerPixel();
			for (int i = 0; i < PIXEL_PER_THREAD; i++) {
				bool debug = false;
				long iter = current + i;
				vec4& pixel = buffer.frameBuffer[iter];

				// debug = current + i == threadPool.getDebugPosition();

				if (samples == 0) {
					PixelData newPixel = shootRayD(object, iter, 1, debug);
					buffer.frameBuffer[iter] = newPixel.color;
					buffer.boundHitBuffer[iter] = newPixel.boundHit;
					buffer.shapeHitBuffer[iter] = newPixel.shapeHit;
				}
				else {
					PixelData newPixel = shootRay(object, iter, DEPTH, debug);
					vec4& col = newPixel.color;
					buffer.frameBuffer[iter] = (buffer.frameBuffer[iter] * (samples - 1) + newPixel.color) / samples;
					buffer.boundHitBuffer[iter] = newPixel.boundHit;
					buffer.shapeHitBuffer[iter] = newPixel.shapeHit;
				}
			}
		}
		if (threadPool.shouldStop()) break;
		std::unique_lock<std::mutex> wait_lock(threadPool.mux_wait);
		threadPool.cv_should_stop.wait(wait_lock);
	}
}

void ThreadPool::startWork(const Scene& scene, Buffer& buffer) {
	const Camera& cam = scene.camera;
	total_work = cam.resolution.x * cam.resolution.y;
	current_work = 0;
	start = std::chrono::system_clock::now();
	if (!SINGLE_THREAD) {
		while (m_threadPool.size() < std::thread::hardware_concurrency() * 2 / 3) {
			std::thread newThread(threadWork, std::ref(*this), std::ref(scene), std::ref(buffer));
			m_threadPool.push_back(std::move(newThread));
		}
	}
	else {
		while (m_threadPool.size() < 1) {
			std::thread newThread(threadWork, std::ref(*this), std::ref(scene), std::ref(buffer));
			m_threadPool.push_back(std::move(newThread));
		}
	}
}

bool ThreadPool::isDone() {
	std::unique_lock<std::mutex> lg_lock(mux_work);
	if (should_stop) return true;
	return samplePerPixel > 4000;
}

void ThreadPool::updateCamera() {
	std::unique_lock<std::mutex> lg_lock(mux_work);
	current_work = 0;
	samplePerPixel = 0;
	start = std::chrono::system_clock::now();
	cv_should_stop.notify_all();
}

void ThreadPool::stopRender() {
	{
		std::unique_lock<std::mutex> wait_lock(mux_wait);
		std::unique_lock<std::mutex> work_lock(mux_work);
		should_stop = true;
	}
	cv_should_stop.notify_all();
	for (auto& thread : m_threadPool) {
		if (thread.joinable()) thread.join();
	}
	m_threadPool.clear();
}

long ThreadPool::getWork() {
	std::unique_lock<std::mutex> lg_lock(mux_work);
	uint32_t work = current_work;
	if (current_work >= total_work - PIXEL_PER_THREAD) {
		current_work = total_work % PIXEL_PER_THREAD;
		samplePerPixel++;
		end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		start = end;
		std::cout << "This frame took: " << duration.count() << "ms\n";
	}
	current_work += PIXEL_PER_THREAD;
	return work;
}


void printDebug(std::string str) {
	std::unique_lock<std::mutex> console_lock(mux_console);
	std::cout << str << std::endl;
}