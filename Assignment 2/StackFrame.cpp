#include "StackFrame.h"
#include <iostream>
#include <fstream>
#include "errors.h"
#include "constants.h"
using namespace std;

int stringCompare(string a, string b) {
    int result = 0;
    int m = a.length(), n = b.length();
    int loop = (m < n) ? m : n;
    for(int i = 0; i < loop && !result; i++) {
        if(int(a[i]) < int(b[i])) result = -1;
        else if(int(a[i]) > int(b[i])) result = 1;
    }
    if(!result && m < n) result = -1;
    else if(!result && m > n) result = 1;
    return result;
}

Node::Node() : key(""), balance(0), pLeft(NULL), pRight(NULL) {}

Node::Node(Value data, int type, string key) : data(data), type(type), key(key), balance(0), pLeft(NULL), pRight(NULL) {}

AVL::AVL() : root(NULL), count(0) {}

int AVL::getHeight(Node *root) {
    if(!root) return 0;
    int leftH = 1 + getHeight(root->pLeft);
    int rightH = 1 + getHeight(root->pRight);
    if(leftH > rightH) return leftH;
    return rightH;
}

void AVL::setBalance(Node *root) {
    if(!root) return;
    int leftSubH = getHeight(root->pLeft);
    int rightSubH = getHeight(root->pRight);
    if(leftSubH - rightSubH > 1) root->balance = -1;
    else if(leftSubH - rightSubH < -1) root->balance = 1;
    else root->balance = 0;
}

Node* AVL::leftRotate(Node *root) {
    if(!root) return NULL;
    Node *newRoot = root->pRight;
    Node *temp = newRoot->pLeft;
    newRoot->pLeft = root;
    root->pRight = temp;
    setBalance(root);
    setBalance(newRoot);
    return newRoot;
}

Node* AVL::rightRotate(Node *root) {
    if(!root) return NULL;
    Node *newRoot = root->pLeft;
    Node *temp = newRoot->pRight;
    newRoot->pRight = root;
    root->pLeft = temp;
    setBalance(root);
    setBalance(newRoot);
    return newRoot;
}

Node* AVL::insertNode(Node *root, Value data, int type, string key) {
    if(!root) return new Node(data, type, key);
    if(stringCompare(root->key, key) == 1) root->pLeft =  insertNode(root->pLeft, data, type, key);
    else if(stringCompare(root->key, key) == -1) root->pRight = insertNode(root->pRight, data, type, key);
    else {
        root->data = data;
        root->type = type;
        root->key = key;
        return root;
    }
    setBalance(root);
    if(root->balance == -1 && stringCompare(root->pLeft->key, key) == 1) {
        return rightRotate(root);
    }
    else if(root->balance == -1 && stringCompare(root->pLeft->key, key) == -1) {
        root->pLeft = leftRotate(root->pLeft);
        return rightRotate(root);
    }
    else if(root->balance == 1 && stringCompare(root->pRight->key, key) == -1) {
        return leftRotate(root);
    }
    else if(root->balance == 1 && stringCompare(root->pRight->key, key) == 1) {
        root->pRight = rightRotate(root->pRight);
        return leftRotate(root);
    }
    count++;
    return root;
}

Value AVL::getNodeData(Node *root, string key) {
    if(!root) {
        Value t;
        t.iVal = -1;
        t.fVal = -1;
        return t;
    }
    if(stringCompare(root->key, key) == 1) {
        return getNodeData(root->pLeft, key);
    }
    else if(stringCompare(root->key, key) == -1) {
        return getNodeData(root->pRight, key);
    }
    return root->data;
}

int AVL::getNodeType(Node *root, string key) {
    if(!root) {
        return -1;
    }
    if(stringCompare(root->key, key) == 1) {
        return getNodeType(root->pLeft, key);
    }
    else if(stringCompare(root->key, key) == -1) {
        return getNodeType(root->pRight, key);
    }
    return root->type;
}

string AVL::parentNode(Node *root, string key) {
    if(!root) {
        return "";
    }
    if(stringCompare(root->key, key) == 1) {
        if(root->pLeft && !stringCompare(root->pLeft->key, key)) return root->key;
        return parentNode(root->pLeft, key);
    }
    if(stringCompare(root->key, key) == -1) {
        if(root->pRight && !stringCompare(root->pRight->key, key)) return root->key;
        return parentNode(root->pRight, key);
    }
    return "null";
}

void AVL::deleteTree(Node *root) {
    if(root) {
        deleteTree(root->pLeft);
        deleteTree(root->pRight);
        free(root);
    }
    root = NULL;
}

void AVL::insert(Value data, int type, string key) {
    root = insertNode(root, data, type, key);
}

Value AVL::getData(string key) {
    return getNodeData(root, key);
}

int AVL::getType(string key) {
    return getNodeType(root, key);
}

string AVL::parent(string key) {
    return parentNode(root, key);
}

int AVL::height() {
    return getHeight(root);
}

void AVL::clear() {
    deleteTree(root);
}

int AVL::size() {
    return this->count;
}

StackFrame::StackFrame() : opStackMaxSize(OPERAND_STACK_MAX_SIZE), localVarSpaceSize(LOCAL_VARIABLE_SPACE_SIZE) {}

StackFrame::~StackFrame() {
    while(!operandStack.empty()) {
        operandStack.pop();
    }
    localVarSpace.clear();
}

void StackFrame::run(string filename) {
    ifstream myFile(filename);
    string operation;
    int curLine = 0;
    if(myFile.is_open()) {
        while(getline(myFile, operation)) {
            curLine++;
            int n = operation.length();
            int index = 0;
            string op = "";
            string value = "";
            while(operation[index] != ' ' && operation[index] != '\r' && index < n) {
                op += operation[index];
                index++;
            }
            if(operation[index] != '\r' && index < n) index++;
            while(operation[index] != '\r' && index < n) {
                value += operation[index];
                index++;
            }
            doCommand(op, value, curLine);
        }
    }
    myFile.close();
    return;
}

void StackFrame::doCommand(string op, string value, int curLine) {
    if(!stringCompare(op, "iconst")) {
        if(int(operandStack.size()) == opStackMaxSize) throw StackFull(curLine);
        else {
            Value v;
            v.iVal = stoi(value);
            operandStack.push(v);
            Value t;
            t.iVal = 0;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "fconst")) {
        if(int(operandStack.size()) == opStackMaxSize) throw StackFull(curLine);
        else {
            Value v;
            v.fVal = stof(value);
            operandStack.push(v);
            Value t;
            t.iVal = 1;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "iload")) {
        Value v = localVarSpace.getData(value);
        int t1 = localVarSpace.getType(value);
        if(t1 == -1) throw UndefinedVariable(curLine);
        else {
            if(t1 == 1) throw TypeMisMatch(curLine);
            else if(int(operandStack.size()) == opStackMaxSize) throw StackFull(curLine);
            else {
                operandStack.push(v);
                Value t;
                t.iVal = t1;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fload")) {
        Value v = localVarSpace.getData(value);
        int t1 = localVarSpace.getType(value);
        if(t1 == -1) throw UndefinedVariable(curLine);
        else {
            if(t1 == 0) throw TypeMisMatch(curLine);
            else if(int(operandStack.size()) == opStackMaxSize) throw StackFull(curLine);
            else {
                operandStack.push(v);
                Value t;
                t.iVal = t1;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "istore")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack.top();
            operandStack.pop();
            Value v = operandStack.top();
            operandStack.pop();
            if(t.iVal) throw TypeMisMatch(curLine);
            else {
                if(localVarSpace.size() == localVarSpaceSize) throw LocalSpaceFull(curLine);
                else {
                    localVarSpace.insert(v, t.iVal, value);
                }
            }
        }
    }
    else if(!stringCompare(op, "fstore")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack.top();
            operandStack.pop();
            Value v = operandStack.top();
            operandStack.pop();
            if(!t.iVal) throw TypeMisMatch(curLine);
            else {
                if(localVarSpace.size() == localVarSpaceSize) throw LocalSpaceFull(curLine);
                else {
                    localVarSpace.insert(v, t.iVal, value);
                }
            }
        }
    }
    else if(!stringCompare(op, "top")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack.top();
            operandStack.pop();
            Value v = operandStack.top();
            operandStack.push(t);
            if(!t.iVal) cout << v.iVal << endl;
            else cout << v.fVal << endl;
        }
    }
    else if(!stringCompare(op, "val")) {
        Value v = localVarSpace.getData(value);
        int t = localVarSpace.getType(value);
        if(t == -1) throw UndefinedVariable(curLine);
        else {
            if(!t) cout << v.iVal << endl;
            else cout << v.fVal << endl;
        }
    }
    else if(!stringCompare(op, "par")) {
        string par = localVarSpace.parent(value);
        if(par == "") throw UndefinedVariable(curLine);
        else cout << par << endl;
    }
    else if(!stringCompare(op, "iadd")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal + v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fadd")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal += v1.fVal;
            else v.fVal += v1.iVal;
            operandStack.push(v);
            Value t;
            t.iVal = 1;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "isub")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal - v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fsub")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal -= v1.fVal;
            else v.fVal -= v1.iVal;
            operandStack.push(v);
            Value t;
            t.iVal = 1;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "imul")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal * v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fmul")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal *= v1.fVal;
            else v.fVal *= v1.iVal;
            operandStack.push(v);
            Value t;
            t.iVal = 1;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "idiv")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                if(!v1.iVal) throw DivideByZero(curLine);
                else {
                    Value v;
                    v.iVal = v2.iVal / v1.iVal;
                    operandStack.push(v);
                    Value t;
                    t.iVal = 0;
                    operandStack.push(t);
                }
            }
        }
    }
    else if(!stringCompare(op, "fdiv")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if((t1.iVal && !v1.fVal) || (!t1.iVal && !v1.iVal)) throw DivideByZero(curLine);
            else {
                Value v;
                if(t2.iVal) v.fVal = v2.fVal;
                else v.fVal = v2.iVal;
                if(t1.iVal) v.fVal /= v1.fVal;
                else v.fVal /= v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 1;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "irem")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                if(!v1.iVal) throw DivideByZero(curLine);
                else {
                    Value v;
                    v.iVal = v2.iVal - (v2.iVal / v1.iVal) * v1.iVal;
                    operandStack.push(v);
                    Value t;
                    t.iVal = 0;
                    operandStack.push(t);
                }
            }
        }
    }
    else if(!stringCompare(op, "ineg")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v1.iVal * -1;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fneg")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value v;
            if(t1.iVal) v.fVal = v1.fVal * -1;
            else v.fVal = float(v1.iVal) * -1;
            operandStack.push(v);
            Value t;
            t.iVal = 1;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "iand")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal & v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "ior")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal | v1.iVal;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "ieq")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal == v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "feq")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 == temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack.push(v);
            Value t;
            t.iVal = 0;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "ineq")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal != v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fneq")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 != temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack.push(v);
            Value t;
            t.iVal = 0;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "ilt")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal < v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "flt")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 < temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack.push(v);
            Value t;
            t.iVal = 0;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "igt")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal > v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "fgt")) {
        if(operandStack.size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            Value t2 = operandStack.top();
            operandStack.pop();
            Value v2 = operandStack.top();
            operandStack.pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 > temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack.push(v);
            Value t;
            t.iVal = 0;
            operandStack.push(t);
        }
    }
    else if(!stringCompare(op, "ibnot")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(!v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "i2f")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.fVal = float(v1.iVal);
                operandStack.push(v);
                Value t;
                t.iVal = 1;
                operandStack.push(t);
            }
        }
    }
    else if(!stringCompare(op, "f2i")) {
        if(operandStack.size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack.top();
            operandStack.pop();
            Value v1 = operandStack.top();
            operandStack.pop();
            if(!t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = int(v1.fVal);
                operandStack.push(v);
                Value t;
                t.iVal = 0;
                operandStack.push(t);
            }
        }
    }
    else return;
}