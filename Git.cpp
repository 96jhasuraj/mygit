#include "Git.h"
#include <iostream>
#include <filesystem>
#include <fstream> 
#include <openssl/sha.h>
#include <iomanip> 
#include <sstream> 
#include <string> 
#include<zlib.h>
#include <sys/stat.h>
#include <set>
namespace fs = std::filesystem;
char NULLCHAR = '\0';
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
const std::string Git::get_index_file()
{
    return index_file;
}
void Git::create_init_directories()
{
    fs::create_directories(git_dir);
    fs::create_directories(objects_dir);
    fs::create_directories(refs_dir + "/heads");
    std::ofstream index_ofs(index_file, std::ios::binary); 
    if (index_ofs.is_open()) 
    {
        index_ofs.close(); // Just create an empty file
    }

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
std::string Git::hash_object(const std::filesystem::path& path,bool save_on_disk=true) 
{
    if(!hash_checkup(path))
        return "";
    return hash_object(get_file_content(path), "blob", save_on_disk);
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
std::string Git::hash_object(const std::string& data, const std::string& obj_type, bool save_on_disk)
{
    std::stringstream header_ss;
    header_ss << obj_type << " " << data.length() << '\0';
    std::string full_data = header_ss.str() + data;
    std::string sha1 = calculate_sha1(full_data);
    std::filesystem::path object_path = get_object_dir();
    object_path /= sha1.substr(0, 2); // First two chars for subdirectory
    object_path /= sha1.substr(2);   // Remaining chars for filename
    if (save_on_disk)
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
    //std::cout << obj_type << " " <<data << "\n";
    if (obj_type == "blob")
    {
        std::cout << "Object type: " << obj_type << "\n";
        std::cout << "Content:\n" << data << "\n";
    }
    else if (obj_type == "tree") 
    {
        std::cout << "Tree contents:\n";
        size_t offset = 0;
        while (offset < data.length()) 
        {
            size_t space_pos = data.find(' ', offset);
            std::string mode_str = data.substr(offset, space_pos - offset);
            offset = space_pos + 1;

            size_t null_pos = data.find('\0', offset);
            std::string path = data.substr(offset, null_pos - offset);
            offset = null_pos + 1;
            unsigned char sha1_binary[20];
            data.copy(reinterpret_cast<char*>(sha1_binary), 20, offset);
            offset += 20;

            std::stringstream ss;
            for (int i = 0; i < 20; ++i) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(sha1_binary[i]) & 0xFF);
            }
            std::string sha1_hex = ss.str();
            std::cout << mode_str << " " << sha1_hex << "\t" << path << "\n";
        }
    }
    else if (obj_type == "commit") 
    {
        std::cout << "Commit details:\n";

        std::istringstream iss(data);
        std::string line;
        std::string tree_hash;
        std::vector<std::string> parent_hashes;
        std::string author_line;
        std::string committer_line;

        while (std::getline(iss, line) && !line.empty()) 
        {
            if (line.rfind("tree ", 0) == 0) 
            { 
                tree_hash = line.substr(5);
            }
            else if (line.rfind("parent ", 0) == 0) 
            {
                parent_hashes.push_back(line.substr(7));
            }
            else if (line.rfind("author ", 0) == 0) {
                author_line = line.substr(7);
            }
            else if (line.rfind("committer ", 0) == 0) 
            {
                committer_line = line.substr(10);
            }
        }

        std::cout << "tree " << tree_hash << "\n";
        for (const auto& parent : parent_hashes) {
            std::cout << "parent " << parent << "\n";
        }
        std::cout << "author " << author_line << "\n";
        std::cout << "committer " << committer_line << "\n";

        std::cout << "\n"; 
        std::string commit_message_raw;
        std::string msg_line;
        while (std::getline(iss, msg_line)) {
            commit_message_raw += msg_line + "\n";
        }
        if (!commit_message_raw.empty() && commit_message_raw.back() == '\n') {
            commit_message_raw.pop_back();
        }
        std::cout << commit_message_raw << "\n"; 

    }
}

void Git::read_index_file() {
    index_entries.clear();
    std::ifstream file(get_index_file(), std::ios::binary);
    char NULLchar = '\0';
    while (file.peek() != EOF) 
    { 
        IndexEntry entry;
        file.read(reinterpret_cast<char*>(&entry.mtime_s), sizeof(entry.mtime_s));
        file.read(reinterpret_cast<char*>(&entry.mode), sizeof(entry.mode));
        file.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));
        file.read(reinterpret_cast<char*>(entry.sha1_binary), 20);
        std::string path_buffer;
        char path_char;
        while (file.get(path_char) && path_char != NULLchar) {
            path_buffer += path_char;
        }
        entry.path = path_buffer;
        index_entries[entry.path] = entry;
    }
    file.close();
}

void Git::write_index_file() {
    std::ofstream file(get_index_file(), std::ios::binary | std::ios::trunc);

    std::vector<IndexEntry> sorted_entries;
    for (const auto& pair : index_entries) {
        sorted_entries.push_back(pair.second);
    }
    std::sort(sorted_entries.begin(), sorted_entries.end(), [](const IndexEntry& a, const IndexEntry& b) 
        {
            return a.path < b.path;
        }
    );

    for (const auto& entry : sorted_entries) 
    {
        file.write(reinterpret_cast<const char*>(&entry.mtime_s), sizeof(entry.mtime_s));
        file.write(reinterpret_cast<const char*>(&entry.mode), sizeof(entry.mode));
        file.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));
        file.write(reinterpret_cast<const char*>(entry.sha1_binary), 20);
        file.write(entry.path.c_str(), entry.path.length());
        file.write("\0", 1);
    }
    file.close();

}


void Git::add_single_file(const std::string& relative_path_in_repo)
{
	read_index_file();
    fs::path absolute_file_path = fs::path(repo_path) / relative_path_in_repo;
    std::string blob_hash_hex = hash_object(absolute_file_path);
    struct stat file_stat;
    if (!blob_hash_hex.empty() && stat(absolute_file_path.string().c_str(), &file_stat) == 0)
    {
        IndexEntry new_entry;
        new_entry.path = relative_path_in_repo;
        new_entry.set_sha1_hex(blob_hash_hex);
        new_entry.mtime_s = static_cast<uint32_t>(file_stat.st_mtime);
        new_entry.size = static_cast<uint32_t>(file_stat.st_size);
        new_entry.mode = 0100644;  // TODO : Determine the correct mode based on the file type
        index_entries[new_entry.path] = new_entry;
        std::cout << "Staged: " << new_entry.path << " (hash: " << blob_hash_hex << ")\n";
    }
    else {
		std::cout << " not added to index: " << relative_path_in_repo << "\n";
    }
    for (auto& entry : index_entries)
    {
        std::cout << entry.first << " " << entry.second.get_sha1_hex() << "\n";
    }
}

void Git::add_all(std::string dir_path)
{
    for (const auto& entry : fs::recursive_directory_iterator(dir_path))
    {
        if (entry.is_regular_file() && !ignore_from_adding(entry.path().string()))
        {
            std::string rel_path = fs::relative(entry.path(), dir_path).string();
            add_single_file(rel_path);
        }
    }
}
bool Git::ignore_from_adding(const std::string path)
{
    if (path.find(".git") != std::string::npos ||
        path.find("/.git/") != std::string::npos ||
        path.find("\\.git\\") != std::string::npos ||
        path.find(".git\\") == 0 ||
        path.find(".git/")==0 || path=="") {
        return true;
    }
    return false;
}

void Git::ls_files()
{
	std::cout << "Listing files in the index:\n";
    read_index_file();
    for(auto & entry : index_entries) 
    {
        std::cout << entry.first << " " << entry.second.get_sha1_hex() << "\n";
	}

}
std::set<std::string> Git::get_working_dir()
{
    std::set<std::string> working_dir_paths;
    for (const auto& entry : fs::recursive_directory_iterator(repo_path))
    {
        fs::path relative_path_fs = fs::relative(entry.path(), repo_path);
        std::string path_str = relative_path_fs.string();

        if (ignore_from_adding(path_str))
        {
            continue;
        }

        if (fs::is_regular_file(entry.status()))
        {
            working_dir_paths.insert(path_str);
        }
    }
	return working_dir_paths;
}
std::set<std::string> Git::get_staging_dir()
{
    std::set<std::string> staged_paths;
    for (const auto& pair : index_entries)
    {
        staged_paths.insert(pair.first);
    }
    return staged_paths;
}

std::vector<std::string> Git::get_modified_files(std::set<std::string>& staged_paths, std::set<std::string>& working_dir_paths)
{
    std::vector<std::string> changed_paths;
    for (const std::string& path : staged_paths)
    {
        if (working_dir_paths.count(path))
        {
            std::string absolute_file_path = (fs::path(repo_path) / path).string();
            std::string current_working_hash;
            current_working_hash = hash_object(get_file_content(absolute_file_path), "blob", false);
            if (index_entries.at(path).get_sha1_hex() != current_working_hash)
            {
                changed_paths.push_back(path);
            }
        }
    }
    return changed_paths;
}
void Git::status() 
{

    std::vector<std::string> new_paths;
    std::vector<std::string> deleted_paths;
    
    read_index_file();

    
    std::set<std::string> staged_paths = get_staging_dir();
    std::set<std::string> working_dir_paths=get_working_dir();

    std::vector<std::string> changed_paths = get_modified_files(staged_paths, working_dir_paths);

    std::set_difference(staged_paths.begin(), staged_paths.end(),working_dir_paths.begin(), working_dir_paths.end(),std::back_inserter(deleted_paths));
    std::set_difference(working_dir_paths.begin(), working_dir_paths.end(),staged_paths.begin(), staged_paths.end(),std::back_inserter(new_paths));
    

    std::cout << "\nDeleted files :\n";
    for (const auto& file : deleted_paths)
    {
        std::cout << "\t" << file << "\n";
    }

    std::cout << "\nChanges not staged for commit:\n";
    for (const auto& file : changed_paths) 
    {
        std::cout << "\t" << file << "\n";
    }


    std::cout << "\nUntracked files:\n";
    for (const auto& file : new_paths) 
    {
        std::cout << "\t" << file << "\n";
    }

}

void Git::diff()
{
    read_index_file();

    std::set<std::string> staged_paths = get_staging_dir();
    std::set<std::string> working_dir_paths = get_working_dir();
    std::vector<std::string> changed_paths = get_modified_files(staged_paths, working_dir_paths);
    for ( auto& path : changed_paths)
    {
        std::string absolute_file_path = (fs::path(repo_path) / path).string();
        std::string file_ondisk = get_file_content(absolute_file_path);
        std::string sha_file_index = index_entries.at(path).get_sha1_hex();
        std::string ignore;
        std::string file_on_index;
        std::tie(ignore, file_on_index) = read_object(sha_file_index);
		std::cout << "Comparing file: " << path << "\n";
		compare(file_ondisk, file_on_index);
    }
}

void Git::compare(const std::string& file_ondisk, const std::string& file_on_index)
{
    if (file_ondisk == file_on_index)
    {
        std::cout << "No changes detected.\n";
    }
    else
    {
		print_diff(file_on_index, file_ondisk);
    }
}
std::vector<std::string> Git::split_string_by_newline(const std::string& s) 
{
    std::vector<std::string> lines;
    std::string line;
    std::istringstream iss(s);
    while (std::getline(iss, line)) 
    {
        if (!line.empty() && line.back() == '\r') 
        {
			line.pop_back(); // encountered issue with Windows line endings in diff implementation
        }
        lines.push_back(line);
    }
    return lines;
}

void Git::print_diff(const std::string& old_content, const std::string& new_content){
    std::vector<std::string> old_lines = split_string_by_newline(old_content);
    std::vector<std::string> new_lines = split_string_by_newline(new_content);

    std::cout << "--- a/" << "old_content" << "\n";
    std::cout << "+++ b/" << "new_content" << "\n";

    int iold = 0; 
    int inew = 0;

    while (iold < old_lines.size() || inew < new_lines.size()) 
    {
        if (iold < old_lines.size() && inew < new_lines.size()) 
        {
            if (old_lines[iold] == new_lines[inew])
            {
                std::cout << " " << old_lines[iold] << "\n";
            }
            else 
            {
                std::cout << "-" << old_lines[iold] << "\n";
                std::cout << "+" << new_lines[inew] << "\n";
            }
            iold++;
            inew++;
        }
        else if (iold < old_lines.size()) 
        {
            std::cout << "-" << old_lines[iold] << "\n";
            iold++;
        }
        else 
        { 
            std::cout << "+" << new_lines[inew] << "\n";
            inew++;
        }
    }
    std::cout << "\n"; // Add a newline for separation
}
void Git::add(const std::string& relative_path) {
    
    if (relative_path == ".")
    {
        add_all(repo_path);
    }
    else 
    {
        std::string full_path = repo_path + "/" + relative_path;
		if (fs::is_directory(full_path)) 
        {
			add_all(full_path);
        }
		else if (!ignore_from_adding(full_path) && fs::is_regular_file(full_path))
        {
			std::cout << "Adding file: " << relative_path << "\n";
            add_single_file(relative_path);
        }
    }

    write_index_file();
}

std::string Git::write_tree() {// Format: 'mode path\0SHA1_binary'
    read_index_file();

    std::vector<IndexEntry> sorted_entries;
    for (const auto& pair : index_entries) {
        sorted_entries.push_back(pair.second);
    }

    std::sort(sorted_entries.begin(), sorted_entries.end(),[](const IndexEntry& a, const IndexEntry& b) 
    {
            return a.path < b.path;
    });

    std::string tree_content;

    for (const auto& entry : sorted_entries) {
        std::stringstream mode_ss;
        mode_ss << std::setw(6) << std::setfill('0') << entry.mode; 
        std::string mode_str = mode_ss.str();
        tree_content.append(mode_str);
        tree_content.append(" "); 
        tree_content.append(entry.path);
        tree_content.append(1, NULLCHAR); 
        tree_content.append(reinterpret_cast<const char*>(entry.sha1_binary), 20);
    }

    std::string tree_sha1 = hash_object(tree_content, "tree",true);
    std::cout << "Tree object written: " << tree_sha1 << "\n";
    return tree_sha1;
}


void Git::commit(const std::string& message, const std::string& author_name, const std::string& author_email) {

    std::string tree_hash =  write_tree(); 
	std::string current_branch = "main"; // TODO : Get current branch from HEAD
    std::string branch_ref_path = refs_dir+"/heads/" + current_branch;
    std::string parent_hash = get_file_content(branch_ref_path);
    std::vector<std::string> commit_lines;
    commit_lines.push_back("tree " + tree_hash);
    commit_lines.push_back("parent " + parent_hash);
    commit_lines.push_back("author " + author_name + " <" + author_email + "> ");
    commit_lines.push_back("committer " + author_name + " <" + author_email + "> ");
    commit_lines.push_back(message);

    std::string commit_content = "";
    for (const auto& line : commit_lines) {
        commit_content += line + "\n";
    }
    std::string commit_hash = hash_object(commit_content, "commit",true);
    std::ofstream ofs(branch_ref_path);
    ofs << commit_hash << "\n";
    ofs.close();
    std::cout << "Committed to " << current_branch << ": " << commit_hash << "\n";
}
void Git::commit(const std::string& message)
{
	commit(message, "Suraj", "suraj.jha96@gmail.com");//TODO : Get author name and email from config file
}