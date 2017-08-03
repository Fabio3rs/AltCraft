#include "Event.hpp"
#include "Utility.hpp"
#include "ThreadGame.hpp"
#include "ThreadRender.hpp"
#include "ThreadNetwork.hpp"

const char *getTimeSinceProgramStart(void) {
	static auto initialTime = std::chrono::steady_clock().now();
	auto now = std::chrono::steady_clock().now();
	std::chrono::duration<double> seconds = now - initialTime;
	static char buffer[30];
	sprintf(buffer, "%.2f", seconds.count());
	return buffer;
}

INITIALIZE_EASYLOGGINGPP

int main() {
	el::Configurations loggerConfiguration;
	el::Helpers::installCustomFormatSpecifier(
			el::CustomFormatSpecifier("%startTime", std::bind(getTimeSinceProgramStart)));
	std::string format = "[%startTime][%level][%thread][%fbase]: %msg";
	loggerConfiguration.set(el::Level::Info, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Error, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Fatal, el::ConfigurationType::Format, format);
	loggerConfiguration.set(el::Level::Warning, el::ConfigurationType::Format, format);
	el::Helpers::setThreadName("Render");
	el::Loggers::reconfigureAllLoggers(loggerConfiguration);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
	LOG(INFO) << "Logger is configured";

	LOG(WARNING) << "Sizeof EventData is " << sizeof(EventData);

	ThreadGame game;
	ThreadNetwork network;
	ThreadRender render;

	std::thread threadGame(&ThreadGame::Execute, game);
	std::thread threadNetwork(&ThreadNetwork::Execute, network);

	render.Execute();

	threadGame.join();
	threadNetwork.join();
	return 0;
}