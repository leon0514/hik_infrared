#include <pybind11/pybind11.h>
#include "HikvisionCamera.h"
#include <unistd.h> // for sleep

namespace py = pybind11;

// PYBIND11_MODULE 是一个宏，用于创建一个Python可导入的模块
// 第一个参数 (hikvision_sdk) 是模块名，必须与编译出的库文件名（不含扩展名）一致
// 第二个参数 (m) 是一个 py::module_ 对象，代表了这个模块本身
PYBIND11_MODULE(hikvision_sdk, m) {
    m.doc() = "Python bindings for Hikvision Camera SDK"; // 模块的文档字符串

    // 使用 py::class_ 来绑定我们的C++类 HikvisionCamera
    py::class_<HikvisionCamera>(m, "HikvisionCamera")
        // .def(py::init<...>) 绑定构造函数
        // py::arg("...") 为Python中的参数命名，使其支持关键字参数
        .def(py::init<const std::string&, WORD, const std::string&, const std::string&>(),
             py::arg("ip"), py::arg("port"), py::arg("user"), py::arg("password"))
        
        // .def("...") 绑定成员函数
        // 第一个参数是Python中的函数名，第二个参数是C++中的成员函数指针
        .def("login", &HikvisionCamera::login, "Login to the device")
        .def("logout", &HikvisionCamera::logout, "Logout from the device")
        .def("setup_alarm", &HikvisionCamera::setupAlarm, "Setup alarm channel")
        .def("close_alarm", &HikvisionCamera::closeAlarm, "Close alarm channel");

    // 我们还可以添加一个独立的函数到模块中，用于在Python中实现长时间等待
    m.def("wait_for_quit", []() {
        std::cout << "Python is now waiting. Press Ctrl+C in the console to quit." << std::endl;
        // 在这里使用一个无限循环来保持程序运行，以便可以接收报警回调
        // 实际应用中可能有更优雅的事件循环
        while (true) {
            sleep(1);
        }
    }, "A simple function to keep the main thread alive.");
}