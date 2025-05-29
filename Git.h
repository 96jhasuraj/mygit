#pragma once
#include<string>
#include<iostream>
#include<filesystem>
#include<vector>    

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
    void write_file_binary(const std::filesystem::path& , const std::vector<unsigned char>& );
    std::string read_binary_file(const std::filesystem::path& path);
    std::vector<unsigned char> compress_data(const std::string& data);
    std::string decompress_data(const std::vector<unsigned char>& compressed_data);
    std::string hash_object(const std::string& data, const std::string& obj_type);
    void compress_write(std::filesystem::path& path, const std::string full_data);
    std::tuple<std::string, std::string> read_object(const std::string& hash);
    std::vector<unsigned char> read_file_binary_as_unsigned_char(const std::string& file_path);

public:
    Git(const std::string& path = ".");
    void init();
    std::string hash_object(const std::filesystem::path& path);
    std::string calculate_sha1(const std::string& data);
    void find_object(const std::string& sha1_prefix);
    const std::string get_object_dir();
    void cat_file(const std::string& sha1_prefix);
};
