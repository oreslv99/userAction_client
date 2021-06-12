#include "rules.h"

//
// public
//
rules::rules()
	:timerInterval(0), socketRetryInterval(0), afk(nullptr), fileIo(nullptr), process(nullptr), print(nullptr)
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

		if (socket->isOnline() == true)
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
			else
			{
				// 서버에 저장된 정책에 변화가 없거나, 정말 실패
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
int rules::getTimerInterval() const
{
	return this->timerInterval;
}
int rules::getSocketRetryInterval() const
{
	return this->socketRetryInterval;
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
bool rules::deserializeRule(jsonDocumentW document)
{
	// C/C++ 은 Reflection 을 지원하지 않기 때문에.. 아래처럼 그냥 다 작성해야함 (C# 할까..)
	// TODO : string 전부 const 로 바꾸자

	// 감시주기
	if ((document.HasMember(L"timerInterval") == false) || (document[L"timerInterval"].IsInt() == false))
	{
		log->write(logId::error, L"[%s:%d] Invalid rule. [timerInterval]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		this->timerInterval = document[L"timerInterval"].GetInt();
	}

	// 서버 재연결 주기
	if ((document.HasMember(L"serverRetryInterval") == false) || (document[L"serverRetryInterval"].IsInt() == false))
	{
		log->write(logId::error, L"[%s:%d] Invalid rule. [serverRetryInterval]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		this->timerInterval = document[L"serverRetryInterval"].GetInt();
	}

	// 감시기능
	if ((document.HasMember(L"feature") == false) || (document[L"feature"].IsObject() == false))
	{
		log->write(logId::error, L"[%s:%d] Invalid rule. [feature]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		jsonObjectW feature = document[L"feature"].GetObjectW();
		jsonMemberIteratorW featureIter;
		for (featureIter = feature.begin(); featureIter != feature.end(); featureIter++)
		{
			// 자리비움
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"idle") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW idleObject = featureIter->value.GetObjectW();
				jsonMemberIteratorW idleIter;
				for (idleIter = idleObject.begin(); idleIter != idleObject.end(); idleIter++)
				{
					if ((idleIter->name.GetStringLength() > 0) && (::_wcsicmp(idleIter->name.GetString(), L"enabled") == 0) &&
						(idleIter->value.IsBool() == true))
					{
						buffer->watch_afk_enabled = idleIter->value.GetBool();

						// 2020-11-26 orseL
						buffer->watch_afk_event = ((buffer->watch_afk_enabled == true) ? ::CreateEventW(nullptr, FALSE, FALSE, nullptr) : INVALID_HANDLE_VALUE);
					}
					if ((idleIter->name.GetStringLength() > 0) && (::_wcsicmp(idleIter->name.GetString(), L"in") == 0) &&
						(idleIter->value.IsInt() == true))
					{
						buffer->watch_afk_TimeInAfk = idleIter->value.GetInt();
					}
					if ((idleIter->name.GetStringLength() > 0) && (::_wcsicmp(idleIter->name.GetString(), L"awake") == 0) &&
						(idleIter->value.IsInt() == true))
					{
						buffer->watch_afk_TimeAwakeAfk = idleIter->value.GetInt();
					}
				}
			}
			// 프로세스
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"process") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW processObject = featureIter->value.GetObjectW();
				jsonMemberIteratorW processIter;
				for (processIter = processObject.begin(); processIter != processObject.end(); processIter++)
				{
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"enabled") == 0) &&
						(processIter->value.IsBool() == true))
					{
						// 사용유무
						buffer->watch_process_enabled = processIter->value.GetBool();
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"browsers") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// 브라우저 정의
						jsonArrayW browsersArray = processIter->value.GetArray();
						jsonValueIteratorW browsersIter;
						for (browsersIter = browsersArray.begin(); browsersIter != browsersArray.end(); browsersIter++)
						{
							if ((browsersIter->IsString() == true) && (browsersIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(browsersIter->GetString(), &item);
								buffer->watch_process_browsers.push_back(item);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"excludes") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// 감시 예외 프로세스 정의
						jsonArrayW excludesArray = processIter->value.GetArray();
						jsonValueIteratorW excludesIter;
						for (excludesIter = excludesArray.begin(); excludesIter != excludesArray.end(); excludesIter++)
						{
							if ((excludesIter->IsString() == true) && (excludesIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(excludesIter->GetString(), &item);
								buffer->watch_process_excludes.push_back(item);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"privates") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// 사생활 보호 프로세스 정의
						jsonArrayW privatesArray = processIter->value.GetArray();
						jsonValueIteratorW privatesIter;
						for (privatesIter = privatesArray.begin(); privatesIter != privatesArray.end(); privatesIter++)
						{
							if ((privatesIter->IsString() == true) && (privatesIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(privatesIter->GetString(), &item);
								buffer->watch_process_private_contents.push_back(item);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"preventCaptionDuplicate") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// 사생활 보호 프로세스 정의
						jsonArrayW duplicateArray = processIter->value.GetArray();
						jsonValueIteratorW duplicatesIter;
						for (duplicatesIter = duplicateArray.begin(); duplicatesIter != duplicateArray.end(); duplicatesIter++)
						{
							if ((duplicatesIter->IsString() == true) && (duplicatesIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(duplicatesIter->GetString(), &item);
								buffer->watch_process_prevent_caption_duplicates.push_back(item);
							}
						}
					}
				}
			}
			// 파일 io
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"fileIo") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW fileIoObject = featureIter->value.GetObjectW();
				jsonMemberIteratorW fileIoIter;
				for (fileIoIter = fileIoObject.begin(); fileIoIter != fileIoObject.end(); fileIoIter++)
				{
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"enabled") == 0) &&
						(fileIoIter->value.IsBool() == true))
					{
						// 사용유무
						buffer->watch_file_io_enabled = fileIoIter->value.GetBool();
					}
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"excludes") == 0) &&
						(fileIoIter->value.IsArray() == true))
					{
						// 감시 예외경로 정의
						jsonArrayW excludesArray = fileIoIter->value.GetArray();
						jsonValueIteratorW excludesIter;
						for (excludesIter = excludesArray.begin(); excludesIter != excludesArray.end(); excludesIter++)
						{
							if ((excludesIter->IsString() == true) && (excludesIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(excludesIter->GetString(), &item);
								buffer->watch_file_io_exclude_paths.push_back(item);
							}
						}
					}
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"extensions") == 0) &&
						(fileIoIter->value.IsArray() == true))
					{
						// 수집 대상 확장명 정의
						jsonArrayW extensionsArray = fileIoIter->value.GetArray();
						jsonValueIteratorW extensionsIter;
						for (extensionsIter = extensionsArray.begin(); extensionsIter != extensionsArray.end(); extensionsIter++)
						{
							if ((extensionsIter->IsString() == true) && (extensionsIter->GetStringLength() > 0))
							{
								wchar_t *item = nullptr;
								copyString(extensionsIter->GetString(), &item);
								buffer->watch_file_io_include_extensions.push_back(item);
							}
						}
					}
				}
			}
			// 프린터 출력
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"print") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW printingObject = featureIter->value.GetObjectW();
				jsonMemberIteratorW printingIter;
				for (printingIter = printingObject.begin(); printingIter != printingObject.end(); printingIter++)
				{
					if ((printingIter->name.GetStringLength() > 0) && (::_wcsicmp(printingIter->name.GetString(), L"enabled") == 0) &&
						(printingIter->value.IsBool() == true))
					{
						// 사용유무
						buffer->watch_printing_enabled = printingIter->value.GetBool();
					}

				}
			}
		}
	}

	return true;
}