#include "threading.h"
#include "loader.h"
#include "input.h"

#include "GLFW/GLFW3.h"
#include <cstdlib>

//int main() {
//    GLFWwindow* window;
//
//    float WINDOW_WIDHT = 800;
//    float WINDOW_HEIGHT = 800;
//
//
//    std::vector<vec4> frameBuffer;
//    frameBuffer.resize((int)WINDOW_WIDHT * (int)WINDOW_HEIGHT);
//
//    auto drawPixel = [&](const int& x, const int& y, const vec4& color) {
//        frameBuffer[x + y * WINDOW_WIDHT] = color;
//    };
//
//    auto drawPixelf = [&](const float& x, const float& y, const vec4& color) {
//        int new_x = (x + 1.0f) * WINDOW_WIDHT / 2;
//        int new_y = (y + 1.0f) * WINDOW_HEIGHT / 2;
//        drawPixel(new_x, new_y, color);
//    };
//
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//
//    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "Ray tracing", NULL, NULL);
//
//    if (!window)
//    {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//
//    glfwSetMouseButtonCallback(window, mouse_button_callback);
//
//    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1);
//
//    
//    vec4 light(0.0f, 1.0f, 0.0f, 1.0f);
//    float cosTheta = 0.99;
//    DefaultEnvironment env(light, cosTheta);
//    Packet packet;
//
//    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//    const float sqrt2 = 1.0f / sqrt(2.0);
//    long long n = 10000000;
//    long long light_sample = 0;
//
//    for (int i = 0; i < WINDOW_WIDHT * WINDOW_HEIGHT; i++) {
//        float x = i % (int)WINDOW_WIDHT;
//        float y = i / (int)WINDOW_WIDHT;
//        x = x / WINDOW_WIDHT * 2 - 1.0f;
//        y = y / WINDOW_HEIGHT * 2 - 1.0f;
//
//        if (x * x + y * y < 1.0f)
//            frameBuffer[i] = { 1.0f, 0.3f, 0.3f, 1.0f };
//        else
//            frameBuffer[i] = { 0.3f, 0.3f, 1.0f, 1.0f };
//    }
//
//    //packet.normal = { sqrt2, sqrt2, 0.0f, 1.0f };
//    packet.normal = { 0.0f, 1.0f, 0.0f, 1.0f };
//    for (long long i = 0; i < n; i++) {
//        //auto sample = random_in_cone(cosTheta, packet.normal);
//        //auto sample = env.generateSample(packet);
//        auto sample = random_cos_weight(packet.normal);
//        Ray ray = Ray(light, sample.sample);
//        if (sample.sample.dot(light) > cosTheta) {
//            light_sample++;
//            packet.color = { env.sun_intensity, env.sun_intensity, env.sun_intensity, 1.0f };
//        }
//        else packet.color = { 0.0f, 0.0f, 0.0f, 1.0f };
//        vec4 result = packet.color * sample.sample.dot(packet.normal) * sample.inv_pdf;
//        drawPixelf(sample.sample.v(0), sample.sample.v(2), result);
//        
//
//        color = color + result;
//    }
//    color = color * (1.0f / n);
//
//    std::cout << "Actual light sample rate: " << 0.5f * (1.0f - cos(2.0f * acosf(cosTheta))) << "\n";
//    std::cout << "Predict light sample rate: " << light_sample / (float)n << "\n";
//    std::cout << "Light sample: " << light_sample << "\n";
//    std::cout << "Final color: " << color.v(0) << " " << color.v(1) << " " << color.v(2) << "\n\n";
//   
//
//    while (!glfwWindowShouldClose(window)) {
//        float ratio;
//        int width, height;
//
//
//        glfwGetFramebufferSize(window, &width, &height);
//        ratio = width / (float)height;
//
//        glViewport(0, 0, width, height);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &frameBuffer.front());
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//
//    }
//
//
//
//    glfwDestroyWindow(window);
//
//    glfwTerminate();
//    exit(EXIT_SUCCESS);
//}