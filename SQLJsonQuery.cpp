//
//  SQLJsonQuery.cpp
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#include "SQLJsonQuery.h"
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

using namespace SQLJson;

time_t Query::QueryToTime(const std::string _query_time) {
	struct tm _tm;
	memset(&_tm, 0, sizeof(struct tm));
	strptime(_query_time.c_str(), "%Y-%m-%d %H:%M:%S", &_tm);
	return mktime(&_tm);
}

const std::string Query::TimeToQuery(time_t _time) {
	struct tm* _tm = (struct tm*)calloc(1, sizeof(struct tm));
	localtime_r(&_time, _tm);
	char _buffer[0xFF] = {NULL,};
	strftime(_buffer, sizeof _buffer, "%Y-%m-%d %H:%M:%S", _tm);
	free(_tm);
	return std::string(_buffer);
}

time_t Query::CurrentTime() {
	time_t _time = time(NULL);
	struct tm _gmt;
	memset(&_gmt, 0, sizeof(struct tm));
	gmtime_r(&_time, &_gmt);
	time_t _gmt_time = mktime(&_gmt);
	return _gmt_time;
}


Query::Query() {
	db = NULL;

	//READ TABLE AND QUERY INFO
	std::ifstream _document;
	Json::Reader _reader;
	_document.open(DB_INFO_PATH.c_str());
	if(_document.is_open()) {
		if(!_reader.parse(_document, *this)) {
			std::cout << "경로 " << DB_INFO_PATH << "가 파싱되지 않습니다.\n";
		}
	} else {
		std::cout << "경로 " << DB_INFO_PATH << "가 열리지 않습니다.\n";
	}
	_document.close();

	bool _need_create_table = access(DB_PATH.c_str(), F_OK) != 0;
	sqlite3_open(DB_PATH.c_str(), &db);
	
	if((*this)["table"] == Json::nullValue)
		assert(0 && "not found table");
	
	if(_need_create_table) {
		//CREATE TABLE ONCE
		
		std::stringstream _stream_default_indexed;
		std::stringstream _stream_default_field;
		if((*this)["default_table_field"] != Json::nullValue) {
			for(int i = 0; i < (*this)["default_table_field"].size(); i++) {
				Json::Value _field = (*this)["default_table_field"][i];
				
				if(_field["name"] == Json::nullValue)
					assert(0 && "not found field name");
				
				if(_field["type"] == Json::nullValue)
					assert(0 && "not found field type");
				
				if(_field["indexed"] == Json::nullValue)
					assert(0 && "not found field indexed");
				
				_stream_default_field << ", " << _field["name"].asString() << " " << _field["type"].asString();
				if(_field["indexed"].asBool())
					_stream_default_indexed << ", " << _field["name"].asString();
			}
		}
		
		for(int i = 0; i < (*this)["table"].size(); i++) {
			Json::Value _table = (*this)["table"][i];
			
			if(_table["name"] == Json::nullValue)
				assert(0 && "not found table name");
			
			if(_table["include_default_table_field"] == Json::nullValue)
				assert(0 && "not found table include field");
			
			std::stringstream _stream_indexed;
			std::stringstream _stream_field;
			std::stringstream _stream;
			_stream << "BEGIN; ";
			_stream << "CREATE TABLE IF NOT EXISTS " << _table["name"].asString() << " ";
			_stream << "(";
			if(_table["field"] == Json::nullValue)
				assert(0 && "not found field");
			if(_table["field"].size() < 1)
				assert(0 && "not found field");
			_stream_field << "localid INTEGER PRIMARY KEY, ";
			_stream_field << "localsync BOOLEAN, ";
			_stream_field << "localreceive DATETIME";
			_stream_indexed << "localid, localsync, localreceive";
			for(int j = 0; j < _table["field"].size(); j++) {
				Json::Value _field = _table["field"][j];
				
				if(_field["name"] == Json::nullValue)
					assert(0 && "not found field name");
				
				if(_field["type"] == Json::nullValue)
					assert(0 && "not found field type");
				
				if(_field["indexed"] == Json::nullValue)
					assert(0 && "not found field indexed");
				
				_stream_field << ", " << _field["name"].asString() << " " << _field["type"].asString();
				if(_field["indexed"].asBool())
					_stream_indexed << ", " << _field["name"].asString();
			}
			_stream << _stream_field.str();
			if(_table["include_default_table_field"].asBool())
				_stream << _stream_default_field.str();
			_stream << "); ";
			_stream << "CREATE INDEX IF NOT EXISTS " << _table["name"].asString() << "_index ON " << _table["name"].asString() << " (" << _stream_indexed.str();
			if(_table["include_default_table_field"].asBool())
				_stream << _stream_default_indexed.str();
			_stream << "); ";
			_stream << "END;";
			
			assert(Excute(_stream.str(), NULL, NULL));
		}
	} else {
		assert(Excute("VACUUM;", NULL, NULL));
	}
}

int Query::_SQL_DATA_CALLBACK(void* _reference, int _field_length, char** _field, char** _field_name) {
	typedef struct info {
		std::vector<SQLJson::Data*> _vec;
		Live* _live;
	} info;
	info* _info = (info*)_reference;
	
	int _unique = -1;
	for(int i = 0; i < _field_length; i++) {
		if(strcmp("localid", _field_name[i]) == 0) {
			_unique = atol(_field[i]);
			break;
		}
	}
	Data* _data = _info->_live->GetIsOpenedData(_unique);
	if(!_data)
		_data = _info->_live->CreateRegistData(_unique);
	else
		_data->Retain();
	
	for(int i = 0; i < _field_length; i++) {
		if(_field[i])
			(*_data)[_field_name[i]] = std::string(_field[i]);
	}
	
	_info->_vec.push_back(_data);
	
	return 0;
}

int Query::_SQL_COUNT_CALLBACK(void* _reference, int _field_length, char** _field, char** _field_name) {
	int* _count = (int*)_reference;
	_count++;
	return 0;
}

Live* Query::GetLive(std::string _name) {
	std::list<Live*>::iterator _it = list_live.begin();
	while(_it != list_live.end()) {
		if((*_it)->GetName() == _name)
			return *_it;
		_it++;
	}
	Live* _live = new Live(this, _name);
	list_live.push_back(_live);
	return _live;
}

bool Query::Excute(std::string _query, int (*_callback)(void*, int, char**, char**), void* _reference) {
	char* _db_message = NULL;
	int _code = sqlite3_exec(db, _query.c_str(), _callback, _reference, &_db_message);
	
	if(_db_message) {
#ifdef DEBUG
		std::cout << _db_message;
#endif
		sqlite3_free(_db_message);
	}
	
	if(_code == SQLITE_OK)
		return true;
	return false;
}

Json::Value Query::GetTable(std::string _name) {
	Json::Value _table = Json::nullValue;
	for(int i = 0; i < (*this)["table"].size(); i++) {
		_table = (*this)["table"][i];
		if(_table["name"].asString() == _name) {
			return _table;
		}
	}
	assert((_table != Json::nullValue) && "not found table name");
	return _table;
}

Json::Value Query::GetQuery(std::string _name) {
	Json::Value _query = Json::nullValue;
	for(int i = 0; i < (*this)["query"].size(); i++) {
		_query = (*this)["query"][i];
		if(_query["name"].asString() == _name) {
			return _query;
		}
	}
	assert((_query != Json::nullValue) && "not found query name");
	return _query;
}

Json::Value Query::ToLocal(Json::Value _table, Json::Value _value) {
	Json::Value _local;
	Value::Members _members = _value.getMemberNames();
	
	std::list<Json::Value> _list;
	
	for(int i = 0; i < (*this)["default_table_field"].size(); i++)
		_list.push_back((*this)["default_table_field"][i]);
	for(int i = 0; i < _table["field"].size(); i++)
		_list.push_back(_table["field"][i]);
	
	for(int i = 0; i < _members.size(); i++) {
		std::list<Json::Value>::iterator _it = _list.begin();
		while (_it != _list.end()) {
			if(_members[i] == (*_it)["name"].asString()) {
				_local[_members[i]] = _value[_members[i]].asString();
				_list.erase(_it);
				break;
			}
			_it++;
		}
	}
	
	return _local;
}

Query* __share_query = NULL;

Query* Query::Share() {
	if(__share_query == NULL)
		__share_query = new Query();
	return __share_query;
}

bool Query::Add(std::string _table_name, Json::Value _value, bool _is_response) {
	Json::Value _local = ToLocal(GetTable(_table_name), _value);
	
	if(_is_response) {
		_local["localsync"] = true;
		_local["localreceive"] = TimeToQuery(CurrentTime());
	} else {
		_local["localsync"] = false;
	}
	
	std::stringstream _stream;
	_stream << "INSERT OR REPLACE INTO " << _table_name;
	_stream << " (";
	Value::Members _members = _local.getMemberNames();
	for(int i = 0; i < _members.size(); i++)
		_stream << (i != 0 ? ", " : "") << _members[i];
	_stream << ") ";
	_stream << "VALUES (";
	for(int i = 0; i < _members.size(); i++)
		_stream << (i != 0 ? ", " : "") << "'" << _local[_members[i]].asString() << "'";
	_stream << ")";
	
	if(Excute(_stream.str(), NULL, NULL))
		return true;
	return false;
}

std::vector<SQLJson::Data*> Query::Select(std::string _query_name, ...) {
	Json::Value _query = GetQuery(_query_name);
	
	char _tmp_str[USHRT_MAX] = {0,};
	va_list _ap;
	va_start(_ap, _query_name);
	vsprintf(_tmp_str, _query["query"].asString().c_str(), _ap);
	va_end(_ap);
	
	struct _info {
		std::vector<SQLJson::Data*> _vec;
		Live* _live;
	} _info;
	_info._live = GetLive(_query["table"].asString());
	
	if(Excute(_tmp_str, _SQL_DATA_CALLBACK, &_info))
		return _info._vec;
	return std::vector<SQLJson::Data*>();
}

int Query::Count(std::string _query_name, ...) {
	Json::Value _query = GetQuery(_query_name);
	
	char _tmp_str[USHRT_MAX] = {0,};
	va_list _ap;
	va_start(_ap, _query_name);
	vsprintf(_tmp_str, _query["query"].asString().c_str(), _ap);
	va_end(_ap);
	
	int _count = 0;
	
	if(Excute(_query["query"].asString().c_str(), _SQL_COUNT_CALLBACK, &_count))
		return _count;
	return -1;
}
