//
// Created by qiaoz on 12/7/2017.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <cstring>
#include <map>

using namespace std;
class solution {

private:

    bool DEBUG;
    int clck;
    vector<string> vect_lines; // stores all operations
    vector<string> vect_operands; // stores all operands
    vector<int>* t_vars; // stores the value of ans
    map<string,int> labelIndex; // a map stores where each label appears
    int n; // the number of registers
    int* ans; // an array stores calculation result

public :

    solution(ifstream &file_in, int clck_in = 10, bool DEBUG_in = false){
        if(!file_in.is_open()) {
            cout<<"File not open"<<endl;
        }

        string input;
        file_in>>input;
        vector<string> inputStatus =splitString(input.c_str());
        n = (int)inputStatus.size();
        const int size = n;
        ans = new int[size];
        for (int i = 0; i < n; i++) {
            int d = atoi(inputStatus[i].c_str());
            ans[i] = d;
        }
        // initiate vect_lines and vect_operands
        while(!file_in.eof()) {
            string oper;
            file_in>>oper;
            if (oper == "end") break;
            if (oper[0] != 'l') {
                vect_lines.push_back(oper);
            } else {
                labelIndex.insert(pair<string,int>(oper,(int)vect_lines.size()));
                file_in>>oper;
                vect_lines.push_back(oper);
            }
            string operands;
            file_in>>operands;
            vect_operands.push_back(operands);
        }

        DEBUG = DEBUG_in;
        clck = clck_in;
        t_vars = new vector<int>(0);
    }

    void calculate(int ans[], string oper, const char* input) {
        vector<string> operands =splitString(input);
        int targetIndex = parseRegister(operands[0]);
        int operand1Index = parseRegister(operands[1]);
        int operand2Index = parseRegister(operands[2]);
        if(oper == "add") {
            ans[targetIndex] = ans[operand1Index] + ans[operand2Index];
        }
        if(oper == "addi") {
            ans[targetIndex] = ans[operand1Index] + operand2Index;
        }
        if(oper == "sub") {
            ans[targetIndex] = ans[operand1Index] - ans[operand2Index];
        }
        if(oper == "mult") {
            ans[targetIndex] = ans[operand1Index] * ans[operand2Index];
        }
        if(oper == "div") {
            ans[targetIndex] = ans[operand1Index] / ans[operand2Index];
        }
    }

    int parseRegister(string target) {
        int index = 0;
        if (target[0] == '$') {
            target.erase(0,1);
        }
        sscanf(target.c_str(),"%d",&index);
        return index;
    }

    vector<string> splitString(const char * input) {

        char sequence[100]={0};
        strncpy(sequence,input,100);
        char *tokenPtr;

        tokenPtr=strtok(sequence,",");
        vector<string> strings;
        while(tokenPtr!=NULL)
        {
            string str = tokenPtr;
            strings.push_back(str);
            tokenPtr=strtok(NULL,",");
        }
        return strings;
    }

    void dbg(const string &msg){}

    vector<int> *alu(){
        for (int i = 0; i < vect_lines.size(); i++) {
            if (!mips_clock()) {
                i--;
                continue;
            } else {
                string oper = vect_lines[i];
                string operands = vect_operands[i];
                if (oper == "b") {
                    int jumpTo = labelIndex.find(operands)->second;
                    i = jumpTo - 1;
                    continue;
                }
                if (oper == "beq") {
                    vector<string> conditions = splitString(operands.c_str());
                    int operand1Index = parseRegister(conditions[0]);
                    int operand2Index = parseRegister(conditions[1]);
                    if (ans[operand1Index] == ans[operand2Index]) {
                        string label = conditions[2];
                        int jumpTo = labelIndex.find(label)->second;
                        i = jumpTo - 1;
                        continue;
                    }
                }
                if (oper == "bnq") {
                    vector<string> conditions = splitString(operands.c_str());
                    int operand1Index = parseRegister(conditions[0]);
                    int operand2Index = parseRegister(conditions[1]);
                    if (ans[operand1Index] != ans[operand2Index]) {
                        string label = conditions[2];
                        int jumpTo = labelIndex.find(label)->second;
                        i = jumpTo - 1;
                        continue;
                    }
                }
                calculate(ans, oper, operands.c_str());
            }
        }
        t_vars = new vector<int>(0);
        vector<int> *myAnswer;
        for (int i = 0; i < n; i++) {
            t_vars->push_back(ans[i]);
        }

        return t_vars;
    }

    int mips_clock(){
        chrono::milliseconds timespan(clck);

        this_thread::sleep_for(timespan);
        static int cycle = 0;
        if (cycle == 0)
            cycle = 1;
        else
            cycle = 0;
        return cycle;
    }

};


