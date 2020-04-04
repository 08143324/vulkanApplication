#pragma once
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
#include "main_task.h"

#define FENCE_TIMEOUT 100000000
#define PathPre "G:/vulkan/"

#define VertShaderPath PathPre ## "shaders/commonTexLight_vert.spv"
#define FragShaderPath PathPre ## "shaders/commonTexLight_frag.spv"

class VulkanManager
{
public:
	static struct WindowInfo info;
	static float xAngle;
	static float yAngle;
	static float zAngle;
	static bool memoryTypeFromProperties(VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);
//	static void init_vulkan_instance();//����Vulkanʵ��
//	static void enumerate_vulkan_phy_devices();//��ʼ�������豸
//	static void create_vulkan_devices();//�����߼��豸
//	static void create_vulkan_CommandBuffer();//���������
//	static void create_vulkan_swapChain();//��ʼ��������
//	static void create_vulkan_DepthBuffer();//������Ȼ������
//	static void create_render_pass();//������Ⱦͨ��
//	static void init_queue();//��ȡ�豸��֧��ͼ�ι����Ķ���
//	static void create_frame_buffer();//����֡����
//	static void createDrawableObject();//��������������
//	static void drawObject();//ִ�г����е��������
	static void draw();//ִ��Vulkan����
	static void doVulkan();//ִ��Vulkan����
//	static void initPipeline();//��ʼ������
//	static void createFence();//����դ��
//	static void initPresentInfo();//��ʼ����ʾ��Ϣ
//	static void initMatrix();//��ʼ������
//	static void flushUniformBuffer();//��һ�±����������뻺��
//	static void flushTexToDesSet();//�����������������������
//	static void destroyFence();//����դ��
//	static void destroyPipeline();//���ٹ���
//	static void destroyDrawableObject();//���ٻ���������
//	static void destroy_frame_buffer();//����֡����
//	static void destroy_render_pass();//������Ⱦͨ��
//	static void destroy_vulkan_DepthBuffer();//������Ȼ������
//	static void destroy_vulkan_swapChain();//���ٽ�����
//	static void destroy_vulkan_CommandBuffer();//���������
//	static void destroy_vulkan_devices();//�����߼��豸
//	static void destroy_vulkan_instance();//����ʵ��
};