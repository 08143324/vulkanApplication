#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")  /*windows--��������������������� console--�������������������*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#ifndef NOMINMAX
#define NOMINMAX /* ��Ҫ��Windows����min()��max() */
#endif
#define APP_NAME_STR_LEN 80
#endif

#include <vulkan/vulkan.h>

/*
������Ϣ����
*/
struct WindowInfo {
#define APP_NAME_STR_LEN 80
	char name[APP_NAME_STR_LEN]; //��������� 
	HWND window;                 //windows������
	int width, height;
};
void init_window(struct WindowInfo &info, int32_t default_width, int32_t default_height);
void destroy_window(struct WindowInfo &info);