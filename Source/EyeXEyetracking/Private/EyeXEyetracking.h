// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Created by Fredrik Lindh (Temaran)
// Contact me at: temaran (at) gmail (dot) com
// Last changed: 2014-05-17
// You're free to do whatever you want with the code except claiming it's you who wrote it.
// I would also appreciate it if you kept this file header as a thank you for the code :)

#pragma once

#include "EyeXHost.h"

class FEyeXEyetracking : public IEyeXEyetracking
{
public:
	static FEyeXHost& GetHost()
	{
		return *_host;
	}

public:
	/** IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;

private:
	static FEyeXHost* _host;
};

IMPLEMENT_MODULE(FEyeXEyetracking, EyeXEyetracking)
