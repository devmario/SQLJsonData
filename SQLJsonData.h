//
//  SQLJsonData.h
//  GelatoManiaWithFriend
//
//  Created by wonhee jang on 12. 10. 22..
//
//

#ifndef __GelatoManiaWithFriend__SQLJsonData__
#define __GelatoManiaWithFriend__SQLJsonData__

#include <iostream>
#include "json.h"

namespace SQLJson {
	class Live;
	
	class Data : public Json::Value {
		friend class Query;
		friend class Live;
		
	private:
		
		int unique_id;
		int retain;
		
		Live* live;
		
		int update;
		bool is_live;
		
		Data(Live* _live, int _unique);
		virtual ~Data();
		
	public:
		
		//USE
		/******************************************************************************/
		bool Update(Json::Value _value, bool _is_response);
		bool Update(bool _is_response);
		bool Delete();
		
		unsigned int GetUniqueID();
		
		bool GetIsLived();
		int GetUpdateCount();
		
		int GetRetainCount();
		
		void Retain();
		void Release();
		/******************************************************************************/
	};
};

#endif /* defined(__GelatoManiaWithFriend__SQLJsonData__) */
