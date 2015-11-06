#include "scanner.hpp"
#include "automaton_lexer.hpp"

int main(int argc, char *argv[]){
    if(argc != 2 && argc != 3){
        std::cout << "usage: lxq ifile [ofiles_directry]" << std::endl;
        return 0;
    }

    std::string place;
    if(argc == 3){
        place = argv[2];
    }

    scanner::scan(argv[1], place);
    return 0;
}
