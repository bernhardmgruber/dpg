#pragma once

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <regex>
#include <tuple>

class ThreadedCommandConsole
{
public:
    typedef std::function<void(std::vector<std::string>)> Handler;

    ThreadedCommandConsole();

    ~ThreadedCommandConsole();

    void addCommand(std::string name, std::regex expr, Handler func);

    void runHandlers();

private:
    std::thread consoleThread;

    std::map<std::string, std::tuple<std::regex,Handler>> handlers;

    std::queue<std::function<void()>> pendingHandlers;

    std::mutex handlersMutex;
    std::mutex pendingHandlersMutex;

    void runConsoleLoop();
};

