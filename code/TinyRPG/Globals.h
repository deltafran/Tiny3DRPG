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
	extern [[nodiscard]] ::PlatformSystem& PlatformSystem() noexcept;
	extern [[nodiscard]] ::WindowSystem& WindowSystem() noexcept;
	extern [[nodiscard]] ::InputSystem& InputSystem() noexcept;
	extern [[nodiscard]] ::RendererSystem& RendererSystem() noexcept;
	extern [[nodiscard]] ::Engine& Engine() noexcept;
	extern [[nodiscard]] ::Application& Application(const Configuration& config) noexcept; // => Application.cpp
}