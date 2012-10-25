//
//  SQLJsonLive.cpp
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#include "SQLJsonLive.h"
#include "SQLJsonQuery.h"

using namespace SQLJson;

Live::Live(Query* _query, std::string _name) {
	query = _query;
	name = _name;
}

std::string Live::GetName() {
	return name;
}

Data* Live::GetIsOpenedData(unsigned int _index_unique) {
	std::list<Data*>::iterator _it = list_data.begin();
	while(_it != list_data.end()) {
		if((*_it)->GetUniqueID() == _index_unique)
			return *_it;
		_it++;
	}
	return NULL;
}

Data* Live::CreateRegistData(unsigned int _index_unique) {
	Data* _data = new Data(this, _index_unique);
	list_data.push_back(_data);
	return _data;
}

void Live::UnregistData(Data* _data) {
	list_data.remove(_data);
}