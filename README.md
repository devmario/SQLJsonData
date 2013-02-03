이 라이브러리는 sqlite를 이용하여 controller에서 굳이 view에게 변경사항을 전달하지 않아도 되게 할려고 만든 라이브러리입니다.

+ http://jsoncpp.sourceforge.net/ 에서 jsoncpp 다운로드(SQLJson을 사용할라면 jsoncpp필요)

+ sqlite3필요

+ SQLJsonQuery.h의 DB_PATH설정(사용할 db경로:최초 한번 코드호출시 생성됨)
만일 4번의json을 바꾸면 이파일을 삭제시켜야함

+ SQLJsonQuery.h의 DB_INFO_PATH설정(db의 스키마 정보 json경로)
SQLJson이 실행되면(최초) 이파일을 불러들여 테이블을 생성함
sql.json에 sample스키마가 들어있음

+ sql.json설명
default_table_field:이 필드들은 모든 테이블에 자동적으로 추가된다.추가시키기 싫으면 include_default_table_field을 false로 설정
table 생성되는 테이블 스키마들이다.name은 table이름이다.
query는 쿼리에대한 태그와 쿼리포매터로 이루어져 있다


+ 다음은 샘플임

```c++
  //셀렉트(포매터,sql.json확인해보삼)
	std::vector<SQLJson::Data*> _vec0 = SQLJson::Query::Share()->Select("select_user_by_id", 1);
	if(_vec0.size() > 0) {
		std::cout << _vec0[0]->toStyledString() << _vec0[0]->GetRetainCount() << "\n" << _vec0[0]->GetUpdateCount() << "\n";
		_vec0[0]->Update(_test, true);
		//있으면 업데이트
	} else {
		//없으면 추기
		SQLJson::Query::Share()->Add("user", _test, true);
	}

	//이상테는 메모리에 SQLJson::Data가 1개 올라가있음(이전 쿼리가 Add되었다면 0개)

	//셀렉트(포매터,sql.json확인해보삼)
	std::vector<SQLJson::Data*> _vec1 = SQLJson::Query::Share()->Select("select_user_by_id", 1);
	for(int i = 0; i < _vec1.size(); i++) {
		std::cout << _vec1[i]->toStyledString() << _vec1[i]->GetRetainCount() << "\n" << _vec1[i]->GetUpdateCount() << "\n";
		(*_vec1[i])["name"] = "modify";
		//데이터바꾸기(바꾸고 업데이트)
		_vec1[i]->Update(true);
	}

	//이상테는 메모리에 SQLJson::Data가 2개 올라가있음

	//셀렉트(포매터,sql.json확인해보삼)
	std::vector<SQLJson::Data*> _vec2 = SQLJson::Query::Share()->Select("select_user_by_id", 1);

	//이상테는 메모리에 SQLJson::Data가 3개 올라가있음

	for(int i = 0; i < _vec2.size(); i++) {
		//출력후 메모리에서 내림
		std::cout << _vec2[i]->toStyledString() << _vec2[i]->GetRetainCount() << "\n" << _vec2[i]->GetUpdateCount() << "\n";
		_vec2[i]->Release();
	}
	for(int i = 0; i < _vec1.size(); i++) {
		//출력후 메모리에서 내림
		std::cout << _vec1[i]->toStyledString() << _vec1[i]->GetRetainCount() << "\n" << _vec1[i]->GetUpdateCount() << "\n";
		_vec1[i]->Release();
	}
	for(int i = 0; i < _vec0.size(); i++) {
		//출력후 메모리에서 내림
		std::cout << _vec0[i]->toStyledString() << _vec0[i]->GetRetainCount() << "\n" << _vec0[i]->GetUpdateCount() << "\n";
		_vec0[i]->Release();
	}
```


+ SQLJson::Data의 GetUpdateCount와 GetIsLive를 매프레임마다 체크하여  화면에서 데이터 변경상태를 보여주세요
GetUpdateCount는 SQL Update문에 올라가면 해당Data에 대해서 카운트를 1올려줍니다.
GetIsLive는 필드에서 DELETE됬을때 false로 바뀝니다.

+ 화면에서 더이상 데이터가 필요하지 않으면(출력하지 않으면) 메모리에서 내리기 위하여 Release를 호출하세요
sqlite에데이터는 계속 존제합니다.
