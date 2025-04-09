#include <iostream>
#include "init.hpp"
#include <log4cplus/configurator.h>
#include "device.hpp"
#include "window.hpp"

int main() {
    log4cplus::BasicConfigurator::doConfigure();
    compound::Init::setAppName("compound-test");
    const compound::Init& init = compound::Init::get();
    compound::Window window(init, 800, 450, "test");
    compound::Device device(init, window);
    return 0;
}