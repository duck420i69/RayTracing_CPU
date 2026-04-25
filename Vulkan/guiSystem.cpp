#include "vulkan.h"

namespace GUI {
	class Frame {


	private:
		std::vector<Frame> children;
	};

	class Button : public Frame {
	public:
		Button() : Frame() {
			pfnOnClickCallback = []() {};
		}

		Button(void onClick()) : Frame() {
			pfnOnClickCallback = onClick;
		}
		
		void setOnClickListener(void onClick()) {
			pfnOnClickCallback = onClick;
		}

		void onClick() {
			pfnOnClickCallback();
		}

	private:
		void (*pfnOnClickCallback)(void);
	};

	std::vector<Frame> ui;

	void draw() {

	}


}