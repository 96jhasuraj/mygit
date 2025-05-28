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
public:
    Git(const std::string& path = ".");
    void init();
};
