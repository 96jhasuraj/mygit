#define _CRT_SECURE_NO_WARNINGS  
#include <iostream>  
#include <filesystem>  
#include "Git.h"

namespace fs = std::filesystem;  

void print_intro()
{
    std::cout << "usage: git <command> [<args>]\n\n";
    std::cout << "These are commands supported for now:\n\n";
    std::cout << "   init       Create an empty Git repository or reinitialize an existing one\n";
}
void handle_user_input(int argc, char* argv[],Git git)  
{  
    if (argc < 2) {
        print_intro();
    }
    std::string command = argv[1];
    
    if (command == "init") {
        git.init();
    }
    else {
        std::cout << "git: '" << command << "' is not a supported command. See 'git --help'.\n";
        return;
    }
}

int main(int argc, char* argv[])  
{  
    Git objGit;
    handle_user_input(argc, argv,objGit);
    
    return 0;  
}