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

const std::string Git::get_object_dir()
{
	return objects_dir;
}
void Git::create_init_directories()
{
    fs::create_directories(git_dir);
    fs::create_directories(objects_dir);
    fs::create_directories(refs_dir + "/heads");
    fs::create_directories(refs_dir + "/tags");
}
std::vector<unsigned char> Git::read_file_binary_as_unsigned_char(const std::string& file_path) 
{
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}
std::string read_file_binary(const std::string& filepath) 
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void Git::write_file_message(std::string file, std::string message)
{
    std::ofstream file_p(file);
    file_p << message;
    file_p.close();
}
void Git::write_file_binary(const std::filesystem::path& path, const std::vector<unsigned char>& content) 
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out_file(path, std::ios::binary);
    out_file.write(reinterpret_cast<const char*>(content.data()), content.size());
    out_file.close();
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
std::string Git::decompress_data(const std::vector<unsigned char>& compressed_data) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    if (inflateInit(&zs) != Z_OK)
    {
        throw std::runtime_error("Failed to initialize zlib inflate.");
    }

    zs.avail_in = compressed_data.size();
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<unsigned char*>(compressed_data.data()));

    size_t chunk_size = 512;
    std::string decompressed_str;
    std::vector<unsigned char> out_buffer(chunk_size);

    int ret;
    do 
    {
        zs.avail_out = chunk_size;
        zs.next_out = out_buffer.data();
        ret = inflate(&zs, Z_NO_FLUSH);

        if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) 
        {
            inflateEnd(&zs);
            throw std::runtime_error("Zlib decompression error: " + std::to_string(ret) + " msg: " + (zs.msg ? zs.msg : "no message"));
        }

        size_t bytes_written = chunk_size - zs.avail_out;
        decompressed_str.append(reinterpret_cast<char*>(out_buffer.data()), bytes_written);
    } while (ret != Z_STREAM_END && zs.avail_in > 0);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END && zs.avail_in == 0) 
    {
        throw std::runtime_error("Zlib decompression: incomplete stream (no Z_STREAM_END)");
    }
    return decompressed_str;
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

bool hash_checkup(const std::filesystem::path& path)
{
    if (std::filesystem::is_directory(path))
    {
        std::cout << "Use 'hash-tree' for folders.\n";
        return false;
    }

    if (!std::filesystem::exists(path))
    {
        std::cout << "File not found.\n";
        return false;
    }
    return true;
}
std::string get_file_content(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return file_content;
}
std::string Git::hash_object(const std::filesystem::path& path) 
{
    if(!hash_checkup(path))
        return "";
    return hash_object(get_file_content(path), "blob");
}
void Git::compress_write(std::filesystem::path& path, const std::string full_data)
{
    if (!std::filesystem::exists(path))
    {
        try
        {
            std::vector<unsigned char> compressed_content = Git::compress_data(full_data);
            Git::write_file_binary(path, compressed_content);
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Error storing object: " << e.what() << std::endl;
            exit(1);
        }
    }
}
std::string Git::hash_object(const std::string& data, const std::string& obj_type) 
{
    std::stringstream header_ss;
    header_ss << obj_type << " " << data.length() << '\0';
    std::string full_data = header_ss.str() + data;
    std::string sha1 = calculate_sha1(full_data);
    std::filesystem::path object_path = get_object_dir();
    object_path /= sha1.substr(0, 2); // First two chars for subdirectory
    object_path /= sha1.substr(2);   // Remaining chars for filename
    compress_write(object_path,full_data);
    return sha1;
}

void Git::find_object(const std::string& sha1_prefix)
{
    if(sha1_prefix.length()<2)
    {
        std::cout<<"sha1 prefix must be at least 2 characters long\n";
        return;
	}

    std::filesystem::path obj_dir_path = get_object_dir();
    obj_dir_path/=sha1_prefix.substr(0, 2);
    std::string rest_of_prefix = sha1_prefix.substr(2);

    std::vector<std::string> matching_objects;

    if (std::filesystem::exists(obj_dir_path) && std::filesystem::is_directory(obj_dir_path)) {
        for (const auto& file : std::filesystem::directory_iterator(obj_dir_path)) {
            if (file.is_regular_file()) {
                std::string filename = file.path().filename().string();
                if (filename.rfind(rest_of_prefix, 0) == 0) {
                    matching_objects.push_back(filename);
                }
            }
        }
    }

    if (matching_objects.empty()) 
    {
        std::cout<<"object " << sha1_prefix << " not found";
    }

    if (matching_objects.size() >= 2) 
    {
		std::cout << "multiple objects found with prefix '" << sha1_prefix << "':\n";
    }
    for (const auto& obj : matching_objects) 
    {
        std::cout<<sha1_prefix.substr(0, 2) + obj<<std::endl;
	}
}

std::tuple<std::string, std::string> Git::read_object(const std::string& hash) {
    std::string dir_path = get_object_dir() + "/" + hash.substr(0, 2);
    std::string file_path = dir_path + "/" + hash.substr(2);
    std::vector<unsigned char> compressed_content = read_file_binary_as_unsigned_char(file_path);

    std::string decompressed_content;
    try {
        decompressed_content = decompress_data(compressed_content);
    }
    catch (const std::runtime_error& e) 
    {
        std::cerr << "Error decompressing object " << hash << ": " << e.what() << std::endl;
        return std::make_tuple("", ""); 
    }
    size_t null_pos = decompressed_content.find('\0');

    if (null_pos == std::string::npos) 
    {
        std::cerr << "Error missing null terminator @hash " << hash << std::endl;
        return std::make_tuple("", "");
    }

    std::string header = decompressed_content.substr(0, null_pos);
    std::string data = decompressed_content.substr(null_pos + 1);

    int space_pos = header.find(' ');
    if (space_pos == std::string::npos) 
    {
        std::cerr << "Error incorrect Git object header for hash " << hash << std::endl;
        return std::make_tuple("", "");
    }
    std::string type = header.substr(0, space_pos);

    return std::make_tuple(type, data);
}

void Git::cat_file(const std::string& sha1_prefix) 
{
    std::string obj_type;
    std::string data;
    std::tie(obj_type, data) = read_object(sha1_prefix); 
    std::cout << obj_type << data << "\n";
    if (obj_type == "blob")
    {
        std::cout << "Object type: " << obj_type << "\n";
        std::cout << "Content:\n" << data << "\n";
    }
}