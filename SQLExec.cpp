/**
 * @file SQLExec.cpp - implementation of SQLExec class
 * @author Kevin Lundeen
 * @see "Seattle University, CPSC5300, Winter 2023"
 */
#include "SQLExec.h"
#include "ParseTreeToString.h"
#include "SchemaTables.h"
using namespace std;
using namespace hsql;

// define static data
Tables *SQLExec::tables = nullptr;

// make query result be printable
ostream &operator<<(ostream &out, const QueryResult &qres) {
    if (qres.column_names != nullptr) {
        for (auto const &column_name: *qres.column_names)
            out << column_name << " ";
        out << endl << "+";
        for (unsigned int i = 0; i < qres.column_names->size(); i++)
            out << "----------+";
        out << endl;
        for (auto const &row: *qres.rows) {
            for (auto const &column_name: *qres.column_names) {
                Value value = row->at(column_name);
                switch (value.data_type) {
                    case ColumnAttribute::INT:
                        out << value.n;
                        break;
                    case ColumnAttribute::TEXT:
                        out << "\"" << value.s << "\"";
                        break;
                    default:
                        out << "???";
                }
                out << " ";
            }
            out << endl;
        }
    }
    out << qres.message;
    return out;
}

QueryResult::~QueryResult() {
    //just in case the pointers are nullptr
    if(!this->column_names)
        delete column_names;

    if(!this->column_attributes)
        delete column_attributes;
    
    if(!this->rows) {
        for (auto row: *rows)
            delete row;
        delete rows;
    }
}


/**
 * @brief Executes create, drop, and show statements
 * 
 * @param statement the statement to be executed
 * @return QueryResult* the result of the statement
 */
QueryResult *SQLExec::execute(const SQLStatement *statement) {
    // Initializes _tables table if not null
    if (!SQLExec::tables) {
        SQLExec::tables = new Tables();
    }

    try {
        switch (statement->type()) {
            case kStmtCreate:
                return create((const CreateStatement *) statement);
            case kStmtDrop:
                return drop((const DropStatement *) statement);
            case kStmtShow:
                return show((const ShowStatement *) statement);
            default:
                return new QueryResult("not implemented");
        }
    } catch (DbRelationError &e) {
        throw SQLExecError(string("DbRelationError: ") + e.what());
    }
}

/**
 * @brief Sets up the column definitions
 * 
 * @param col the column to be changed
 * @param column_name name of the column
 * @param column_attribute type of the column
 */
void SQLExec::column_definition(const ColumnDefinition *col, Identifier &column_name, ColumnAttribute &column_attribute) {
    column_name = col->name;
    switch(col->type) {
        case ColumnDefinition::INT:
            column_attribute.set_data_type(ColumnAttribute::INT);
            break;
        case ColumnDefinition::TEXT:
            column_attribute.set_data_type(ColumnAttribute::TEXT);
        default:
            throw SQLExecError("Column type not supported");
    }
}

/**
 * @brief Executes a create statement
 * 
 * @param statement the create statement to be executed
 * @return QueryResult* the result of the create statement
 */
QueryResult *SQLExec::create(const CreateStatement *statement) {
    //check create type (future proofed for other types)
    switch(statement->type) {
        case CreateStatement::kTable: 
            { 
                //add columns to table
                Identifier name = statement->tableName;
                Identifier colName;
                ColumnNames colNames;
                ColumnAttributes colAttributes;

                for(auto *col : *statement->columns) {
                    ColumnAttribute colAttribute(ColumnAttribute::INT);
                    //create a column binding for column to a name and attribute and add
                    //to colNames and colAttributes
                    column_definition(col, colName, colAttribute);
                    colNames.push_back(colName);
                    colAttributes.push_back(colAttribute);
                }

                //insert an empty row into the new table to instantiate change
                ValueDict row;
                row["table_name"] = name;
                Handle handle = SQLExec::tables->insert(&row);
                try {
                    Handles handleList;
                    DbRelation &cols = SQLExec::tables->get_table(Columns::TABLE_NAME);
                    try {
                        //add columns to schema, and remove existing on error
                        for(int index = 0; index < colNames.size(); index++) {
                            row["column_name"] = colNames[index];
                            //add type of column appropriately
                            if(colAttributes[index].get_data_type() == ColumnAttribute::INT)
                                row["data_type"] = ColumnAttribute::INT;
                            else    
                                row["data_type"] = ColumnAttribute::TEXT;
                            handleList.push_back(cols.insert(&row));
                        }

                        //create actual relation in system, accounting for prexistence
                        DbRelation &table = SQLExec::tables->get_table(name);
                        (statement->ifNotExists ? table.create_if_not_exists() : table.create());
                    } catch(...) {
                        try {
                            //delete remaining handles
                            for(auto &handle : handleList) 
                                cols.del(handle);
                        } catch (...) {
                        //...doesn't really matter if there's an error, 
                        //just need to try to delete the handle if it exists
                        }
                        return new QueryResult("Error in table creation"); 
                    }
                } catch(...) {
                    //delete the handle
                    try {
                        SQLExec::tables->del(handle);
                    } catch (...) {
                        //...doesn't really matter if there's an error, 
                        //just need to try to delete the handle if it exists
                    }
                    return new QueryResult("Error in table creation"); 
                }
                return new QueryResult("Table successfully created");
                break;
            }
        default: 
            return new QueryResult("CREATE TABLE only supported"); 
    }
    return nullptr;
}

/**
 * @brief Executes a drop statement
 * 
 * @param statement the statement to be executed
 * @return QueryResult* the result of the drop statement
 */
QueryResult *SQLExec::drop(const DropStatement *statement) {
    switch(statement->type) {
        case DropStatement::kTable:
            //new scope block to prevent any scoping issues/warnings with "default"
            {
                //check table is not a schema table
                Identifier tableName = statement->name;
                if(tableName == Tables::TABLE_NAME || tableName == Columns::TABLE_NAME)
                    throw SQLExecError("Error: schema tables cannot be dropped");

                DbRelation &table = SQLExec::tables->get_table(tableName);
                ValueDict location;

                //remove columns
                DbRelation &columns = SQLExec::tables->get_table(Columns::TABLE_NAME);
                Handles *columnHandles = columns.select(&location);
                for(Handle &handle : *columnHandles) 
                    columns.del(handle);

                //drop table and remove from schema
                table.drop();
            }
            return new QueryResult("Table successfully dropped!");
        default:
            return new QueryResult("only drop table implemented"); // FIXME
    }
}

/**
 * @brief Executes a show statement
 * 
 * @param statement the statement to be executed
 * @return QueryResult* the result of the show statement
 */
QueryResult *SQLExec::show(const ShowStatement *statement) {
    switch (statement->type) {
        case ShowStatement::kTables:
            return show_tables();
        case ShowStatement::kColumns:
            return show_columns(statement);
        default:
            throw SQLExecError("invalid show type");
    }
}

/**
 * @brief Shows all tables
 * 
 * @return QueryResult* the result of the show
 */
QueryResult *SQLExec::show_tables() {
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");

    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute(ColumnAttribute::TEXT));

    Handles *handles = SQLExec::tables->select();

    ValueDicts *rows = new ValueDicts;
    for (auto const &handle : *handles) {
        ValueDict *row = SQLExec::tables->project(handle, column_names);
        Identifier table_name = row->at("table_name").s;
        rows->push_back(row);
    }

    delete handles;

    return new QueryResult(column_names, column_attributes, rows, "showing tables");
}

/**
 * @brief Shows all columns
 * 
 * @param statement the statement to be executed
 * @return QueryResult* the result of show
 */
QueryResult *SQLExec::show_columns(const ShowStatement *statement) {
    ColumnNames *column_names = new ColumnNames;
    column_names->push_back("table_name");
    column_names->push_back("column_name");
    column_names->push_back("data_type");
    
    ColumnAttributes *column_attributes = new ColumnAttributes;
    column_attributes->push_back(ColumnAttribute::TEXT);
    
    ValueDict col;
    col["table_name"] = Value(statement->tableName);

    Handles *handles = SQLExec::tables->select(&col);

    ValueDicts *rows = new ValueDicts;
    for (auto const &handle : *handles) {
        ValueDict *row = SQLExec::tables->project(handle, column_names);
        rows->push_back(row);
    }

    delete handles;

    return new QueryResult(column_names, column_attributes, rows, "showing columns");
}

