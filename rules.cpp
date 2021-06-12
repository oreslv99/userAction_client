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

	// 정책파일 확인
	std::wstring filePath;
	wchar_t *profile = nullptr;
	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &profile)))
	{
		filePath += profile;
		filePath += L"\\.userAction\\rule.json";

		// SHGetKnownFolderPath 로 확인한 wchar_t buffer 는 CoTaskMemFree 로 release
		safeCoTaskMemFree(profile);

		// json object
		jsonDocumentW document;

		if (socket->isOnline == true)
		{
			// 서버와 통신이 o, 파일로 저장하고 읽음
			std::wstring buffer;
			if (socket->request(requestId::rule, &buffer) == true)
			{
				// 파일 열기 (있으면 삭제하고 다시 만듬)
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
			// 서버와 통신이 x, 이전 파일을 읽음
			log->write(logId::info, L"[%s:%d] Read latest rule.", __FUNCTIONW__, __LINE__);
		}
		
		// 정책 파일로부터 읽기
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

		// 모두 실패시 기본값 적용
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

	// 파일 핸들
	std::wifstream fileStream(filePath);
	if (fileStream.is_open() == false)
	{
		log->write(logId::error, L"[%s:%03d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
		return false;
	}

	// 파일의 인코딩 타입을 utf8 로 설정
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

	// TODO: json 이 아닌경우 어떻게 되는지 확인
	buffer->Parse(jsonString.c_str());
	return buffer->IsObject();
}