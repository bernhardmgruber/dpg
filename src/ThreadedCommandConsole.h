#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <regex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

class ThreadedCommandConsole {
public:
	typedef std::function<void(std::vector<std::string>)> Handler;

	ThreadedCommandConsole();

	~ThreadedCommandConsole();

	void addCommand(const std::string& name, std::regex expr, Handler func);

	void runHandlers();

private:
	std::thread consoleThread;

	std::map<std::string, std::tuple<std::regex, Handler>> handlers;

	std::queue<std::function<void()>> pendingHandlers;

	std::mutex handlersMutex;
	std::mutex pendingHandlersMutex;

	void runConsoleLoop();
};
