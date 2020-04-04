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

//�����¼��Ĵ�����
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	struct WindowInfo *info = reinterpret_cast<struct WindowInfo *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		break;
	case WM_LBUTTONDOWN://����������
		preX = LOWORD(lParam);//���ĺ�����  
		preY = HIWORD(lParam);//����������  
		mouseLeftDown = true;
		break;
	case WM_LBUTTONUP:
		mouseLeftDown = false;
		break;
	case WM_MOUSEMOVE: {//����ƶ��¼�  		
		if (mouseLeftDown)
		{
			int x = LOWORD(lParam);//���ĺ�����  
			int y = HIWORD(lParam);//����������  
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

//��ʼ������ķ���
void init_window(struct WindowInfo &info, int32_t default_width, int32_t default_height, HINSTANCE hInstance)
{
	info.width = default_width;
	info.height = default_height;
	assert(info.width > 0);
	assert(info.height > 0);

	//���ô��������
	sprintf_s(info.name, "Hello Vulkan");

	//��ʼ������ṹ
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

	// ע�ᴰ��
	if (!RegisterClassEx(&win_class))
	{
		// ע��ʧ�ܣ���ӡ������Ϣ
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}

	// ���ɴ���
	RECT wr = { 0, 0, info.width, info.height };
	//�����������
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	info.window = CreateWindowEx(
		0,						//���ڵ���չ���
		info.name,				//ָ��ע��������ָ��
		info.name,				//ָ�򴰿����Ƶ�ָ��
		WS_OVERLAPPEDWINDOW |	//���ڵķ��
		WS_VISIBLE | WS_SYSMENU,
		100, 100,				//�������Ͻ�����
		wr.right - wr.left,		//������
		wr.bottom - wr.top,		//����߶�
		NULL,					//�����ڵľ��
		NULL,					//�˵��ľ�������Ӵ��ڵı�ʶ��
		hInstance,		//Ӧ�ó���ʵ���ľ�� 
		&info);					//ָ�򴰿ڵĴ�������


	if (!info.window)
	{
		// ����ʧ�ܣ���ӡ������Ϣ
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}
	//���ô��ڵ��û�����Ϊ���崴����info���ݣ��ѱ���Ҫʱ���ԴӴ����ȡ
	SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR)&info);

	ShowWindow(info.window, SW_SHOWNA);
	UpdateWindow(info.window);
}
//���ٴ���
void destroy_window(struct WindowInfo &info) {
	DestroyWindow(info.window);
}

//�����д�������ʱ��������
int main(int argc, char **argv)
{
	init_window(VulkanManager::info, 1366, 768, GetModuleHandle(NULL));
	VulkanManager::doVulkan();
	//������Ϣѭ��
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		DispatchMessage(&msg);
	}
	destroy_window(VulkanManager::info);
	return (int)msg.wParam;
}