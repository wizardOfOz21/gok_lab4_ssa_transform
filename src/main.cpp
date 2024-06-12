#include <string>
#include <fstream>
#include <algorithm>

#include "parser.tab.hpp"
#include "lexer.hpp"
#include "builder.hpp"
#include "ast/program.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "cfg/cfg.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Введите имя файла с входными данными" << std::endl;
        return 0;
    }

    bool enable_optimizations = false;
    if (argc > 2)
    {
        enable_optimizations = true;
    }

    std::string name(argv[1]);
    std::fstream input_stream(name);
    FooLexer lexer(input_stream, 1);

    init(enable_optimizations);
    ProgramAST *root = 0;
    yy::parser parser(root, lexer);
    parser();

    try
    {
        root->declare();
        root->codegen();
    }
    catch (DeclareExeption e)
    {
        std::cout << "Ошибка объявления (:" << std::endl;
        return 0;
    }
    catch (CodegenExeption e)
    {
        std::cout << "Ошибка кодогенерации (:" << std::endl;
        return 0;
    }
    catch (std::exception e)
    {
        std::cout << "Непредвиденная ошибка (:" << std::endl;
        return 0;
    }

    input_stream.close();

    FuncAST *f = dynamic_cast<FuncAST *>(root->decls[2]);

    ssa_transform(f, true); 

    // std::cout << "Кодогенерация прошла успешно" << std::endl;
    // TheModule->print(outs(), nullptr);
    return 0;
}
