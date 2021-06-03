#pragma once

enum featureType
{
	afk,
	proc,
	prn,
	io,
};

__interface IFeature
{
	bool initialize();
	bool watch();
	featureType getType();
};