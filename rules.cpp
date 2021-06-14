#include "rules.h"

//
// public
//
rules::rules()
	:timerInterval(0), serverRetryInterval(0), afk(nullptr), fileIo(nullptr), process(nullptr), print(nullptr)
{
}
rules::~rules()
{
	//safeDelete(this->print);
	//safeDelete(this->process);
	//safeDelete(this->fileIo);
	//safeDelete(this->afk);
}
void rules::initialize(winSock *socket, HWND window)
{
	bool result = false;

	this->afk = new ruleAFK;
	this->fileIo = new ruleFileIo;
	this->process = new ruleProcess;
	this->print = new rulePrint;

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

		if (socket->isOnline() == true)
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
					help->writeLog(logId::info, L"[%s:%d] Read server .", __FUNCTIONW__, __LINE__);
				}
				else
				{
					help->writeLog(logId::error, L"[%s:%d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
				}
			}
			else
			{
				// ������ ����� ��å�� ��ȭ�� ���ų�, ���� ����
			}
		}
		else
		{
			// ������ ����� x, ���� ������ ����
			help->writeLog(logId::info, L"[%s:%d] Read latest rule.", __FUNCTIONW__, __LINE__);
		}
		
		// ��å ���Ϸκ��� �б�
		if (getJsonDocumentFromFile(filePath, &document) == true)
		{
			result = deserializeRule(document);
		}
	}

	if (result == false)
	{
		// TODO : ���� ������� �ʴ� ��츦 ����ؼ� ��å safeDelete �� ��
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

	this->fileIo->window = window;
}
int rules::getTimerInterval() const
{
	return this->timerInterval;
}
int rules::getServerRetryInterval() const
{
	return this->serverRetryInterval;
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
void rules::release()
{
	safeDelete(this->print);
	safeDelete(this->process);
	safeDelete(this->fileIo);
	safeDelete(this->afk);
}

//
// private
//
bool rules::getJsonDocumentFromFile(const std::wstring filePath, jsonDocumentW *buffer)
{
	if ((filePath.length() < 0) || (buffer == nullptr))
	{
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// ���� �ڵ�
	std::wifstream fileStream(filePath);
	if (fileStream.is_open() == false)
	{
		help->writeLog(logId::error, L"[%s:%03d] Can not open: %s", __FUNCTIONW__, __LINE__, filePath.c_str());
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
		help->writeLog(logId::error, L"[%s:%03d] Invalid parameter.", __FUNCTIONW__, __LINE__);
		return false;
	}

	// TODO: json �� �ƴѰ�� ��� �Ǵ��� Ȯ��
	buffer->Parse(jsonString.c_str());
	return buffer->IsObject();
}
bool rules::deserializeRule(jsonDocumentW &document)
{
	// C/C++ �� Reflection �� �������� �ʱ� ������.. �Ʒ�ó�� �׳� �� �ۼ��ؾ��� (C# �ұ�..)
	// TODO : string ���� const �� �ٲ���

	// �����ֱ�
	if ((document.HasMember(L"timerInterval") == false) || (document[L"timerInterval"].IsInt() == false))
	{
		help->writeLog(logId::error, L"[%s:%d] Invalid rule. [timerInterval]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		this->timerInterval = document[L"timerInterval"].GetInt();
	}

	// ���� �翬�� �ֱ�
	if ((document.HasMember(L"serverRetryInterval") == false) || (document[L"serverRetryInterval"].IsInt() == false))
	{
		help->writeLog(logId::error, L"[%s:%d] Invalid rule. [serverRetryInterval]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		this->serverRetryInterval = document[L"serverRetryInterval"].GetInt();
	}

	// ���ñ��
	std::wstring temp;
	if ((document.HasMember(L"features") == false) || (document[L"features"].IsObject() == false))
	{
		help->writeLog(logId::error, L"[%s:%d] Invalid rule. [features]", __FUNCTIONW__, __LINE__);
		return false;
	}
	else
	{
		jsonObjectW featureObj = document[L"features"].GetObjectW();
		jsonMemberIteratorW featureIter;
		for (featureIter = featureObj.begin(); featureIter != featureObj.end(); featureIter++)
		{
			// �ڸ����
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"afk") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW afkObj = featureIter->value.GetObjectW();
				jsonMemberIteratorW afkIter;
				for (afkIter = afkObj.begin(); afkIter != afkObj.end(); afkIter++)
				{
					if ((afkIter->name.GetStringLength() > 0) && (::_wcsicmp(afkIter->name.GetString(), L"enabled") == 0) &&
						(afkIter->value.IsBool() == true))
					{
						this->afk->enabled = afkIter->value.GetBool();
					}
					if ((afkIter->name.GetStringLength() > 0) && (::_wcsicmp(afkIter->name.GetString(), L"in") == 0) &&
						(afkIter->value.IsInt() == true))
					{
						this->afk->in = afkIter->value.GetInt();
					}
					if ((afkIter->name.GetStringLength() > 0) && (::_wcsicmp(afkIter->name.GetString(), L"awake") == 0) &&
						(afkIter->value.IsInt() == true))
					{
						this->afk->awake = afkIter->value.GetInt();
					}
				}
			}
			// ���μ���
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"process") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW processObj = featureIter->value.GetObjectW();
				jsonMemberIteratorW processIter;
				for (processIter = processObj.begin(); processIter != processObj.end(); processIter++)
				{
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"enabled") == 0) &&
						(processIter->value.IsBool() == true))
					{
						// �������
						this->process->enabled = processIter->value.GetBool();
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"excludes") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// ���� ���� ���μ��� ����
						jsonArrayW excludesArr = processIter->value.GetArray();
						jsonValueIteratorW excludesIter;
						for (excludesIter = excludesArr.begin(); excludesIter != excludesArr.end(); excludesIter++)
						{
							if ((excludesIter->IsString() == true) && (excludesIter->GetStringLength() > 0))
							{
								temp = excludesIter->GetString();
								//help->toLower(temp);
								this->process->excludes.push_back(temp);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"browsers") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// ������ ����
						jsonArrayW browsersArray = processIter->value.GetArray();
						jsonValueIteratorW browsersIter;
						for (browsersIter = browsersArray.begin(); browsersIter != browsersArray.end(); browsersIter++)
						{
							if ((browsersIter->IsString() == true) && (browsersIter->GetStringLength() > 0))
							{
								temp = browsersIter->GetString();
								//help->toLower(temp);
								this->process->browsers.push_back(temp);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"privates") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// ���Ȱ ��ȣ ���μ��� ����
						jsonArrayW privatesArr = processIter->value.GetArray();
						jsonValueIteratorW privatesIter;
						for (privatesIter = privatesArr.begin(); privatesIter != privatesArr.end(); privatesIter++)
						{
							if ((privatesIter->IsString() == true) && (privatesIter->GetStringLength() > 0))
							{
								temp = privatesIter->GetString();
								//help->toLower(temp);
								this->process->privates.push_back(temp);
							}
						}
					}
					if ((processIter->name.GetStringLength() > 0) && (::_wcsicmp(processIter->name.GetString(), L"preventCaptionDuplicate") == 0) &&
						(processIter->value.IsArray() == true))
					{
						// ���α׷��� ó�� �ʹ� ���� caption �� ���ϴ� ���μ���
						jsonArrayW duplicateArr = processIter->value.GetArray();
						jsonValueIteratorW duplicatesIter;
						for (duplicatesIter = duplicateArr.begin(); duplicatesIter != duplicateArr.end(); duplicatesIter++)
						{
							if ((duplicatesIter->IsString() == true) && (duplicatesIter->GetStringLength() > 0))
							{
								temp = duplicatesIter->GetString();
								//help->toLower(temp);
								this->process->duplicates.push_back(temp);
							}
						}
					}
				}
			}
			// ���� io
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"fileIo") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW fileIoObj = featureIter->value.GetObjectW();
				jsonMemberIteratorW fileIoIter;
				for (fileIoIter = fileIoObj.begin(); fileIoIter != fileIoObj.end(); fileIoIter++)
				{
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"enabled") == 0) &&
						(fileIoIter->value.IsBool() == true))
					{
						// �������
						this->fileIo->enabled = fileIoIter->value.GetBool();
					}
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"excludes") == 0) &&
						(fileIoIter->value.IsArray() == true))
					{
						// ���� ���ܰ�� ����
						jsonArrayW excludesArr = fileIoIter->value.GetArray();
						jsonValueIteratorW excludesIter;
						for (excludesIter = excludesArr.begin(); excludesIter != excludesArr.end(); excludesIter++)
						{
							if ((excludesIter->IsString() == true) && (excludesIter->GetStringLength() > 0))
							{
								temp = excludesIter->GetString();
								//help->toLower(temp);
								this->fileIo->excludes.push_back(temp);
							}
						}
					}
					if ((fileIoIter->name.GetStringLength() > 0) && (::_wcsicmp(fileIoIter->name.GetString(), L"extensions") == 0) &&
						(fileIoIter->value.IsArray() == true))
					{
						// ���� ��� Ȯ��� ����
						jsonArrayW extensionsArr = fileIoIter->value.GetArray();
						jsonValueIteratorW extensionsIter;
						for (extensionsIter = extensionsArr.begin(); extensionsIter != extensionsArr.end(); extensionsIter++)
						{
							if ((extensionsIter->IsString() == true) && (extensionsIter->GetStringLength() > 0))
							{
								temp = extensionsIter->GetString();
								//help->toLower(temp);
								this->fileIo->extensions.push_back(temp);
							}
						}
					}
				}
			}
			// ������ ���
			if ((featureIter->name.GetStringLength() > 0) && (::_wcsicmp(featureIter->name.GetString(), L"print") == 0) &&
				(featureIter->value.IsObject() == true))
			{
				jsonObjectW printObj = featureIter->value.GetObjectW();
				jsonMemberIteratorW printIter;
				for (printIter = printObj.begin(); printIter != printObj.end(); printIter++)
				{
					if ((printIter->name.GetStringLength() > 0) && (::_wcsicmp(printIter->name.GetString(), L"enabled") == 0) &&
						(printIter->value.IsBool() == true))
					{
						// �������
						this->print->enabled = printIter->value.GetBool();
					}

				}
			}
		}
	}

	return true;
}