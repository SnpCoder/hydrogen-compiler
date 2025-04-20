#include<iostream>
#include<fstream>
#include <optional>
#include <sstream>
#include <vector>
#include"tokenization.h"
#include"parser.h"
#include"generation.h"

std::string tokens_to_asm(std::vector<Token> tokens) {
    std::stringstream output;
    output << "global _start\n_start\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::exit) {
            if (i+1 < tokens.size() && tokens.at(i+1).type == TokenType::int_lit) {
                if (i+2 < tokens.size() && tokens.at(i+2).type == TokenType::semi) {
                    output<<"    mov rax, 60\n";
                    output<<"    mov rdi, "<<tokens.at(i+1).value.value()<<"\n";
                    output<<"    syscall";
                }
            }
        }
    }
    return output.str();
}
int main(int argc,char* argv[]){
    if (argc !=2) {
        std::cerr<<"Incorrect usage.Correct usage is .."<<std::endl;
        std::cerr<<"hydro <input.hy>"<<std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<argv[1]<<std::endl;

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1],std::ios::in);
        contents_stream<<input.rdbuf();
        contents = contents_stream.str();
    }
    Tokenizer tokenizer(move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeExit> tree = parser.parse();
    if (!tree.has_value()) {
        std::cerr<<"No exit statement found!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());
    {
        std::fstream file("out.asm",std::ios::out);
        file << generator.generate();
    }
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}