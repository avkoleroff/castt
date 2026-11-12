#include "parser/parser.hpp"
#include <iostream>
#include <fstream>

using namespace std;

// вывод в файл
void printIndent(ostream& out, int depth) {
    for (int i = 0; i < depth; ++i) out << "  ";
}

void printType(ostream& out, const TypeNode& t) {
    if (t.kind == TypeNode::ARRAY) {
        out << "array[" << t.size << "] of ";
        if (t.element) printType(out, *t.element);
        else out << "unknown";
    } else {
        out << (t.name.empty() ? "unknown" : t.name);
    }
}

void printExpr(ostream& out, const ExprNode& e, int depth = 0) {
    printIndent(out, depth);
    switch (e.kind) {
        case ExprNode::INT: out << "int: " << e.value << "\n"; break;
        case ExprNode::DOUBLE: out << "double: " << e.value << "\n"; break;
        case ExprNode::CHAR: out << "char: " << e.value << "\n"; break;
        case ExprNode::STRING: out << "string: " << e.value << "\n"; break;
        case ExprNode::BOOL: out << "bool: " << e.value << "\n"; break;
        case ExprNode::NAME: out << "name: " << e.value << "\n"; break;
        case ExprNode::BINARY:
            out << "binary: " << e.value << "\n";
            if (e.left) printExpr(out, *e.left, depth + 1);
            if (e.right) printExpr(out, *e.right, depth + 1);
            break;
        case ExprNode::CALL:
            out << "call: " << e.value << "\n";
            for (const auto& arg : e.elems)
                if (arg) printExpr(out, *arg, depth + 1);
            break;
        default:
            out << "expr: " << e.value << "\n";
    }
}

void printStmt(ostream& out, const StmtNode& s, int depth = 0) {
    printIndent(out, depth);
    switch (s.kind) {
        case StmtNode::VAR_DECL:
            out << "var_decl: ";
            if (s.varDecl && s.varDecl->type) printType(out, *s.varDecl->type);
            out << " " << (s.varDecl ? s.varDecl->name : "") << "\n";
            break;
        case StmtNode::IF:
            out << "if\n";
            if (s.cond) printExpr(out, *s.cond, depth + 1);
            for (const auto& st : s.thenBlock) if (st) printStmt(out, *st, depth + 1);
            for (const auto& st : s.elseBlock) if (st) printStmt(out, *st, depth + 1);
            break;
        case StmtNode::WHILE:
            out << "while\n";
            if (s.cond) printExpr(out, *s.cond, depth + 1);
            for (const auto& st : s.thenBlock) if (st) printStmt(out, *st, depth + 1);
            break;
        case StmtNode::FOR:
            out << "for\n";
            printIndent(out, depth + 1); out << "var: " << s.loopVar << "\n";
            printIndent(out, depth + 1); out << "from:\n";
            if (s.from) printExpr(out, *s.from, depth + 2);
            printIndent(out, depth + 1); out << "to:\n";
            if (s.to) printExpr(out, *s.to, depth + 2);
            printIndent(out, depth + 1); out << "step:\n";
            if (s.step) printExpr(out, *s.step, depth + 2);
            for (const auto& st : s.thenBlock) if (st) printStmt(out, *st, depth + 1);
            break;
        case StmtNode::ASSIGN:
            out << "assign\n";
            if (s.assign) {
                printIndent(out, depth + 1); out << "name: " << s.assign->name << "\n";
                for (const auto& t : s.assign->targets) {
                    printIndent(out, depth + 1);
                    if (t->kind == ExprNode::INDEX) out << "index\n";
                    else if (t->kind == ExprNode::FIELD) out << "field: " << t->field << "\n";
                }
                if (s.assign->value) printExpr(out, *s.assign->value, depth + 1);
            }
            break;
        case StmtNode::INPUT:
            out << "input\n";
            for (const auto& t : s.targets) {
                printIndent(out, depth + 1); out << "target: " << t->name << "\n";
            }
            break;
        case StmtNode::OUTPUT:
            out << "output\n";
            for (const auto& a : s.args) {
                if (a) printExpr(out, *a, depth + 1);
            }
            break;
        case StmtNode::CALL_STMT:
            out << "call_stmt: " << s.callName << "\n";
            for (const auto& a : s.args) {
                if (a) printExpr(out, *a, depth + 1);
            }
            break;
        case StmtNode::RETURN:
            out << "return\n";
            if (s.retValue) printExpr(out, *s.retValue, depth + 1);
            break;
        case StmtNode::RETURN_VOID:
            out << "return_void\n";
            break;
        case StmtNode::EMPTY:
            out << "empty\n";
            break;
        default:
            out << "stmt: kind=" << s.kind << "\n";
    }
}

void printProgram(ostream& out, const ProgramNode& p) {
    out << "АСТ\n";
    out << "Структуры:\n";
    for (const auto& s : p.structs) {
        out << "  Структура: " << s->name << "\n";
        for (const auto& f : s->fields) {
            out << "    Field [";
            if (f->type) printType(out, *f->type);
            out << "]: ";
            for (size_t i = 0; i < f->names.size(); ++i) {
                if (i) out << ", ";
                out << f->names[i];
            }
            out << "\n";
        }
    }
    if (!p.structs.empty()) out << "\n";

    out << "Enum'ы:\n";
    for (const auto& e : p.enums) {
        out << "  enum: " << e->name << " [";
        for (size_t i = 0; i < e->values.size(); ++i) {
            if (i) out << ", ";
            out << e->values[i];
        }
        out << "]\n";
    }
    if (!p.enums.empty()) out << "\n";

    out << "Функции:\n";
    for (const auto& f : p.functions) {
        out << "  Функция: " << f->name << "(";
        for (size_t i = 0; i < f->params.size(); ++i) {
            if (i) out << ", ";
            if (f->params[i]->type) printType(out, *f->params[i]->type);
            out << " " << f->params[i]->name;
        }
        out << ") -> ";
        if (f->retVoid) out << "ничто";
        else if (f->retTypeWhere) printType(out, *f->retTypeWhere);
        else out << "ничто";
        out << "\n";
        for (const auto& st : f->body)
            if (st) printStmt(out, *st, 2);
    }
    if (!p.functions.empty()) out << "\n";

    out << "Глобальные statement'ы:\n";
    for (const auto& st : p.statements)
        if (st) printStmt(out, *st, 1);
    out << "EOF\n";
}

// запуск парсера

int main() {
    ifstream file("test.txt");
    string source((istreambuf_iterator<char>(file)),
                       istreambuf_iterator<char>());
    file.close();

    Parser parser(source);
    auto program = parser.parseProgram();

    ofstream outFile("ast_output.txt");

    if (parser.ok()) {
        cout << "OK";
        printProgram(outFile, *program);
    } else {
        cerr << "Парсер умер: " << parser.error() << "\n";
        outFile << "Ошибка парсера: " << parser.error() << "\n";
        return 1;
    }

    outFile.close();
    return 0;
}
