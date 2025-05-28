#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        std::cout << "usage: git <command> [<args>]\n\n";
        std::cout << "These are commands supported for now:\n\n";
        std::cout << "   init       Create an empty Git repository or reinitialize an existing one\n";
        return 0;
    }

    std::string command = argv[1];
    
    if (command == "init") {
		std::cout << "Initializing a new Git repository..implementation under progress.\n";
    }
    else {
        std::cout << "git: '" << command << "' is not a supported command. See 'git --help'.\n";
        return 1;
    }

    return 0;
}