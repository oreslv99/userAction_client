#pragma once
#include "stdafx.h"
#include ".\\extern\\rapidJson\\document.h"
#include ".\\extern\\rapidJson\\writer.h"
#include ".\\extern\\rapidJson\\istreamwrapper.h"

// re-define
typedef rapidjson::GenericDocument<rapidjson::UTF8<>> jsonDocumentA;
typedef rapidjson::GenericDocument<rapidjson::UTF16<>> jsonDocumentW;

class rapidJson
{
public:
	rapidJson();
	~rapidJson();
	bool initialize();

private:
	

};