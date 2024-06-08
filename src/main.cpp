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
#include "cfg/mocks.hpp"

void trace_cfg(const vector<Node *>& nodes) {
    std::ofstream cfg_out("cfg");
        print_cfg(nodes, cfg_out);
    cfg_out.close();
}

void trace_dtree(const vector<Node *>& nodes) {
    std::ofstream dtree_out("dtree");
        print_dtree(nodes, dtree_out);
    dtree_out.close();
}

void trace_doms(const vector<Node *>& nodes) {
    std::ofstream doms_out("doms");
    for (auto n : nodes)
    {
        doms_out << n->get_name() << " : {";
        for (auto d : n->Dom)
        {
            doms_out << d->get_name() << ",";
        }
        doms_out << "}" << std::endl;
    }
    doms_out.close();
}

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

    auto cfg_nodes = make_cfg(f->body);
    Node* root_node = cfg_nodes.first;
    // auto n = pair<Node *, Node *>(get_cfg_mock(1), nullptr);
    ByPass bp;
    auto nodes = bp.get_nodes(root_node);

    get_dtree(nodes);

    trace_cfg(nodes);
    trace_dtree(nodes);
    trace_doms(nodes);

    // std::cout << "Кодогенерация прошла успешно" << std::endl;
    // TheModule->print(outs(), nullptr);
    return 0;
}
