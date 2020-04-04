#ifndef VULKANEXBASE_MYVULKANMANAGER_H
#define VULKANEXBASE_MYVULKANMANAGER_H
#endif
#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")  /*windows--��������������������� console--�������������������*/
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
	//ѭ��ȷ���ڴ���������
	for (uint32_t i = 0; i < 32; i++)
	{
		//����Ӧ���ͱ���λΪ1
		if ((typeBits & 1) == 1)
		{
			//������������ƥ��
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	//û���ҵ��������������
	return false;
}

void VulkanManager::draw()
{
	//����vulkanʵ��
	VkInstance instance;
	vector<char *> instanceExtensionNames;//��ʼ��������չ�б�������չ��win32������Ⱦ����Ҫ��
	instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);//�˴���ֲWindows�������
	instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);//�˴���ֲWindows��Ҫ��ȡVK_KHR_SURFACE_EXTENSION_NAME��չ

	VkApplicationInfo app_info = {};//����Ӧ����Ϣ�ṹ��ʵ��
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//�ṹ�������
	app_info.pNext = NULL;//�Զ������ݵ�ָ��
	app_info.pApplicationName = "HelloVulkan";//Ӧ�õ�����
	app_info.applicationVersion = 1;//Ӧ�õİ汾��
	app_info.pEngineName = "HelloVulkan";//Ӧ�õ���������
	app_info.engineVersion = 1;//Ӧ�õ�����汾��
	app_info.apiVersion = VK_API_VERSION_1_0;//ʹ�õ�Vulkanͼ��Ӧ�ó���API�汾

	VkInstanceCreateInfo inst_info = {};//����ʵ��������Ϣ�ṹ��ʵ��
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;//�ṹ�������
	inst_info.pNext = NULL;//�Զ������ݵ�ָ��
	inst_info.flags = 0;//������ʹ�õı�־
	inst_info.pApplicationInfo = &app_info;//��Ӧ����Ϣ�ṹ��
	inst_info.enabledExtensionCount = instanceExtensionNames.size();//��չ������
	inst_info.ppEnabledExtensionNames = instanceExtensionNames.data();//��չ�����б�����
	inst_info.enabledLayerCount = 0;//�����Ĳ�����
	inst_info.ppEnabledLayerNames = NULL;//�����Ĳ������б�

	VkResult result;//�洢���н���ĸ�������

	result = vkCreateInstance(&inst_info, NULL, &instance);//����ʵ��
	if (result == VK_SUCCESS)
	{
		printf("Vulkanʵ�������ɹ�!\n");
	}
	else
	{
		printf("Vulkanʵ������ʧ��!\n");
	}

	//ѡ�������豸������ѡ���һ��
	uint32_t gpuCount = 0;//�洢�����豸�����ı���
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);//��ȡ�����豸����
	assert(result == VK_SUCCESS);
	printf("[VulkanӲ���豸����Ϊ%d��]", gpuCount);
	vector<VkPhysicalDevice> gpus;
	VkPhysicalDeviceMemoryProperties memoryroperties;
	gpus.resize(gpuCount);//���������豸�б�ߴ�
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());//��������豸�б�
	assert(result == VK_SUCCESS);
	vkGetPhysicalDeviceMemoryProperties(gpus[0], &memoryroperties);//��ȡ��һ�����豸���ڴ�����

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, NULL);//��ȡ�����豸0�ж��м��������
	printf("[VulkanӲ���豸0֧�ֵĶ��м�������Ϊ%d]\n", queueFamilyCount);
	vector<VkQueueFamilyProperties> queueFamilyprops;
	queueFamilyprops.resize(queueFamilyCount);//����м��������ı�vector����
	vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queueFamilyCount, queueFamilyprops.data());//��������豸0���м��������б�
	printf("[�ɹ���ȡVulkanӲ���豸0֧�ֵĶ��м��������б�]\n");

	//�����߼��豸
	VkDevice device;
	VkDeviceQueueCreateInfo queueInfo = {};//�����豸���д�����Ϣ�ṹ��ʵ��
	uint32_t queueGraphicsFamilyIndex = 0;
	bool found = false;//������־
	for (unsigned int i = 0; i < queueFamilyCount; i++) {//�������ж��м���
		if (queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {//����ǰ���м���֧��ͼ�ι���
			queueInfo.queueFamilyIndex = i;//�󶨴˶��м�������
			queueGraphicsFamilyIndex = i;//��¼֧��ͼ�ι����Ķ��м�������
			printf("[֧��GRAPHICS������һ�����м��������Ϊ%d]\n", i);
			printf("[�˼����е�ʵ�ʶ���������%d]\n", queueFamilyprops[i].queueCount);
			found = true;
			break;
		}
	}

	float queue_priorities[1] = { 0.0 };//�����������ȼ�����
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;//�����ṹ������
	queueInfo.pNext = NULL;//�Զ������ݵ�ָ��
	queueInfo.queueCount = 1;//ָ����������
	queueInfo.pQueuePriorities = queue_priorities;//����ÿ�����е����ȼ�
	queueInfo.queueFamilyIndex = queueGraphicsFamilyIndex;//�󶨶��м�������
	
	vector<const char *> deviceExtensionNames;
	deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);//����������չ

	VkDeviceCreateInfo deviceInfo = {};//�����߼��豸������Ϣ�ṹ��ʵ��
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;//�����ṹ������
	deviceInfo.pNext = NULL;//�Զ������ݵ�ָ��
	deviceInfo.queueCreateInfoCount = 1;//ָ���豸���д�����Ϣ�ṹ������
	deviceInfo.pQueueCreateInfos = &queueInfo;//�����豸���д�����Ϣ�ṹ���б�
	deviceInfo.enabledExtensionCount = deviceExtensionNames.size();//������չ����
	deviceInfo.ppEnabledExtensionNames = deviceExtensionNames.data();//������չ�б�
	deviceInfo.enabledLayerCount = 0;//������Layer������
	deviceInfo.ppEnabledLayerNames = NULL;//������Layer�������б�
	deviceInfo.pEnabledFeatures = NULL;//���õ��豸����
	result = vkCreateDevice(gpus[0], &deviceInfo, NULL, &device);//�����߼��豸
	assert(result == VK_SUCCESS);//����߼��豸�Ƿ񴴽��ɹ�

	//�����������
	VkCommandPool cmdPool;
	VkCommandPoolCreateInfo cmd_pool_info = {};//��������ش�����Ϣ�ṹ��ʵ��
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; 	//�����ṹ������
	cmd_pool_info.pNext = NULL;//�Զ������ݵ�ָ��
	cmd_pool_info.queueFamilyIndex = queueGraphicsFamilyIndex;//��������м�������
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	//ִ�п��Ʊ�־
	result = vkCreateCommandPool(device, &cmd_pool_info, NULL, &cmdPool);//���������
	assert(result == VK_SUCCESS);//�������ش����Ƿ�ɹ�

	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBAI = {};//��������������Ϣ�ṹ��ʵ��
	cmdBAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;//�����ṹ������
	cmdBAI.pNext = NULL;//�Զ������ݵ�ָ��
	cmdBAI.commandPool = cmdPool;//ָ�������
	cmdBAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//���������弶��
	cmdBAI.commandBufferCount = 1;//��������������
	result = vkAllocateCommandBuffers(device, &cmdBAI, &cmdBuffer);//���������
	assert(result == VK_SUCCESS);//�������Ƿ�ɹ�

	VkCommandBufferBeginInfo cmd_buf_info;
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;//�����ṹ������
	cmd_buf_info.pNext = NULL;//�Զ������ݵ�ָ��
	cmd_buf_info.flags = 0;//����ʹ�ñ�־
	cmd_buf_info.pInheritanceInfo = NULL;//�����̳���Ϣ

	VkCommandBuffer cmd_bufs[1];
	cmd_bufs[0] = cmdBuffer;//Ҫ�ύ������ִ�е����������

	VkPipelineStageFlags* pipe_stage_flags = new VkPipelineStageFlags();//Ŀ����߽׶�
	*pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info[1];
	submit_info[0].pNext = NULL;//�Զ������ݵ�ָ��
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;//�����ṹ������
	submit_info[0].pWaitDstStageMask = pipe_stage_flags;//����Ŀ����߽׶�
	submit_info[0].commandBufferCount = 1;//���������
	submit_info[0].pCommandBuffers = cmd_bufs;//�ύ�����������
	submit_info[0].signalSemaphoreCount = 0;//�ź�������
	submit_info[0].pSignalSemaphores = NULL;//�ź�������

	VkQueue queueGraphics;
	vkGetDeviceQueue(device, queueGraphicsFamilyIndex, 0, &queueGraphics);


	//����KHR���洴����Ϣ�ṹ��ʵ��
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;//�����ṹ������
	createInfo.pNext = nullptr;//�Զ������ݵ�ָ��
	createInfo.flags = 0;//��δ��ʹ�õı�־
	createInfo.hwnd = info.window;//��������

	//����KHR����
	VkSurfaceKHR surface;
	result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
	assert(result == VK_SUCCESS);

	//�����豸��Ӧ�Ķ��м����б�
	VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queueFamilyCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
		printf("���м�������=%d %s��ʾ\n", i, (pSupportsPresent[i] == 1 ? "֧��" : "��֧��"));
	}

	queueGraphicsFamilyIndex = UINT32_MAX;//֧��ͼ�ι����Ķ��м�������
	uint32_t queuePresentFamilyIndex = UINT32_MAX;//֧����ʾ(����)�����Ķ��м�������
	for (uint32_t i = 0; i <queueFamilyCount; ++i)//�����豸��Ӧ�Ķ��м����б�
	{
		//�����ǰ�������Ķ��м���֧��Graphis��ͼ�Σ�����
		if ((queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)//���˶��м���֧��ͼ�ι���
		{
			//��¼֧��Graphis��ͼ�Σ������Ķ��м�������
			if (queueGraphicsFamilyIndex == UINT32_MAX) queueGraphicsFamilyIndex = i;
			//�����ǰ�������Ķ��м���֧��Present����ʾ����������
			if (pSupportsPresent[i] == VK_TRUE)//�����ǰ���м���֧����ʾ����
			{
				queueGraphicsFamilyIndex = i;//��¼�˶��м�������Ϊ֧��ͼ�ι�����
				queuePresentFamilyIndex = i;//��¼�˶��м�������Ϊ֧����ʾ������
				printf("���м�������=%d ͬʱ֧��Graphis��ͼ�Σ���Present����ʾ������\n", i);
				break;
			}
		}
	}

	if (queuePresentFamilyIndex == UINT32_MAX)//��û���ҵ�ͬʱ֧��������Ķ��м���
	{
		for (size_t i = 0; i < queueFamilyCount; ++i)//�����豸��Ӧ�Ķ��м����б�
		{
			if (pSupportsPresent[i] == VK_TRUE)//�ж��Ƿ�֧����ʾ����
			{
				queuePresentFamilyIndex = i;//��¼�˶��м�������Ϊ֧����ʾ������
				break;
			}
		}
	}
	free(pSupportsPresent);//�ͷŴ洢�Ƿ�֧�ֳ��ֹ����Ĳ���ֵ�б�

						   //û���ҵ�֧��Graphis��ͼ�Σ���Present����ʾ�������Ķ��м���
						   //û���ҵ�֧��ͼ�λ���ʾ�����Ķ��м���
	if (queueGraphicsFamilyIndex == UINT32_MAX || queuePresentFamilyIndex == UINT32_MAX)
	{
		printf("û���ҵ�֧��Graphis��ͼ�Σ���Present����ʾ�������Ķ��м���\n");
		assert(false);//��û��֧��ͼ�λ���ʾ�����Ķ��м����������ֹ
	}

	uint32_t formatCount;//֧�ֵĸ�ʽ����
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, NULL);//��ȡ֧�ֵĸ�ʽ����
	printf("֧�ֵĸ�ʽ����Ϊ %d\n", formatCount);
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));//�����Ӧ�����Ŀռ�
	vector<VkFormat> formats;
	formats.resize(formatCount);//������ӦVector�ߴ�
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, surfFormats);//��ȡ֧�ֵĸ�ʽ��Ϣ
	for (unsigned int i = 0; i<formatCount; i++) {//��¼֧�ֵĸ�ʽ��Ϣ
		formats[i] = surfFormats[i].format;
		printf("[%d]֧�ֵĸ�ʽΪ%d\n", i, formats[i]);
	}
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {//�����������
		formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
	}
	free(surfFormats);

	//��ȡKHR���������
	VkSurfaceCapabilitiesKHR surfCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpus[0], surface, &surfCapabilities);
	assert(result == VK_SUCCESS);

	//��ȡ֧�ֵ���ʾģʽ����
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, NULL);
	assert(result == VK_SUCCESS);
	printf("��ʾģʽ����Ϊ%d\n", presentModeCount);

	//������ӦVector�ߴ�
	vector<VkPresentModeKHR> presentModes;
	presentModes.resize(presentModeCount);
	//��ȡ֧�ֵ���ʾģʽ�б�
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, presentModes.data());
	for (unsigned int i = 0; i<presentModeCount; i++)
	{
		printf("��ʾģʽ[%d]���Ϊ%d\n", i, presentModes[i]);
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;//ȷ����������ʾģʽ
	for (size_t i = 0; i < presentModeCount; i++)//������ʾģʽ�б�
	{
		//���Ҳ֧��VK_PRESENT_MODE_MAILBOX_KHRģʽ��������Ч�ʸߣ���ѡ��
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			//���û������VK_PRESENT_MODE_MAILBOX_KHRģʽ������VK_PRESENT_MODE_IMMEDIATE_KHRģʽ
			//Ҳ��VK_PRESENT_MODE_FIFO_KHRģʽǿ����ѡ��
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//ȷ��surface�Ŀ�ȡ��߶�
	//���surface�����еĳߴ�û�ж��壨���Ϊ0xFFFFFFFF��ʾû���壩
	VkExtent2D swapchainExtent;
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchainExtent.width = 1366;//���ÿ��Ϊ������
		swapchainExtent.height = 768;//���ø߶�Ϊ����߶�
											  //�������ֵ���Ƶ����ֵ����Сֵ֮��
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}
		//�߶�����ֵ���Ƶ����ֵ����Сֵ֮��
		if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
		printf("ʹ���Լ����õ� ��� %d �߶� %d\n", swapchainExtent.width, swapchainExtent.height);
	}
	else
	{
		//��������ȷ���ߴ�
		swapchainExtent = surfCapabilities.currentExtent;
		printf("ʹ�û�ȡ��surface�����е� ��� %d �߶� %d\n", swapchainExtent.width, swapchainExtent.height);
	}

	uint32_t screenWidth = swapchainExtent.width;//��¼ʵ�ʲ��õĿ��
	uint32_t screenHeight = swapchainExtent.height;//��¼ʵ�ʲ��õĸ߶�

										  //�����������е�����ͼ������
	uint32_t desiredMinNumberOfSwapChainImages = surfCapabilities.minImageCount + 1;
	//���������Ƶ���Χ��
	if ((surfCapabilities.maxImageCount > 0) && (desiredMinNumberOfSwapChainImages > surfCapabilities.maxImageCount))
	{
		desiredMinNumberOfSwapChainImages = surfCapabilities.maxImageCount;
	}

	//KHR����任��־
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)//��֧������ı任
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else//����֧�ֱ任
	{
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchain_ci = {};//����������������Ϣ�ṹ��ʵ��
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;//�ṹ������
	swapchain_ci.pNext = NULL;//�Զ������ݵ�ָ��
	swapchain_ci.surface = surface;//ָ��KHR����
	swapchain_ci.minImageCount = desiredMinNumberOfSwapChainImages;//����ͼ������
	swapchain_ci.imageFormat = formats[0];//ͼ���ʽ
	swapchain_ci.imageExtent.width = swapchainExtent.width;//������ͼ����
	swapchain_ci.imageExtent.height = swapchainExtent.height;//������ͼ��߶�
	swapchain_ci.preTransform = preTransform;//ָ���任��־
	swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//���Alphaֵ
	swapchain_ci.imageArrayLayers = 1;//ͼ���������
	swapchain_ci.presentMode = swapchainPresentMode;//����������ʾģʽ
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;//ǰ��������
	swapchain_ci.clipped = true;//��������
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;//ɫ�ʿռ�
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//ͼ����;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	//ͼ����ģʽ
	swapchain_ci.queueFamilyIndexCount = 0;//���м�������
	swapchain_ci.pQueueFamilyIndices = NULL;//���м��������б�

	if (queueGraphicsFamilyIndex != queuePresentFamilyIndex)//��֧��ͼ�κ���ʾ�����Ķ��м��岻��ͬ
	{
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_ci.queueFamilyIndexCount = 2;//����������Ķ��м�����������Ϊ2
		uint32_t queueFamilyIndices[2] = { queueGraphicsFamilyIndex,queuePresentFamilyIndex };
		swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;//����������Ķ��м��������б�
	}

	VkSwapchainKHR swapChain;
	result = vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swapChain);//����������
	assert(result == VK_SUCCESS);//��齻�����Ƿ񴴽��ɹ�

								 //��ȡ�������е�ͼ������
	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, NULL);
	assert(result == VK_SUCCESS);
	printf("[SwapChain�е�Image����Ϊ%d]\n", swapchainImageCount);//����Ƿ��ȡ�ɹ�
	vector<VkImage> swapchainImages;
	swapchainImages.resize(swapchainImageCount);//����ͼ���б�ߴ�
												//��ȡ�������е�ͼ���б�
	result = vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, swapchainImages.data());
	assert(result == VK_SUCCESS);
	vector<VkImageView> swapchainImageViews;
	swapchainImageViews.resize(swapchainImageCount);//����ͼ����ͼ�б�ߴ�
	for (uint32_t i = 0; i < swapchainImageCount; i++)//Ϊ�������еĸ���ͼ�񴴽�ͼ����ͼ
	{
		VkImageViewCreateInfo color_image_view = {};//����ͼ����ͼ������Ϣ�ṹ��ʵ��
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//���ýṹ������
		color_image_view.pNext = NULL;//�Զ������ݵ�ָ��
		color_image_view.flags = 0;//������ʹ�õı�־
		color_image_view.image = swapchainImages[i];//��Ӧ������ͼ��
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;//ͼ����ͼ������
		color_image_view.format = formats[0];//ͼ����ͼ��ʽ
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;//����Rͨ������
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;//����Gͨ������
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;//����Bͨ������
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;//����Aͨ������
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//ͼ����ͼʹ�÷���
		color_image_view.subresourceRange.baseMipLevel = 0;//����Mipmap����
		color_image_view.subresourceRange.levelCount = 1;//Mipmap���������
		color_image_view.subresourceRange.baseArrayLayer = 0;//���������
		color_image_view.subresourceRange.layerCount = 1;//����������
		result = vkCreateImageView(device, &color_image_view, NULL, &swapchainImageViews[i]);//����ͼ����ͼ
		assert(result == VK_SUCCESS);//����Ƿ񴴽��ɹ�
	}

	VkFormat depthFormat = VK_FORMAT_D16_UNORM;//ָ�����ͼ��ĸ�ʽ
	VkFormatProperties depthFormatProps;
	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory memDepth;
	VkImageCreateInfo image_info = {};//�������ͼ�񴴽���Ϣ�ṹ��ʵ��
	vkGetPhysicalDeviceFormatProperties(gpus[0], depthFormat, &depthFormatProps);//��ȡ�����豸֧�ֵ�ָ����ʽ������
																				 //ȷ��ƽ�̷�ʽ
	if (depthFormatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//�Ƿ�֧��������Ƭ��֯��ʽ
	{
		image_info.tiling = VK_IMAGE_TILING_LINEAR;//����������Ƭ��֯��ʽ
		printf("tilingΪVK_IMAGE_TILING_LINEAR��\n");
	}
	else if (depthFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)//�Ƿ�֧��������Ƭ��֯��ʽ
	{
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;//����������Ƭ��֯��ʽ
		printf("tilingΪVK_IMAGE_TILING_OPTIMAL��\n");
	}
	else
	{
		printf("��֧��VK_FORMAT_D16_UNORM��\n");//��ӡ��֧��ָ����ʽ����ʾ��Ϣ
	}
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;	//ָ���ṹ������
	image_info.pNext = NULL;//�Զ������ݵ�ָ��
	image_info.imageType = VK_IMAGE_TYPE_2D;//ͼ������
	image_info.format = depthFormat;//ͼ���ʽ
	image_info.extent.width = screenWidth;//ͼ����
	image_info.extent.height = screenHeight;//ͼ��߶�
	image_info.extent.depth = 1;//ͼ�����
	image_info.mipLevels = 1;//ͼ��mipmap����
	image_info.arrayLayers = 1;//ͼ�����������
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;//����ģʽ
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//��ʼ����
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;//ͼ����;
	image_info.queueFamilyIndexCount = 0;//���м�������
	image_info.pQueueFamilyIndices = NULL;//���м��������б�
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//����ģʽ
	image_info.flags = 0;//��־

	VkMemoryAllocateInfo mem_alloc = {};//�����ڴ������Ϣ�ṹ��ʵ��
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//�ṹ������
	mem_alloc.pNext = NULL;//�Զ������ݵ�ָ��
	mem_alloc.allocationSize = 0;//������ڴ��ֽ���
	mem_alloc.memoryTypeIndex = 0;//�ڴ����������

	VkImageViewCreateInfo view_info = {};//�������ͼ����ͼ������Ϣ�ṹ��ʵ��
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;//���ýṹ������
	view_info.pNext = NULL;//�Զ������ݵ�ָ��
	view_info.image = VK_NULL_HANDLE;//��Ӧ��ͼ��
	view_info.format = depthFormat;//ͼ����ͼ�ĸ�ʽ
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;//����Rͨ������
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;//����Gͨ������
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;//����Bͨ������
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;//����Aͨ������
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;//ͼ����ͼʹ�÷���
	view_info.subresourceRange.baseMipLevel = 0;//����Mipmap����
	view_info.subresourceRange.levelCount = 1;//Mipmap���������
	view_info.subresourceRange.baseArrayLayer = 0;//���������
	view_info.subresourceRange.layerCount = 1;//����������
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;//ͼ����ͼ������
	view_info.flags = 0;//��־

	result = vkCreateImage(device, &image_info, NULL, &depthImage);//�������ͼ��
	assert(result == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;//��ȡͼ���ڴ�����
	vkGetImageMemoryRequirements(device, depthImage, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;//��ȡ�����ڴ��ֽ���
	VkFlags requirements_mask = 0;//��Ҫ���ڴ���������
	bool flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &mem_alloc.memoryTypeIndex);//��ȡ�����ڴ���������
	assert(flag);//����ȡ�Ƿ�ɹ�
	printf("ȷ���ڴ����ͳɹ� ��������Ϊ%d\n", mem_alloc.memoryTypeIndex);
	result = vkAllocateMemory(device, &mem_alloc, NULL, &memDepth);	//�����ڴ�
	assert(result == VK_SUCCESS);
	result = vkBindImageMemory(device, depthImage, memDepth, 0);//��ͼ����ڴ�
	assert(result == VK_SUCCESS);
	view_info.image = depthImage;//ָ��ͼ����ͼ��Ӧͼ��
	result = vkCreateImageView(device, &view_info, NULL, &depthImageView);//�������ͼ����ͼ
	assert(result == VK_SUCCESS);

	VkSemaphore imageAcquiredSemaphore;
	VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;		//�����ź���������Ϣ�ṹ��ʵ��
	imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;//�ṹ������
	imageAcquiredSemaphoreCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
	imageAcquiredSemaphoreCreateInfo.flags = 0;//������ʹ�õı�־

	result = vkCreateSemaphore(device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);//�����ź���
	assert(result == VK_SUCCESS);//����ź����Ƿ񴴽��ɹ�

	VkAttachmentDescription attachments[2];//����������Ϣ����
	attachments[0].format = formats[0];//������ɫ�����ĸ�ʽ
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;//���ò���ģʽ
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//����ʱ�Ը����Ĳ���
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;//�洢ʱ�Ը����Ĳ���
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//ģ�����ʱ�Ը����Ĳ���
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//ģ��洢ʱ�Ը����Ĳ���
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//��ʼ�Ĳ���
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//����ʱ�����ղ���
	attachments[0].flags = 0;//����λ����
	attachments[1].format = depthFormat;//������ȸ����ĸ�ʽ
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;//���ò���ģʽ
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//����ʱ�Ը����Ĳ���
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//�洢ʱ�Ը����Ĳ���
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//ģ�����ʱ�Ը����Ĳ���
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//ģ��洢ʱ�Ը����Ĳ���
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 	//��ʼ�Ĳ���
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//����ʱ�Ĳ���
	attachments[1].flags = 0;//����λ����

	VkAttachmentReference color_reference = {};//��ɫ��������
	color_reference.attachment = 0;//��Ӧ����������Ϣ�����±�
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//���ø�������

	VkAttachmentReference depth_reference = {};//��ȸ�������
	depth_reference.attachment = 1;//��Ӧ����������Ϣ�����±�
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;//���ø�������

	VkSubpassDescription subpass = {};//������Ⱦ��ͨ�������ṹ��ʵ��
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;//���ù��߰󶨵�
	subpass.flags = 0;//��������
	subpass.inputAttachmentCount = 0;//���븽������
	subpass.pInputAttachments = NULL;//���븽���б�
	subpass.colorAttachmentCount = 1;//��ɫ��������
	subpass.pColorAttachments = &color_reference;//��ɫ�����б�
	subpass.pResolveAttachments = NULL;//Resolve����
	subpass.pDepthStencilAttachment = &depth_reference;//���ģ�帽��
	subpass.preserveAttachmentCount = 0;//preserve��������
	subpass.pPreserveAttachments = NULL;//preserve�����б�

	VkRenderPass renderPass;
	VkRenderPassCreateInfo rp_info = {};//������Ⱦͨ��������Ϣ�ṹ��ʵ��
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;//�ṹ������
	rp_info.pNext = NULL;//�Զ������ݵ�ָ��
	rp_info.attachmentCount = 2;//����������
	rp_info.pAttachments = attachments;//�����б�
	rp_info.subpassCount = 1;//��Ⱦ��ͨ������
	rp_info.pSubpasses = &subpass;//��Ⱦ��ͨ���б�
	rp_info.dependencyCount = 0;//��ͨ����������
	rp_info.pDependencies = NULL;//��ͨ�������б�

	result = vkCreateRenderPass(device, &rp_info, NULL, &renderPass);//������Ⱦͨ��
	assert(result == VK_SUCCESS);

	VkClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.2f;//֡���������R����ֵ
	clear_values[0].color.float32[1] = 0.2f;//֡���������G����ֵ
	clear_values[0].color.float32[2] = 0.2f;//֡���������B����ֵ
	clear_values[0].color.float32[3] = 0.2f;//֡���������A����ֵ
	clear_values[1].depthStencil.depth = 1.0f;//֡������������ֵ
	clear_values[1].depthStencil.stencil = 0;//֡���������ģ��ֵ

	VkRenderPassBeginInfo rp_begin;
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;//��Ⱦͨ��������Ϣ�ṹ������
	rp_begin.pNext = NULL;//�Զ������ݵ�ָ��
	rp_begin.renderPass = renderPass;//ָ��Ҫ��������Ⱦͨ��
	rp_begin.renderArea.offset.x = 0;//��Ⱦ������ʼX����
	rp_begin.renderArea.offset.y = 0;//��Ⱦ������ʼY����
	rp_begin.renderArea.extent.width = screenWidth;//��Ⱦ������
	rp_begin.renderArea.extent.height = screenHeight;//��Ⱦ����߶�
	rp_begin.clearValueCount = 2;//֡�������ֵ����
	rp_begin.pClearValues = clear_values;//֡�������ֵ����

	VkImageView view_attachments[2];//����ͼ����ͼ����
	view_attachments[1] = depthImageView;//�������ͼ����ͼ

	VkFramebufferCreateInfo fb_info = {};//����֡���崴����Ϣ�ṹ��ʵ��
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;//�ṹ������
	fb_info.pNext = NULL;//�Զ������ݵ�ָ��
	fb_info.renderPass = renderPass;//ָ����Ⱦͨ��
	fb_info.attachmentCount = 2;//��������
	fb_info.pAttachments = view_attachments;//����ͼ����ͼ����
	fb_info.width = screenWidth;//���
	fb_info.height = screenHeight;//�߶�
	fb_info.layers = 1;//����

	uint32_t i;//ѭ�����Ʊ���
	VkFramebuffer* framebuffers;
	framebuffers = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));//Ϊ֡�������ж�̬�����ڴ�
	assert(framebuffers);//����ڴ�����Ƿ�ɹ�
						 //�����������еĸ���ͼ��
	for (i = 0; i < swapchainImageCount; i++)
	{
		view_attachments[0] = swapchainImageViews[i];//������ɫ������Ӧͼ����ͼ
		VkResult result = vkCreateFramebuffer(device, &fb_info, NULL, &framebuffers[i]);//����֡����
		assert(result == VK_SUCCESS);//����Ƿ񴴽��ɹ�
		printf("[����֡����%d�ɹ���]\n", i);
	}

	VkBufferCreateInfo buf_info = {};//����һ�±������崴����Ϣ�ṹ��ʵ��
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;	//�ṹ�������
	buf_info.pNext = NULL;//�Զ������ݵ�ָ��
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;//�������;
	buf_info.size = 64;//�������ֽ���
	buf_info.queueFamilyIndexCount = 0;	//���м�������
	buf_info.pQueueFamilyIndices = NULL;//���м��������б�
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//����ģʽ
	buf_info.flags = 0;//��־

	VkBuffer uniformBuf;
	result = vkCreateBuffer(device, &buf_info, NULL, &uniformBuf);//����һ�±�������
	assert(result == VK_SUCCESS);//��鴴���Ƿ�ɹ�

	vkGetBufferMemoryRequirements(device, uniformBuf, &mem_reqs);//��ȡ�˻�����ڴ�����

	VkMemoryAllocateInfo ubo_alloc_info = {};//�����ڴ������Ϣ�ṹ��ʵ��
	ubo_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//�ṹ������
	ubo_alloc_info.pNext = NULL;//�Զ������ݵ�ָ��
	ubo_alloc_info.memoryTypeIndex = 0;//�ڴ���������
	ubo_alloc_info.allocationSize = mem_reqs.size;//�����ڴ�����ֽ���

	requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;//��Ҫ���ڴ���������
	flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &ubo_alloc_info.memoryTypeIndex);	//��ȡ�����ڴ���������
	if (flag) { printf("ȷ���ڴ����ͳɹ� ��������Ϊ%d", ubo_alloc_info.memoryTypeIndex); }
	else { printf("ȷ���ڴ�����ʧ��!"); }

	VkDeviceMemory memUniformBuf;//һ�±��������ڴ�
	result = vkAllocateMemory(device, &ubo_alloc_info, NULL, &memUniformBuf);//�����ڴ�
	assert(result == VK_SUCCESS);//����ڴ�����Ƿ�ɹ�
	result = vkBindBufferMemory(device, uniformBuf, memUniformBuf, 0);//���ڴ�Ͷ�Ӧ�����
	assert(result == VK_SUCCESS);//���󶨲����Ƿ�ɹ�

	VkDescriptorBufferInfo uniformBufferInfo;//һ�±�������������Ϣ
	uniformBufferInfo.buffer = uniformBuf;//ָ��һ�±�������
	uniformBufferInfo.offset = 0;//��ʼƫ����
	uniformBufferInfo.range = 16 * sizeof(float);//һ�±����������ֽ���

	//������������
	VkDescriptorPoolSize type_count[1];//�������سߴ�ʵ������
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//��������
	type_count[0].descriptorCount = 1;//��������

	VkDescriptorPool descPool;
	VkDescriptorPoolCreateInfo descriptor_pool = {};//�����������ش�����Ϣ�ṹ��ʵ��
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;//�ṹ������
	descriptor_pool.pNext = NULL;//�Զ������ݵ�ָ��
	descriptor_pool.maxSets = 1;//�������������
	descriptor_pool.poolSizeCount = 1;//�������سߴ�ʵ������
	descriptor_pool.pPoolSizes = type_count;//�������سߴ�ʵ������
	
	result = vkCreateDescriptorPool(device, &descriptor_pool, NULL, &descPool);//������������
	assert(result == VK_SUCCESS);//����������ش����Ƿ�ɹ�
	printf("�������ش����ɹ�!");

	VkDescriptorSetLayout descLayouts;
	VkDescriptorSetLayoutBinding layout_bindings[1];//���������ְ�����
	layout_bindings[0].binding = 0;//�˰󶨵İ󶨵���
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//��������
	layout_bindings[0].descriptorCount = 1;//��������
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	//Ŀ����ɫ���׶�
	layout_bindings[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};	//�������������ִ�����Ϣ�ṹ��ʵ��
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;//�ṹ������
	descriptor_layout.pNext = NULL;//�Զ������ݵ�ָ��
	descriptor_layout.bindingCount = 1;//���������ְ󶨵�����
	descriptor_layout.pBindings = layout_bindings;//���������ְ�����

	result = vkCreateDescriptorSetLayout(device, &descriptor_layout, NULL, &descLayouts);//��������������
	assert(result == VK_SUCCESS);//������������ִ����Ƿ�ɹ�
	VkDescriptorSetAllocateInfo alloc_info[1];//����������������Ϣ�ṹ��ʵ������
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;//�ṹ������
	alloc_info[0].pNext = NULL;//�Զ������ݵ�ָ��
	alloc_info[0].descriptorPool = descPool;//ָ����������
	alloc_info[0].descriptorSetCount = 1;//����������
	alloc_info[0].pSetLayouts = &descLayouts;//�����������б�

	VkDescriptorSet descSet;
	result = vkAllocateDescriptorSets(device, alloc_info, &descSet);//����������
	assert(result == VK_SUCCESS);//��������������Ƿ�ɹ�

	VkWriteDescriptorSet writes[1];
	writes[0] = {}; //����һ�±���д��������ʵ������
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;//�ṹ������
	writes[0].pNext = NULL;	//�Զ������ݵ�ָ��
	writes[0].descriptorCount = 1;//��������
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//��������
	writes[0].pBufferInfo = &uniformBufferInfo;//��Ӧһ�±����������Ϣ
	writes[0].dstArrayElement = 0;//Ŀ��������ʼԪ��
	writes[0].dstBinding = 0;//Ŀ��󶨱��


	uint32_t dataByteCount = 18 * sizeof(float);//������ռ�ڴ����ֽ���
	float *vdata = new float[18]{ //��������
		0, 75, 0,
		1, 0, 0,
		-45, 0, 0,
		0, 1, 0,
		45, 0, 0,
		0, 0, 1
	};

	VkBuffer vertexDatabuf;
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;//���ýṹ������
	buf_info.pNext = NULL;//�Զ������ݵ�ָ��
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;//�������;Ϊ��������
	buf_info.size = dataByteCount;//�����������ֽ���
	buf_info.queueFamilyIndexCount = 0;//���м�������
	buf_info.pQueueFamilyIndices = NULL;//���м��������б�
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//����ģʽ
	buf_info.flags = 0;//��־

	result = vkCreateBuffer(device, &buf_info, NULL, &vertexDatabuf);//��������
	assert(result == VK_SUCCESS);//��黺�崴���Ƿ�ɹ�

	vkGetBufferMemoryRequirements(device, vertexDatabuf, &mem_reqs);//��ȡ�����ڴ�����
	assert(dataByteCount <= mem_reqs.size);//����ڴ������ȡ�Ƿ���ȷ

	VkMemoryAllocateInfo vb_alloc_info = {};//�����ڴ������Ϣ�ṹ��ʵ��
	vb_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;//�ṹ������
	vb_alloc_info.pNext = NULL;//�Զ������ݵ�ָ��
	vb_alloc_info.memoryTypeIndex = 0;//�ڴ���������
	vb_alloc_info.allocationSize = mem_reqs.size;//�ڴ����ֽ���

	requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;//��Ҫ���ڴ���������
	flag = memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits, requirements_mask, &vb_alloc_info.memoryTypeIndex);//��ȡ�����ڴ���������
	if (flag)
	{
		printf("ȷ���ڴ����ͳɹ� ��������Ϊ%d", vb_alloc_info.memoryTypeIndex);
	}
	else
	{
		printf("ȷ���ڴ�����ʧ��!");
	}
	VkDeviceMemory vertexDataMem;
	result = vkAllocateMemory(device, &vb_alloc_info, NULL, &vertexDataMem);//Ϊ�������ݻ�������ڴ�
	assert(result == VK_SUCCESS);

	uint8_t *pData;//CPU����ʱ�ĸ���ָ��
	result = vkMapMemory(device, vertexDataMem, 0, mem_reqs.size, 0, (void **)&pData);//���豸�ڴ�ӳ��ΪCPU�ɷ���
	assert(result == VK_SUCCESS);//���ӳ���Ƿ�ɹ�
	memcpy(pData, vdata, dataByteCount);//���������ݿ������豸�ڴ�
	vkUnmapMemory(device, vertexDataMem);//����ڴ�ӳ��
	result = vkBindBufferMemory(device, vertexDatabuf, vertexDataMem, 0);//���ڴ��뻺��
	assert(result == VK_SUCCESS);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};//�������߲��ִ�����Ϣ�ṹ��ʵ��
	VkPipelineLayout pipelineLayout;
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;	//�ṹ������
	pPipelineLayoutCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;//���ͳ�����Χ������
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;//���ͳ�����Χ���б�
	pPipelineLayoutCreateInfo.setLayoutCount = 1;//���������ֵ�����
	pPipelineLayoutCreateInfo.pSetLayouts = &descLayouts;//�����������б�

	result = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);//�������߲���
	assert(result == VK_SUCCESS);//��鴴���Ƿ�ɹ�

	VkDynamicState dynamicStateEnables[9];//��̬״̬���ñ�־
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);	//�������б�־Ϊfalse

	VkPipelineDynamicStateCreateInfo dynamicState = {};//���߶�̬״̬������Ϣ
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;//�ṹ������
	dynamicState.pNext = NULL;//�Զ������ݵ�ָ��
	dynamicState.pDynamicStates = dynamicStateEnables;//��̬״̬���ñ�־����
	dynamicState.dynamicStateCount = 0;//���õĶ�̬״̬������

	VkVertexInputBindingDescription vertexBinding;//���ߵĶ����������ݰ�����
	vertexBinding.binding = 0;//��Ӧ�󶨵�
	vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	//��������Ƶ��Ϊÿ����
	vertexBinding.stride = sizeof(float) * 6;//ÿ�����ݵĿ���ֽ���

	VkVertexInputAttributeDescription vertexAttribs[2];//���ߵĶ���������������
	vertexAttribs[0].binding = 0;//��1�������������Եİ󶨵�
	vertexAttribs[0].location = 0;//��1�������������Ե�λ������
	vertexAttribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;//��1�������������Ե����ݸ�ʽ
	vertexAttribs[0].offset = 0;//��1�������������Ե�ƫ����

	vertexAttribs[1].binding = 0;//��2�������������Եİ󶨵�
	vertexAttribs[1].location = 1;//��2�������������Ե�λ������
	vertexAttribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;//��2�������������Ե����ݸ�ʽ
	vertexAttribs[1].offset = 12;//��2�������������Ե�ƫ����

	VkPipelineVertexInputStateCreateInfo vi;//���߶�����������״̬������Ϣ
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;//�Զ������ݵ�ָ��
	vi.flags = 0;//������ʹ�õı�־
	vi.vertexBindingDescriptionCount = 1;//�����������������
	vi.pVertexBindingDescriptions = &vertexBinding;//��������������б�
	vi.vertexAttributeDescriptionCount = 2;//����������������
	vi.pVertexAttributeDescriptions = vertexAttribs;//�����������������б�

	VkPipelineInputAssemblyStateCreateInfo ia;//����ͼԪ��װ״̬������Ϣ
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.pNext = NULL;//�Զ������ݵ�ָ��
	ia.flags = 0;//������ʹ�õı�־
	ia.primitiveRestartEnable = VK_FALSE;//�ر�ͼԪ����
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	//����������ͼԪ�б�ģʽ

	VkPipelineRasterizationStateCreateInfo rs;//���߹�դ��״̬������Ϣ
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.pNext = NULL;//�Զ������ݵ�ָ��
	rs.flags = 0;//������ʹ�õı�־
	rs.polygonMode = VK_POLYGON_MODE_FILL;//���Ʒ�ʽΪ���
	rs.cullMode = VK_CULL_MODE_NONE;//��ʹ�ñ������
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	//���Ʒ���Ϊ��ʱ��
	rs.depthClampEnable = VK_TRUE;//��Ƚ�ȡ
	rs.rasterizerDiscardEnable = VK_FALSE;//���ù�դ����������ΪTRUE���դ���������κ�ƬԪ��
	rs.depthBiasEnable = VK_FALSE;//���������ƫ��
	rs.depthBiasConstantFactor = 0;	//���ƫ�Ƴ�������
	rs.depthBiasClamp = 0;//���ƫ��ֵ�����ޣ���Ϊ����Ϊ���ޣ�Ϊ����Ϊ���ޣ�
	rs.depthBiasSlopeFactor = 0;//���ƫ��б������
	rs.lineWidth = 1.0f;//�߿�ȣ������߻���ģʽ�����ã�

	VkPipelineColorBlendAttachmentState att_state[1];//������ɫ��ϸ���״̬����
	att_state[0].colorWriteMask = 0xf;//����д������
	att_state[0].blendEnable = VK_FALSE;//�رջ��
	att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;//����Alphaͨ����Ϸ�ʽ
	att_state[0].colorBlendOp = VK_BLEND_OP_ADD;//����RGBͨ����Ϸ�ʽ
	att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;//����Դ��ɫ�������
	att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;//����Ŀ����ɫ�������
	att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;//����ԴAlpha�������
	att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;//����Ŀ��Alpha�������


	VkPipelineColorBlendStateCreateInfo cb;//���ߵ���ɫ���״̬������Ϣ
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.pNext = NULL;//�Զ������ݵ�ָ��
	cb.flags = 0;//��δ��ʹ�õı�־
	cb.attachmentCount = 1;	//��ɫ��ϸ�������
	cb.pAttachments = att_state;//��ɫ��ϸ����б�
	cb.logicOpEnable = VK_FALSE;//�������߼�����
	cb.logicOp = VK_LOGIC_OP_NO_OP;//�߼���������Ϊ��
	cb.blendConstants[0] = 1.0f;//��ϳ���R����
	cb.blendConstants[1] = 1.0f;//��ϳ���G����
	cb.blendConstants[2] = 1.0f;//��ϳ���B����
	cb.blendConstants[3] = 1.0f;//��ϳ���A����

	VkViewport viewports;//�ӿ���Ϣ
	viewports.minDepth = 0.0f;//�ӿ���С���
	viewports.maxDepth = 1.0f;//�ӿ�������
	viewports.x = 0;//�ӿ�X����
	viewports.y = 0;//�ӿ�Y����
	viewports.width = 1366;//�ӿڿ��
	viewports.height = 768;//�ӿڸ߶�

	VkRect2D scissor;//���ô�����Ϣ
	scissor.extent.width = 1366;//���ô��ڵĿ��
	scissor.extent.height = 768;//���ô��ڵĸ߶�
	scissor.offset.x = 0;//���ô��ڵ�X����
	scissor.offset.y = 0;//���ô��ڵ�Y����

	VkPipelineViewportStateCreateInfo vp = {};//�����ӿ�״̬������Ϣ
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.pNext = NULL;//�Զ������ݵ�ָ��
	vp.flags = 0;//������ʹ�õı�־
	vp.viewportCount = 1;//�ӿڵ�����
	vp.scissorCount = 1;//���ô��ڵ�����
	vp.pScissors = &scissor;//���ô�����Ϣ�б�
	vp.pViewports = &viewports;//�ӿ���Ϣ�б�

	VkPipelineDepthStencilStateCreateInfo ds;//������ȼ�ģ��״̬������Ϣ
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.pNext = NULL;//�Զ������ݵ�ָ��
	ds.flags = 0;//������ʹ�õı�־
	ds.depthTestEnable = VK_TRUE;//������Ȳ���
	ds.depthWriteEnable = VK_TRUE;//�������ֵд��
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;//��ȼ��Ƚϲ���
	ds.depthBoundsTestEnable = VK_FALSE;//�ر���ȱ߽����
	ds.minDepthBounds = 0;//��С��ȱ߽�
	ds.maxDepthBounds = 0;//�����ȱ߽�
	ds.stencilTestEnable = VK_FALSE;//�ر�ģ�����
	ds.back.failOp = VK_STENCIL_OP_KEEP;//δͨ��ģ�����ʱ�Ĳ���
	ds.back.passOp = VK_STENCIL_OP_KEEP;//ģ����ԡ���Ȳ��Զ�ͨ��ʱ�Ĳ���
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;//ģ����ԵıȽϲ���
	ds.back.compareMask = 0;//ģ����ԱȽ�����
	ds.back.reference = 0;//ģ����Բο�ֵ
	ds.back.depthFailOp = VK_STENCIL_OP_KEEP;//δͨ����Ȳ���ʱ�Ĳ���
	ds.back.writeMask = 0;//д������
	ds.front = ds.back;

	VkPipelineMultisampleStateCreateInfo ms;//���߶��ز���״̬������Ϣ
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pNext = NULL;//�Զ������ݵ�ָ��
	ms.flags = 0;//������ʹ�õı�־λ
	ms.pSampleMask = NULL;//��������
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//��դ���׶β�������
	ms.sampleShadingEnable = VK_FALSE;//�رղ�����ɫ
	ms.alphaToCoverageEnable = VK_FALSE;//������alphaToCoverage
	ms.alphaToOneEnable = VK_FALSE;//������alphaToOne
	ms.minSampleShading = 0.0;//��С������ɫ
	
	SpvData spvVertData = FileUtil::loadSPV(VertShaderPath);//���ض���SPV
	SpvData spvFragData = FileUtil::loadSPV(FragShaderPath);//����ƬԪSPV
	VkPipelineShaderStageCreateInfo shaderStages[2];
//	init_glslang();
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = NULL;
	shaderStages[0].pSpecializationInfo = NULL;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";
	VkShaderModuleCreateInfo moduleCreateInfo;//׼��������ɫ��ģ�鴴����Ϣ
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
	moduleCreateInfo.flags = 0;//������ʹ�õı�־
	moduleCreateInfo.codeSize = spvVertData.size;//������ɫ��SPV �������ֽ���
	moduleCreateInfo.pCode = spvVertData.data;//������ɫ��SPV ����
	result = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[0].module);
	assert(result == VK_SUCCESS);
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = NULL;
	shaderStages[1].pSpecializationInfo = NULL;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;//׼��ƬԪ��ɫ��ģ�鴴����Ϣ
	moduleCreateInfo.pNext = NULL;//�Զ������ݵ�ָ��
	moduleCreateInfo.flags = 0;//������ʹ�õı�־
	moduleCreateInfo.codeSize = spvFragData.size;//ƬԪ��ɫ��SPV �������ֽ���
	moduleCreateInfo.pCode = spvFragData.data;//ƬԪ��ɫ��SPV ����
	result = vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStages[1].module);
	assert(result == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo pipelineInfo;//ͼ�ι��ߴ�����Ϣ
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = NULL;//�Զ������ݵ�ָ��
	pipelineInfo.layout = pipelineLayout;//ָ�����߲���
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//�����߾��
	pipelineInfo.basePipelineIndex = 0;//����������
	pipelineInfo.flags = 0;	//��־
	pipelineInfo.pVertexInputState = &vi;//���ߵĶ�����������״̬��Ϣ
	pipelineInfo.pInputAssemblyState = &ia;//���ߵ�ͼԪ��װ״̬��Ϣ
	pipelineInfo.pRasterizationState = &rs;//���ߵĹ�դ��״̬��Ϣ
	pipelineInfo.pColorBlendState = &cb;//���ߵ���ɫ���״̬��Ϣ
	pipelineInfo.pTessellationState = NULL;//���ߵ�����ϸ��״̬��Ϣ
	pipelineInfo.pMultisampleState = &ms;//���ߵĶ��ز���״̬��Ϣ
	pipelineInfo.pDynamicState = &dynamicState;//���ߵĶ�̬״̬��Ϣ
	pipelineInfo.pViewportState = &vp;//���ߵ��ӿ�״̬��Ϣ
	pipelineInfo.pDepthStencilState = &ds; //���ߵ����ģ�����״̬��Ϣ
	pipelineInfo.stageCount = 2;//���ߵ���ɫ�׶�����
	pipelineInfo.pStages = shaderStages;//���ߵ���ɫ�׶��б�
	pipelineInfo.renderPass = renderPass;//ָ������Ⱦͨ��
	pipelineInfo.subpass = 0;//���ù���ִ�ж�Ӧ����Ⱦ��ͨ��

	VkPipelineCache pipelineCache;
	VkPipelineCacheCreateInfo pipelineCacheInfo;//���߻��崴����Ϣ
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.pNext = NULL;//�Զ������ݵ�ָ��
	pipelineCacheInfo.initialDataSize = 0;//��ʼ���ݳߴ�
	pipelineCacheInfo.pInitialData = NULL;//��ʼ�������ݣ��˴�ΪNULL
	pipelineCacheInfo.flags = 0;//������ʹ�õı�־λ

	result = vkCreatePipelineCache(device, &pipelineCacheInfo, NULL, &pipelineCache);//�������߻���
	assert(result == VK_SUCCESS);//�����߻��崴���Ƿ�ɹ�

	VkPipeline pipeline;//����
	vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, NULL, &pipeline);//��������
	assert(result == VK_SUCCESS);//�����ߴ����Ƿ�ɹ�

	VkFence taskFinishFence;//�ȴ�������ϵ�դ��
	VkFenceCreateInfo fenceInfo;//դ��������Ϣ�ṹ��ʵ��
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;//�ṹ������
	fenceInfo.pNext = NULL;//�Զ������ݵ�ָ��
	fenceInfo.flags = 0;//������ʹ�õı�־λ
	vkCreateFence(device, &fenceInfo, NULL, &taskFinishFence);//����ʱդ��

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;	//�ṹ������
	present.pNext = NULL;//�Զ������ݵ�ָ��
	present.swapchainCount = 1;//������������
	present.pSwapchains = &swapChain;//�������б�
	present.waitSemaphoreCount = 0;//�ȴ����ź�������
	present.pWaitSemaphores = NULL;//�ȴ����ź����б�
	present.pResults = NULL;//���ֲ��������־�б�

	MatrixState3D::setCamera(0, 0, 200, 0, 0, 0, 0, 1, 0);//��ʼ�������
	MatrixState3D::setInitStack();//��ʼ�������任����
	float ratio = (float)screenWidth / (float)screenHeight;//����Ļ�����
	MatrixState3D::setProjectFrustum(-ratio, ratio, -1, 1, 1.5f, 1000);//����ͶӰ����

	uint32_t currentBuffer = 0;
	FPSUtil::init();//��ʼ��FPS����
	while (true)//ÿѭ��һ�λ���һ֡����
	{
		FPSUtil::calFPS();//����FPS
		FPSUtil::before();//һ֡��ʼ
		//��ȡ�������еĵ�ǰ֡����
		result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &currentBuffer);
		//Ϊ��Ⱦͨ�����õ�ǰ֡����
		rp_begin.framebuffer = framebuffers[currentBuffer];

		vkResetCommandBuffer(cmdBuffer, 0);//�ָ�����嵽��ʼ״̬
		result = vkBeginCommandBuffer(cmdBuffer, &cmd_buf_info);//���������

		xAngle = xAngle + 1.0f;//�ı���ɫ��������ת��
		if (xAngle >= 360) { xAngle = 0; }//������ɫ��������ת�Ƿ�Χ
		MatrixState3D::pushMatrix();//�����ֳ�
		MatrixState3D::rotate(xAngle, 1, 0, 0);//��ת�任
		float* vertexUniformData = MatrixState3D::getFinalMatrix();//��ȡ���ձ任����
		MatrixState3D::popMatrix();//�ָ��ֳ�
		uint8_t *pData;//CPU����ʱ�ĸ���ָ��
		VkResult result = vkMapMemory(device, memUniformBuf, 0, 64, 0, (void **)&pData);//���豸�ڴ�ӳ��ΪCPU�ɷ���
		assert(result == VK_SUCCESS);
		memcpy(pData, vertexUniformData, 64);//�����վ������ݿ������Դ�
		vkUnmapMemory(device, memUniformBuf);	//����ڴ�ӳ��

		writes[0].dstSet = descSet;//������������Ӧ��д������
		vkUpdateDescriptorSets(device, 1, writes, 0, NULL);//����������

		vkCmdBeginRenderPass(cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);//������Ⱦͨ��
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);//����ǰʹ�õ��������ָ�����߰�
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, NULL);//������塢���߲��֡���������
		const VkDeviceSize offsetsVertex[1] = { 0 };//��������ƫ��������
		vkCmdBindVertexBuffers(//�����������뵱ǰʹ�õ�������
			cmdBuffer,				//��ǰʹ�õ������
			0,					//�������ݻ������б��е�������
			1,					//�󶨶��㻺�������
			&(vertexDatabuf),	//�󶨵Ķ������ݻ����б�
			offsetsVertex		//�����������ݻ�����ڲ�ƫ����
		);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);//ִ�л���
		vkCmdEndRenderPass(cmdBuffer);//������Ⱦͨ��

		result = vkEndCommandBuffer(cmdBuffer);//���������

		submit_info[0].waitSemaphoreCount = 1;//�ȴ����ź�������
		submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;//�ȴ����ź����б�

		result = vkQueueSubmit(queueGraphics, 1, submit_info, taskFinishFence);//�ύ�����
		do {	//�ȴ���Ⱦ���
			result = vkWaitForFences(device, 1, &taskFinishFence, VK_TRUE, FENCE_TIMEOUT);
		} while (result == VK_TIMEOUT);
		vkResetFences(device, 1, &taskFinishFence);//����դ��

		present.pImageIndices = &currentBuffer;//ָ���˴γ��ֵĽ�����ͼ������
		result = vkQueuePresentKHR(queueGraphics, &present);//ִ�г���
		FPSUtil::after(60);//����FPS������ָ����ֵ
	}

	vkDestroyShaderModule(device, shaderStages[0].module, NULL);
	vkDestroyShaderModule(device, shaderStages[1].module, NULL);
	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineCache(device, pipelineCache, NULL);
	vkDestroyDescriptorPool(device, descPool, NULL);
	vkFreeDescriptorSets(device, descPool,1,&descSet);
	vkDestroyDescriptorSetLayout(device, descLayouts, NULL);//���ٶ�Ӧ����������
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);//���ٹ��߲���
	vkDestroyBuffer(device, uniformBuf, NULL);//���ٶ��㻺��
	vkFreeMemory(device, memUniformBuf, NULL);//�ͷ��豸�ڴ�
	vkDestroyBuffer(device, vertexDatabuf, NULL);//���ٶ��㻺��
	vkFreeMemory(device, vertexDataMem, NULL);//�ͷ��豸�ڴ�
	//ѭ�����ٽ������и���ͼ���Ӧ��֡����
	for (unsigned int i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	}
	free(framebuffers);
	printf("����֡����ɹ���\n");
	vkDestroyRenderPass(device, renderPass, NULL);
	vkDestroySemaphore(device, imageAcquiredSemaphore, NULL);
	vkDestroyImageView(device, depthImageView, NULL);
	vkDestroyImage(device, depthImage, NULL);
	vkFreeMemory(device, memDepth, NULL);
	printf("������Ȼ�����سɹ�!\n");
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyImageView(device, swapchainImageViews[i], NULL);
		printf("[����SwapChain ImageView %d �ɹ�]\n", i);
	}
	vkDestroySwapchainKHR(device, swapChain, NULL);
	printf("����SwapChain�ɹ�!\n");
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