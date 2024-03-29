#include "main.hpp"
#include "hooks.hpp"
#include "config.hpp"
#include "questui/shared/QuestUI.hpp"

#include "UI/ViewController.hpp"

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getLogger().info("Installing Config...");
    getModConfig().Init(modInfo);
    getLogger().info("Installed Config!");

    getLogger().info("Installing SqoreToggler UI...");
    QuestUI::Init();
    QuestUI::Register::RegisterAllModSettingsViewController<Sqores::SqoreViewController*>(modInfo, "Sqore Toggler");
    getLogger().info("Installed SqoreToggler UI!");

    getLogger().info("Installing hooks...");
    
    auto& logger = getLogger();
    Hooks::InstallHooks(logger);

    getLogger().info("Installed all hooks!");
}
