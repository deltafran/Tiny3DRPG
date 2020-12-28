#pragma once

struct Configuration;

class PlatformSystem;
class WindowSystem;
class InputSystem;
class RendererSystem;
class Engine;
class Application;

namespace Globals
{
	/*
	* эти функции объявлены в месте описания класса
	*/
	extern [[nodiscard]] ::PlatformSystem& PlatformSystem() noexcept;                      // => PlatformSystem.cpp
	extern [[nodiscard]] ::WindowSystem& WindowSystem() noexcept;                          // => WindowSystem.cpp
	extern [[nodiscard]] ::InputSystem& InputSystem() noexcept;                            // => InputSystem.cpp
	extern [[nodiscard]] ::RendererSystem& RendererSystem() noexcept;                      // => RendererSystem.cpp
	extern [[nodiscard]] ::Engine& Engine() noexcept;                                      // => Engine.cpp
	extern [[nodiscard]] ::Application& Application(const Configuration& config) noexcept; // => Application.cpp
}