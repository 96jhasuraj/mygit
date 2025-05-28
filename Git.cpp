#include "Git.h"
#include <iostream>
#include <filesystem>
#include <fstream> 
#include <openssl/sha.h>
#include <iomanip> 
#include <sstream> 
#include <string> 


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

void Git::create_init_directories()
{
    fs::create_directories(git_dir);
    fs::create_directories(objects_dir);
    fs::create_directories(refs_dir + "/heads");
    fs::create_directories(refs_dir + "/tags");
}
void Git::write_file_message(std::string file, std::string message)
{
    std::ofstream file_p(file);
    file_p << message;
    file_p.close();
}
void Git::init_setup()
{
    create_init_directories();
    write_file_message(head_file, "ref: refs/heads/main\n");
    write_file_message(config_file, "user_name: Default\n revisit read what all goes into the config file\n");
}
void Git::init()
{
    try {
        if (fs::exists(git_dir)) {
            std::cout << "Reinitializing existing Git repository in " << git_dir << "\n";
            fs::remove_all(git_dir);
        }
        init_setup();
        std::cout << "Initialized empty Git repository in " << git_dir << "/\n";
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}
std::string Git::calculate_sha1(const std::string& data)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.data()), data.length(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) 
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string Git::hash_object(const std::string& data)
{
    std::string oid = calculate_sha1(data); 
    std::filesystem::path objects_path = objects_dir;
    objects_path /= oid;

    try {
        std::ofstream out_file(objects_path, std::ios::binary);
        if (!out_file.is_open()) 
        {
            std::cerr << "Error: Could not open object file for writing: " << objects_path << std::endl;
            exit(1);
        }
        out_file.write(data.data(), data.length());
        out_file.close();
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        std::cerr << "Error writing object: " << e.what() << std::endl;
        exit(1);
    }

    return oid;
}