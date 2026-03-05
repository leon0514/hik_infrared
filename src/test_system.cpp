#include "system.hpp"

int main()
{
    std::cout << "Testing system functionality." << std::endl;
    json filters;
    filters["deviceIp"] = "172.16.22.203";
    filters["algorithmCode"] = "";
    filters["fwqCode"] = "";
    auto tasks_opt = get_task_list(filters);
    if (tasks_opt)
    {
        for (const auto &task : *tasks_opt)
        {
            std::cout << "Task ID: " << task.id << ", Device IP: " << task.deviceIp << std::endl;
        }
    }
    else
    {
        std::cout << "No tasks retrieved." << std::endl;
    }

    int channel = get_channel_by_ip("172.16.22.203");
    std::cout << "Channel for device IP : " << channel << std::endl;

    return 0;
}