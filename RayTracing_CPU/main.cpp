#include "threading.h"
#include "loader.h"
#include "input.h"

#include "GLFW/GLFW3.h"
#include <cstdlib>



int main() {
    GLFWwindow* window;
  
    float WINDOW_WIDHT = 1200;
    float WINDOW_HEIGHT = 800;

    Buffer frameBuffer(WINDOW_WIDHT, WINDOW_HEIGHT);

    vec4 direction = { -1.0f, -1.0f, 0.0f, 1.0f };
    direction.normalize();

    Camera cam({ 6.0f, 1.5f, -3.0f, 1.0f }, direction, { WINDOW_WIDHT, WINDOW_HEIGHT }, 45);


    //Scene scene = loadobj("test/Odd scene/oddscene.obj");
    Scene scene = loadobj("test/Odd scene/sphere3.obj");
    //Scene scene = loadobj("test/Sponza-master/sponza.obj");

    scene.objects[2]->material = std::make_shared<Glass>();
    scene.objects[2]->material->diffuse_color = { 0.3f, 0.9f, 0.3f,1 };
    scene.objects[2]->material->specular_color = { 1.0f, 1.0f, 1.0f, 1 };
    scene.objects[2]->material->refraction = 1.5f;

    vec4 light_direction = { 1.0f, 3.0f, 1.0f, 1.0f };
    light_direction.normalize();
    scene.environment = std::make_shared<DefaultEnvironment>(light_direction, 0.999);


    bool mouse_click = false;
    bool hold_tab = false;

    const float theta = 1.0f / 200.0f * pi;

    vec4 up = { 0.0f, 1.0f, 0.0f, 1.0f };
    vec4 right = { 0.0f, 0.0f, 1.0f, 1.0f };
    vec4 front = { 1.0f, 0.0f, 0.0f, 1.0f };


    int showFrame = 0;

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "Ray tracing", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ThreadPool threadPool;
    threadPool.startWork(cam, scene, frameBuffer);

    auto start = std::chrono::system_clock::now();
    
    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        auto end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        start = end;

        glfwSetWindowTitle(window, ("Ray Tracing " + std::to_string(duration.count()) + " ms").c_str());

        bool camera_change = false;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            vec4 rotation_matrix[3] = { 
                {std::cos(theta), 0.0f, -std::sin(theta), 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {std::sin(theta), 0.0f, std::cos(theta), 0.0f} 
            };
            cam.dir = rotation_matrix[0] * cam.dir.v(0) + rotation_matrix[1] * cam.dir.v(1) + rotation_matrix[2] * cam.dir.v(2);
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            vec4 rotation_matrix[3] = {
                {std::cos(-theta), 0.0f, -std::sin(-theta), 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {std::sin(-theta), 0.0f, std::cos(-theta), 0.0f}
            };
            cam.dir = rotation_matrix[0] * cam.dir.v(0) + rotation_matrix[1] * cam.dir.v(1) + rotation_matrix[2] * cam.dir.v(2);
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            cam.dir.v(1) += 0.02f * std::sqrt(1.0f - cam.dir.v(1) * cam.dir.v(1));
            if (cam.dir.v(1) > 0.999f) cam.dir.v(1) = 0.999f;
            float c = std::sqrt((1.0f - cam.dir.v(1) * cam.dir.v(1)) / (cam.dir.v(0) * cam.dir.v(0) + cam.dir.v(2) * cam.dir.v(2)));
            cam.dir.v(0) *= c;
            cam.dir.v(2) *= c;
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            cam.dir.v(1) -= 0.02f * std::sqrt(1.0f - cam.dir.v(1) * cam.dir.v(1));
            if (cam.dir.v(1) < -0.999f) cam.dir.v(1) = -0.999f;
            float c = std::sqrt((1.0f - cam.dir.v(1) * cam.dir.v(1)) / (cam.dir.v(0) * cam.dir.v(0) + cam.dir.v(2) * cam.dir.v(2)));
            cam.dir.v(0) *= c;
            cam.dir.v(2) *= c;
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }


        right = up.cross(cam.dir * -1.0f);
        right.normalize();
        front = up.cross(right);


        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + front * 0.05f, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + right * -0.05f, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + right * 0.05f, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + front * -0.05f, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + up * 0.05f, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + up * -0.05f, cam.dir);
            camera_change = true;
        }
        if (camera_change) threadPool.updateCamera();
        
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !hold_tab) {
            hold_tab = true;
            showFrame = (showFrame + 1) % 3;
            if (showFrame == 1) {
                uint16_t maxHit = 0;
                for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                    maxHit = std::max(maxHit, frameBuffer.boundHitBuffer[i]);
                }
                std::cout << "Showing Frame: Bounding Box Hit, Max hitted: " << maxHit << "\n\n";
            }
            else if (showFrame == 2) {
                uint16_t maxHit = 0;
                for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                    maxHit = std::max(maxHit, frameBuffer.shapeHitBuffer[i]);
                }
                std::cout << "Showing Frame: Shape Check, Max hitted: " << maxHit << "\n\n";
            }
        }
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) hold_tab = false;


        if (showFrame == 0) glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &frameBuffer.frameBuffer.front());
        else if (showFrame == 1) {
            uint16_t maxHit = 0;
            std::vector<vec4> newFrameBuffer;
            newFrameBuffer.resize(frameBuffer.h * frameBuffer.w);
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                maxHit = std::max(maxHit, frameBuffer.boundHitBuffer[i]);
            }
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                newFrameBuffer[i] = { frameBuffer.boundHitBuffer[i] / (float)maxHit, 1.0f - frameBuffer.boundHitBuffer[i] / (float)maxHit, 0.0f, 1.0f };
            }
            glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &newFrameBuffer.front());
        }
        else if (showFrame == 2) {
            uint16_t maxHit = 0;
            std::vector<vec4> newFrameBuffer;
            newFrameBuffer.resize(frameBuffer.h * frameBuffer.w);
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                maxHit = std::max(maxHit, frameBuffer.shapeHitBuffer[i]);
            }
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                newFrameBuffer[i] = { frameBuffer.shapeHitBuffer[i] / (float)maxHit, 1.0f - frameBuffer.shapeHitBuffer[i] / (float)maxHit, 0.0f, 1.0f };
            }
            glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &newFrameBuffer.front());
        }


        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouse_click) {
            mouse_click = true;
            PixelData pixel = shootRay(scene, cam, get_buffer_pos(WINDOW_WIDHT, WINDOW_HEIGHT), 10, true);
            vec4& col = pixel.color;
            std::cout << "Final color: " << col.v(0) << " " << col.v(1) << " " << col.v(2) << "\n\n\n";
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) mouse_click = false;


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    threadPool.stopRender();

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


