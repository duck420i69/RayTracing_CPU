#include "input.h"
#include "threading.h"

#include "loader/loader.h"
#include "integrater/DebugRenderer.h"

#include "GLFW/GLFW3.h"

#if 1
vec3 aces_approx(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    vec3 result;
    for (int i = 0; i < 3; i++) result(i) = std::clamp((v(i) * (a * v(i) + b) / (v(i) * (c * v(i) + d)) + e), 0.0f, 1.0f);
    return result;
}


int main() {
    GLFWwindow* window;

    float WINDOW_WIDHT = 800;
    float WINDOW_HEIGHT = 600;

    Buffer frameBuffer(WINDOW_WIDHT, WINDOW_HEIGHT);
    Buffer presentBuffer(WINDOW_WIDHT, WINDOW_HEIGHT);
    presentBuffer.boundHitBuffer.resize(0);
    presentBuffer.shapeHitBuffer.resize(0);

    vec3 direction = { 1.0f, 0.0f, 0.0f };
    direction.normalize();

    //Scene scene = loadobj("test/Odd scene/house.obj");
    Scene scene = loadobj("test/Odd scene/sphere3.obj");
    //Scene scene = loadobj("test/Odd scene/monke.obj");
    //Scene scene = loadobj("test/bmw27/bmw27_cpu.obj");
    //Scene scene = loadobj("test/Sponza-master/sponza.obj");

    scene.camera = Camera({ 0.0f, 1.0f, 0.0f }, direction, { WINDOW_WIDHT, WINDOW_HEIGHT }, 45);

    /*Scene scene;
    std::deque<std::unique_ptr<BVHNode>> bvh;
    MaterialBuilder builder;
    auto sphere = std::make_shared<Sphere>();
    sphere->center = { 0.0f, 0.0f, 0.0f };
    sphere->r = 2.0f;
    sphere->material = builder.setDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f })
        .setSpecularColor({ 1.0f, 1.0f, 1.0f, 1.0f })
        .setShininess(1000.0f)
        .setRefraction(1.5f)
        .setName("GGX").build();
    bvh.emplace_back(std::make_unique<BVHNode>(sphere->getBound(), scene.objects.size()));
    scene.objects.emplace_back(sphere);
    size_t i = 0;
    constructLinearBVH(constructBVH(std::move(bvh), BuildStrat::TOPDOWN), scene.tree, i);*/

    vec3 light_direction = { 1.0f, 3.0f, 1.0f };
    light_direction.normalize();

    //scene.environment = std::make_shared<DefaultEnvironment>(light_direction, 0.99f, 10.0f);
    //scene.environment = std::make_shared<DefaultEnvironment>(light_direction, -1.0f, 10.0f);
    //scene.environment = loadEnvHDR("skybox/syferfontein_6d_clear_puresky_4k.hdr");
    scene.environment = loadEnvHDR("skybox/kloppenheim_05_puresky_2k.hdr");
    //scene.environment = loadEnvHDR("skybox/san_giuseppe_bridge_4k.hdr");

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

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ThreadPool threadPool;
    threadPool.startWork(scene, frameBuffer);

    InputData input;

    input.movementSpeed = (scene.tree.bvh[0].bound.max(0) - scene.tree.bvh[0].bound.min(0)) / 1000.0f;
    input.rotationSpeed = 1.0f / 200.0f * pi;
    input.scene = &scene;
    input.showFrame = 0;
    input.threadPool = &threadPool;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowUserPointer(window, &input);

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

        if (input.showFrame == 0) {
            uint32_t i = 0;
            for (i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                presentBuffer.frameBuffer[i].v = aces_approx(frameBuffer.frameBuffer[i].v);
                presentBuffer.frameBuffer[i].w = 1.0f;
            }

            if (input.rayPaths.size() > 0) {
                for (i = 0; i < input.rayPaths.size() - 1; i++) {
                    drawLine(
                        presentBuffer, scene.camera, 
                        input.rayPaths[i].o, input.rayPaths[i + 1].o, 
                        { 1.0f, 0.0f, 1.0f, 1.0f });
                }
                drawLine(
                    presentBuffer, scene.camera, 
                    input.rayPaths[i].o, 
                    input.rayPaths[i].o + input.rayPaths[i].d * 1000.0f, 
                    { 1.0f, 1.0f, 1.0f, 1.0f });

            }
            glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &presentBuffer.frameBuffer.front());
        }
        else if (input.showFrame == 1) {
            uint16_t maxHit = 0;
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                maxHit = std::max(maxHit, frameBuffer.boundHitBuffer[i]);
            }
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                presentBuffer.frameBuffer[i] = { frameBuffer.boundHitBuffer[i] / (float)maxHit, 1.0f - frameBuffer.boundHitBuffer[i] / (float)maxHit, 0.0f, 1.0f };
            }
            glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &presentBuffer.frameBuffer.front());
        }
        else if (input.showFrame == 2) {
            uint16_t maxHit = 0;
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                maxHit = std::max(maxHit, frameBuffer.shapeHitBuffer[i]);
            }
            for (uint32_t i = 0; i < frameBuffer.h * frameBuffer.w; i++) {
                presentBuffer.frameBuffer[i] = { frameBuffer.shapeHitBuffer[i] / (float)maxHit, 1.0f - frameBuffer.shapeHitBuffer[i] / (float)maxHit, 0.0f, 1.0f };
            }
            glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &presentBuffer.frameBuffer.front());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    threadPool.stopRender();

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
#endif