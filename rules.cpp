#include "rules.h"

//
// public
//
rules::rules()
	:afk(nullptr), fileIo(nullptr), process(nullptr), print(nullptr)
{
}
rules::~rules()
{
}
void rules::initialize(bool isOnline, winSock *socket)
{
	bool result = false;

	this->afk = new ruleAFK;
	this->fileIo = new ruleFileIo;
	this->process = new ruleProcess;
	this->print = new rulePrint;

	if (isOnline == true)
	{
		//socket->
	}
	else
	{

		// ini ���ϰ��
		// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
		wchar_t *profile = nullptr;
		if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
		{
			std::wstring ruleFilePath;
			ruleFilePath += profile;
			ruleFilePath += L"\\.userAction\\rule.json";

			// SHGetKnownFolderPath �� Ȯ���� wchar_t buffer �� CoTaskMemFree �� release
			safeCoTaskMemFree(profile);

			// ��å���� ������ ����
			FILE *file = nullptr;
			::_wfopen_s(&file, ruleFilePath.c_str(), L"r");
			if (file != nullptr)
			{
				::fclose(file);
			}
		}
	}

	if (result == false)
	{
		// ��� ���н� �⺻�� ����
		this->afk->enabled = true;
		this->afk->in = 5000;
		this->afk->awake = 3000;

		this->process->enabled = true;
		this->process->excludes = { L"", L"", L"", };
		this->process->browsers = { L"", L"", L"", };
		this->process->privates = { L"", L"", L"", };
		this->process->duplicates = { L"", L"", L"", };

		this->print->enabled = true;
	}
}
ruleAFK *rules::getAFKRule()
{
	return this->afk;
}
ruleFileIo *rules::getFileIoRule()
{
	return this->fileIo;
}
ruleProcess *rules::getProcessRule() 
{
	return this->process;
}
rulePrint *rules::getPrintRule() 
{ 
	return this->print; 
}