#include <stdio.h>
#include <assert.h>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include "main_task.h"
#include "VulkanManager.h"

using namespace std;

int preX = 0;
int preY = 0;
bool mouseLeftDown = false;

//各种事件的处理方法
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct WindowInfo *info = reinterpret_cast<struct WindowInfo *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		break;
	case WM_LBUTTONDOWN://鼠标左键按下
		preX = LOWORD(lParam);//鼠标的横坐标  
		preY = HIWORD(lParam);//鼠标的纵坐标  
		mouseLeftDown = true;
		break;
	case WM_LBUTTONUP:
		mouseLeftDown = false;
		break;
	case WM_MOUSEMOVE: {//鼠标移动事件  		
		if (mouseLeftDown)
		{
			int x = LOWORD(lParam);//鼠标的横坐标  
			int y = HIWORD(lParam);//鼠标的纵坐标  
			float xDis = (float)(x - preX);
			float yDis = (float)(y - preY);
			VulkanManager::yAngle = VulkanManager::yAngle + xDis * 180 / 200;
			VulkanManager::zAngle = VulkanManager::zAngle + yDis * 180 / 200;
			preX = x;
			preY = y;
		}
		break;
	}
	default:
		break;
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

//初始化窗体的方法
void init_window(struct WindowInfo &info, int32_t default_width, int32_t default_height, HINSTANCE hInstance)
{
	info.width = default_width;
	info.height = default_height;
	assert(info.width > 0);
	assert(info.height > 0);

	//设置窗体标题栏
	sprintf_s(info.name, "Hello Vulkan");

	//初始化窗体结构
	WNDCLASSEX win_class;
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = hInstance;
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //(HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = info.name;
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	// 注册窗体
	if (!RegisterClassEx(&win_class))
	{
		// 注册失败，打印错误信息
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}

	// 生成窗体
	RECT wr = { 0, 0, info.width, info.height };
	//调整窗体矩形
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	info.window = CreateWindowEx(
		0,						//窗口的扩展风格
		info.name,				//指向注册类名的指针
		info.name,				//指向窗口名称的指针
		WS_OVERLAPPEDWINDOW |	//窗口的风格
		WS_VISIBLE | WS_SYSMENU,
		100, 100,				//窗体左上角坐标
		wr.right - wr.left,		//窗体宽度
		wr.bottom - wr.top,		//窗体高度
		NULL,					//父窗口的句柄
		NULL,					//菜单的句柄或是子窗口的标识符
		hInstance,		//应用程序实例的句柄 
		&info);					//指向窗口的创建数据


	if (!info.window)
	{
		// 生成失败，打印错误信息
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	//设置窗口的用户数据为窗体创建的info数据，已被需要时可以从窗体获取
	SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR)&info);

	ShowWindow(info.window, SW_SHOWNA);
	UpdateWindow(info.window);
}
//销毁窗体
void destroy_window(struct WindowInfo &info) {
	DestroyWindow(info.window);
}

//命令行窗口运行时的主方法
int main(int argc, char **argv)
{
	init_window(VulkanManager::info, 1366, 768, GetModuleHandle(NULL));
	VulkanManager::doVulkan();
	//建立消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		DispatchMessage(&msg);
	}
	destroy_window(VulkanManager::info);
	return (int)msg.wParam;
}