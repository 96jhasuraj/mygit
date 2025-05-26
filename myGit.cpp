#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

class MyGit 
{
public:
    void init() 
    {
        std::cout << "Initializing new MyGit repository..." << std::endl;

        fs::path mygit_dir = ".mygit";
        if (fs::exists(mygit_dir)) 
        {
            std::cerr << "Error: .mygit directory already exists. Repository already initialized." << std::endl;
            return;
        }

        try {
            fs::create_directory(mygit_dir);
            std::cout << "Created .mygit directory." << std::endl;

            fs::create_directory(mygit_dir / "objects");
            std::cout << "Created .mygit/objects." << std::endl;

            fs::create_directory(mygit_dir / "refs");
            std::cout << "Created .mygit/refs." << std::endl;

            fs::create_directory(mygit_dir / "refs" / "heads");
            std::cout << "Created .mygit/refs/heads." << std::endl;

            fs::create_directory(mygit_dir / "refs" / "tags");
            std::cout << "Created .mygit/refs/tags." << std::endl;

            std::ofstream head_file(mygit_dir / "HEAD");
            if (head_file.is_open()) 
            {
                head_file << "ref: refs/heads/main\n";
                head_file.close();
                std::cout << "Initialized .mygit/HEAD to point to refs/heads/main." << std::endl;
            }
            else 
            {
                std::cerr << "Error: Could not create .mygit/HEAD file." << std::endl;
                return;
            }

			std::ofstream index_file(mygit_dir / "index"); //index is for staging area
            if (index_file.is_open()) 
            {
                index_file.close();
                std::cout << "Initialized empty .mygit/index file." << std::endl;
            }
            else 
            {
                std::cerr << "Error: Could not create .mygit/index file." << std::endl;
                return;
            }

            std::ofstream config_file(mygit_dir / "config");
            if (config_file.is_open()) 
            {
                config_file.close();  //userinfo etc
                std::cout << "Initialized empty .mygit/config file." << std::endl;
            }
            else 
            {
                std::cerr << "Error: Could not create .mygit/config file." << std::endl;
                return;
            }

            std::cout << "MyGit repository initialized successfully!" << std::endl;

        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "Filesystem error during initialization: " << ex.what() << std::endl;
        }
        catch (const std::exception& ex) {
            std::cerr << "An unexpected error occurred: " << ex.what() << std::endl;
        }
    }
};

int main() {
    MyGit my_git;
    my_git.init();
    return 0;
}