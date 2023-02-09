#ifndef __STACK_FRAME_H__
#define __STACK_FRAME_H__

#include <string>
#include <stack>

/*
Compare string
*/
int stringCompare(std::string a, std::string b); // -1 - a smaller than b, 0 - a equal b, 1 - a greater than b
/*
Value declaration
*/
struct Value {
    int iVal;
    float fVal;
};
/*
Node declaration
*/
class Node {
    Value data;
    int type;
    std::string key;
    int balance; // 0 - balanced, -1 - left heavy, 1 - right heavy
    Node *pLeft;
    Node *pRight;
    friend class AVL;
public:
    Node();
    Node(Value data, int type, std::string key);
};
/*
AVL declaration
*/
class AVL {
    Node *root;
    int count;
    Node *leftRotate(Node *root);
    Node *rightRotate(Node *root);
    Node *insertNode(Node *root, Value data, int type, std::string key);
    Value getNodeData(Node *root, std::string key);
    int getNodeType(Node *root, std::string key);
    std::string parentNode(Node *root, std::string key);
    int getHeight(Node *root);
    void deleteTree(Node *root);
    void setBalance(Node *root);
public:
    AVL();
    void insert(Value data, int type, std::string key);
    Value getData(std::string key);
    int getType(std::string key);
    std::string parent(std::string key);
    int height();
    void clear();
    int size();
};
/*
StackFrame declaration
*/
class StackFrame {
    int opStackMaxSize; // max size of operand stack
    int localVarSpaceSize; // size of local variable array
    std::stack<Value> operandStack;
    AVL localVarSpace;
public:
    /*
    Constructor of StackFrame
    */
    StackFrame();
    ~StackFrame();
    /*
    Run the method written in the testcase
    @param filename name of the file
    */
    void run(std::string filename);
    void doCommand(std::string op, std::string value, int curLine);
};

#endif // !__STACK_FRAME_H__