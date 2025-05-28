#pragma once
#include<string>
#include<iostream>
class Git
{
private:
    std::string repo_path;
    std::string git_dir;
    std::string objects_dir;
    std::string refs_dir;
    std::string index_file;
    std::string head_file;
    std::string config_file;
    void create_init_directories();
    void write_file_message(std::string, std::string);
    void init_setup();

public:
    Git(const std::string& path = ".");
    void init();
    std::string hash_object(const std::string& data);
    std::string calculate_sha1(const std::string& data);

};
