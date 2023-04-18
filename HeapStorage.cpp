//HeapStorage.cpp
//Ryan Silveira
//4/17/23
//Echidna-4300
//Contains methods for SlottedPage, HeapFile, and HeapTable classes.

#include "HeapStorage.h"
#include <cstring>
#include "db_cxx.h"

using namespace std;
using u16 = u_int16_t;
using u32 = u_int32_t;
//SlottedPage PUBLIC METHODS STARTS HERE
//Basic constructor.  
//memcpy - source pointer, destination pointer, number of bytes to copy
//location - newsize - size is the formula to find 
SlottedPage::SlottedPage(Dbt& block, BlockID block_id, bool is_new):DbBlock(block, block_id, is_new){
    if(is_new) {
        this->num_records = 0;
        this->end_free = DbBlock::BLOCK_SZ - 1;
        put_header();
    } else{
        get_header(this->num_records, this->end_free);
    }
}

// Add a new record to the block. Return its id.
RecordID SlottedPage::add(const Dbt* data) {
    if (!has_room(data->get_size()))
        throw DbBlockNoRoomError("not enough room for new record");
    u16 id = ++this->num_records;
    u16 size = (u16) data->get_size();
    this->end_free -= size;
    u16 loc = this->end_free + 1;
    put_header();
    put_header(id, size, loc);
    memcpy(this->address(loc), data->get_data(), size);
    return id;
}

//Given a record ID, get the bits stored in that record
Dbt* SlottedPage::get(RecordID record_id) {
    u16 size, location;
    get_header(size, location, record_id);
    if(loc == 0)
        return nullptr;
    return new Dbt(this->address(loc), size);
}

//This method replaces at location recordID with the given data encapsulated isn the Dbt.
void SlottedPage::put(RecordID recordID, const Dbt &data) {
    u16 size = get_n(4*recordID); //This is the size of the entry
    u16 location = get_n(4*recordID+2); //This is the offset, gotten using the id
    u16 newsize = (u16)data.get_size(); //This is the new size of the data in the entry
    if(newsize>size) { //If the new entry is larger
        if(!this->has_room(newsize-size)) {
            cout << "No space for new page." << endl;
        } else{
            memcpy(this->address(location-newsize-size), data.get_data(), newsize); //Copy from start of old data to end of new data
            this->slide(location, location-newsize-size); //
        }
    } else{ //if newsize is smaller than oldsize
        memcpy(this->address(location), data.get_data(), newsize); //copy data from data of newsize over this->address
        this->slide(location+newsize, location+size);
    }
    get_header(size, location, recordID);
    put_header(recordID, newSize, location);
}

//delete a record given the record ID.  
void SlottedPage::del(RecordID record_id){
    u16 size, location;
    this->get_header(size, location, record_id);
    this->put_header(record_id);
    this->slide(location, location + size);
}
//This method returns all of the ids containted within the object.
RecordIDs* SlottedPage::ids(void){
	u16 size,loc;
	RecordIDs* idsets = new RecordIDs;
	for (u16 i = 1; i <= this->num_records; i++) {
		get_header(size, loc, i);
		if (loc > 0) {
			idsets->push_back(i);
		}
	}
	return idsets;
}
// SLOTTEDPAGE PROTECTED METHODS START HERE

u16 SlottedPage::size() const {
    u16 size, loc;
    u16 count = 0;
    for (RecordID record_id = 1; record_id <= this->num_records; record_id++) {
        get_header(size, loc, record_id);
        if (loc != 0)
            count++;
    }
    return count;
}


//Pass by reference, so size and location are changed to the values held at record_id.  The +2 is the offset.
void SlottedPage::get_header(u16 &size, u16 &loc, RecordID id) const{
    size = get_n((u16)4*id);
    loc = get_n((u16)(4*id+2));
}

//Put_header is the opposite, setting the values at given record ID
void SlottedPage::put_header(RecordID id, u16 size, u16 loc){
    if(id == 0){
        size = this->num_records;
        loc = this->end_free;
    }
    put_n((u16)4*id, size);
    put_n((u16)(4*id+2), loc);
}

// Get 2-byte integer at given offset in block.
u16 SlottedPage::get_n(u16 offset) {
    return *(u16*)this->address(offset);
}

// Put a 2-byte integer at given offset in block.
void SlottedPage::put_n(u16 offset, u16 n) {
    *(u16*)this->address(offset) = n;
}

// Make a void* pointer for a given offset into the data block.
void* SlottedPage::address(u16 offset) {
    return (void*)((char*)this->block.get_data() + offset);
}

// Store the size and offset for given id. For id of zero, store the block header.
void SlottedPage::put_header(RecordID id, u16 size, u16 loc) {
    if (id == 0) {
        size = this->num_records;
        loc = this->end_free;
    }
    put_n(4*id, size);
    put_n(4*id + 2, loc);
}

//Check available room in the page
bool SlottedPage::has_room(u_int16_t size) {
	u16 available=this->end_free - (this->num_records + 2) * 4;
	return (size <= available);
}
//move data down to make room
void SlottedPage::slide(u_int16_t start, u_int16_t end){
	u16 shift = end - start;
	if(shift==0) return;
	// slide data
	memcpy(this->address(this->end_free + 1 + shift), this->address(this->end_free + 1), start);
	//correct headers
	u16 size, location;
	RecordIDs* idset = this->ids();
	for(RecordID id:*idset){
		get_header(size, location,id);
		if (location <= start) {
			location += shift;
			put_header(id, size, location);
		}
	}
	this->end_free += shift;
	this->put_header();
	delete idset; 
}

// HEAPFILE PUBLIC METHODS START HERE

// This method gets a new block of data adds it to the file, then returns the pointer to the new object.
SlottedPage* HeapFile::get_new(void) {
    char block[DbBlock::BLOCK_SZ];
    std::memset(block, 0, sizeof(block));
    Dbt data(block, sizeof(block));

    int block_id = ++this->last;
    Dbt key(&block_id, sizeof(block_id));

    // write out an empty block and read it back in so Berkeley DB is managing the memory
    SlottedPage* page = new SlottedPage(data, this->last, true);
    this->db.put(nullptr, &key, &data, 0); // write it out with initialization applied
    this->db.get(nullptr, &key, &data, 0);
    return page;
}

void HeapFile::create(void){
    u16 flags = DB_CREATE | DB_EXCEL;
    this->db_open(flags);
    SlottedPage* block = this->get_new();
    this->put(block);
    delete block;
}

void HeapFile::open(void){
    this->db_open;
    this->closed = false;
}

void HeapFile::close(void){
    this->db.close(0);
    this->closed = true;
}

void HeapFile::drop(void){
    this->close();
    const char** Home = new const char*[1024];
    _DB_ENV->get_home(Home);
    string dbfilepath = string(*Home) + "/" + this->dbfilenamel
    delete[] Home;
    if(remove(dbfilepath.c_str()))
        throw std::logic_error("could not remove DB file");
}

SlottedPage* HeapFile::get(BlockID block_id){
    Dbt key(&block_id, sizeof(block_id)), block;
    this->db.get(NULL, &key, &block, 0);
    return new SlottedPage(block, block_id);
}

SlottedPage* HeapFile::get_new(void) {
    char block[DbBlock::BLOCK_SZ];
    std::memset(block, 0, sizeof(block));
    Dbt data(block, sizeof(block));

    int block_id = ++this->last;
    Dbt key(&block_id, sizeof(block_id));

    // write out an empty block and read it back in so Berkeley DB is managing the memory
    SlottedPage* page = new SlottedPage(data, this->last, true);
    this->db.put(nullptr, &key, &data, 0); // write it out with initialization done to it
    delete page;
    this->db.get(nullptr, &key, &data, 0);
    return new SlottedPage(data, this->last);
}

void HeapFile::put(DbBlock* block) {
    BlockID block_id = block->get_block_id();
    Dbt key(&block_id, sizeof(block_id));
    this->db.put(nullptr, &key, block->get_block(), 0);
}

BlockIDs* HeapFile::block_ids() {
    BlockIDs* block_ids = new BlockIDs();
    for (BlockID block_id = 1; block_id <= this->last; block_id++)
        block_ids->push_back(block_id);
    return block_ids;
}

//HeapTable STARTS HERE
//file is the HeapFile associated with the HeapTable object
void HeapTable::create() {
    file.create();
}

//This is just a more complicated version of the above
void HeapTable::create_if_not_exists() {
    try {
        open();
    } catch (DbException &e) {
        create();
    }
}

//this calls drop() from the HeapFile on this one
void HeapTable::drop() {
    file.drop();
}

//as above, so below
void HeapTable::open() {
    file.open();
}

//closes the table
void HeapTable::close() {
    file.close();
}

//Handle is a pair of blockID, recordID defined in the abstract classes
Handle HeapTable::insert(const ValueDict *row) {
    open();
    ValueDict *full_row = validate(row);
    Handle handle = append(full_row);
    delete full_row;
    return handle;
}


// this is the SQL UPDATE analogue.
void HeapTable::update(const Handle handle, const ValueDict *new_values) {
    throw DbRelationError("Not implemented");
}

// DELETE operation analogue.  Take the block and record ID out, go find it and delete.
void HeapTable::del(const Handle handle) {
    open();
    BlockID block_id = handle.first;
    RecordID record_id = handle.second;
    SlottedPage *block = this->file.get(block_id);
    block->del(record_id);
    this->file.put(block);
    delete block;
    close();
}

//I have no idea what the empty select is supposed to do.
Handles *HeapTable::select() {
    return select(nullptr);
}

// SELECT operation analogue.  Load up your block IDs from the file, then your IDs from the block and where they match, push back a handle object and return it once they're all checked.
Handles* HeapTable::select(const ValueDict* where) {
    Handles* handles = new Handles();
    BlockIDs* block_ids = file.block_ids();
    for (auto const& block_id: *block_ids) {
        SlottedPage* block = file.get(block_id);
        RecordIDs* record_ids = block->ids();
        for (auto const& record_id: *record_ids)
            handles->push_back(Handle(block_id, record_id));
        delete record_ids;
        delete block;
    }
    delete block_ids;
    return handles;
}

// I guess this is just a public call so that you can use it when you only have the handle.  It figures out the column names and goes and grabs them.
ValueDict *HeapTable::project(Handle handle) {
    return project(handle, &this->column_names);
}

// This is the part that actually does the projecting.  
ValueDict *HeapTable::project(Handle handle, const ColumnNames *column_names) {
    BlockID block_id = handle.first;
    RecordID record_id = handle.second;
    SlottedPage *block = file.get(block_id);
    Dbt *data = block->get(record_id);
    ValueDict *row = unmarshal(data);
    delete data;
    delete block;
    if (column_names->empty())
        return row;
    ValueDict *result = new ValueDict();
    for (auto const &column_name: *column_names) {
        if (row->find(column_name) == row->end())
            throw DbRelationError("Column does not exist: '" + column_name + "'");
        (*result)[column_name] = (*row)[column_name];
    }
    delete row;
    return result;
}

//Check if this row is acceptable to insert.
ValueDict *HeapTable::validate(const ValueDict *row) const {
    ValueDict *full_row = new ValueDict();
    for (auto const &column_name: this->column_names) {
        Value value;
        ValueDict::const_iterator column = row->find(column_name);
        if (column == row->end())
            throw DbRelationError("Incoherent screaming.");
        else
            value = column->second;
        (*full_row)[column_name] = value;
    }
    return full_row;
}

//Add another record.  Returns the handle of the new row.
Handle HeapTable::append(const ValueDict *row) {
    Dbt *data = marshal(row);
    SlottedPage *block = this->file.get(this->file.get_last_block_id());
    RecordID record_id;
    try {
        record_id = block->add(data);
    } catch (DbBlockNoRoomError &e) {
        delete block;
        block = this->file.get_new();
        record_id = block->add(data);
    }
    this->file.put(block);
    delete block;
    delete[] (char *) data->get_data();
    delete data;
    return Handle(this->file.get_last_block_id(), record_id);
}



// return the bits to go into the file
// caller responsible for freeing the returned Dbt and its enclosed ret->get_data().
Dbt* HeapTable::marshal(const ValueDict* row) {
    char *bytes = new char[DbBlock::BLOCK_SZ]; // more than we need (we insist that one row fits into DbBlock::BLOCK_SZ)
    uint offset = 0;
    uint col_num = 0;
    for (auto const& column_name: this->column_names) {
        ColumnAttribute ca = this->column_attributes[col_num++];
        ValueDict::const_iterator column = row->find(column_name);
        Value value = column->second;
        if (ca.get_data_type() == ColumnAttribute::DataType::INT) {
            *(int32_t*) (bytes + offset) = value.n;
            offset += sizeof(int32_t);
        } else if (ca.get_data_type() == ColumnAttribute::DataType::TEXT) {
            uint size = value.s.length();
            *(u16*) (bytes + offset) = size;
            offset += sizeof(u16);
            memcpy(bytes+offset, value.s.c_str(), size); // assume ascii for now
            offset += size;
        } else {
            throw DbRelationError("Only know how to marshal INT and TEXT");
        }
    }
    char *right_size_bytes = new char[offset];
    memcpy(right_size_bytes, bytes, offset);
    delete[] bytes;
    Dbt *data = new Dbt(right_size_bytes, offset);
    return data;
}

ValueDict* HeapTable::unmarshal(Dbt* data) const
{
    ValueDict* row = new ValueDict();
    char* bytes = (char*)data->get_data();
    uint offset = 0;
    uint col_num = 0;
    for (const Identifier column_name : this->column_names) {
        ColumnAttribute ca = this->column_attributes[col_num++];
        if (ca.get_data_type() == ColumnAttribute::DataType::INT) {
            Value value = Value(*(u32*)(bytes + offset));
            (*row)[column_name] = value;
            offset += sizeof(u32);
        } else if (ca.get_data_type() == ColumnAttribute::DataType::TEXT) {
            u16 size = *(u16*)(bytes + offset);
            offset += sizeof(u16);
            Value value(std::string(bytes + offset, size));
            (*row)[column_name] = value;
            offset += size;
        } else {
            throw DbRelationError("Only know how to unmarshal INT and TEXT");
        }
    }
    return row;
}