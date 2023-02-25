#include "threading.h"
#include <chrono>


constexpr int DEPTH = 4;


void threadWork(ThreadPool& threadPool, const Camera& cam, const Scene& object, Buffer& buffer) {
	while (!threadPool.shouldStop()) {
		while (!threadPool.isDone()) {
			long current = threadPool.getWork(cam);
			int samples = threadPool.getSamplePerPixel();
			for (int i = 0; i < PIXEL_PER_THREAD; i++) {
				long iter = current + i;
				vec4& pixel = buffer.frameBuffer[iter];
				bool debug = false;

				// debug = current + i == threadPool.getDebugPosition();

				if (samples == 0) {
					PixelData newPixel = shootRayD(object, cam, iter, 1, debug);
					vec4& col = newPixel.color;
					for (int j = 0; j < 3; j++) pixel.v(j) = sqrt(col.v(j));
					buffer.boundHitBuffer[iter] = newPixel.boundHit;
					buffer.shapeHitBuffer[iter] = newPixel.shapeHit;
				}
				else {
					PixelData newPixel = shootRay(object, cam, iter, DEPTH, debug);
					vec4& col = newPixel.color;

					//if (col.v(0) + col.v(1) + col.v(2) > 100) std::cout << "Final color: " << col.v(0) << " " << col.v(1) << " " << col.v(2) << "\n\n\n";

					if (debug) std::cout << "Final color: " << col.v(0) << " " << col.v(1) << " " << col.v(2) << "\n\n\n";

					for (int j = 0; j < 3; j++) pixel.v(j) = pixel.v(j) * pixel.v(j) * (samples - 1) + col.v(j);
					pixel.v /= samples;
					for (int j = 0; j < 3; j++) pixel.v(j) = sqrt(pixel.v(j));

					buffer.boundHitBuffer[iter] = newPixel.boundHit;
					buffer.shapeHitBuffer[iter] = newPixel.shapeHit;
				}
			}
		}
		std::unique_lock<std::mutex> wait_lock(threadPool.mux_wait);
		threadPool.cv_should_stop.wait(wait_lock);
	}
}

void ThreadPool::startWork(const Camera& cam, const Scene& scene, Buffer& buffer) {
	total_work = cam.resolution.x * cam.resolution.y;
	current_work = 0;
	start = std::chrono::system_clock::now();
	while (m_threadPool.size() < std::thread::hardware_concurrency() * 2 / 3) {
		std::thread newThread(threadWork, std::ref(*this), std::ref(cam), std::ref(scene), std::ref(buffer));
		m_threadPool.push_back(std::move(newThread));
	}
	//while (m_threadPool.size() < 1) {
	//	std::thread newThread(threadWork, std::ref(*this), std::ref(cam), std::ref(scene), std::ref(buffer));
	//	m_threadPool.push_back(std::move(newThread));
	//}
}

bool ThreadPool::isDone() {
	std::unique_lock<std::mutex> lg_lock(mux_work);
	if (should_stop) return true;
	return samplePerPixel > 100;
}

void ThreadPool::updateCamera() {
	std::unique_lock<std::mutex> lg_lock(mux_work);
	current_work = 0;
	samplePerPixel = 0;
	start = std::chrono::system_clock::now();
	cv_should_stop.notify_all();
}

void ThreadPool::stopRender() {
	should_stop = true;
	cv_should_stop.notify_all();
	for (auto& thread : m_threadPool) {
		thread.join();
	}
}

long ThreadPool::getWork(const Camera& cam) {
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