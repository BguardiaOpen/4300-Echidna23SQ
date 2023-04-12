//Ryan Silveira
// 4/3/2023
//CPSC 4300

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "db_cxx.h"
#include "SQLParser.h"
#include "sqlhelper.h"

using namespace std;
using namespace hsql;

void execute(string sqlcmd);
string parse(SQLParserResult* &result, string inputString);

u_int32_t env_flags = DB_CREATE | DB_INIT_MPOOL; //If the environment does not exist, create it.  Initialize memory.
u_int32_t db_flags = DB_CREATE; //If the database does not exist, create it.

int main(int argc, char **argv) {
    if(argc != 2){
        cerr << "Missing path" << endl;
        return -1;
    }
    string dbPath = argv[1];
    DbEnv *env = new DbEnv(0);
    try{
        env->open(dbPath, env_flags, 0);
        env.set_message_stream(cout);
        env.set_error_stream(cerr);
    }
    catch (DbException &e){
        cerr << "Error with database: " << dbPath << endl;
        cerr << e.what() << endl;
        exit(-1);
    } catch(exception &e){
        cerr << "Error with database: " << dbPath << endl;
        cerr << e.what() << endl;
        exit(-1);
    }
    while(true){
        string sqlcmd;
        cout << "SQL>";
        getline(cin, sqlcmd);
        if(sqlcmd == "quit"){
            break;
        }
        execute(sqlcmd);
    }
    return 0;
}

void execute(string sqlcmd){
    hsql::SQLParserResult* result = hsql::SQLParser::parseSQLString(sqlcmd);
    if(!result->isValid()){
        cout << "Invalid command: " << sqlcmd << endl;
    }
    else { 
        for(int i = 0; i < result->size(); i++){
            SQLStatement *statement = result->getStatement(i)
        }
    }
    delete result;
}