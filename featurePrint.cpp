#include "featurePrint.h"

const std::wstring REGISTRY_PATH = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\";
const std::wstring EVENTVIEWER_CHANNEL_PATH = L"Microsoft-Windows-PrintService/Operational";
const std::wstring QUERY = L"Event/System[EventID=307]";

//
// public
//
featurePrint::featurePrint()
	:rule(nullptr)
{
}
featurePrint::~featurePrint()
{
	safeDelete(this->rule);
}

bool featurePrint::initialize(rules *rule)
{
	this->rule = rule->getPrintRule();
	if (this->rule->enabled == false)
	{
		// 해당 기능사용 안함
		log->write(errId::warning, L"[%s:%03d] Feature print is disabled.", __FUNCTIONW__, __LINE__);
		return true;
	}

	// 이벤트 뷰어에서 프린트 서비스를 따로 볼 수있게 다음 항목을 등록
	const std::wstring path = REGISTRY_PATH + EVENTVIEWER_CHANNEL_PATH;
	setRegistryKey(HKEY_LOCAL_MACHINE, path);

	// 기존 로그삭제 
	//	: 기존에 얼마나 출력한 로그가 쌓여있는지 알 수 없고,
	//	: 그 양에 따라서 EvtNext 사용되는 EVT_HANDLE 의 크기를 가늠할 수 없음.
	if (::EvtClearLog(nullptr, EVENTVIEWER_CHANNEL_PATH.c_str(), nullptr, 0) == FALSE)
	{
		log->write(errId::warning, L"[%s:%03d] code[%d] EvtClearLog is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
		//return false;
	}

	return true;
}
bool featurePrint::watch()
{
	::Sleep(1);

	// 출력시 발생되는 이벤트 id 순서 
	//	: 800 >> 801 >> 842 >> 812 >> 805 >> 307
	// 특정 조건: L"Event/System[EventID=xxx]"
	// 전체		: L"Event/System"

	// https://docs.microsoft.com/en-us/windows/win32/api/winevt/ne-winevt-evt_query_flags
	// EvtQueryReverseDirection : 가장 오래전 기록된 레코드
	// EvtQueryForwardDirection : 가장 최근 기록된 레코드
	DWORD flags = EvtQueryChannelPath | EvtQueryForwardDirection;
	EVT_HANDLE result = ::EvtQuery(nullptr, EVENTVIEWER_CHANNEL_PATH.c_str(), QUERY.c_str(), flags);
	if (result != nullptr)
	{
		seekEvent(result);

		// release
		::EvtClose(result);
		result = nullptr;
	}
	else
	{
		DWORD err = ::GetLastError();
		switch (err)
		{
		case ERROR_EVT_CHANNEL_NOT_FOUND:
			log->write(errId::error, L"[%s:%03d] code[%d] The channel is not found.", __FUNCTIONW__, __LINE__, err);
			break;
		case ERROR_EVT_INVALID_QUERY:
			log->write(errId::error, L"[%s:%03d] code[%d] The query is invalid.", __FUNCTIONW__, __LINE__, err);
			break;
		default:
			log->write(errId::error, L"[%s:%03d] code[%d] The query is invalid.", __FUNCTIONW__, __LINE__, err);
			break;
		}

		return false;
	}

	return true;
}
bool featurePrint::isHighPriority()
{
	return false;
}

//
// private
//
void featurePrint::setRegistryKey(HKEY hive, std::wstring path)
{
	HKEY result = nullptr;
	::RegOpenKeyExW(hive, path.c_str(), 0, KEY_WRITE, &result);

	// 없으면 생성
	if (result == nullptr)
	{
		DWORD disposition = 0;
		::RegCreateKeyExW(hive, path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &result, &disposition);
		if (result == nullptr)
		{
			return;
		}
	}

	::RegCloseKey(result);
}
void featurePrint::getRegistryValue(HKEY hive, std::wstring subKey, std::wstring name, DWORD valueType, void* value, DWORD valueSize)
{
	HKEY result = nullptr;
	if (::RegOpenKeyExW(hive, subKey.c_str(), 0, KEY_QUERY_VALUE, &result) == ERROR_SUCCESS)
	{
		::RegQueryValueExW(result, name.c_str(), nullptr, &valueType, (BYTE*)value, &valueSize);
	}

	::RegCloseKey(result);
}
void featurePrint::parseDocument(tinyxml2::XMLDocument *document)
{
	/*
	
	예시)

	<Event xmlns="http://schemas.microsoft.com/win/2004/08/events/event">
	  <System>
		<Provider Name="Microsoft-Windows-PrintService" Guid="{747ef6fd-e535-4d16-b510-42c90f6873a1}" />
		<EventID>307</EventID>
		<Version>0</Version>
		<Level>4</Level>
		<Task>26</Task>
		<Opcode>11</Opcode>
		<Keywords>0x4000000000000840</Keywords>
		<TimeCreated SystemTime="2020-10-08T09:40:50.007040200Z" />
		<EventRecordID>635</EventRecordID>
		<Correlation />
		<Execution ProcessID="3860" ThreadID="21552" />
		<Channel>Microsoft-Windows-PrintService/Operational</Channel>
		<Computer>DESKTOP-SGSJTLH</Computer>
		<Security UserID="S-1-5-21-3060918736-165454301-2844304238-1001" />
	  </System>
	  <UserData>
		<DocumentPrinted xmlns="http://manifests.microsoft.com/win/2005/08/windows/printing/spooler/core/events">
		  <Param1>45</Param1>
		  <Param2>문서 인쇄</Param2>
		  <Param3>orseL</Param3>
		  <Param4>\\DESKTOP-SGSJTLH</Param4>
		  <Param5>Microsoft Print to PDF</Param5>
		  <Param6>F:\02 업무\_release\zzz.pdf</Param6>
		  <Param7>110128</Param7>
		  <Param8>2</Param8>
		</DocumentPrinted>
	  </UserData>
	</Event>

	time created (TODO : add 9 hours because it's GMT)
	param5 : 프린터 이름
	param6 : 파일 경로
	param7 : 파일 크기
	param8 : 출력 부수

	*/

	// event >> system >> timeCreated
	std::string eventTime = document->FirstChildElement("Event")->FirstChildElement("System")->FirstChildElement("TimeCreated")->Attribute("SystemTime");
	
	// event >> userData >> documentPrinted
	tinyxml2::XMLElement *documentPrinted = document->FirstChildElement("Event")->FirstChildElement("UserData")->FirstChildElement("DocumentPrinted");
	std::string printerName = documentPrinted->FirstChildElement("Param5")->GetText();
	std::string filePath = documentPrinted->FirstChildElement("Param6")->GetText();
	std::string copies = documentPrinted->FirstChildElement("Param8")->GetText();
	
	// 로그형태
	std::string logFormat;
	logFormat.resize(1024);

	// jsondata 에서는 \ >> \\ 로 변경해야함 (경로)
	size_t startPos = 0;
	while ((startPos = filePath.find("\\", startPos)) != std::string::npos)
	{
		filePath.replace(startPos, 1, "\\\\");
		startPos += 2;
	}

	// jsondata 포맷
	::wsprintfA(const_cast<char*>(logFormat.data()), "{\"eventTime\":\"%s\", \"printerName\":\"%s\", \"filePath\":\"%s\", \"copies\":%s}", 
		eventTime.c_str(), printerName.c_str(), filePath.c_str(), copies.c_str());

	//// 아래와 같이 c++ string 클래스를 이용한 변환시 깨져서 기록됨
	//std::string logFormatW;
	//logFormatW.assign(logFormat.begin(), logFormat.end());

	// 2021-06-03 : c 스타일로 변환하여 출력
	size_t length = ::MultiByteToWideChar(CP_ACP, 0, logFormat.c_str(), -1, nullptr, 0);
	length++;
	wchar_t *logFormatW = (wchar_t*)::calloc(length, sizeof(wchar_t*));
	::MultiByteToWideChar(CP_ACP, 0, logFormat.c_str(), -1, logFormatW, length);
	
	log->write(errId::user, L"printEvent %s", logFormatW);
	safeFree(logFormatW);

	// 기록이 완료됬으면 이전 이벤트 뷰어 데이터 삭제
	if (::EvtClearLog(nullptr, EVENTVIEWER_CHANNEL_PATH.c_str(), nullptr, 0) == FALSE)
	{
		log->write(errId::warning, L"[%s:%03d] code[%d] EvtClearLog is failed.", __FUNCTIONW__, __LINE__, ::GetLastError());
	}
}
void featurePrint::renderEvent(EVT_HANDLE fragment)
{
	// 버퍼 크기확인
	DWORD bufferSize;
	DWORD propertyCount;
	::EvtRender(nullptr, fragment, EvtRenderEventXml, 0, nullptr, &bufferSize, &propertyCount);
	
	// 버퍼 크기할당 후 데이터 확인
	std::wstring buffer;
	buffer.resize(bufferSize);
	::EvtRender(nullptr, fragment, EvtRenderEventXml, bufferSize, const_cast<wchar_t*>(buffer.data()), &bufferSize, &propertyCount);
	if (buffer.length() > 0)
	{
		std::string bufferA;
		bufferA.assign(buffer.begin(), buffer.end());

		// xml 파싱
		tinyxml2::XMLDocument document;
		tinyxml2::XMLError error = document.Parse(bufferA.c_str());
		if (error == tinyxml2::XMLError::XML_SUCCESS)
		{
			parseDocument(&document);
		}
	}
}
void featurePrint::seekEvent(EVT_HANDLE queryResult)
{
	// queryResult 의 가장 마지막 record 로 이동
	::EvtSeek(queryResult, 0, nullptr, 0, EvtSeekRelativeToLast);

	// 마지막 record 확인
	EVT_HANDLE evtHandle[1];
	DWORD returned = 0;
	if (::EvtNext(queryResult, _countof(evtHandle), evtHandle, INFINITE, 0, &returned) == TRUE)
	{
		for (DWORD i = 0; i < returned; i++)
		{
			renderEvent(evtHandle[i]);
		}
	}

	// 해제
	for (DWORD i = 0; i < _countof(evtHandle); i++)
	{
		if (evtHandle[i] != nullptr)
		{
			::EvtClose(evtHandle[i]);
			evtHandle[i] = nullptr;
		}
	}
}