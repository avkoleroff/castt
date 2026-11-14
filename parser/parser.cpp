#include "parser.hpp"
#include <stdexcept>

using namespace std;

namespace {
template<class T> shared_ptr<T> makeNode(const Token& t) {
    auto n = make_shared<T>();
    n->line = t.line;
    n->col  = t.col;
    return n;
}
}

// хендл токенов

Parser::Parser(const string& source) : lexer_(source) {
    advance(); 
    advance(); 
}

void Parser::advance() {
    cur_ = next_;
    do {
        next_ = lexer_.nextToken();
    } while (next_.type == TokenType::NEWLINE);
}

// проверки

bool Parser::check(TokenType t)          { return cur_.type == t; }
bool Parser::checkKw(const string& s)    { return check(TokenType::KEYWORD) && cur_.value == s; }
bool Parser::checkSym(const string& s)   { return check(TokenType::SYMBOL)  && cur_.value == s; }

bool Parser::accept(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

bool Parser::acceptKw(const string& s) {
    if (!checkKw(s)) return false;
    advance();
    return true;
}

bool Parser::acceptSym(const string& s) {
    if (!checkSym(s)) return false;
    advance();
    return true;
}

bool Parser::expectKw(const string& s) {
    if (!acceptKw(s)) fail("ожидается '" + s + "'");
    return true;
}
bool Parser::expectSym(const string& s) {
    if (!acceptSym(s)) fail("ожидается '" + s + "'");
    return true;
}

// вывод ошибок

void Parser::fail(const string& msg, int line, int col) {
    if (error_.empty()) {
        int l = (line  < 0) ? cur_.line : line;
        int c = (col   < 0) ? cur_.col  : col;
        error_ = to_string(l) + ":" + to_string(c) + ": " + msg;
    }
    throw runtime_error(error_);
}

// парсинг имен

bool Parser::parseName(string& out) {
    if (!check(TokenType::IDENTIFIER))
        fail("ожидается имя");
    out = cur_.value;
    advance();
    return true;
}

bool Parser::isTypeKeyword() {
    return checkKw("целое")     || checkKw("дробное") ||
           checkKw("логическое")|| checkKw("символ") ||
           checkKw("строка");
}

bool Parser::nextIsTypeStart() {
    return next_.type == TokenType::KEYWORD &&
          (next_.value == "цепь"   || next_.value == "целое"   ||
           next_.value == "дробное"|| next_.value == "логическое" ||
           next_.value == "символ" || next_.value == "строка");
}

// парсинг массивов

shared_ptr<TypeNode> Parser::parseTypeElement() {
    auto node = makeNode<TypeNode>(cur_);
    if (isTypeKeyword()) {
        node->name = cur_.value;
        advance();
    } else {
        parseName(node->name);
    }
    return node;
}

shared_ptr<TypeNode> Parser::parseType() {
    if (!checkKw("цепь"))
        return parseTypeElement();

    auto node = makeNode<TypeNode>(cur_);
    advance();                       // consume «цепь»
    node->kind = TypeNode::ARRAY;
    expectSym("[");
    if (!check(TokenType::INT))
        fail("ожидается целочисленный размер цепи");
    node->size = cur_.value;
    advance();
    expectSym("]");
    expectKw("из");
    node->element = parseTypeElement();
    return node;
}

// парсинг floid'а

shared_ptr<TypeNode> Parser::parseReturnType() {
    if (acceptKw("ничто"))
        return nullptr;
    return parseType();
}



shared_ptr<ExprNode> Parser::parsePostfix(shared_ptr<ExprNode> base) {
    while (checkSym("[") ||
          (cur_.type == TokenType::SYMBOL && cur_.value == "." && next_.type == TokenType::IDENTIFIER)) {
        auto node = makeNode<ExprNode>(cur_);
        node->left = base;

        if (acceptSym("[")) {
            node->kind  = ExprNode::INDEX;
            node->index = parseExpression();
            expectSym("]");
        } else {
            advance();
            node->kind = ExprNode::FIELD;
            parseName(node->field);
        }
        base = node;
    }
    return base;
}

shared_ptr<ExprNode> Parser::parseArgs() {
    auto node = makeNode<ExprNode>(cur_);
    node->kind = ExprNode::ARRAY;
    expectSym("(");
    if (!checkSym(")")) {
        do {
            node->elems.push_back(parseExpression());
        } while (acceptSym(";"));
    }
    expectSym(")");
    return node;
}

shared_ptr<ExprNode> Parser::parseNameExpr() {
    auto node = makeNode<ExprNode>(cur_);
    node->kind = ExprNode::NAME;
    parseName(node->value);
    return node;
}

shared_ptr<ExprNode> Parser::parseCast() {
    auto node = makeNode<ExprNode>(cur_);
    node->kind = ExprNode::CAST;
    advance(); 
    expectSym("(");
    node->castExpr = parseExpression();
    expectSym(")");
    expectKw("в");
    if (!isTypeKeyword() || checkKw("строка"))
        fail("ожидается скалярный тип преобразования");
    node->typeName = cur_.value;
    advance();
    return node;
}

// expression

shared_ptr<ExprNode> Parser::parseBaseExpr() {
    auto node = makeNode<ExprNode>(cur_);
    if (checkKw("взываю к Преображению с даром"))
        return parseCast();
        
    if (checkKw("воплощение") || checkKw("Исполнение заклинания")) {
        bool isCall = checkKw("Исполнение заклинания"); 
        advance();
        node->kind = isCall ? ExprNode::CALL : ExprNode::STRUCT_CREATE;
        parseName(node->value);
        if (isCall) expectKw("с дарами");
        node->elems = parseArgs()->elems;
        return node;
    }
    
    if (acceptSym("(")) {
        auto inner = parseExpression();
        expectSym(")");
        return inner;
    }
    
    if (acceptSym("[")) {
        node->kind = ExprNode::ARRAY;
        do {
            node->elems.push_back(parseExpression());
        } while (acceptSym(";"));
        expectSym("]");
        return node;
    }
    
    if (check(TokenType::IDENTIFIER))
        return parseNameExpr();
        
    switch (cur_.type) {
        case TokenType::INT:    node->kind = ExprNode::INT;    break;
        case TokenType::DOUBLE: node->kind = ExprNode::DOUBLE; break;
        case TokenType::CHAR:   node->kind = ExprNode::CHAR;   break;
        case TokenType::STRING: node->kind = ExprNode::STRING; break;
        default:
            if (checkKw("истина") || checkKw("ложь"))
                node->kind = ExprNode::BOOL;
            else
                fail("ожидается выражение");
    }
    node->value = cur_.value;
    advance();
    return node;
}

// уровни expression

shared_ptr<ExprNode> Parser::parseExprLevel1() {
    return parsePostfix(parseBaseExpr());
}

std::shared_ptr<ExprNode> Parser::parseExprLevel2() {
    if (checkSym("++") || checkSym("--")) {
        auto node = makeNode<ExprNode>(cur_);
        node->kind  = ExprNode::UNARY;
        node->value = cur_.value;
        advance();
        node->right = parseAssignTargetExpr(); 
        return node;
    }

    if (checkSym("-") || checkSym("!") || checkSym("~")) {
        auto node = makeNode<ExprNode>(cur_);
        node->kind  = ExprNode::UNARY;
        node->value = cur_.value;
        advance();
        node->right = parseExprLevel2();
        return node;
    }
    return parseExprLevel1();
}

shared_ptr<ExprNode> Parser::parseBinary(int level) {
    if (level == 2) return parseExprLevel2();

    static const vector<vector<string>> ops = {
        {}, {}, {}, {"*", "/", "%"}, {"+", "-"},
        {"<<", ">>"},
        {"<", ">", "<=", ">="},
        {"==", "!="},
        {"&"}, {"^"}, {"|"},
        {"&&"}, {"||"}, {","}
    };

    auto left = parseBinary(level - 1);
    while (true) {
        bool found = false;
        for (const auto& op : ops[level]) {
            if (checkSym(op)) { found = true; break; }
        }
        if (!found) break;

        auto node = makeNode<ExprNode>(cur_);
        node->kind  = ExprNode::BINARY;
        node->value = cur_.value;
        advance();

        node->left  = left;
        node->right = parseBinary(level - 1);
        left = node;

        if (level == 6 || level == 7) break;
    }
    return left;
}

#define LEVEL(N) shared_ptr<ExprNode> Parser::parseExprLevel##N() { return parseBinary(N); }
LEVEL(3) LEVEL(4) LEVEL(5) LEVEL(6) LEVEL(7) LEVEL(8) LEVEL(9)
LEVEL(10) LEVEL(11) LEVEL(12) LEVEL(13)
#undef LEVEL

shared_ptr<ExprNode> Parser::parseExpression() {
    return parseExprLevel13();
}

void Parser::parseAssignTarget(AssignNode& a) {
    a.line = cur_.line;
    a.col  = cur_.col;

    auto base = parseNameExpr();
    a.name = base->value;

    auto full = parsePostfix(base);
    vector<shared_ptr<ExprNode>> rev;
    while (full != base) {
        rev.push_back(full);
        full = full->left;
    }
    a.targets.assign(rev.rbegin(), rev.rend());
}

shared_ptr<ExprNode> Parser::parseAssignTargetExpr() {
    if (!check(TokenType::IDENTIFIER))
        fail("ожидается цель присваивания (имя)");
    return parsePostfix(parseNameExpr());
}

// statement

shared_ptr<StmtNode> Parser::parseVarDecl() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::VAR_DECL;
    advance();        

    auto type = parseType();
    do {
        auto decl = makeNode<VarDeclNode>(cur_);
        decl->type = type;
        parseName(decl->name);
        expectKw("со значением");
        decl->init = parseExpression();
        stmt->varDecls.push_back(decl);
    } while (acceptSym(";"));

    stmt->varDecl = stmt->varDecls.front();
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseAssign() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::ASSIGN;
    advance(); 

    stmt->assign = make_shared<AssignNode>();
    parseAssignTarget(*stmt->assign);
    expectKw("значение");
    stmt->assign->value = parseExpression();
    expectSym(".");
    return stmt;
}

// ввод / вывод
shared_ptr<StmtNode> Parser::parseInput() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::INPUT;
    advance(); 
    expectSym("(");
    do {
        auto target = make_shared<AssignNode>();
        parseAssignTarget(*target);
        stmt->targets.push_back(target);
    } while (acceptSym(";"));
    expectSym(")");
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseOutput() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::OUTPUT;
    advance();
    expectSym("(");
    do {
        stmt->args.push_back(parseExpression());
    } while (acceptSym(";"));
    expectSym(")");
    expectSym(".");
    return stmt;
}

// ветвление (if / for/ while)

shared_ptr<StmtNode> Parser::parseIf() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::IF;
    advance();
    expectSym("(");
    stmt->cond = parseExpression();
    expectSym(")");
    expectSym(",");
    expectKw("да свершится следующее");
    expectSym(".");
    stmt->thenBlock = parseBlock();

    if (acceptKw("Если же нет")) {
        expectSym(",");
        expectKw("да свершится иное");
        expectSym(".");
        stmt->elseBlock = parseBlock();
    }
    expectKw("Да свершится предначертанное");
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseWhile() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::WHILE;
    advance();
    expectSym(",");
    expectKw("взываю к вам");
    expectSym(".");
    expectKw("Покуда истинно предзнаменование");
    expectSym("(");
    stmt->cond = parseExpression();
    expectSym(")");
    expectSym(",");
    expectKw("вершите");
    expectSym(".");
    stmt->thenBlock = parseBlock();
    expectKw("Да замкнётся круг");
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseFor() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::FOR;
    advance();
    expectSym(",");
    expectKw("проведите");
    parseName(stmt->loopVar);
    expectKw("от");
    expectSym("(");
    stmt->from = parseExpression();
    expectSym(")");
    expectKw("до");
    expectSym("(");
    stmt->to = parseExpression();
    expectSym(")");
    expectKw("с шагом");
    expectSym("(");
    stmt->step = parseExpression();
    expectSym(")");
    expectSym(".");
    stmt->thenBlock = parseBlock();
    expectKw("Да завершится шествие");
    expectSym(".");
    return stmt;
}

// вызов функции

shared_ptr<StmtNode> Parser::parseCallStmt() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::CALL_STMT;
    advance();
    parseName(stmt->callName);
    expectKw("с дарами");
    stmt->args = parseArgs()->elems;
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseReturn() {
    auto stmt = makeNode<StmtNode>(cur_);
    bool isVoid = checkKw("Да завершится призыв");
    advance();
    stmt->kind = isVoid ? StmtNode::RETURN_VOID : StmtNode::RETURN;
    if (!isVoid)
        stmt->retValue = parseExpression();
    expectSym(".");
    return stmt;
}

// oh void, void, beautiful void! (пустой statement);

shared_ptr<StmtNode> Parser::parseEmpty() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::EMPTY;
    expectSym(".");
    return stmt;
}

shared_ptr<StmtNode> Parser::parseExprStmt() {
    auto stmt = makeNode<StmtNode>(cur_);
    stmt->kind = StmtNode::EXPR_STMT;
    stmt->expr = parseExpression();
    expectSym(".");
    return stmt;
}

// парсер statement

shared_ptr<StmtNode> Parser::parseStmt() {
    if (checkSym("."))          return parseEmpty();
    if (checkKw("Да будет"))     return parseVarDecl();
    if (checkKw("Да примет"))    return parseAssign();
    if (checkKw("Взываю к Слуху с даром")) return parseInput();
    if (checkKw("Взываю к Голосу с даром")) return parseOutput();
    if (checkKw("Если истинно предзнаменование")) return parseIf();
    if (checkKw("О великие силы повторения")) return parseWhile();
    if (checkKw("О хранители пути")) return parseFor();
    if (checkKw("Да исполнится")) return parseCallStmt();
    if (checkKw("Да станет ответом") || checkKw("Да завершится призыв")) return parseReturn();
    if (checkSym("++") || checkSym("--") ||
    checkKw("воплощение") || checkKw("Исполнение заклинания") ||
    checkKw("взываю к Преображению с даром")) return parseExprStmt();
    fail("ожидается инструкция");
    return nullptr;
}

// конец блока

vector<shared_ptr<StmtNode>> Parser::parseBlock() {
    vector<shared_ptr<StmtNode>> block;
    while (!check(TokenType::END_OF_FILE) &&
           !checkKw("Если же нет") &&
           !checkKw("Да свершится предначертанное") &&
           !checkKw("Да замкнётся круг") &&
           !checkKw("Да завершится шествие") &&
           !checkKw("Да запечатается заклинание")) {
        block.push_back(parseStmt());
    }
    return block;
}

// параметры

vector<shared_ptr<ParamNode>> Parser::parseParams() {
    vector<shared_ptr<ParamNode>> params;
    expectSym("(");
    if (!checkSym(")")) {
        do {
            auto param = make_shared<ParamNode>();
            if (cur_.type == TokenType::IDENTIFIER && nextIsTypeStart()) {
                parseName(param->name);
                param->type = parseType();
            } else {
                param->type = parseType();
                parseName(param->name);
            }
            params.push_back(param);
        } while (acceptSym(";"));
    }
    expectSym(")");
    return params;
}

// функции

shared_ptr<FuncNode> Parser::parseFunction() {
    auto func = makeNode<FuncNode>(cur_);
    advance();                       // consume «Запишем заклинание»
    parseName(func->name);
    expectKw("с дарами");
    func->params = parseParams();
    expectSym(",");
    expectKw("возвращающее");
    func->retTypeWhere = parseReturnType();
    func->retVoid = !func->retTypeWhere;
    expectSym(".");
    func->body = parseBlock();
    expectKw("Да запечатается заклинание");
    expectSym(".");
    return func;
}

// структуры (WIP)

shared_ptr<StructNode> Parser::parseStruct() {
    auto struc = makeNode<StructNode>(cur_);
    advance();
    parseName(struc->name);
    expectSym(".");
    do {
        auto field = makeNode<StructFieldNode>(cur_);
        field->type = parseType();
        do {
            string name;
            parseName(name);
            field->names.push_back(name);
        } while (acceptSym(";"));
        expectSym(".");
        struc->fields.push_back(field);
    } while (!checkKw("Да завершится облик") && !check(TokenType::END_OF_FILE));
    expectKw("Да завершится облик");
    expectSym(".");
    return struc;
}


shared_ptr<EnumNode> Parser::parseEnum() {
    auto enm = makeNode<EnumNode>(cur_);
    advance();
    parseName(enm->name);
    expectSym(".");
    do {
        string name;
        parseName(name);
        enm->values.push_back(name);
    } while (acceptSym(","));
    expectSym(".");
    expectKw("Да завершится перечень");
    expectSym(".");
    return enm;
}

// main

shared_ptr<ProgramNode> Parser::parseProgram() {
    if (!ok()) return nullptr;
    try {
        auto prog = make_shared<ProgramNode>();
        while (checkKw("Запишем заклинание")) {
            prog->functions.push_back(parseFunction());
        }
        while (!check(TokenType::END_OF_FILE)) {
            if (checkKw("Наречём новый облик")) {
                prog->structs.push_back(parseStruct());
            } else if (checkKw("Наречём перечень")) {
                prog->enums.push_back(parseEnum());
            } else {
                prog->statements.push_back(parseStmt());
            }
        }
        return prog;
    } catch (const runtime_error&) {
        return nullptr;
    }
}
