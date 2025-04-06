#include <iostream>
#include "init.hpp"
#include <log4cplus/configurator.h>
#include "device.hpp"

int main() {
    log4cplus::BasicConfigurator::doConfigure();
    compound::Init::setAppName("compound-test");
    const compound::Init& init = compound::Init::get();
    const compound::Device& device = compound::Device(init);
    return 0;
}