#include "StackFrame.h"
#include <iostream>
#include <fstream>
#include "errors.h"
#include "constants.h"
using namespace std;

Stack::Node::Node() {
    this->next = NULL;
}

Stack::Node::Node(Value data) {
    this->data = data;
    this->next = NULL;
}

Stack::Node::Node(Value data, Node *next) {
    this->data = data;
    this->next = next;
}

Stack::Stack() {
    this->head = NULL;
    this->tail = NULL;
    count = 0;
}

Stack::~Stack() {
    while(this->head) {
        Node *temp = this->head;
        this->head = this->head->next;
        free(temp);
    }
    this->tail = this->head;
    count = 0;
}

void Stack::push(Value data) {
    this->head = new Node(data, head);
    if(!this->tail) this->tail = this->head;
    count++;
}

Value Stack::pop() {
    Node *temp = this->head;
    Value value = temp->data;
    this->head = this->head->next;
    if(!this->head) this->tail = NULL;
    count--;
    free(temp);
    return value;
}

Value Stack::top() {
    return this->head->data;
}

int Stack::size() {
    return this->count;
}

StackFrame::StackFrame() : opStackMaxSize(OPERAND_STACK_MAX_SIZE), localVarArrSize(LOCAL_VARIABLE_ARRAY_SIZE) {}

StackFrame::~StackFrame() {
    delete operandStack;
    delete[] localVarArr;
}

void StackFrame::run(string filename) {
    ifstream myFile(filename);
    string operation[200];
    int numLine = 0;
    if(myFile.is_open()) {
        while(getline(myFile, operation[numLine])) {
        numLine++;
        }
    }
    else return;
    operandStack = new Stack();
    localVarArr = new Value[localVarArrSize];
    int visitedArrInd[128] = {};
    for(int i = 0; i < numLine; i++) {
        int curLine = i + 1;
        int n = operation[i].length();
        int index = 0;
        string op = "";
        string value = "";
        while(operation[i][index] != ' ' && operation[i][index] != '\r' && index < n) {
            op += operation[i][index];
            index++;
        }
        if(operation[i][index] != '\r' && index < n) index++;
        while(operation[i][index] != '\r' && index < n) {
            value += operation[i][index];
            index++;
        }
        try {
            doCommand(op, value, curLine, visitedArrInd, operandStack, localVarArr);
        }
        catch(TypeMisMatch& TMM) {
            cout << TMM.what();
            return;
        }
        catch(DivideByZero& DBZ) {
            cout << DBZ.what();
            return;
        }
        catch(StackFull& SF) {
            cout << SF.what();
            return;
        }
        catch(StackEmpty& SE) {
            cout << SE.what();
            return;
        }
        catch(ArrayOutOfRange& AOOR) {
            cout << AOOR.what();
            return;
        }
        catch(UndefinedVariable& UV) {
            cout << UV.what();
            return;
        }
    }
}

bool StackFrame::stringCompare(string a, string b) {
    int m = a.length(), n = b.length();
    if(m != n) return false;
    for(int i = 0; i < m; i++) {
        if(a[i] != b[i]) return false;
    }
    return true;
}

void StackFrame::doCommand(string op, string value, int curLine, int* visitedArrInd, Stack *operandStack, Value *localVarrArr) {
    if(stringCompare(op, "iconst")) {
        if(operandStack->size() == opStackMaxSize) throw StackFull(curLine);
        else {
            Value v;
            v.iVal = stoi(value);
            operandStack->push(v);
            Value t;
            t.iVal = 0;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "fconst")) {
        if(operandStack->size() == opStackMaxSize) throw StackFull(curLine);
        else {
            Value v;
            v.fVal = stof(value);
            operandStack->push(v);
            Value t;
            t.iVal = 1;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "iload")) {
        int ind = stoi(value);
        if(ind >= localVarArrSize) throw ArrayOutOfRange(curLine);
        else if(!visitedArrInd[ind / 2]) throw UndefinedVariable(curLine);
        else {
            if(localVarArr[ind].iVal) throw TypeMisMatch(curLine);
            else if(operandStack->size() == opStackMaxSize) throw StackFull(curLine);
            else {
                operandStack->push(localVarArr[ind + 1]);
                operandStack->push(localVarArr[ind]);
            }
        }
    }
    else if(stringCompare(op, "fload")) {
        int ind = stoi(value);
        if(ind >= localVarArrSize) throw ArrayOutOfRange(curLine);
        else if(!visitedArrInd[ind / 2]) throw UndefinedVariable(curLine);
        else {
            if(!localVarArr[ind].iVal) throw TypeMisMatch(curLine);
            else if(operandStack->size() == opStackMaxSize) throw StackFull(curLine);
            else {
                operandStack->push(localVarArr[ind + 1]);
                operandStack->push(localVarArr[ind]);
            }
        }
    }
    else if(stringCompare(op, "istore")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack->pop();
            Value v = operandStack->pop();
            if(t.iVal) throw TypeMisMatch(curLine);
            else {
                int ind = stoi(value);
                if(ind >= localVarArrSize) throw ArrayOutOfRange(curLine);
                else {
                    visitedArrInd[ind / 2] = 1;
                    localVarArr[ind] = t;
                    localVarArr[ind + 1] = v;
                }
            }
        }
    }
    else if(stringCompare(op, "fstore")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack->pop();
            Value v = operandStack->pop();
            if(!t.iVal) throw TypeMisMatch(curLine);
            else {
                int ind = stoi(value);
                if(ind >= localVarArrSize) throw ArrayOutOfRange(curLine);
                else {
                    visitedArrInd[ind / 2] = 1;
                    localVarArr[ind] = t;
                    localVarArr[ind + 1] = v;
                }
            }
        }
    }
    else if(stringCompare(op, "top")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t = operandStack->pop();
            Value v = operandStack->top();
            operandStack->push(t);
            if(!t.iVal) cout << v.iVal << endl;
            else cout << v.fVal << endl;
        }
    }
    else if(stringCompare(op, "val")) {
        int ind = stoi(value);
        if(ind >= localVarArrSize) throw ArrayOutOfRange(curLine);
        else if(!visitedArrInd[ind / 2]) throw UndefinedVariable(curLine);
        else {
            if(!localVarArr[ind].iVal) cout << localVarArr[ind + 1].iVal << endl;
            else cout << localVarArr[ind + 1].fVal << endl;
        }
    }
    else if(stringCompare(op, "iadd")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal + v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fadd")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal += v1.fVal;
            else v.fVal += v1.iVal;
            operandStack->push(v);
            Value t;
            t.iVal = 1;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "isub")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal - v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fsub")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal -= v1.fVal;
            else v.fVal -= v1.iVal;
            operandStack->push(v);
            Value t;
            t.iVal = 1;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "imul")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal * v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fmul")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            Value v;
            if(t2.iVal) v.fVal = v2.fVal;
            else v.fVal = v2.iVal;
            if(t1.iVal) v.fVal *= v1.fVal;
            else v.fVal *= v1.iVal;
            operandStack->push(v);
            Value t;
            t.iVal = 1;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "idiv")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                if(!v1.iVal) throw DivideByZero(curLine);
                else {
                    Value v;
                    v.iVal = v2.iVal / v1.iVal;
                    operandStack->push(v);
                    Value t;
                    t.iVal = 0;
                    operandStack->push(t);
                }
            }
        }
    }
    else if(stringCompare(op, "fdiv")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if((t1.iVal && !v1.fVal) || (!t1.iVal && !v1.iVal)) throw DivideByZero(curLine);
            else {
                Value v;
                if(t2.iVal) v.fVal = v2.fVal;
                else v.fVal = v2.iVal;
                if(t1.iVal) v.fVal /= v1.fVal;
                else v.fVal /= v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 1;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "irem")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                if(!v1.iVal) throw DivideByZero(curLine);
                else {
                    Value v;
                    v.iVal = v2.iVal - (v2.iVal / v1.iVal) * v1.iVal;
                    operandStack->push(v);
                    Value t;
                    t.iVal = 0;
                    operandStack->push(t);
                }
            }
        }
    }
    else if(stringCompare(op, "ineg")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v1.iVal * -1;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fneg")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value v;
            if(t1.iVal) v.fVal = v1.fVal * -1;
            else v.fVal = float(v1.iVal) * -1;
            operandStack->push(v);
            Value t;
            t.iVal = 1;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "iand")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal & v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "ior")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = v2.iVal | v1.iVal;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "ieq")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal == v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "feq")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 == temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack->push(v);
            Value t;
            t.iVal = 0;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "ineq")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal != v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fneq")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 != temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack->push(v);
            Value t;
            t.iVal = 0;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "ilt")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal < v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "flt")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 < temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack->push(v);
            Value t;
            t.iVal = 0;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "igt")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            if(t1.iVal != t2.iVal || t1.iVal || t2.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(v2.iVal > v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "fgt")) {
        if(operandStack->size() < 4) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            Value t2 = operandStack->pop();
            Value v2 = operandStack->pop();
            float temp1, temp2;
            if(t2.iVal) temp2 = v2.fVal;
            else temp2 = float(v2.iVal);
            if(t1.iVal) temp1 = v1.fVal;
            else temp1 = float(v1.iVal);
            Value v;
            if(temp2 > temp1) v.iVal = 1;
            else v.iVal = 0;
            operandStack->push(v);
            Value t;
            t.iVal = 0;
            operandStack->push(t);
        }
    }
    else if(stringCompare(op, "ibnot")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                if(!v1.iVal) v.iVal = 1;
                else v.iVal = 0;
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "i2f")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            if(t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.fVal = float(v1.iVal);
                operandStack->push(v);
                Value t;
                t.iVal = 1;
                operandStack->push(t);
            }
        }
    }
    else if(stringCompare(op, "f2i")) {
        if(operandStack->size() < 2) throw StackEmpty(curLine);
        else {
            Value t1 = operandStack->pop();
            Value v1 = operandStack->pop();
            if(!t1.iVal) throw TypeMisMatch(curLine);
            else {
                Value v;
                v.iVal = int(v1.fVal);
                operandStack->push(v);
                Value t;
                t.iVal = 0;
                operandStack->push(t);
            }
        }
    }
}