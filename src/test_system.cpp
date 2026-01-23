#include "system.hpp"


int main()
{
    std::cout << "Testing system functionality." << std::endl;
    json filters;
    auto tasks_opt = get_task_list(filters);
    if (tasks_opt) 
    {
        for (const auto& task : *tasks_opt) {
            std::cout << "Task ID: " << task.id << ", Device IP: " << task.deviceIp << std::endl;
        } 
    } 
    else {
        std::cout << "No tasks retrieved." << std::endl;
    }   
    return 0;
}