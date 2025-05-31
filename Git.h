#pragma once
#include<string>
#include<iostream>
#include<filesystem>
#include<vector>    
#include <map>

class Git
{
private:
    struct IndexEntry {
        uint32_t mtime_s;
        uint32_t mode;
        uint32_t size;
        unsigned char sha1_binary[20];
        std::string path;

        std::string get_mode_str() const {
            if ((mode & 0xF000) == 0xA000) return "100755"; // Symlink
            if ((mode & 0xF000) == 0x8000) {
                if (mode & 0100) return "100755"; // Executable
                return "100644"; // Non-executable
            }
            if ((mode & 0xF000) == 0x4000) return "040000"; // Directory (tree)
            return std::to_string(mode);
        }
        void set_sha1_hex(const std::string& hex_str) {
            for (int i = 0; i < 20; ++i) {
                std::string byte_str = hex_str.substr(i * 2, 2);
                sha1_binary[i] = static_cast<unsigned char>(std::stoul(byte_str, nullptr, 16));
            }
        }
    };
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
    std::map<std::string, IndexEntry> index_entries;
    void read_index_file();
    const std::string get_index_file();
    void add_single_file(const std::string& relative_path_in_repo);
    void add_all(std::string dir_path);
    void write_index_file();
    bool ignore_from_adding(const std::string path);
public:
    Git(const std::string& path = ".");
    void init();
    std::string hash_object(const std::filesystem::path& path);
    std::string calculate_sha1(const std::string& data);
    void find_object(const std::string& sha1_prefix);
    const std::string get_object_dir();
    void cat_file(const std::string& sha1_prefix);
    void add(const std::string& path);

};
