#include "rules.h"

//
// public
//
rules::rules()
	:afk(nullptr), process(nullptr), print(nullptr)
{
}
rules::~rules()
{
}
void rules::initialize(bool isOnline, winSock *socket)
{
	bool result = false;
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
	}
}
ruleAFK *rules::getAFKRule() 
{
	return this->afk;
}
ruleProcess *rules::getProcessRule() 
{
	return this->process;
}
rulePrint *rules::getPrintRule() 
{ 
	return this->print; 
}