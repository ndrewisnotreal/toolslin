#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "memory.h"
#include "ui.h"
#include "classes.h"
#include "globals.h"
#include "esp.h"
#include "config.h"

std::string GetBindName(int bind) {
    if (bind == -1) return "None";
    if (bind >= 0x101 && bind <= 0x105) {
        if (bind == 0x101) return "Left Click";
        if (bind == 0x102) return "Middle Click";
        if (bind == 0x103) return "Right Click";
        if (bind == 0x104) return "Mouse 4";
        if (bind == 0x105) return "Mouse 5";
    }
    if (bind >= 8 && bind < 256) {
        Display* dpy = glfwGetX11Display();
        if (dpy) {
            KeySym ks = XkbKeycodeToKeysym(dpy, bind, 0, 0);
            if (ks != NoSymbol) {
                const char* name = XKeysymToString(ks);
                if (name) return std::string(name);
            }
        }
        return "Key " + std::to_string(bind);
    }
    return "Unknown";
}

int main() {
    if (!getenv("XDG_RUNTIME_DIR")) {
        setenv("XDG_RUNTIME_DIR", "/run/user/0", 1);
    }

    std::cout << "[*] Starting DBD Tool...\n";

    const char* offsetPaths[] = {
        "Offsets.json",
        "../Offsets.json",
        "../../Offsets.json",
    };
    bool offsetsLoaded = false;
    for (const char* p : offsetPaths) {
        if (LoadOffsets(p)) { offsetsLoaded = true; break; }
    }
    if (!offsetsLoaded)
        std::cerr << "[!] Offsets.json not found\n";

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    GLFWmonitor*       monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode    = glfwGetVideoMode(monitor);

    // Use a non-descript title that won't be flagged by window-title scanning
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Steam", NULL, NULL);
    if (!window) { glfwTerminate(); return 1; }

    // Set X11 WM_CLASS to a benign value so scanners see a normal process window
    {
        Display* dpy = glfwGetX11Display();
        Window   xwnd = glfwGetX11Window(window);
        if (dpy && xwnd) {
            XClassHint hint;
            hint.res_name  = (char*)"steam";
            hint.res_class = (char*)"Steam";
            XSetClassHint(dpy, xwnd, &hint);
            XFlush(dpy);
        }
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);

    InitializeUI();
    SetupUinput(); // initialize virtual keyboard
    std::string defConfig = Config::GetDefault();
    if (defConfig == "config.json") {
        Config::Load("config.json");
    } else {
        Config::Load("configs/" + defConfig);
    }
    ESP_StartScanThread();

    std::cout << "[SUCCEED] The Overlay has been initialized.\n";
    std::cout << "[*] HOME = open/close menu  |  END = exit\n";

    bool isInteractive = false;
    bool homePressed   = false;
    bool endPressed    = false;

    while (!glfwWindowShouldClose(window)) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        static int bindState = 0;
        Display* xdpy = glfwGetX11Display();
        if (xdpy) {
            char keys[32];
            XQueryKeymap(xdpy, keys);
            
            Window root = DefaultRootWindow(xdpy);
            Window root_ret, child_ret;
            int root_x, root_y, win_x, win_y;
            unsigned int mask = 0;
            XQueryPointer(xdpy, root, &root_ret, &child_ret, &root_x, &root_y, &win_x, &win_y, &mask);

            if (g_AimbotIsBinding) {
                if (bindState == 0) bindState = 1;
                
                bool anythingDown = false;
                for (int i = 8; i < 256; i++) {
                    if (keys[i / 8] & (1 << (i % 8))) { anythingDown = true; break; }
                }
                if (mask & (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask)) anythingDown = true;

                if (bindState == 1 && !anythingDown) {
                    bindState = 2; // Ready to catch the next press
                } else if (bindState == 2) {
                    int foundBind = -1;
                    if (mask & Button1Mask) foundBind = 0x101;
                    else if (mask & Button2Mask) foundBind = 0x102;
                    else if (mask & Button3Mask) foundBind = 0x103;
                    else if (mask & Button4Mask) foundBind = 0x104;
                    else if (mask & Button5Mask) foundBind = 0x105;
                    else {
                        for (int i = 8; i < 256; i++) {
                            if (keys[i / 8] & (1 << (i % 8))) {
                                foundBind = i;
                                break;
                            }
                        }
                    }
                    if (foundBind != -1) {
                        g_Cheats_AimbotBind = foundBind;
                        g_AimbotIsBinding = false;
                        bindState = 0;
                    }
                }
            } else {
                bindState = 0;
            }

            auto isBindDown = [&](int bind) -> bool {
                if (bind >= 0x101 && bind <= 0x105) {
                    int btn = bind - 0x100;
                    unsigned int targetMask = 0;
                    if (btn == 1) targetMask = Button1Mask;
                    else if (btn == 2) targetMask = Button2Mask;
                    else if (btn == 3) targetMask = Button3Mask;
                    else if (btn == 4) targetMask = Button4Mask;
                    else if (btn == 5) targetMask = Button5Mask;
                    return (mask & targetMask) != 0;
                } else if (bind >= 8 && bind < 256) {
                    return (keys[bind / 8] & (1 << (bind % 8))) != 0;
                }
                return false;
            };

            g_AimbotKeyDown = (!g_AimbotIsBinding && g_Cheats_AimbotBind != -1) ? isBindDown(g_Cheats_AimbotBind) : false;
        }

        if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
            if (!endPressed) { 
                endPressed = true; 
                ESP_Cleanup();
                break; 
            }
        } else { endPressed = false; }

        if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
            if (!homePressed) {
                isInteractive = !isInteractive;
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH,
                                    isInteractive ? GLFW_FALSE : GLFW_TRUE);
                homePressed = true;
            }
        } else { homePressed = false; }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderUI(isInteractive);
        RunESP();

        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (g_FPS_Limit > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - frameStart).count();
            long target = 1000000L / g_FPS_Limit;
            if (elapsed < target)
                std::this_thread::sleep_for(std::chrono::microseconds(target - elapsed));
        }
    }

    ESP_Cleanup();
    Config::Save();
    Memory::Detach();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
