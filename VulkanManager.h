#pragma once
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
//	static void init_vulkan_instance();//创建Vulkan实例
//	static void enumerate_vulkan_phy_devices();//初始化物理设备
//	static void create_vulkan_devices();//创建逻辑设备
//	static void create_vulkan_CommandBuffer();//创建命令缓冲
//	static void create_vulkan_swapChain();//初始化交换链
//	static void create_vulkan_DepthBuffer();//创建深度缓冲相关
//	static void create_render_pass();//创建渲染通道
//	static void init_queue();//获取设备中支持图形工作的队列
//	static void create_frame_buffer();//创建帧缓冲
//	static void createDrawableObject();//创建绘制用物体
//	static void drawObject();//执行场景中的物体绘制
	static void draw();//执行Vulkan任务
	static void doVulkan();//执行Vulkan任务
//	static void initPipeline();//初始化管线
//	static void createFence();//创建栅栏
//	static void initPresentInfo();//初始化显示信息
//	static void initMatrix();//初始化矩阵
//	static void flushUniformBuffer();//将一致变量数据送入缓冲
//	static void flushTexToDesSet();//将纹理等数据与描述集关联
//	static void destroyFence();//销毁栅栏
//	static void destroyPipeline();//销毁管线
//	static void destroyDrawableObject();//销毁绘制用物体
//	static void destroy_frame_buffer();//销毁帧缓冲
//	static void destroy_render_pass();//销毁渲染通道
//	static void destroy_vulkan_DepthBuffer();//销毁深度缓冲相关
//	static void destroy_vulkan_swapChain();//销毁交换链
//	static void destroy_vulkan_CommandBuffer();//销毁命令缓冲
//	static void destroy_vulkan_devices();//销毁逻辑设备
//	static void destroy_vulkan_instance();//销毁实例
};