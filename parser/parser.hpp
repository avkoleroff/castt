#include <string>
#include <vector>
#include <memory>
#include "lexer/lexer.hpp"

using namespace std;

struct TypeNode {
    enum Kind { BASE, ARRAY } kind = BASE;
    string name;
    shared_ptr<TypeNode> element;
    string size;
    int line = 0, col = 0;
};

struct ExprNode {
    enum Kind {
        INT, DOUBLE, CHAR, STRING, BOOL,
        NAME,
        BINARY, UNARY,
        INDEX, FIELD,
        ARRAY,
        STRUCT_CREATE,
        CALL,
        CAST
    } kind;
    string value;
    shared_ptr<ExprNode> left, right;
    string field;
    shared_ptr<ExprNode> index;
    vector<shared_ptr<ExprNode>> elems;
    string typeName; 
    shared_ptr<ExprNode> castExpr;
    int line = 0, col = 0;
};

struct VarDeclNode {
    shared_ptr<TypeNode> type;
    string name;
    shared_ptr<ExprNode> init;
    int line = 0, col = 0;
};

struct AssignNode {
    string name;
    vector<shared_ptr<ExprNode>> targets;
    shared_ptr<ExprNode> value;
    int line = 0, col = 0;
};

struct StmtNode {
    enum Kind {
        VAR_DECL, ASSIGN, INPUT, OUTPUT,
        IF, WHILE, FOR, CALL_STMT, RETURN, RETURN_VOID, EMPTY, EXPR_STMT
    } kind;
    shared_ptr<VarDeclNode> varDecl;
    vector<shared_ptr<VarDeclNode>> varDecls;
    shared_ptr<AssignNode> assign;
    vector<shared_ptr<AssignNode>> targets;
    vector<shared_ptr<ExprNode>> args;
    shared_ptr<ExprNode> cond;
    vector<shared_ptr<StmtNode>> thenBlock;
    vector<shared_ptr<StmtNode>> elseBlock;

    string loopVar;
    shared_ptr<ExprNode> from, to, step;

    string callName;
    shared_ptr<ExprNode> retValue;
    shared_ptr<ExprNode> expr;
    int line = 0, col = 0;
};

struct StructFieldNode {
    shared_ptr<TypeNode> type;
    vector<string> names;
    int line = 0, col = 0;
};

struct StructNode {
    string name;
    vector<shared_ptr<StructFieldNode>> fields;
    int line = 0, col = 0;
};

struct EnumNode {
    string name;
    vector<string> values;
    int line = 0, col = 0;
};

struct ParamNode {
    shared_ptr<TypeNode> type;
    string name;
};

struct FuncNode {
    string name;
    vector<shared_ptr<ParamNode>> params;
    shared_ptr<TypeNode> retTypeWhere;
    bool retVoid = false;
    vector<shared_ptr<StmtNode>> body;
    int line = 0, col = 0;
};

struct ProgramNode {
    vector<shared_ptr<FuncNode>> functions;
    vector<shared_ptr<StmtNode>> statements;
    vector<shared_ptr<StructNode>> structs;
    vector<shared_ptr<EnumNode>> enums;
};

// парсер

class Parser {
public:
    explicit Parser(const string& source);
    shared_ptr<ProgramNode> parseProgram();
    bool ok() const { return error_.empty(); }
    const string& error() const { return error_; }

private:
    Lexer lexer_;
    Token cur_;
    Token next_;
    shared_ptr<ExprNode> parseBinary(int level);
    shared_ptr<ExprNode> parsePostfix(shared_ptr<ExprNode> base);
    bool fieldAhead() const;
    string error_;

    void advance();
    bool check(TokenType t);
    bool checkKw(const string& v);
    bool checkSym(const string& v); 
    bool accept(TokenType t);
    bool acceptKw(const string& v);
    bool acceptSym(const string& v);
    bool expectSym(const string& v);
    bool expectKw(const string& v);
    void skipNewlines();

bool nextIsTypeStart();
    // имена
    bool parseName(string& out);

    // типы
    shared_ptr<TypeNode> parseType();
    shared_ptr<TypeNode> parseTypeElement();
    shared_ptr<TypeNode> parseReturnType();

    // выражения
    shared_ptr<ExprNode> parseExpression();
    shared_ptr<ExprNode> parseExprLevel13();
    shared_ptr<ExprNode> parseExprLevel12();
    shared_ptr<ExprNode> parseExprLevel11();
    shared_ptr<ExprNode> parseExprLevel10();
    shared_ptr<ExprNode> parseExprLevel9();
    shared_ptr<ExprNode> parseExprLevel8();
    shared_ptr<ExprNode> parseExprLevel7();
    shared_ptr<ExprNode> parseExprLevel6();
    shared_ptr<ExprNode> parseExprLevel5();
    shared_ptr<ExprNode> parseExprLevel4();
    shared_ptr<ExprNode> parseExprLevel3();
    shared_ptr<ExprNode> parseExprLevel2();
    shared_ptr<ExprNode> parseExprLevel1();
    shared_ptr<StmtNode> parseExprStmt();
    shared_ptr<ExprNode> parseBaseExpr();
    shared_ptr<ExprNode> parseNameExpr();
    shared_ptr<ExprNode> parseCast();
    shared_ptr<ExprNode> parseArgs();
    bool isTypeKeyword();

    // инструкции
    shared_ptr<StmtNode> parseStmt();
    shared_ptr<StmtNode> parseVarDecl();
    shared_ptr<StmtNode> parseAssign();
    shared_ptr<StmtNode> parseInput();
    shared_ptr<StmtNode> parseOutput();
    shared_ptr<StmtNode> parseIf();
    shared_ptr<StmtNode> parseWhile();
    shared_ptr<StmtNode> parseFor();
    shared_ptr<StmtNode> parseCallStmt();
    shared_ptr<StmtNode> parseReturn();
    shared_ptr<StmtNode> parseEmpty();
    vector<shared_ptr<StmtNode>> parseBlock();
    void parseAssignTarget(AssignNode& a); 
    std::shared_ptr<ExprNode> parseAssignTargetExpr();

    // декларации
    shared_ptr<FuncNode> parseFunction();
    shared_ptr<StructNode> parseStruct();
    shared_ptr<EnumNode> parseEnum();
    vector<shared_ptr<ParamNode>> parseParams();

    void fail(const string& msg, int line = -1, int col = -1);
};
