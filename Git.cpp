#include "Git.h"
#include <iostream>
#include <filesystem>
#include <fstream> 
namespace fs = std::filesystem;

Git::Git(const std::string& path)
    : repo_path(fs::absolute(path).string()),
    git_dir(repo_path + "/.git"),
    objects_dir(git_dir + "/objects"),
    refs_dir(git_dir + "/refs"),
    index_file(git_dir + "/index"),
    head_file(git_dir + "/HEAD"),
    config_file(git_dir + "/config")
{
}

void Git::init()
{
    try {
        if (fs::exists(git_dir)) {
            std::cout << "Reinitializing existing Git repository in " << git_dir << "\n";
            fs::remove_all(git_dir);
        }
        fs::create_directories(git_dir);
        fs::create_directories(objects_dir);
        fs::create_directories(refs_dir + "/heads");
        fs::create_directories(refs_dir + "/tags");

        std::ofstream head(head_file);
        head << "ref: refs/heads/main\n";
        head.close();

        std::ofstream config(config_file);
        config << "user_name: Default\n";
        config << "revisit read what all goes into the config file\n";
        config.close();

        std::cout << "Initialized empty Git repository in " << git_dir << "/\n";
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}
