#include "ThreadTask.h"
#include "VulkanManager.h"

void ThreadTask::doTask()
{
	VulkanManager::draw();//����Vulkanʵ��
}

ThreadTask::ThreadTask()
{

}
ThreadTask:: ~ThreadTask()
{

}