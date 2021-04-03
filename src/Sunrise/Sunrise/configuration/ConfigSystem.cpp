#include "srpch.h"
#include "ConfigSystem.h"
#include "../fileSystem/FileManager.h"


#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace sunrise {
    
    using namespace nlohmann;

    //const std::string configDir = R"(./config/)";

    ConfigSystem configSystem = ConfigSystem();


    ConfigSystem::Config& ConfigSystem::global()
    {
        // TODO: insert return statement here
        return *config;
    }

    void ConfigSystem::readFromDisk()
    {
        auto filePath = FileManager::getConfigDir() + "global.config";

        std::ifstream f(filePath);
        if (f.good()) {
            std::string str((std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>());
            json jData = json::parse(str);
            config = Config::fromJson(jData);
            //writeToDisk();
        }
        else {
            resetToDefault();
            writeToDisk();
        }

    }

    void ConfigSystem::writeToDisk()
    {
        auto filePath = FileManager::getConfigDir() + "global.config";

        auto data = config->toJson();

        {
            std::ofstream out;
            out.open(filePath, std::fstream::out);
            //out.open(file, std::fstream::out);
            out << std::setw(4) << data << std::endl;
            out.close();
        }
    }

    void ConfigSystem::resetToDefault()
    {
        if (config != nullptr)
            delete config;

        config = new Config;

        config->windows = {};
        config->windows.resize(1);

        config->windows[0].mode = ConfigSystem::Config::Window::WindowMode::windowed;
        config->windows[0].monitor = "";
        config->windows[0].size = glm::ivec2(1920, 1080);
        config->windows[0].size = glm::vec2(0.5, 0.5);



    }

    void ConfigSystem::writeHelpDoc()
    {
        auto filePath = std::string("config/") + "readme.md";
        
        if (FileManager::exists(filePath))
            return;

        SR_CORE_INFO("Writting config help file");


        constexpr auto readmeConstContents = 
R"V0G0N(
# Surnise Config Documentation

## Intro

The sunrise engine uses user modifiable config files with several file extensions which indicate what the file is ment to do and if it is ment to be modified

## Graphics Settings

## Window Settings
```json
    {
        "HELLO" : "5"
    }
```
### Your Monitors

**NOTE**: To update this list delete this file and it will be recreated with the new monitor infomation

---
)V0G0N";

        std::string monitorNames = "";

        int monitorsCount = 0;
        auto monitors = glfwGetMonitors(&monitorsCount);

        for (size_t i = 0; i < monitorsCount; i++)
        {
            std::string line;

            line += "- ";
            line += glfwGetWin32Monitor(monitors[i]);
            line += "\n";

            monitorNames += line;
        }

        FileManager::saveStringToFile(readmeConstContents + monitorNames, filePath);

    }

    nlohmann::json ConfigSystem::Config::toJson()
    {
        json j;

        std::vector<json> jwindows;

        jwindows.resize(windows.size());

        for (size_t i = 0; i < windows.size(); i++)
        {
            jwindows[i]["monitor"] = windows[i].monitor;
            switch (windows[i].mode)
            {
            case ConfigSystem::Config::Window::WindowMode::windowed:

                jwindows[i]["mode"] = "windowed";
                break;
            case ConfigSystem::Config::Window::WindowMode::FullscreenBorderless:

                jwindows[i]["mode"] = "FullscreenBorderless";
                break;
            case ConfigSystem::Config::Window::WindowMode::Fullscreen:

                jwindows[i]["mode"] = "Fullscreen";
                break;
            default:
                break;
            }

            jwindows[i]["size"]["x"] = windows[i].size.x;
            jwindows[i]["size"]["y"] = windows[i].size.y;

            jwindows[i]["monitorLocalPostion"]["x"] = windows[i].monitorLocalPostion.x;
            jwindows[i]["monitorLocalPostion"]["y"] = windows[i].monitorLocalPostion.y;
        }
        j["windows"] = jwindows;


        std::vector<json> jcams;
        jcams.resize(cameras.size());

        for (size_t i = 0; i < windows.size(); i++) {
            jcams[i]["offset"]["x"] = cameras[i].offset.x;
            jcams[i]["offset"]["y"] = cameras[i].offset.y;
            jcams[i]["offset"]["z"] = cameras[i].offset.z;


            jcams[i]["rotAxis"]["x"] = cameras[i].rotAxis.x;
            jcams[i]["rotAxis"]["y"] = cameras[i].rotAxis.y;
            jcams[i]["rotAxis"]["z"] = cameras[i].rotAxis.z;

            jcams[i]["rotAngleDeg"] = cameras[i].rotAngleDeg;
        }

        j["cameras"] = jcams;
        return j;
    }

    ConfigSystem::Config* ConfigSystem::Config::fromJson(nlohmann::json& j)
    {
        auto config = new Config();

        auto jwins = j["windows"];
        config->windows.resize(jwins.size());

        for (size_t i = 0; i < jwins.size(); i++)
        {
            config->windows[i].monitor = jwins[i]["monitor"];

            std::string windowMode = jwins[i]["mode"];

            if (windowMode == "Fullscreen")
                config->windows[i].mode = ConfigSystem::Config::Window::WindowMode::Fullscreen;
            else if (windowMode == "FullscreenBorderless")
                config->windows[i].mode = ConfigSystem::Config::Window::WindowMode::FullscreenBorderless;
            else //if (windowMode == "windowed")
                config->windows[i].mode = ConfigSystem::Config::Window::WindowMode::windowed;

            config->windows[i].size.x = jwins[i]["size"]["x"];
            config->windows[i].size.y = jwins[i]["size"]["y"];

            config->windows[i].monitorLocalPostion.x = jwins[i]["monitorLocalPostion"]["x"];
            config->windows[i].monitorLocalPostion.y = jwins[i]["monitorLocalPostion"]["y"];
        }


        auto jcams = j["cameras"];
        config->cameras.resize(jcams.size());

        for (size_t i = 0; i < jcams.size(); i++) {
            config->cameras[i].offset.x = jcams[i]["offset"]["x"];
            config->cameras[i].offset.y = jcams[i]["offset"]["y"];
            config->cameras[i].offset.z = jcams[i]["offset"]["z"];


            config->cameras[i].rotAxis.x = jcams[i]["rotAxis"]["x"];
            config->cameras[i].rotAxis.y = jcams[i]["rotAxis"]["y"];
            config->cameras[i].rotAxis.z = jcams[i]["rotAxis"]["z"];


            config->cameras[i].rotAngleDeg = jcams[i]["rotAngleDeg"];
        }

        if (config->cameras.size() < config->windows.size()) {
            for (size_t i = config->cameras.size(); i < config->windows.size(); i++)
            {
                config->cameras.emplace_back();

                config->cameras[i].offset = glm::vec3(0);
                config->cameras[i].rotAxis = glm::vec3(0);
                config->cameras[i].rotAngleDeg = 0;

            }
        }

        return config;
    }

}