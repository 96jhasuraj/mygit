#include "Git.h"
#include <iostream>
#include <filesystem>
#include <fstream> 
#include <openssl/sha.h>
#include <iomanip> 
#include <sstream> 
#include <string> 
#include<zlib.h>

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
void Git::write_file_binary(const std::filesystem::path& path, const std::vector<unsigned char>& content) {
    try 
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out_file(path, std::ios::binary);
        out_file.write(reinterpret_cast<const char*>(content.data()), content.size());
        out_file.close();
    }
    catch (const std::filesystem::filesystem_error& e) 
    {
        throw std::runtime_error("write_file_binary: Filesystem error writing to " + path.string() + ": " + e.what());
    }
}
std::vector<unsigned char> Git::compress_data(const std::string& data) {
    std::vector<unsigned char> compressed_data;
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) 
    {
        throw std::runtime_error("Failed to initialize zlib deflate.");
    }
    zs.avail_in = data.size();
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    size_t buffer_size = deflateBound(&zs, (uLong)data.size());
    compressed_data.resize(buffer_size);
    zs.avail_out = buffer_size;
    zs.next_out = compressed_data.data();

    int ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) 
    { 
        deflateEnd(&zs);
        if (ret == Z_OK) 
        { 
            throw std::runtime_error("Zlib compression buffer too small or incomplete.");
        }
        throw std::runtime_error("Zlib compression failed with error code: " + std::to_string(ret));
    }

    compressed_data.resize(buffer_size - zs.avail_out);
    deflateEnd(&zs);
    return compressed_data;
}
void Git::init_setup()
{
    create_init_directories();
    write_file_message(head_file, "ref: refs/heads/main\n");
    write_file_message(config_file, "user_name: Default\n revisit read what all goes into the config file\n");
}
void Git::init()
{
    try 
    {
        if (fs::exists(git_dir)) 
        {
            std::cout << "Reinitializing existing Git repository in " << git_dir << "\n";
            fs::remove_all(git_dir);
        }
        init_setup();
        std::cout << "Initialized empty Git repository in " << git_dir << "/\n";
    }
    catch (const std::exception& e) 
    {
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

std::string Git::hash_object(const std::filesystem::path& path) 
{
    if (std::filesystem::is_directory(path)) 
    {
        throw std::runtime_error("Use 'hash-tree' for folders.");
    }

    if (!std::filesystem::exists(path)) 
    {
        throw std::runtime_error("File not found: " + path.string());
    }

    std::ifstream file(path, std::ios::binary);
    std::string file_content((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();
    return hash_object(file_content, "blob");
}
std::string Git::hash_object(const std::string& data, const std::string& obj_type) 
{
    std::stringstream header_ss;
    header_ss << obj_type << " " << data.length() << '\0';
    std::string full_data = header_ss.str() + data;
    std::string sha1 = calculate_sha1(full_data);
    std::filesystem::path object_path = objects_dir;
    object_path /= sha1.substr(0, 2); // First two chars for subdirectory
    object_path /= sha1.substr(2);   // Remaining chars for filename
    if (!std::filesystem::exists(object_path)) 
    {
            try 
            {
                std::vector<unsigned char> compressed_content = compress_data(full_data);
                write_file_binary(object_path, compressed_content);
            }
            catch (const std::runtime_error& e) 
            {
                std::cerr << "Error storing object: " << e.what() << std::endl;
                exit(1);
            }
    }
    return sha1;
}

std::string Git::find_object(const std::string& sha1_prefix)
{

    std::filesystem::path obj_dir_path = objects_dir;
    obj_dir_path/=sha1_prefix.substr(0, 2);
    std::string rest_of_prefix = sha1_prefix.substr(2);

    std::vector<std::string> matching_objects;

    if (std::filesystem::exists(obj_dir_path) && std::filesystem::is_directory(obj_dir_path)) {
        for (const auto& entry : std::filesystem::directory_iterator(obj_dir_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.rfind(rest_of_prefix, 0) == 0) {
                    matching_objects.push_back(filename);
                }
            }
        }
    }

    if (matching_objects.empty()) 
    {
        throw std::runtime_error("object '" + sha1_prefix + "' not found");
    }

    if (matching_objects.size() >= 2) 
    {
		std::cout << "multiple objects found with prefix '" << sha1_prefix << "':\n";
    }
	// add logic to print all matching objects after testing
    return sha1_prefix.substr(0, 2) + matching_objects[0];
}