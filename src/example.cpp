#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/platform.h"

#include "shader_processor.h"

#define LOG_LINE(x) std::cout << x << '\n'
#define LOG(x) std::cout << x

// forward decal functions
std::vector<std::filesystem::path> findFiles(const std::string& folderPath);
bool runProcess(const std::wstring& exe, const std::wstring& args);
void processShaders(const std::vector<std::filesystem::path>& files);


std::vector<std::filesystem::path> findFiles(const std::string& folderPath)
{
	namespace fs = std::filesystem;

	std::vector<fs::path> files;

	if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
	{
		LOG_LINE("Invalid path for shader folder");
		return std::vector<fs::path>();
	}

	LOG_LINE("Files in shader folder : ");
	for (const auto& file : fs::directory_iterator(folderPath))
	{
		if (fs::is_regular_file(file.path()))
		{
			LOG_LINE(file.path().filename());
			files.push_back(file.path());
		}
	}

	return files;
}

void processShaders(const std::vector<std::filesystem::path>& files)
{
	namespace fs = std::filesystem;

	std::string type = "";
	const fs::path shaderExec = fs::path(SHADER_TOOL_PATH) / "shadercRelease.exe";
	const fs::path outputDir = fs::path(SHADER_BIN_PATH);
	const fs::path sourceDir = fs::path(SHADER_ROOT_PATH);
	const fs::path varyingPath = fs::path(SHADER_TOOL_PATH) / "varying.def.sc";


	// If shader folder in build does not exist create it
	if (!std::filesystem::exists(SHADER_BIN_PATH))
	{
		std::filesystem::create_directory(SHADER_BIN_PATH);
	}

	for (const auto& file : files)
	{
		const std::string filename = file.filename().string();

		if (filename.find(".vs.") != std::string::npos)
		{
			type = "vertex";
			LOG_LINE(filename << " is Vertex");
		}
		else if (filename.find(".fs.") != std::string::npos)
		{
			type = "fragment";
			// run fragment compile command
			LOG_LINE(filename << " is Fragment");
		}
		else
		{
			LOG_LINE("Skipping " << filename << " (Unknown type)");
			continue;
		}

		fs::path inputFile = sourceDir / file;
		fs::path outputFile = outputDir / (file.stem().string() + ".bin");

		// Build command
		std::wstring cmd =
			L"-f \"" + inputFile.wstring() + L"\" "
			L"-o \"" + outputFile.wstring() + L"\" "
			L"--type " + std::wstring(type.begin(), type.end()) + L" "
			L"--platform windows "
			L"--profile 120 "
			L"-i \"" + fs::path(SHADER_TOOL_PATH).wstring() + L"\" "
			L"--varyingdef \"" + varyingPath.wstring() + L"\"";

		if (!runProcess(shaderExec.wstring(), cmd))
		{
			LOG_LINE("Failed to run shaderc.");
		}
	}
}

bool runProcess(const std::wstring& exe, const std::wstring& args)
{
	STARTUPINFOW si{};
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi{};

	// CreateProcessW requires a mutable buffer
	std::vector<wchar_t> cmd(args.begin(), args.end());
	cmd.push_back(L'\0');

	BOOL ok = CreateProcessW(
		exe.c_str(),          // lpApplicationName (full path to exe)
		cmd.data(),           // lpCommandLine (arguments only)
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		nullptr,
		&si,
		&pi
	);

	if (!ok)
		return false;

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
}

int main()
{
	bgfx::Init init;
	init.type = bgfx::RendererType::Count;
	init.resolution.width = 1;
	init.resolution.height = 1;
	init.resolution.reset = BGFX_RESET_NONE;

	// Provide window handle
	HWND hwnd = CreateWindowEx(
		0, "STATIC", "HiddenWindow",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, 1, 1, nullptr, nullptr,
		GetModuleHandle(nullptr), nullptr);

	init.platformData.nwh = hwnd;
	init.platformData.ndt = nullptr;

	if (!bgfx::init(init))
	{
		LOG_LINE("BGFX failed to init.");
		return EXIT_FAILURE;
	}

	LOG_LINE("BGFX init successful");

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, init.resolution.width, init.resolution.height);

	// Render one frame
	bgfx::touch(0);
	bgfx::frame();

	//// Shader root path macro defined in CMake
	//const auto& shaderFiles = findFiles(SHADER_ROOT_PATH);
	//// convert the .sc to .bin
	//processShaders(shaderFiles);

	std::cout << "=============== SHADER PROCESSOR ===============" << std::endl;
	
	const auto& files = shader::processor::findShaderFiles(std::filesystem::path(SHADER_ROOT_PATH));

	shader::processor::processShaders(files, SHADER_BIN_PATH,
                                      SHADER_TOOL_PATH, L"windows", L"120");

	bgfx::shutdown();

	return EXIT_SUCCESS;
}