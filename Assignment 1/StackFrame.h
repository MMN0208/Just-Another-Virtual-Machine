#ifndef __STACK_FRAME_H__
#define __STACK_FRAME_H__

#include <string>

/*
Value declaration
*/
struct Value{
    int iVal;
    float fVal;
};
/*
Stack declaration
*/
class Stack{
    class Node{
        Value data;
        Node *next;
        friend class Stack;
    public:
        Node();
        Node(Value data);
        Node(Value data, Node *next);
    };
    Node *head;
    Node *tail;
    int count;
public:
    Stack();
    ~Stack();
    void push(Value data);
    Value pop();
    Value top();
    int size();
};
/*
StackFrame declaration
*/
class StackFrame {
    int opStackMaxSize; // max size of operand stack
    int localVarArrSize; // size of local variable array
    Stack *operandStack;
    Value *localVarArr;
public:
    /*
    Constructor of StackFrame
    */
    StackFrame();
    /*
    Destructor of StackFrame
    */
    ~StackFrame();
    /*
    Run the method written in the testcase
    @param filename name of the file
    */
    void run(std::string filename);
    bool stringCompare(std::string a, std::string b);
    void doCommand(std::string op, std::string value, int curLine, int* visitedArrInd, Stack *operandStack, Value *localVarrArr);
};

#endif // !__STACK_FRAME_H__