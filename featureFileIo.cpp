#include "featureFileIo.h"

//
// public
//
featureFileIo::featureFileIo()
	:rule(nullptr), event(INVALID_HANDLE_VALUE)
{}
featureFileIo::~featureFileIo()
{}
bool featureFileIo::initialize(void *rule, void *extra, DWORD extraSize)
{
	this->rule = reinterpret_cast<rules*>(rule)->getFileIoRule();
	return true;
}
bool featureFileIo::watch()
{
	return true;
}
bool featureFileIo::isHighPriority()
{
	return true;
}