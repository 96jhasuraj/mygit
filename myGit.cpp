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
    std::cout << "   hash-string       sha1 of string entered\n";
    std::cout << "   hash-object WIP\n";
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
    else if (command == "hash-string" and argc==3) {
        std::cout<<git.calculate_sha1(argv[2])<<std::endl;
    }
    else if (command == "find-object" and argc == 3) {
        std::cout << git.find_object(argv[2]) << std::endl;
    }
    else if (command == "hash-object" and argc == 3) {
        std::cout << git.hash_object(argv[2]) << std::endl;
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