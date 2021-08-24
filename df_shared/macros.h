#pragma once

#if defined WINDOWS

#define EXPORT __declspec(dllexport)

#elif defined LINUX

#define EXPORT

#elif defined MACOS

#define EXPORT

#endif