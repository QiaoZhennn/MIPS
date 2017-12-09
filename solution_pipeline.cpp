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
#include <algorithm>

#define NOT_IN_PIPELINE 0 // instruction is waiting to be executed or have been written back
#define FETCH 1 // instruction is at INSTRUCTION FETCH stage
#define EXECUTE 2 // instruction is at EXECUTE stage
#define WRITE_BACK 3 // instruction is at WRITE BACK stage

using namespace std;


class solution {

private:

    bool DEBUG;
    int clck;
    vector<int> vect_stages; // indicate the stage status of each instruction in pipeline
    vector<string> vect_opers; // stores all operations
    vector<string> vect_operands; // stores all operands
    vector<int> *t_vars; // stores the value of ans
    map<string, int> labelIndex; // a map stores where each label appears
    int numberOfInstructions; // the number of instructions
    int n; // the number of registers
    int *ans; // an array stores calculation result

public :

    solution(ifstream &file_in, int clck_in = 10, bool DEBUG_in = false) {
        if (!file_in.is_open()) {
            cout << "File not open" << endl;
        }
        string input;
        file_in >> input;
        vector<string> inputStatus = splitString(input.c_str());
        n = (int) inputStatus.size();
        const int size = n;
        ans = new int[size];
        for (int i = 0; i < n; i++) {
            int d = atoi(inputStatus[i].c_str());
            ans[i] = d;
        }
        // initiate vect_opers, vect_operands and vect_status
        while (!file_in.eof()) {
            string oper;
            file_in >> oper;
            if (oper == "end") break;
            if (oper[0] != 'l') { // if operator is not "label"
                vect_opers.push_back(oper);
            } else { // if operator is "label"
                labelIndex.insert(pair<string, int>(oper, (int) vect_opers.size())); //store "label" and corresponding instructionIndex
                file_in >> oper; // real operator is the word after "label"
                vect_opers.push_back(oper);
            }
            string operands;
            file_in >> operands;
            vect_operands.push_back(operands);
            vect_stages.push_back(NOT_IN_PIPELINE);
        }
        numberOfInstructions = vect_stages.size();
        DEBUG = DEBUG_in;
        clck = clck_in;
        t_vars = new vector<int>(0);
    }

    // for example: oper is "add", input is "$1,$2,$3", ans[] stores current values of all registers. return the value of $1 after calculate.
    int calculate(int ans[], string oper, const char *input) {
        vector<string> operands = splitString(input);
        int targetIndex = parseRegister(operands[0]);
        int operand1Index = parseRegister(operands[1]);
        int operand2Index = parseRegister(operands[2]);
        int result;
        if (oper == "add") {
            result = ans[operand1Index] + ans[operand2Index];
        }
        if (oper == "addi") {
            result = ans[operand1Index] + operand2Index;
        }
        if (oper == "sub") {
            result = ans[operand1Index] - ans[operand2Index];
        }
        if (oper == "mult") {
            result = ans[operand1Index] * ans[operand2Index];
        }
        if (oper == "div") {
            result = ans[operand1Index] / ans[operand2Index];
        }
        return result;
    }

    // "$1" to 1
    int parseRegister(string target) {
        int index = 0;
        if (target[0] == '$') {
            target.erase(0, 1);
        }
        sscanf(target.c_str(), "%d", &index);
        return index;
    }

    // "$1,$2,$3" to {"$1","$2","$3"}
    vector<string> splitString(const char *input) {
        char sequence[100] = {0};
        strncpy(sequence, input, 100);
        char *tokenPtr;
        tokenPtr = strtok(sequence, ",");
        vector<string> strings;
        while (tokenPtr != NULL) {
            string str = tokenPtr;
            strings.push_back(str);
            tokenPtr = strtok(NULL, ",");
        }
        return strings;
    }

    void dbg(const string &msg) {}

    int instructionIndex = 0; // indicate which instruction to fetch
    // INSTRUCTION FETCH unit
    vector<string> fetch() { // concat operator and operands to one vector
        vector<string> instruction;
        if (instructionIndex < numberOfInstructions) {
            instruction.push_back(vect_opers[instructionIndex]);
            instruction.push_back(vect_operands[instructionIndex]);
            instructionIndex++;
        }
        return instruction;
    }

    // EXECUTE unit
    map<string, int> execute(int ans[], string oper, string operands) { // can determine whether current instruction is a branch or not using map
        map<string, int> myMap;
        if (oper == "b") {
            int jumpTo = labelIndex.find(operands)->second;
            myMap.insert(pair<string, int>("branch", jumpTo));
        } else if (oper == "beq") {
            vector<string> conditions = splitString(operands.c_str());
            int operand1Index = parseRegister(conditions[0]);
            int operand2Index = parseRegister(conditions[1]);
            if (ans[operand1Index] == ans[operand2Index]) {
                string label = conditions[2];
                int jumpTo = labelIndex.find(label)->second;
                myMap.insert(pair<string, int>("branch", jumpTo));
            }
            myMap.insert(pair<string, int>("branch", -1));
        } else if (oper == "bnq") {
            vector<string> conditions = splitString(operands.c_str());
            int operand1Index = parseRegister(conditions[0]);
            int operand2Index = parseRegister(conditions[1]);
            if (ans[operand1Index] != ans[operand2Index]) {
                string label = conditions[2];
                int jumpTo = labelIndex.find(label)->second;
                myMap.insert(pair<string, int>("branch", jumpTo));
            }
            myMap.insert(pair<string, int>("branch", -1));
        } else {
            int result = calculate(ans, oper, operands.c_str());
            myMap.insert(pair<string, int>("calculate", result));
        }
        return myMap;
    }

    // WRITE BACK unit
    void writeBack(string dest, int result) {
        int index = parseRegister(dest);
        ans[index] = result;
        cout<<" WRITE "<<result<<" To "<<dest<<endl;
    }

    vector<int> *alu() {
        int clock = 0;
        string oper;
        string operands;
        int result = 0;
        while (vect_stages[numberOfInstructions - 1] != WRITE_BACK) {
            if (!mips_clock()) {
                continue;
            } else {
                cout << "clock: " << clock << endl;

                // handle write back of current clock
                for (int i = 0; i < numberOfInstructions; i++) {
                    if (vect_stages[i] == EXECUTE) {
                        string input = vect_operands[i];
                        vector<string> operands = splitString(input.c_str());
                        cout << "I" << i<<": "<<vect_opers[i]<<" "<<vect_operands[i]<<" ->";
                        writeBack(operands[0],result);
                        vect_stages[i] = WRITE_BACK;
                        break;
                    }
                }

                // handle execution of current clock
                for (int i = 0; i < numberOfInstructions; i++) {
                    if (vect_stages[i] == FETCH) {
                        cout << "I" << i<<": "<<vect_opers[i]<<" "<<vect_operands[i]<< " -> EXECUTE" << endl;
                        map<string, int> myMap = execute(ans, oper, operands);
                        // if current instruction is common calculation
                        map<string, int>::iterator key = myMap.find("calculate");
                        if (key != myMap.end()) {
                            result = key->second;
                            vect_stages[i] = EXECUTE;
                            break;
                        }
                        // if current instruction is branch
                        key = myMap.find("branch");
                        if (key != myMap.end()) {
                            int jumpTo = key->second;
                            if (jumpTo != -1) { // if jumpTo = -1, means we do not need to jump, just stall one cycle.
                                // jump to another instruction. We can set the status of all remaining instructions
                                // which wait to execute to NOT_IN_PIPELINE. And set instructionIndex to jumpTo target.
                                for (int k = i; k < jumpTo; k++) {
                                    vect_stages[k] = WRITE_BACK; // set the instructions which are jumped over to WRITE_BACK, so they cannot be fetched.
                                }
                                for (int j = jumpTo; j < numberOfInstructions; j++) {
                                    vect_stages[j] = NOT_IN_PIPELINE;
                                }
                                instructionIndex = jumpTo;
                            }
                            if (jumpTo > i || jumpTo == -1) { // jump backward
                                vect_stages[i] = WRITE_BACK;
                            } else { // jump forward
                                vect_stages[i] = NOT_IN_PIPELINE;
                            }
                            goto branchStall; // if current execute instruction is branch, then stall for one cycle (jump over next FETCH)
                        }
                    }
                }

                // handle instruction fetch of current clock
                for (int i = 0; i < numberOfInstructions; i++) {
                    if (vect_stages[i] == NOT_IN_PIPELINE) {
                        cout << "I" << i<<": "<<vect_opers[i]<<" "<<vect_operands[i]<< " -> FETCH" << endl;
                        vector<string> instruction = fetch();
                        oper = instruction[0];
                        operands = instruction[1];
                        vect_stages[i] = FETCH;
                        break;
                    }
                }
                branchStall: // if last execute instruction is branch, then stall for one cycle (jump over next FETCH)
                clock++;
                for (int i = 0; i < n; i++) {
                    cout << ans[i] << "  ";
                }
                cout << endl << "-------------------------------------" << endl << endl;
            }
        }
        t_vars = new vector<int>(0);
        for (int i = 0; i < n; i++) {
            t_vars->push_back(ans[i]);
        }
        return t_vars;
    }

    int mips_clock() {
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


