#ifndef VULKANEXBASE_MYVULKANMANAGER_H
#define VULKANEXBASE_MYVULKANMANAGER_H
#endif
#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")  /*windows--不带命令行输出窗口运行 console--带命令行输出窗口运行*/
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#endif
#include <vulkan/vulkan.h>
#include <vector>
#include <thread>
#include <assert.h>
#include <Windows.h>
#include "main_task.h"
#include "VulkanManager.h"
#include "FileUtil.h"
#include "FPSUtil.h"
#include "MatrixState3D.h"
#include "ThreadTask.h"

using namespace std;

struct WindowInfo VulkanManager::info;
float VulkanManager::xAngle = 0;
float VulkanManager::yAngle = 0;
float VulkanManager::zAngle = 0;

bool VulkanManager::memoryTypeFromProperties(VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
	//循环确定内存类型索引
	for (uint32_t i = 0; i < 32; i++)
	{
		//若对应类型比特位为1
		if ((typeBits & 1) == 1)
		{
			//此类型与掩码匹配
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	//没有找到所需的类型索引
	return false;
}

void VulkanManager::draw()
{
	//创建vulkan实例
	VkInstance instance;
	vector<char *> instanceExtensionNames;//初始化所需扩展列表，下列扩展是win32下做渲染必须要的
	instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);//此处移植Windows不需更改
	instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);//此处移植Windows需要获取VK_KHR_SURFACE_EXTENSION_NAME扩展

	VkApplicationInfo app_info = {};//构建应用信息结构体实例
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//结构体的类型
	app_info.pNext = NULL;//自定义数据的指针
	app_info.pApplicationName = "HelloVulkan";//应用的名称
	app_info.applicationVersion = 1;//应用的版本号
	app_info.pEngineName = "HelloVulkan";//应用的引擎名称
	app_info.engineVersion = 1;//应用的引擎版本号
	app_info.apiVersion = VK_API_VERSION_1_0;//使用的Vulkan图形应用程序API版本

	VkInstanceCreateInfo inst_info = {};//构建实例创建信息结构体实例
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;//结构体的类型
	inst_info.pNext = NULL;//自定义数据的指针
	inst_info.flags = 0;//供将来使用的标志
	inst_info.pApplicationInfo = &app_info;//绑定应用信息结构体
	inst_info.enabledExtensionCount = instanceExtensionNames.size();//扩展的数量
	inst_info.ppEnabledExtensionNames = instanceExtensionNames.data();//扩展名称列表数据
	inst_info.enabledLayerCount = 0;//启动的层数量
	inst_info.ppEnabledLayerNames = NULL;//启动的层名称列表

	VkResult result;//存储运行结果的辅助变量

	result = vkCreateInstance(&inst_info, NULL, &instance);//创建实例
	if (result == VK_SUCCESS)
	{
		printf("Vulkan实例创建成功!\n");
	}
	else
	{
		printf("Vulkan实例创建失败!\n");
	}

	//选择物理设备，这里选择第一个
	uint32_t gpuCount = 0;//存储物理设备数量的变量
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);//获取物理设备数量
	assert(result == VK_SUCCESS);
	printf("[Vulkan硬件设备数量为%d个]", gpuCount);
	vector<VkPhysicalDevice> gpus;
	VkPhysicalDeviceMemoryProperties memoryroperties;
	gpus.resize(gpuCount);//设置物理设备列表尺寸
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());//填充物理设备列表
	assert(result == VK_SUCCESS);
	vkGetPhysicalDeviceMemoryProperties(gpus[0], &memoryroperties);//获取第一物理设备的内存属性

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, NULL);//获取物理设备0中队列家族的数量
	printf("[Vulkan硬件设备0支持的队列家族数量为%d]\n", queueFamilyCount);
	vector<VkQueueFamilyProperties> queueFamilyprops;
	queueFamilyprops.resize(queueFamilyCount);//随队列家族数量改变vector长度
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, queueFamilyprops.data());//填充物理设备0队列家族属性列表
	printf("[成功获取Vulkan硬件设备0支持的队列家族属性列表]\n");

	//创建逻辑设备
	VkDevice device;
	VkDeviceQueueCreateInfo queueInfo = {};//构建设备队列创建信息结构体实例
	uint32_t queueGraphicsFamilyIndex = 0;
	bool found = false;//辅助标志
	for (unsigned int i = 0; i < queueFamilyCount; i++) {//遍历所有队列家族
		if (queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {//若当前队列家族支持图形工作
			queueInfo.queueFamilyIndex = i;//绑定此队列家族索引
			queueGraphicsFamilyIndex = i;//记录支持图形工作的队列家族索引
			printf("[支持GRAPHICS工作的一个队列家族的索引为%d]\n", i);
			printf("[此家族中的实际队列数量是%d]\n", queueFamilyprops[i].queueCount);
			found = true;
			break;
		}
	}

	float queue_priorities[1] = { 0.0 };//创建队列优先级数组
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;//给出结构体类型
	queueInfo.pNext = NULL;//自定义数据的指针
	queueInfo.queueCount = 1;//指定队列数量
	queueInfo.pQueuePriorities = queue_priorities;//给出每个队列的优先级
	queueInfo.queueFamilyIndex = queueGraphicsFamilyIndex;//绑定队列家族索引
	
	vector<const char *> deviceExtensionNames;
	deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);//设置所需扩展

	VkDeviceCreateInfo deviceInfo = {};//构建逻辑设备创建信息结构体实例
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;//给出结构体类型
	deviceInfo.pNext = NULL;//自定义数据的指针
	deviceInfo.queueCreateInfoCount = 1;//指定设备队列创建信息结构体数量
	deviceInfo.pQueueCreateInfos = &queueInfo;//给定设备队列创建信息结构体列表
	deviceInfo.enabledExtensionCount = deviceExtensionNames.size();//所需扩展数量
	deviceInfo.ppEnabledExtensionNames = deviceExtensionNames.data();//所需扩展列表
	deviceInfo.enabledLayerCount = 0;//需启动Layer的数量
	deviceInfo.ppEnabledLayerNames = NULL;//需启动Layer的名称列表
	deviceInfo.pEnabledFeatures = NULL;//启用的设备特性
	result = vkCreateDevice(gpus[0], &deviceInfo, NULL, &device);//创建逻辑设备
	assert(result == VK_SUCCESS);//检查逻辑设备是否创建成功

	//创建命令缓冲区
	VkCommandPool cmdPool;
	VkCommandPoolCreateInfo cmd_pool_info = {};//构建命令池创建信息结构体实例
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; 	//给定结构体类型
	cmd_pool_info.pNext = NULL;//自定义数据的指针
	cmd_pool_info.queueFamilyIndex = queueGraphicsFamilyIndex;//绑定所需队列家族索引
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	//执行控制标志
	result = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmdPool);//创建命令池
	assert(result == VK_SUCCESS);//检查命令池创建是否成功

	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBAI = {};//构建命令缓冲分配信息结构体实例
	cmdBAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;//给定结构体类型
	cmdBAI.pNext = NULL;//自定义数据的指针
	cmdBAI.commandPool = cmdPool;//指定命令池
	cmdBAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//分配的命令缓冲级别
	cmdBAI.commandBufferCount = 1;//分配的命令缓冲数量
	result = vkAllocateCommandBuffers(device, &cmdBAI, &cmdBuffer);//分配命令缓冲
	assert(result == VK_SUCCESS);//检查分配是否成功

	VkCommandBufferBeginInfo cmd_buf_info;
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;//给定结构体类型
	cmd_buf_info.pNext = NULL;//自定义数据的指针
	cmd_buf_info.flags = 0;//描述使用标志
	cmd_buf_info.pInheritanceInfo = NULL;//命令缓冲继承信息

	VkCommandBuffer cmd_bufs[1];
	cmd_bufs[0] = cmdBuffer;//要提交到队列执行的命令缓冲数组

	VkPipelineStageFlags* pipe_stage_flags = new VkPipelineStageFlags();//目标管线阶段
	*pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info[1];
	submit_info[0].pNext = NULL;//自定义数据的指针
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;//给定结构体类型
	submit_info[0].pWaitDstStageMask = pipe_stage_flags;//给定目标管线阶段
	submit_info[0].commandBufferCount = 1;//命令缓冲数量
	submit_info[0].pCommandBuffers = cmd_bufs;//提交的命令缓冲数组
	submit_info[0].signalSemaphoreCount = 0;//信号量数量
	submit_info[0].pSignalSemaphores = NULL;//信号量数组

	VkQueue queueGraphics;
	vkGetDeviceQueue(device, queueGraphicsFamilyIndex, 0, &queueGraphics);


	//构建KHR表面创建信息结构体实例
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;//给定结构体类型
	createInfo.pNext = nullptr;//自定义数据的指针
	createInfo.flags = 0;//供未来使用的标志
	createInfo.hwnd = info.window;//给定窗体

	//创建KHR表面
	VkSurfaceKHR surface;
	result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
	assert(result == VK_SUCCESS);

	//遍历设备对应的队列家族列表
	VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queueFamilyCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
		printf("队列家族索引=%d %s显示\n", i, (pSupportsPresent[i] == 1 ? "支持" : "不支持"));
	}

	queueGraphicsFamilyIndex = UINT32_MAX;//支持图形工作的队列家族索引
	uint32_t queuePresentFamilyIndex = UINT32_MAX;//支持显示(呈现)工作的队列家族索引
	for (uint32_t i = 0; i <queueFamilyCount; ++i)//遍历设备对应的队列家族列表
	{
		//如果当前遍历到的队列家族支持Graphis（图形）工作
		if ((queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)//若此队列家族支持图形工作
		{
			//记录支持Graphis（图形）工作的队列家族索引
			if (queueGraphicsFamilyIndex == UINT32_MAX) queueGraphicsFamilyIndex = i;
			//如果当前遍历到的队列家族支持Present（显示工作）工作
			if (pSupportsPresent[i] == VK_TRUE)//如果当前队列家族支持显示工作
			{
				queueGraphicsFamilyIndex = i;//记录此队列家族索引为支持图形工作的
				queuePresentFamilyIndex = i;//记录此队列家族索引为支持显示工作的
				printf("队列家族索引=%d 同时支持Graphis（图形）和Present（显示）工作\n", i);
				break;
			}
		}
	}

	if (queuePresentFamilyIndex == UINT32_MAX)//若没有找到同时支持两项工作的队列家族
	{
		for (size_t i = 0; i < queueFamilyCount; ++i)//遍历设备对应的队列家族列表
		{
			if (pSupportsPresent[i] == VK_TRUE)//判断是否支持显示工作
			{
				queuePresentFamilyIndex = i;//记录此队列家族索引为支持显示工作的
				break;
			}
		}
	}
	free(pSupportsPresent);//释放存储是否支持呈现工作的布尔值列表

						   //没有找到支持Graphis（图形）或Present（显示）工作的队列家族
						   //没有找到支持图形或显示工作的队列家族
	if (queueGraphicsFamilyIndex == UINT32_MAX || queuePresentFamilyIndex == UINT32_MAX)
	{
		printf("没有找到支持Graphis（图形）或Present（显示）工作的队列家族\n");
		assert(false);//若没有支持图形或显示操作的队列家族则程序终止
	}

	uint32_t formatCount;//支持的格式数量
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, NULL);//获取支持的格式数量
	printf("支持的格式数量为 %d\n", formatCount);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));//分配对应数量的空间
	vector<VkFormat> formats;
	formats.resize(formatCount);//调整对应Vector尺寸
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, surfFormats);//获取支持的格式信息
	for (unsigned int i = 0; i<formatCount; i++) {//记录支持的格式信息
		formats[i] = surfFormats[i].format;
		printf("[%d]支持的格式为%d\n", i, formats[i]);
	}
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {//特殊情况处理
		formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
	}
	free(surfFormats);

	//获取KHR表面的能力
	VkSurfaceCapabilitiesKHR surfCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpus[0], surface, &surfCapabilities);
	assert(result == VK_SUCCESS);

	//获取支持的显示模式数量
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, NULL);
	assert(result == VK_SUCCESS);
	printf("显示模式数量为%d\n", presentModeCount);

	//调整对应Vector尺寸
	vector<VkPresentModeKHR> presentModes;
	presentModes.resize(presentModeCount);
	//获取支持的显示模式列表
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, presentModes.data());
	for (unsigned int i = 0; i<presentModeCount; i++)
	{
		printf("显示模式[%d]编号为%d\n", i, presentModes[i]);
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;//确定交换链显示模式
	for (size_t i = 0; i < presentModeCount; i++)//遍历显示模式列表
	{
		//如果也支持VK_PRESENT_MODE_MAILBOX_KHR模式，由于其效率高，便选用
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			//如果没能用上VK_PRESENT_MODE_MAILBOX_KHR模式，但有VK_PRESENT_MODE_IMMEDIATE_KHR模式
			//也比VK_PRESENT_MODE_FIFO_KHR模式强，故选用
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//确定surface的宽度、高度
	//如果surface能力中的尺寸没有定义（宽度为0xFFFFFFFF表示没定义）
	VkExtent2D swapchainExtent;
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchainExtent.width = 1366;//设置宽度为窗体宽度
		swapchainExtent.height = 768;//设置高度为窗体高度
											  //宽度设置值限制到最大值与最小值之间
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}
		//高度设置值限制到最大值与最小值之间
		if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
		printf("使用自己设置的 宽度 %d 高度 %d\n", swapchainExtent.width, swapchainExtent.height);
	}
	else
	{
		//若表面有确定尺寸
		swapchainExtent = surfCapabilities.currentExtent;
		printf("使用获取的surface能力中的 宽度 %d 高度 %d\n", swapchainExtent.width, swapchainExtent.height);
	}

	uint32_t screenWidth = swapchainExtent.width;//记录实际采用的宽度
	uint32_t screenHeight = swapchainExtent.height;//记录实际采用的高度

										  //期望交换链中的最少图像数量
	uint32_t desiredMinNumberOfSwapChainImages = surfCapabilities.minImageCount + 1;
	//将数量限制到范围内
	if ((surfCapabilities.maxImageCount > 0) && (desiredMinNumberOfSwapChainImages > surfCapabilities.maxImageCount))
	{
		desiredMinNumberOfSwapChainImages = surfCapabilities.maxImageCount;
	}

	//KHR表面变换标志
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)//若支持所需的变换
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else//若不支持变换
	{
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};//构建交换链创建信息结构体实例
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;//结构体类型
	swapchain_ci.pNext = NULL;//自定义数据的指针
	swapchain_ci.surface = surface;//指定KHR表面
	swapchain_ci.minImageCount = desiredMinNumberOfSwapChainImages;//最少图像数量
	swapchain_ci.imageFormat = formats[0];//图像格式
	swapchain_ci.imageExtent.width = swapchainExtent.width;//交换链图像宽度
	swapchain_ci.imageExtent.height = swapchainExtent.height;//交换链图像高度
	swapchain_ci.preTransform = preTransform;//指定变换标志
	swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//混合Alpha值
	swapchain_ci.imageArrayLayers = 1;//图像数组层数
	swapchain_ci.presentMode = swapchainPresentMode;//交换链的显示模式
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;//前导交换链
	swapchain_ci.clipped = true;//开启剪裁
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;//色彩空间
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//图像用途
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	//图像共享模式
	swapchain_ci.queueFamilyIndexCount = 0;//队列家族数量
	swapchain_ci.pQueueFamilyIndices = NULL;//队列家族索引列表

	if (queueGraphicsFamilyIndex != queuePresentFamilyIndex)//若支持图形和显示工作的队列家族不相同
	{
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_ci.queueFamilyIndexCount = 2;//交换链所需的队列家族索引数量为2
		uint32_t queueFamilyIndices[2] = { queueGraphicsFamilyIndex,queuePresentFamilyIndex };
		swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;//交换链所需的队列家族索引列表
	}

	VkSwapchainKHR swapChain;
	result = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swapChain);//创建交换链
	assert(result == VK_SUCCESS);//检查交换链是否创建成功

								 //获取交换链中的图像数量
	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, NULL);
	assert(result == VK_SUCCESS);
	printf("[SwapChain中的Image数量为%d]\n", swapchainImageCount);//检查是否获取成功
	vector<VkImage> swapchainImages;
	swapchainImages.resize(swapchainImageCount);//调整图像列表尺寸
												//获取交换链中的图像列表
	result = vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, swapchainImages.data());
	assert(result == VK_SUCCESS);
	vector<VkImageView> swapchainImageViews;
	swapchainImageViews.resize(swapchainImageCount);//调整图像视图列表尺寸
	for (uint32_t i = 0; i < swapchainImageCount; i++)//为交换链中的各幅图像创建图像视图
	{
		VkImageViewCreateInfo color_image_view = {};//构建图像视图创建信息结构体实例
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//设置结构体类型
		color_image_view.pNext = NULL;//自定义数据的指针
		color_image_view.flags = 0;//供将来使用的标志
		color_image_view.image = swapchainImages[i];//对应交换链图像
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;//图像视图的类型
		color_image_view.format = formats[0];//图像视图格式
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;//设置R通道调和
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;//设置G通道调和
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;//设置B通道调和
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;//设置A通道调和
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//图像视图使用方面
		color_image_view.subresourceRange.baseMipLevel = 0;//基础Mipmap级别
		color_image_view.subresourceRange.levelCount = 1;//Mipmap级别的数量
		color_image_view.subresourceRange.baseArrayLayer = 0;//基础数组层
		color_image_view.subresourceRange.layerCount = 1;//数组层的数量
		result = vkCreateImageView(device, &color_image_view, NULL, &swapchainImageViews[i]);//创建图像视图
		assert(result == VK_SUCCESS);//检查是否创建成功
	}

	VkFormat depthFormat = VK_FORMAT_D16_UNORM;//指定深度图像的格式
	VkFormatProperties depthFormatProps;
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory memDepth;
	VkImageCreateInfo image_info = {};//构建深度图像创建信息结构体实例
	vkGetPhysicalDeviceFormatProperties(gpus[0], depthFormat, &depthFormatProps);//获取物理设备支持的指定格式的属性
																				 //确定平铺方式
	if (depthFormatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//是否支持线性瓦片组织方式
	{
		image_info.tiling = VK_IMAGE_TILING_LINEAR;//采用线性瓦片组织方式
		printf("tiling为VK_IMAGE_TILING_LINEAR！\n");
	}
	else if (depthFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//是否支持最优瓦片组织方式
	{
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;//采用最优瓦片组织方式
		printf("tiling为VK_IMAGE_TILING_OPTIMAL！\n");
	}
	else
	{
		printf("不支持VK_FORMAT_D16_UNORM！\n");//打印不支持指定格式的提示信息
	}
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;	//指定结构体类型
	image_info.pNext = NULL;//自定义数据的指针
	image_info.imageType = VK_IMAGE_TYPE_2D;//图像类型
	image_info.format = depthFormat;//图像格式
	image_info.extent.width = screenWidth;//图像宽度
	image_info.extent.height = screenHeight;//图像高度
	image_info.extent.depth = 1;//图像深度
	image_info.mipLevels = 1;//图像mipmap级数
	image_info.arrayLayers = 1;//图像数组层数量
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;//采样模式
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//初始布局
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;//图像用途
	image_info.queueFamilyIndexCount = 0;//队列家族数量
	image_info.pQueueFamilyIndices = NULL;//队列家族索引列表
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//共享模式
	image_info.flags = 0;//标志

	VkMemoryAllocateInfo mem_alloc = {};//构建内存分配信息结构体实例
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//结构体类型
	mem_alloc.pNext = NULL;//自定义数据的指针
	mem_alloc.allocationSize = 0;//分配的内存字节数
	mem_alloc.memoryTypeIndex = 0;//内存的类型索引

	VkImageViewCreateInfo view_info = {};//构建深度图像视图创建信息结构体实例
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//设置结构体类型
	view_info.pNext = NULL;//自定义数据的指针
	view_info.image = VK_NULL_HANDLE;//对应的图像
	view_info.format = depthFormat;//图像视图的格式
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;//设置R通道调和
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;//设置G通道调和
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;//设置B通道调和
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;//设置A通道调和
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;//图像视图使用方面
	view_info.subresourceRange.baseMipLevel = 0;//基础Mipmap级别
	view_info.subresourceRange.levelCount = 1;//Mipmap级别的数量
	view_info.subresourceRange.baseArrayLayer = 0;//基础数组层
	view_info.subresourceRange.layerCount = 1;//数组层的数量
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;//图像视图的类型
	view_info.flags = 0;//标志

	result = vkCreateImage(device, &image_info, NULL, &depthImage);//创建深度图像
	assert(result == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;//获取图像内存需求
	vkGetImageMemoryRequirements(device, depthImage, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;//获取所需内存字节数
	VkFlags requirements_mask = 0;//需要的内存类型掩码
	bool flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &mem_alloc.memoryTypeIndex);//获取所需内存类型索引
	assert(flag);//检查获取是否成功
	printf("确定内存类型成功 类型索引为%d\n", mem_alloc.memoryTypeIndex);
	result = vkAllocateMemory(device, &mem_alloc, NULL, &memDepth);	//分配内存
	assert(result == VK_SUCCESS);
	result = vkBindImageMemory(device, depthImage, memDepth, 0);//绑定图像和内存
	assert(result == VK_SUCCESS);
	view_info.image = depthImage;//指定图像视图对应图像
	result = vkCreateImageView(device, &view_info, NULL, &depthImageView);//创建深度图像视图
	assert(result == VK_SUCCESS);

	VkSemaphore imageAcquiredSemaphore;
	VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;		//构建信号量创建信息结构体实例
	imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;//结构体类型
	imageAcquiredSemaphoreCreateInfo.pNext = NULL;//自定义数据的指针
	imageAcquiredSemaphoreCreateInfo.flags = 0;//供将来使用的标志

	result = vkCreateSemaphore(device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);//创建信号量
	assert(result == VK_SUCCESS);//检测信号量是否创建成功

	VkAttachmentDescription attachments[2];//附件描述信息数组
	attachments[0].format = formats[0];//设置颜色附件的格式
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;//设置采样模式
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//加载时对附件的操作
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;//存储时对附件的操作
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//模板加载时对附件的操作
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//模板存储时对附件的操作
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//初始的布局
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//结束时的最终布局
	attachments[0].flags = 0;//设置位掩码
	attachments[1].format = depthFormat;//设置深度附件的格式
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;//设置采样模式
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//加载时对附件的操作
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//存储时对附件的操作
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//模板加载时对附件的操作
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//模板存储时对附件的操作
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 	//初始的布局
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//结束时的布局
	attachments[1].flags = 0;//设置位掩码

	VkAttachmentReference color_reference = {};//颜色附件引用
	color_reference.attachment = 0;//对应附件描述信息数组下标
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//设置附件布局

	VkAttachmentReference depth_reference = {};//深度附件引用
	depth_reference.attachment = 1;//对应附件描述信息数组下标
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//设置附件布局

	VkSubpassDescription subpass = {};//构建渲染子通道描述结构体实例
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//设置管线绑定点
	subpass.flags = 0;//设置掩码
	subpass.inputAttachmentCount = 0;//输入附件数量
	subpass.pInputAttachments = NULL;//输入附件列表
	subpass.colorAttachmentCount = 1;//颜色附件数量
	subpass.pColorAttachments = &color_reference;//颜色附件列表
	subpass.pResolveAttachments = NULL;//Resolve附件
	subpass.pDepthStencilAttachment = &depth_reference;//深度模板附件
	subpass.preserveAttachmentCount = 0;//preserve附件数量
	subpass.pPreserveAttachments = NULL;//preserve附件列表

	VkRenderPass renderPass;
	VkRenderPassCreateInfo rp_info = {};//构建渲染通道创建信息结构体实例
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;//结构体类型
	rp_info.pNext = NULL;//自定义数据的指针
	rp_info.attachmentCount = 2;//附件的数量
	rp_info.pAttachments = attachments;//附件列表
	rp_info.subpassCount = 1;//渲染子通道数量
	rp_info.pSubpasses = &subpass;//渲染子通道列表
	rp_info.dependencyCount = 0;//子通道依赖数量
	rp_info.pDependencies = NULL;//子通道依赖列表

	result = vkCreateRenderPass(device, &rp_info, NULL, &renderPass);//创建渲染通道
	assert(result == VK_SUCCESS);

	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.2f;//帧缓冲清除用R分量值
	clear_values[0].color.float32[1] = 0.2f;//帧缓冲清除用G分量值
	clear_values[0].color.float32[2] = 0.2f;//帧缓冲清除用B分量值
	clear_values[0].color.float32[3] = 0.2f;//帧缓冲清除用A分量值
	clear_values[1].depthStencil.depth = 1.0f;//帧缓冲清除用深度值
	clear_values[1].depthStencil.stencil = 0;//帧缓冲清除用模板值

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;//渲染通道启动信息结构体类型
	rp_begin.pNext = NULL;//自定义数据的指针
	rp_begin.renderPass = renderPass;//指定要启动的渲染通道
	rp_begin.renderArea.offset.x = 0;//渲染区域起始X坐标
	rp_begin.renderArea.offset.y = 0;//渲染区域起始Y坐标
	rp_begin.renderArea.extent.width = screenWidth;//渲染区域宽度
	rp_begin.renderArea.extent.height = screenHeight;//渲染区域高度
	rp_begin.clearValueCount = 2;//帧缓冲清除值数量
	rp_begin.pClearValues = clear_values;//帧缓冲清除值数组

	VkImageView view_attachments[2];//附件图像视图数组
	view_attachments[1] = depthImageView;//给定深度图像视图

	VkFramebufferCreateInfo fb_info = {};//构建帧缓冲创建信息结构体实例
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;//结构体类型
	fb_info.pNext = NULL;//自定义数据的指针
	fb_info.renderPass = renderPass;//指定渲染通道
	fb_info.attachmentCount = 2;//附件数量
	fb_info.pAttachments = view_attachments;//附件图像视图数组
	fb_info.width = screenWidth;//宽度
	fb_info.height = screenHeight;//高度
	fb_info.layers = 1;//层数

	uint32_t i;//循环控制变量
	VkFramebuffer* framebuffers;
	framebuffers = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));//为帧缓冲序列动态分配内存
	assert(framebuffers);//检查内存分配是否成功
						 //遍历交换链中的各个图像
	for (i = 0; i < swapchainImageCount; i++)
	{
		view_attachments[0] = swapchainImageViews[i];//给定颜色附件对应图像视图
		VkResult result = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);//创建帧缓冲
		assert(result == VK_SUCCESS);//检查是否创建成功
		printf("[创建帧缓冲%d成功！]\n", i);
	}

	VkBufferCreateInfo buf_info = {};//构建一致变量缓冲创建信息结构体实例
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;	//结构体的类型
	buf_info.pNext = NULL;//自定义数据的指针
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;//缓冲的用途
	buf_info.size = 64;//缓冲总字节数
	buf_info.queueFamilyIndexCount = 0;	//队列家族数量
	buf_info.pQueueFamilyIndices = NULL;//队列家族索引列表
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//共享模式
	buf_info.flags = 0;//标志

	VkBuffer uniformBuf;
	result = vkCreateBuffer(device, &buf_info, NULL, &uniformBuf);//创建一致变量缓冲
	assert(result == VK_SUCCESS);//检查创建是否成功

	vkGetBufferMemoryRequirements(device, uniformBuf, &mem_reqs);//获取此缓冲的内存需求

	VkMemoryAllocateInfo ubo_alloc_info = {};//构建内存分配信息结构体实例
	ubo_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//结构体类型
	ubo_alloc_info.pNext = NULL;//自定义数据的指针
	ubo_alloc_info.memoryTypeIndex = 0;//内存类型索引
	ubo_alloc_info.allocationSize = mem_reqs.size;//缓冲内存分配字节数

	requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;//需要的内存类型掩码
	flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &ubo_alloc_info.memoryTypeIndex);	//获取所需内存类型索引
	if (flag) { printf("确定内存类型成功 类型索引为%d", ubo_alloc_info.memoryTypeIndex); }
	else { printf("确定内存类型失败!"); }

	VkDeviceMemory memUniformBuf;//一致变量缓冲内存
	result = vkAllocateMemory(device, &ubo_alloc_info, NULL, &memUniformBuf);//分配内存
	assert(result == VK_SUCCESS);//检查内存分配是否成功
	result = vkBindBufferMemory(device, uniformBuf, memUniformBuf, 0);//将内存和对应缓冲绑定
	assert(result == VK_SUCCESS);//检查绑定操作是否成功

	VkDescriptorBufferInfo uniformBufferInfo;//一致变量缓冲描述信息
	uniformBufferInfo.buffer = uniformBuf;//指定一致变量缓冲
	uniformBufferInfo.offset = 0;//起始偏移量
	uniformBufferInfo.range = 16 * sizeof(float);//一致变量缓冲总字节数

	//配置描述符集
	VkDescriptorPoolSize type_count[1];//描述集池尺寸实例数组
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//描述类型
	type_count[0].descriptorCount = 1;//描述数量

	VkDescriptorPool descPool;
	VkDescriptorPoolCreateInfo descriptor_pool = {};//构建描述集池创建信息结构体实例
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;//结构体类型
	descriptor_pool.pNext = NULL;//自定义数据的指针
	descriptor_pool.maxSets = 1;//描述集最大数量
	descriptor_pool.poolSizeCount = 1;//描述集池尺寸实例数量
	descriptor_pool.pPoolSizes = type_count;//描述集池尺寸实例数组
	
	result = vkCreateDescriptorPool(device, &descriptor_pool, NULL, &descPool);//创建描述集池
	assert(result == VK_SUCCESS);//检查描述集池创建是否成功
	printf("描述集池创建成功!");

	VkDescriptorSetLayout descLayouts;
	VkDescriptorSetLayoutBinding layout_bindings[1];//描述集布局绑定数组
	layout_bindings[0].binding = 0;//此绑定的绑定点编号
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//描述类型
	layout_bindings[0].descriptorCount = 1;//描述数量
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	//目标着色器阶段
	layout_bindings[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};	//构建描述集布局创建信息结构体实例
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;//结构体类型
	descriptor_layout.pNext = NULL;//自定义数据的指针
	descriptor_layout.bindingCount = 1;//描述集布局绑定的数量
	descriptor_layout.pBindings = layout_bindings;//描述集布局绑定数组

	result = vkCreateDescriptorSetLayout(device, &descriptor_layout, NULL, &descLayouts);//创建描述集布局
	assert(result == VK_SUCCESS);//检查描述集布局创建是否成功
	VkDescriptorSetAllocateInfo alloc_info[1];//构建描述集分配信息结构体实例数组
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;//结构体类型
	alloc_info[0].pNext = NULL;//自定义数据的指针
	alloc_info[0].descriptorPool = descPool;//指定描述集池
	alloc_info[0].descriptorSetCount = 1;//描述集数量
	alloc_info[0].pSetLayouts = &descLayouts;//描述集布局列表

	VkDescriptorSet descSet;
	result = vkAllocateDescriptorSets(device, alloc_info, &descSet);//分配描述集
	assert(result == VK_SUCCESS);//检查描述集分配是否成功

	VkWriteDescriptorSet writes[1];
	writes[0] = {}; //完善一致变量写入描述集实例数组
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;//结构体类型
	writes[0].pNext = NULL;	//自定义数据的指针
	writes[0].descriptorCount = 1;//描述数量
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//描述类型
	writes[0].pBufferInfo = &uniformBufferInfo;//对应一致变量缓冲的信息
	writes[0].dstArrayElement = 0;//目标数组起始元素
	writes[0].dstBinding = 0;//目标绑定编号


	uint32_t dataByteCount = 18 * sizeof(float);//数据所占内存总字节数
	float *vdata = new float[18]{ //数据数组
		0, 75, 0,
		1, 0, 0,
		-45, 0, 0,
		0, 1, 0,
		45, 0, 0,
		0, 0, 1
	};

	VkBuffer vertexDatabuf;
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;//设置结构体类型
	buf_info.pNext = NULL;//自定义数据的指针
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;//缓冲的用途为顶点数据
	buf_info.size = dataByteCount;//设置数据总字节数
	buf_info.queueFamilyIndexCount = 0;//队列家族数量
	buf_info.pQueueFamilyIndices = NULL;//队列家族索引列表
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//共享模式
	buf_info.flags = 0;//标志

	result = vkCreateBuffer(device, &buf_info, NULL, &vertexDatabuf);//创建缓冲
	assert(result == VK_SUCCESS);//检查缓冲创建是否成功

	vkGetBufferMemoryRequirements(device, vertexDatabuf, &mem_reqs);//获取缓冲内存需求
	assert(dataByteCount <= mem_reqs.size);//检查内存需求获取是否正确

	VkMemoryAllocateInfo vb_alloc_info = {};//构建内存分配信息结构体实例
	vb_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//结构体类型
	vb_alloc_info.pNext = NULL;//自定义数据的指针
	vb_alloc_info.memoryTypeIndex = 0;//内存类型索引
	vb_alloc_info.allocationSize = mem_reqs.size;//内存总字节数

	requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;//需要的内存类型掩码
	flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &vb_alloc_info.memoryTypeIndex);//获取所需内存类型索引
	if (flag)
	{
		printf("确定内存类型成功 类型索引为%d", vb_alloc_info.memoryTypeIndex);
	}
	else
	{
		printf("确定内存类型失败!");
	}
	VkDeviceMemory vertexDataMem;
	result = vkAllocateMemory(device, &vb_alloc_info, NULL, &vertexDataMem);//为顶点数据缓冲分配内存
	assert(result == VK_SUCCESS);

	uint8_t *pData;//CPU访问时的辅助指针
	result = vkMapMemory(device, vertexDataMem, 0, mem_reqs.size, 0, (void **)&pData);//将设备内存映射为CPU可访问
	assert(result == VK_SUCCESS);//检查映射是否成功
	memcpy(pData, vdata, dataByteCount);//将顶点数据拷贝进设备内存
	vkUnmapMemory(device, vertexDataMem);//解除内存映射
	result = vkBindBufferMemory(device, vertexDatabuf, vertexDataMem, 0);//绑定内存与缓冲
	assert(result == VK_SUCCESS);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};//构建管线布局创建信息结构体实例
	VkPipelineLayout pipelineLayout;
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;	//结构体类型
	pPipelineLayoutCreateInfo.pNext = NULL;//自定义数据的指针
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;//推送常量范围的数量
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;//推送常量范围的列表
	pPipelineLayoutCreateInfo.setLayoutCount = 1;//描述集布局的数量
	pPipelineLayoutCreateInfo.pSetLayouts = &descLayouts;//描述集布局列表

	result = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);//创建管线布局
	assert(result == VK_SUCCESS);//检查创建是否成功

	VkDynamicState dynamicStateEnables[9];//动态状态启用标志
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);	//设置所有标志为false

	VkPipelineDynamicStateCreateInfo dynamicState = {};//管线动态状态创建信息
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;//结构体类型
	dynamicState.pNext = NULL;//自定义数据的指针
	dynamicState.pDynamicStates = dynamicStateEnables;//动态状态启用标志数组
	dynamicState.dynamicStateCount = 0;//启用的动态状态项数量

	VkVertexInputBindingDescription vertexBinding;//管线的顶点输入数据绑定描述
	vertexBinding.binding = 0;//对应绑定点
	vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	//数据输入频率为每顶点
	vertexBinding.stride = sizeof(float) * 6;//每组数据的跨度字节数

	VkVertexInputAttributeDescription vertexAttribs[2];//管线的顶点输入属性描述
	vertexAttribs[0].binding = 0;//第1个顶点输入属性的绑定点
	vertexAttribs[0].location = 0;//第1个顶点输入属性的位置索引
	vertexAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;//第1个顶点输入属性的数据格式
	vertexAttribs[0].offset = 0;//第1个顶点输入属性的偏移量

	vertexAttribs[1].binding = 0;//第2个顶点输入属性的绑定点
	vertexAttribs[1].location = 1;//第2个顶点输入属性的位置索引
	vertexAttribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;//第2个顶点输入属性的数据格式
	vertexAttribs[1].offset = 12;//第2个顶点输入属性的偏移量

	VkPipelineVertexInputStateCreateInfo vi;//管线顶点数据输入状态创建信息
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;//自定义数据的指针
	vi.flags = 0;//供将来使用的标志
	vi.vertexBindingDescriptionCount = 1;//顶点输入绑定描述数量
	vi.pVertexBindingDescriptions = &vertexBinding;//顶点输入绑定描述列表
	vi.vertexAttributeDescriptionCount = 2;//顶点输入属性数量
	vi.pVertexAttributeDescriptions = vertexAttribs;//顶点输入属性描述列表

	VkPipelineInputAssemblyStateCreateInfo ia;//管线图元组装状态创建信息
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.pNext = NULL;//自定义数据的指针
	ia.flags = 0;//供将来使用的标志
	ia.primitiveRestartEnable = VK_FALSE;//关闭图元重启
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	//采用三角形图元列表模式

	VkPipelineRasterizationStateCreateInfo rs;//管线光栅化状态创建信息
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.pNext = NULL;//自定义数据的指针
	rs.flags = 0;//供将来使用的标志
	rs.polygonMode = VK_POLYGON_MODE_FILL;//绘制方式为填充
	rs.cullMode = VK_CULL_MODE_NONE;//不使用背面剪裁
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	//卷绕方向为逆时针
	rs.depthClampEnable = VK_TRUE;//深度截取
	rs.rasterizerDiscardEnable = VK_FALSE;//启用光栅化操作（若为TRUE则光栅化不产生任何片元）
	rs.depthBiasEnable = VK_FALSE;//不启用深度偏移
	rs.depthBiasConstantFactor = 0;	//深度偏移常量因子
	rs.depthBiasClamp = 0;//深度偏移值上下限（若为正作为上限，为负作为下限）
	rs.depthBiasSlopeFactor = 0;//深度偏移斜率因子
	rs.lineWidth = 1.0f;//线宽度（仅在线绘制模式起作用）

	VkPipelineColorBlendAttachmentState att_state[1];//管线颜色混合附件状态数组
	att_state[0].colorWriteMask = 0xf;//设置写入掩码
	att_state[0].blendEnable = VK_FALSE;//关闭混合
	att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;//设置Alpha通道混合方式
	att_state[0].colorBlendOp = VK_BLEND_OP_ADD;//设置RGB通道混合方式
	att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;//设置源颜色混合因子
	att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;//设置目标颜色混合因子
	att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;//设置源Alpha混合因子
	att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;//设置目标Alpha混合因子


	VkPipelineColorBlendStateCreateInfo cb;//管线的颜色混合状态创建信息
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.pNext = NULL;//自定义数据的指针
	cb.flags = 0;//供未来使用的标志
	cb.attachmentCount = 1;	//颜色混合附件数量
	cb.pAttachments = att_state;//颜色混合附件列表
	cb.logicOpEnable = VK_FALSE;//不启用逻辑操作
	cb.logicOp = VK_LOGIC_OP_NO_OP;//逻辑操作类型为无
	cb.blendConstants[0] = 1.0f;//混合常量R分量
	cb.blendConstants[1] = 1.0f;//混合常量G分量
	cb.blendConstants[2] = 1.0f;//混合常量B分量
	cb.blendConstants[3] = 1.0f;//混合常量A分量

	VkViewport viewports;//视口信息
	viewports.minDepth = 0.0f;//视口最小深度
	viewports.maxDepth = 1.0f;//视口最大深度
	viewports.x = 0;//视口X坐标
	viewports.y = 0;//视口Y坐标
	viewports.width = 1366;//视口宽度
	viewports.height = 768;//视口高度

	VkRect2D scissor;//剪裁窗口信息
	scissor.extent.width = 1366;//剪裁窗口的宽度
	scissor.extent.height = 768;//剪裁窗口的高度
	scissor.offset.x = 0;//剪裁窗口的X坐标
	scissor.offset.y = 0;//剪裁窗口的Y坐标

	VkPipelineViewportStateCreateInfo vp = {};//管线视口状态创建信息
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.pNext = NULL;//自定义数据的指针
	vp.flags = 0;//供将来使用的标志
	vp.viewportCount = 1;//视口的数量
	vp.scissorCount = 1;//剪裁窗口的数量
	vp.pScissors = &scissor;//剪裁窗口信息列表
	vp.pViewports = &viewports;//视口信息列表

	VkPipelineDepthStencilStateCreateInfo ds;//管线深度及模板状态创建信息
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.pNext = NULL;//自定义数据的指针
	ds.flags = 0;//供将来使用的标志
	ds.depthTestEnable = VK_TRUE;//开启深度测试
	ds.depthWriteEnable = VK_TRUE;//开启深度值写入
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;//深度检测比较操作
	ds.depthBoundsTestEnable = VK_FALSE;//关闭深度边界测试
	ds.minDepthBounds = 0;//最小深度边界
	ds.maxDepthBounds = 0;//最大深度边界
	ds.stencilTestEnable = VK_FALSE;//关闭模板测试
	ds.back.failOp = VK_STENCIL_OP_KEEP;//未通过模板测试时的操作
	ds.back.passOp = VK_STENCIL_OP_KEEP;//模板测试、深度测试都通过时的操作
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;//模板测试的比较操作
	ds.back.compareMask = 0;//模板测试比较掩码
	ds.back.reference = 0;//模板测试参考值
	ds.back.depthFailOp = VK_STENCIL_OP_KEEP;//未通过深度测试时的操作
	ds.back.writeMask = 0;//写入掩码
	ds.front = ds.back;

	VkPipelineMultisampleStateCreateInfo ms;//管线多重采样状态创建信息
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pNext = NULL;//自定义数据的指针
	ms.flags = 0;//供将来使用的标志位
	ms.pSampleMask = NULL;//采样掩码
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//光栅化阶段采样数量
	ms.sampleShadingEnable = VK_FALSE;//关闭采样着色
	ms.alphaToCoverageEnable = VK_FALSE;//不启用alphaToCoverage
	ms.alphaToOneEnable = VK_FALSE;//不启用alphaToOne
	ms.minSampleShading = 0.0;//最小采样着色
	
	SpvData spvVertData = FileUtil::loadSPV(VertShaderPath);//加载顶点SPV
	SpvData spvFragData = FileUtil::loadSPV(FragShaderPath);//加载片元SPV
	VkPipelineShaderStageCreateInfo shaderStages[2];
//	init_glslang();
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = NULL;
	shaderStages[0].pSpecializationInfo = NULL;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";
	VkShaderModuleCreateInfo moduleCreateInfo;//准备顶点着色器模块创建信息
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;//自定义数据的指针
	moduleCreateInfo.flags = 0;//供将来使用的标志
	moduleCreateInfo.codeSize = spvVertData.size;//顶点着色器SPV 数据总字节数
	moduleCreateInfo.pCode = spvVertData.data;//顶点着色器SPV 数据
	result = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[0].module);
	assert(result == VK_SUCCESS);
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = NULL;
	shaderStages[1].pSpecializationInfo = NULL;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;//准备片元着色器模块创建信息
	moduleCreateInfo.pNext = NULL;//自定义数据的指针
	moduleCreateInfo.flags = 0;//供将来使用的标志
	moduleCreateInfo.codeSize = spvFragData.size;//片元着色器SPV 数据总字节数
	moduleCreateInfo.pCode = spvFragData.data;//片元着色器SPV 数据
	result = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[1].module);
	assert(result == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo pipelineInfo;//图形管线创建信息
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = NULL;//自定义数据的指针
	pipelineInfo.layout = pipelineLayout;//指定管线布局
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//基管线句柄
	pipelineInfo.basePipelineIndex = 0;//基管线索引
	pipelineInfo.flags = 0;	//标志
	pipelineInfo.pVertexInputState = &vi;//管线的顶点数据输入状态信息
	pipelineInfo.pInputAssemblyState = &ia;//管线的图元组装状态信息
	pipelineInfo.pRasterizationState = &rs;//管线的光栅化状态信息
	pipelineInfo.pColorBlendState = &cb;//管线的颜色混合状态信息
	pipelineInfo.pTessellationState = NULL;//管线的曲面细分状态信息
	pipelineInfo.pMultisampleState = &ms;//管线的多重采样状态信息
	pipelineInfo.pDynamicState = &dynamicState;//管线的动态状态信息
	pipelineInfo.pViewportState = &vp;//管线的视口状态信息
	pipelineInfo.pDepthStencilState = &ds; //管线的深度模板测试状态信息
	pipelineInfo.stageCount = 2;//管线的着色阶段数量
	pipelineInfo.pStages = shaderStages;//管线的着色阶段列表
	pipelineInfo.renderPass = renderPass;//指定的渲染通道
	pipelineInfo.subpass = 0;//设置管线执行对应的渲染子通道

	VkPipelineCache pipelineCache;
	VkPipelineCacheCreateInfo pipelineCacheInfo;//管线缓冲创建信息
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.pNext = NULL;//自定义数据的指针
	pipelineCacheInfo.initialDataSize = 0;//初始数据尺寸
	pipelineCacheInfo.pInitialData = NULL;//初始数据内容，此处为NULL
	pipelineCacheInfo.flags = 0;//供将来使用的标志位

	result = vkCreatePipelineCache(device, &pipelineCacheInfo, NULL, &pipelineCache);//创建管线缓冲
	assert(result == VK_SUCCESS);//检查管线缓冲创建是否成功

	VkPipeline pipeline;//管线
	vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, NULL, &pipeline);//创建管线
	assert(result == VK_SUCCESS);//检查管线创建是否成功

	VkFence taskFinishFence;//等待任务完毕的栅栏
	VkFenceCreateInfo fenceInfo;//栅栏创建信息结构体实例
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;//结构体类型
	fenceInfo.pNext = NULL;//自定义数据的指针
	fenceInfo.flags = 0;//供将来使用的标志位
	vkCreateFence(device, &fenceInfo, NULL, &taskFinishFence);//创建时栅栏

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;	//结构体类型
	present.pNext = NULL;//自定义数据的指针
	present.swapchainCount = 1;//交换链的数量
	present.pSwapchains = &swapChain;//交换链列表
	present.waitSemaphoreCount = 0;//等待的信号量数量
	present.pWaitSemaphores = NULL;//等待的信号量列表
	present.pResults = NULL;//呈现操作结果标志列表

	MatrixState3D::setCamera(0, 0, 200, 0, 0, 0, 0, 1, 0);//初始化摄像机
	MatrixState3D::setInitStack();//初始化基本变换矩阵
	float ratio = (float)screenWidth / (float)screenHeight;//求屏幕长宽比
	MatrixState3D::setProjectFrustum(-ratio, ratio, -1, 1, 1.5f, 1000);//设置投影参数

	uint32_t currentBuffer = 0;
	FPSUtil::init();//初始化FPS计算
	while (true)//每循环一次绘制一帧画面
	{
		FPSUtil::calFPS();//计算FPS
		FPSUtil::before();//一帧开始
		//获取交换链中的当前帧索引
		result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &currentBuffer);
		//为渲染通道设置当前帧缓冲
		rp_begin.framebuffer = framebuffers[currentBuffer];

		vkResetCommandBuffer(cmdBuffer, 0);//恢复命令缓冲到初始状态
		result = vkBeginCommandBuffer(cmdBuffer, &cmd_buf_info);//启动命令缓冲

		xAngle = xAngle + 1.0f;//改变三色三角形旋转角
		if (xAngle >= 360) { xAngle = 0; }//限制三色三角形旋转角范围
		MatrixState3D::pushMatrix();//保护现场
		MatrixState3D::rotate(xAngle, 1, 0, 0);//旋转变换
		float* vertexUniformData = MatrixState3D::getFinalMatrix();//获取最终变换矩阵
		MatrixState3D::popMatrix();//恢复现场
		uint8_t *pData;//CPU访问时的辅助指针
		VkResult result = vkMapMemory(device, memUniformBuf, 0, 64, 0, (void **)&pData);//将设备内存映射为CPU可访问
		assert(result == VK_SUCCESS);
		memcpy(pData, vertexUniformData, 64);//将最终矩阵数据拷贝进显存
		vkUnmapMemory(device, memUniformBuf);	//解除内存映射

		writes[0].dstSet = descSet;//更新描述集对应的写入属性
		vkUpdateDescriptorSets(device, 1, writes, 0, NULL);//更新描述集

		vkCmdBeginRenderPass(cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);//启动渲染通道
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);//将当前使用的命令缓冲与指定管线绑定
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, NULL);//将命令缓冲、管线布局、描述集绑定
		const VkDeviceSize offsetsVertex[1] = { 0 };//顶点数据偏移量数组
		vkCmdBindVertexBuffers(//将顶点数据与当前使用的命令缓冲绑定
			cmdBuffer,				//当前使用的命令缓冲
			0,					//顶点数据缓冲在列表中的首索引
			1,					//绑定顶点缓冲的数量
			&(vertexDatabuf),	//绑定的顶点数据缓冲列表
			offsetsVertex		//各个顶点数据缓冲的内部偏移量
		);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);//执行绘制
		vkCmdEndRenderPass(cmdBuffer);//结束渲染通道

		result = vkEndCommandBuffer(cmdBuffer);//结束命令缓冲

		submit_info[0].waitSemaphoreCount = 1;//等待的信号量数量
		submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;//等待的信号量列表

		result = vkQueueSubmit(queueGraphics, 1, submit_info, taskFinishFence);//提交命令缓冲
		do {	//等待渲染完毕
			result = vkWaitForFences(device, 1, &taskFinishFence, VK_TRUE, FENCE_TIMEOUT);
		} while (result == VK_TIMEOUT);
		vkResetFences(device, 1, &taskFinishFence);//重置栅栏

		present.pImageIndices = &currentBuffer;//指定此次呈现的交换链图像索引
		result = vkQueuePresentKHR(queueGraphics, &present);//执行呈现
		FPSUtil::after(60);//限制FPS不超过指定的值
	}

	vkDestroyShaderModule(device, shaderStages[0].module, NULL);
	vkDestroyShaderModule(device, shaderStages[1].module, NULL);
	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineCache(device, pipelineCache, NULL);
	vkDestroyDescriptorPool(device, descPool, NULL);
	vkFreeDescriptorSets(device, descPool,1,&descSet);
	vkDestroyDescriptorSetLayout(device, descLayouts, NULL);//销毁对应描述集布局
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);//销毁管线布局
	vkDestroyBuffer(device, uniformBuf, NULL);//销毁顶点缓冲
	vkFreeMemory(device, memUniformBuf, NULL);//释放设备内存
	vkDestroyBuffer(device, vertexDatabuf, NULL);//销毁顶点缓冲
	vkFreeMemory(device, vertexDataMem, NULL);//释放设备内存
	//循环销毁交换链中各个图像对应的帧缓冲
	for (unsigned int i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	}
	free(framebuffers);
	printf("销毁帧缓冲成功！\n");
	vkDestroyRenderPass(device, renderPass, NULL);
	vkDestroySemaphore(device, imageAcquiredSemaphore, NULL);
	vkDestroyImageView(device, depthImageView, NULL);
	vkDestroyImage(device, depthImage, NULL);
	vkFreeMemory(device, memDepth, NULL);
	printf("销毁深度缓冲相关成功!\n");
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyImageView(device, swapchainImageViews[i], NULL);
		printf("[销毁SwapChain ImageView %d 成功]\n", i);
	}
	vkDestroySwapchainKHR(device, swapChain, NULL);
	printf("销毁SwapChain成功!\n");
	VkCommandBuffer cmdBufferArray[1] = { cmdBuffer };
	vkFreeCommandBuffers(device,cmdPool,1,cmdBufferArray);
	vkDestroyCommandPool(device, cmdPool, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
}

void VulkanManager::doVulkan()
{
	ThreadTask* tt = new ThreadTask();
	thread t1(&ThreadTask::doTask, tt);
	t1.detach();
}