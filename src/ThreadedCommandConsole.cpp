#include <iostream>
#include <iterator>
#include <sstream>

#include "ThreadedCommandConsole.h"

using namespace std;

ThreadedCommandConsole::ThreadedCommandConsole() {
	consoleThread = thread(&ThreadedCommandConsole::runConsoleLoop, this);
	consoleThread.detach();
}

ThreadedCommandConsole::~ThreadedCommandConsole() {
}

void ThreadedCommandConsole::addCommand(string name, regex expr, Handler func) {
	lock_guard<mutex> lock(handlersMutex);
	handlers[name] = make_tuple(expr, func);
}

void ThreadedCommandConsole::runHandlers() {
	lock_guard<mutex> lock(pendingHandlersMutex);

	while (!pendingHandlers.empty()) {
		auto handler = pendingHandlers.front();
		pendingHandlers.pop();
		handler();
	}
}

void ThreadedCommandConsole::runConsoleLoop() {
	while (true) {
		vector<string> tokens;
		string input;
		getline(cin, input);

		istringstream iss(input);
		copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));

		Handler handler;
		{
			lock_guard<mutex> lock(handlersMutex);

			// try to find command
			auto it = handlers.find(tokens.front());
			if (it == handlers.end()) {
				cerr << "No command with name " << tokens.front() << endl;
				continue;
			}
			auto commandEntry = handlers[tokens.front()];

			regex expr = get<0>(commandEntry);
			handler = get<1>(commandEntry);

			// validate input
			if (!regex_match(input, expr)) {
				cerr << "Invalid syntax for command " << tokens.front() << endl;
				continue;
			}
		}

		{
			lock_guard<mutex> lock(pendingHandlersMutex);
			pendingHandlers.push([tokens, handler] { handler(tokens); });
		}
	}
}