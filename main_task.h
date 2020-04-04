#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")  /*windows--不带命令行输出窗口运行 console--带命令行输出窗口运行*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifndef NOMINMAX
#define NOMINMAX /* 不要让Windows定义min()或max() */
#endif
#define APP_NAME_STR_LEN 80
#endif

#include <vulkan/vulkan.h>

/*
窗体信息对象
*/
struct WindowInfo {
#define APP_NAME_STR_LEN 80
	char name[APP_NAME_STR_LEN]; //窗体标题栏 
	HWND window;                 //windows窗体句柄
	int width, height;
};
void init_window(struct WindowInfo &info, int32_t default_width, int32_t default_height);
void destroy_window(struct WindowInfo &info);