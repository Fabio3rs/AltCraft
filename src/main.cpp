#include "core/Core.hpp"

const char *getTimeSinceProgramStart(void) {
    static auto initialTime = std::chrono::steady_clock().now();
    auto now = std::chrono::steady_clock().now();
    std::chrono::duration<double> seconds = now-initialTime;
    static char buffer[30];
    sprintf(buffer, "%.2f", seconds.count());
    return buffer;
}

INITIALIZE_EASYLOGGINGPP

int main() {
    el::Configurations loggerConfiguration;
    el::Helpers::installCustomFormatSpecifier(
            el::CustomFormatSpecifier("%startTime", std::bind(getTimeSinceProgramStart)));
    std::string format = "[%startTime][%level][%fbase]: %msg";
    loggerConfiguration.set(el::Level::Info, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Error, el::ConfigurationType::Format, format);
    loggerConfiguration.set(el::Level::Fatal, el::ConfigurationType::Format, format);
    el::Loggers::reconfigureAllLoggers(loggerConfiguration);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    LOG(INFO) << "Logger is configured";

    Core core;
    core.Exec();

    return 0;
}