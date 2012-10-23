//
//  SQLJsonData.cpp
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#include "SQLJsonData.h"
#include "SQLJsonLive.h"
#include "SQLJsonQuery.h"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace SQLJson;

Data::Data(Live* _live, int _unique) {
	live = _live;
	unique_id = _unique;
	retain = 0;
	update = 0;
	is_live = true;
	Retain();
}

Data::~Data() {
	if(is_live)
		live->UnregistData(this);
}

bool Data::Update(Json::Value _value, bool _is_response) {
	Value::Members _members = _value.getMemberNames();
	for(int i = 0; i < _members.size(); i++) {
		(*this)[_members[i]] = _value[_members[i]].asString();
	}
	return Update(_is_response);
}

bool Data::Update(bool _is_response) {
	if(!is_live)
		return false;
	
	std::string _table_name = live->GetName();
	Json::Value _table_value = SQLJson::Query::Share()->GetTable(_table_name);
	Json::Value _value = SQLJson::Query::Share()->ToLocal(_table_value, *this);
	
	if(_is_response) {
		_value["localsync"] = true;
		_value["localreceive"] = Query::TimeToQuery(Query::CurrentTime());
	} else {
		_value["localsync"] = false;
	}
	
	Value::Members _members = _value.getMemberNames();
	
	std::stringstream _stream;
	_stream << "UPDATE " << _table_name << " SET ";
	for(int i = 0; i < _members.size(); i++) {
		(*this)[_members[i]] = _value[_members[i]].asString();
		_stream << (i != 0 ? ", " : "" ) << _members[i] << "='" << _value[_members[i]].asString() << "'";
	}
	_stream << "WHERE localid='" << unique_id << "'";
	if(SQLJson::Query::Share()->Excute(_stream.str(), NULL, NULL)) {
		update++;
		return true;
	}
	return false;
}

bool Data::Delete() {
	if(!is_live)
		return false;
	
	std::string _table_name = live->GetName();
	Json::Value _table_value = SQLJson::Query::Share()->GetTable(_table_name);
	
	std::stringstream _stream;
	_stream << "DELETE FROM " << _table_name << " WHERE ";
	_stream << "localid='" << unique_id << "'";
	
	if(SQLJson::Query::Share()->Excute(_stream.str(), NULL, NULL)) {
		unique_id = -1;
		live->UnregistData(this);
		is_live = false;
		return true;
	}
	return false;
}

unsigned int Data::GetUniqueID() {
	return unique_id;
}

bool Data::GetIsLived() {
	return is_live;
}

int Data::GetUpdateCount() {
	return update;
}

int Data::GetRetainCount() {
	return retain;
}

void Data::Retain() {
	retain++;
}

void Data::Release() {
	retain--;
	if(retain <= 0)
		delete this;
}