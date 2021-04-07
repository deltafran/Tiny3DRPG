https://github.com/PacktPublishing/Vulkan-Cookbook

push constant - аналог uniform buffer в openGL (constant biffer в dx11)?
	https://stackoverflow.com/questions/50956414/what-is-a-push-constant-in-vulkan
	работает быстрее других вариантов (но лимит на 128 байт). рекомендуется для вещей меняемых каждый кадр - типа матриц
	Может быть только один, но можно передавать структурой