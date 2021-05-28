#pragma once
#include "stdafx.h"
#include ".\\extern\\tinyxml2\\tinyxml2.h"
#include <string>

class tinyXml 
{
public:
	tinyXml();
	~tinyXml();
	bool initialize();
	bool parse();

private:
	tinyxml2::XMLDocument document;

};