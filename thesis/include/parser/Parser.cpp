//TODO include constructor {} for proper parsing

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

void findAndRemove(string &inputString, char c){
    size_t start_pos = inputString.find(c);
    if(start_pos != string::npos)
        inputString = inputString.substr(0,start_pos);

}

int main(int argc, char** argv){
    if (argc == 2) {
        cout << "\033[33mNOTE: output file not specified: defining outputs in Fields.h\033[0m" << endl;
    } 
    else if(argc != 3){
        cerr << "\033[31mERROR: Invalid number of arguments." << endl;
        cerr << "Usage: " << argv[0] << " <input_file> [output_h_file_name]" << endl;
        cerr << "If [output_h_file_name] is omitted, 'Fields.h' will be used by default." << endl;
    
        return 1; 
    }

    ifstream inFile(argv[1]);
    ofstream outFile(argc == 3 ? string(argv[2]) + ".h" : "Fields.h");

    if (!inFile) {
        cerr << "Error: couldn't open input file" << argv[1] << endl;
        return 1;
    }
    if (!outFile) {
        cerr << "Error: couldn't create output file" << (argc == 3 ? argv[2] : "Fields.h") << endl;
        return 1;
    }

    outFile << "// This file has been generated automatically by the C++ parser. DO NOT UPDATE until you don't know what you're doing"<<endl;

    string line;
    while(getline(inFile, line)){
        stringstream ss(line); //stringstream permits you to work with strings, separated by any space ()
        string token;
        vector<string> tokens;

        while(ss >> token){
            tokens.push_back(token);
        }

        if(tokens.empty() || tokens[0] != "FIELD") continue; // I just read the FIELD - lines

        //now i can obtain type and name:
        string type;
        type += tokens[1];
        
        //todo add multiple word type handling (unsigned int, long long )

        //name handling is more complex since we need to parse static arrays too.
        string token_name = tokens[2];
        size_t bracket_start_pos = token_name.find('[');

        if(bracket_start_pos != string::npos){
            string name = token_name.substr(0, bracket_start_pos);

            size_t bracket_end_pos = token_name.find(']');

            string size = token_name.substr(bracket_start_pos + 1, (bracket_end_pos-bracket_start_pos) - 1);

            outFile << "FieldArray(" << type << ", " << name << ", " << size <<")"<<endl;
        }else{
            //we do need to remove constructors ( () or {} ) or semicolon (;):
            /* 
                int a;
                MyClass b{};
                MyClass c();

                there is this case too:
                int a  ;
            */

            findAndRemove(token_name, '(');
            findAndRemove(token_name, '{');
            findAndRemove(token_name, ';');

            outFile<< "Field("<<type<<", "<<token_name<<")"<<endl;
        }
    }

    inFile.close();
    outFile.close();
    cout<<"\033[32mFile created correctly!"<<endl;
    return 0;
}