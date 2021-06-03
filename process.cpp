#include "process.h"

process::process()
{}
process::~process()
{}
bool process::initialize()
{
	return true;
}
bool process::watch()
{
	return true;
}
featureType process::getType()
{
	return featureType::proc;
}