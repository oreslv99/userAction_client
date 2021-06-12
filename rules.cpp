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
	safeDelete(this->print);
	safeDelete(this->process);
	safeDelete(this->fileIo);
	safeDelete(this->afk);
}
void rules::initialize(winSock *socket, HWND window)
{
	bool result = false;

	// ��å���� Ȯ��
	std::wstring filePath;
	wchar_t *profile = nullptr;
	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
	{
		filePath += profile;
		filePath += L"\\.userAction\\rule.json";

		// SHGetKnownFolderPath �� Ȯ���� wchar_t buffer �� CoTaskMemFree �� release
		safeCoTaskMemFree(profile);

		// json object
		jsonDocumentW document;

		if (socket->isOnline == true)
		{
			// ������ ����� o, ���Ϸ� �����ϰ� ����
			std::wstring buffer;
			if (socket->request(requestId::rule, &buffer) == true)
			{
				// ���� ���� (������ �����ϰ� �ٽ� ����)
				std::wofstream data;
				data.open(filePath, std::ios::trunc);
				if (data.is_open() == true)
				{
					data << buffer;
					data.close();
					log->write(logId::info, L"[%s:%d] Read server .", __FUNCTIONW__, __LINE__);
				}
				else
				{
					log->write(logId::error, L"[%s:%d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
				}
			}
		}
		else
		{
			// ������ ����� x, ���� ������ ����
			log->write(logId::info, L"[%s:%d] Read latest rule.", __FUNCTIONW__, __LINE__);
		}
		
		// ��å ���Ϸκ��� �б�
		if (getJsonDocumentFromFile(filePath, &document) == true)
		{
			result = true;
		}
	}

	if (result == false)
	{
		this->afk = new ruleAFK;
		this->fileIo = new ruleFileIo;
		this->process = new ruleProcess;
		this->print = new rulePrint;

		// ��� ���н� �⺻�� ����
		this->afk->enabled = true;
		this->afk->in = 5000;
		this->afk->awake = 3000;

		this->fileIo->enabled = true;
		this->fileIo->window = window;
		this->fileIo->excludes = { L"", L"", L"", };
		this->fileIo->extensions = { L"", L"", L"", };

		this->process->enabled = true;
		this->process->excludes = { L"", L"", L"", };
		this->process->browsers = { L"", L"", L"", };
		this->process->privates = { L"", L"", L"", };
		this->process->duplicates = { L"", L"", L"", };

		this->print->enabled = true;
	}
}
ruleAFK *rules::getAFKRule() const
{
	return this->afk;
}
ruleFileIo *rules::getFileIoRule() const
{
	return this->fileIo;
}
ruleProcess *rules::getProcessRule() const
{
	return this->process;
}
rulePrint *rules::getPrintRule() const
{ 
	return this->print; 
}

//
// private
//
bool rules::getJsonDocumentFromFile(const std::wstring filePath, jsonDocumentW *buffer)
{
	if ((filePath.length() < 0) || (buffer == nullptr))
	{
		log->write(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// ���� �ڵ�
	std::wifstream fileStream(filePath);
	if (fileStream.is_open() == false)
	{
		log->write(logId::error, L"[%s:%03d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
		return false;
	}

	// ������ ���ڵ� Ÿ���� utf8 �� ����
	fileStream.imbue(std::locale(fileStream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));
	rapidjson::WIStreamWrapper streamWrapper(fileStream);
	buffer->ParseStream(streamWrapper);

	return true;
}
bool rules::getJsonDocumentFromString(const std::wstring jsonString, jsonDocumentW *buffer)
{
	if ((jsonString.length() < 0) || (buffer == nullptr))
	{
		log->write(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// TODO: json �� �ƴѰ�� ��� �Ǵ��� Ȯ��
	buffer->Parse(jsonString.c_str());
	return buffer->IsObject();
}