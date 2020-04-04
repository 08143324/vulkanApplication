#include "ThreadTask.h"
#include "VulkanManager.h"

void ThreadTask::doTask()
{
	VulkanManager::draw();//´´½¨VulkanÊµÀı
}

ThreadTask::ThreadTask()
{

}
ThreadTask:: ~ThreadTask()
{

}