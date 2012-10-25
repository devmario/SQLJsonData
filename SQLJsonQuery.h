//
//  SQLJsonQuery.h
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#ifndef __GelatoManiaWithFriend__SQLJsonQuery__
#define __GelatoManiaWithFriend__SQLJsonQuery__

#include <iostream>
#include "SQLJsonLive.h"
#include <vector>
#include <sqlite3.h>
#include <list>

#include "Device.h"
#define DB_PATH (Device::Manager::Share()->GetDocumentPath() + "/data.db")
#define DB_INFO_PATH (Device::Manager::Share()->GetResourcePath() + "/sql.json")

namespace SQLJson {
	class Query : public Json::Value {
		
		friend class Data;
		
	private:
		
		sqlite3* db;
		
		std::list<Live*> list_live;
		
		Query();
		virtual ~Query();
		
		static int _SQL_DATA_CALLBACK(void* _reference, int _field_length, char** _field, char** _field_name);
		static int _SQL_COUNT_CALLBACK(void* _reference, int _field_length, char** _field, char** _field_name);
		
		Live* GetLive(std::string _name);
		
		bool Excute(std::string _query, int (*_callback)(void*, int, char**, char**), void* _reference);
		
		Json::Value GetTable(std::string _name);
		
		Json::Value GetQuery(std::string _name);
		
		Json::Value ToLocal(Json::Value _table, Json::Value _value);
		
	public:
		
		//TIME
		/******************************************************************************/
		static time_t QueryToTime(const std::string _query_time);
		
		static const std::string TimeToQuery(time_t _time);
		
		static time_t CurrentTime();
		/******************************************************************************/
		
		//USE
		/******************************************************************************/
		static Query* Share();
		
		static bool Drop();
		
		bool Add(std::string _table_name, Json::Value _value, bool _is_response);
		
		std::vector<Data*> Select(std::string _query_name, ...);
		
		int Count(std::string _query_name, ...);
		/******************************************************************************/
	};
}

#endif /* defined(__GelatoManiaWithFriend__SQLJsonQuery__) */
