//
//  SQLJsonLive.h
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#ifndef __GelatoManiaWithFriend__SQLJsonLive__
#define __GelatoManiaWithFriend__SQLJsonLive__

#include <iostream>
#include <list>
#include "SQLJsonData.h"

namespace SQLJson {
	class Query;
	
	class Live {
		
		friend class Data;
		friend class Query;
		
	private:
		
		std::string name;
		std::list<Data*> list_data;
		Query* query;
		
		Live(Query* _query, std::string _name);
		
		std::string GetName();
		Data* GetIsOpenedData(unsigned int _index_unique);
		
		Data* CreateRegistData(unsigned int _index_unique);
		void UnregistData(Data* _data);
	};
}

#endif /* defined(__GelatoManiaWithFriend__SQLJsonLive__) */
