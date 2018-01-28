#pragma once
#include <string>

// template<class T>
// class singleton
// {
// public:
// 	singleton()
// 	{
// 	}
// public:
// 	static T* GetInstance()
// 	{
// 		static T instance;
// 		return &instance;
// 	}
// };

int bind_by_port(int port);
std::string SetCurrentWorkDir();