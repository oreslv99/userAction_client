#include "featureFileIo.h"

//
// public
//
featureFileIo::featureFileIo()
	:rule(nullptr), event(INVALID_HANDLE_VALUE)
{}
featureFileIo::~featureFileIo()
{
}
bool featureFileIo::initialize(const rules &rule)
{
	this->rule = rule.getFileIoRule();
	return true;
}
bool featureFileIo::watch()
{
	return true;
}
//featureType featureFileIo::getFeatureType()
//{
//	return featureType::fileIo;
//}