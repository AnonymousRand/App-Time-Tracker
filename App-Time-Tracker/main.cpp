#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <psapi.h>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <fstream>

std::fstream testfile; // test

std::wstring timeToWString(long long seconds) {
	int h = seconds / 3600;
	int m = (seconds - h * 3600) / 60;
	int s = seconds - h * 3600 - m * 60;
	if (h == 0) {
		return std::wstring(L"000000:") + (m < 10 ? L"0" : L"")
			+ std::to_wstring(m) + L":" + (s < 10 ? L"0" : L"")
			+ std::to_wstring(s);
	}
	return std::wstring(5 - (int)(log(h) / log(10)), L'0')
		+ std::to_wstring(h) + L":" + (m < 10 ? L"0" : L"")
		+ std::to_wstring(m) + L":" + (s < 10 ? L"0" : L"")
		+ std::to_wstring(s);
}

long long timeToLLong(std::wstring sTime) {
	std::wstring h = sTime.substr(0, sTime.find(L':'));
	sTime.erase(0, sTime.find(L':') + 1);
	std::wstring m = sTime.substr(0, sTime.find(L':'));
	sTime.erase(0, sTime.find(L':') + 1);
	return std::stoll(h, nullptr) * 3600 + std::stoll(m, nullptr) * 60 + std::stoll(sTime, nullptr);
}

void addToFile(std::wfstream &fileOverwrite, std::wofstream &fileAppend, std::wstring currentTitle) {
	std::wstring title, time, newLine;
	std::wstring previousTitle = L"", previousTime = L"999999:99:99";
	bool match = false;

	// read through file and check if current active name already has an entry
	while (getline(fileOverwrite, title)) {
		getline(fileOverwrite, time);

		if (sizeof(time) > 0 || sizeof(title) > 0 || sizeof(currentTitle) > 0 || true) {
			testfile << "breakpoint test";
		}

		// if yes, update time
		if (title == currentTitle) {
			match = true;
			fileOverwrite.seekp((int)fileOverwrite.tellp() - 13); // move cursor to start of current time line
			time = timeToWString(timeToLLong(time) + 1);
			fileOverwrite << time;
		}

		// sort: if this entry has a greater time value than the previous one, swap them
		if (timeToLLong(time) > timeToLLong(previousTime)) {
			fileOverwrite.seekp((int)fileOverwrite.tellp() - 13 - 155 - 13 - 155 - 2); // move cursor to start of previous title line; 155 instead of the line length 128 since GetModuleFileNameExW() adds a 27-character sequence at the end
			fileOverwrite << title << '\n'; // move cursor to start of previous time line
			fileOverwrite << time << "\n\n"; // move cursor to start of current title line
			fileOverwrite << previousTitle << '\n'; // move cursor to start of current time line
			fileOverwrite << previousTime << "\n\n";
		} else {
			previousTitle = title;
			previousTime = time;
			fileOverwrite << '\n'; // move cursor past extra newline at the end of each entry
		}

		if (match) {
			return;
		}
	}

	// else add entry if it doesn't exist
	fileAppend << currentTitle << L'\n' << L"000000:00:01" << L"\n\n";
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	std::wofstream fileAppendD_exe, fileAppendD_title, fileAppendA_exe, fileAppendA_title;
	std::wfstream fileOverwriteD_exe, fileOverwriteD_title, fileOverwriteA_exe, fileOverwriteA_title;
	testfile.open("test.txt", std::ios::app); // test

	const std::string ENV = std::string(std::getenv("USERPROFILE"));
	wchar_t currentTitle[128];
	DWORD identifier;
	int len;

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		fileAppendD_exe.open(ENV + "\\Documents\\daily_exe.txt", std::ios::app | std::ios::binary); // opening in binary mode avoids automatic '\r' insertions before '\n' which caused a LOT of pain
		fileOverwriteD_exe.open(ENV + "\\Documents\\daily_exe.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendD_title.open(ENV + "\\Documents\\daily_title.txt", std::ios::app | std::ios::binary);
		fileOverwriteD_title.open(ENV + "\\Documents\\daily_title.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendA_exe.open(ENV + "\\Documents\\aggregate_exe.txt", std::ios::app | std::ios::binary);
		fileOverwriteA_exe.open(ENV + "\\Documents\\aggregate_exe.txt", std::ios::in | std::ios::out | std::ios::binary);
		fileAppendA_title.open(ENV + "\\Documents\\aggregate_title.txt", std::ios::app | std::ios::binary);
		fileOverwriteA_title.open(ENV + "\\Documents\\aggregate_title.txt", std::ios::in | std::ios::out | std::ios::binary);

		// get window exe name
		GetWindowThreadProcessId(GetForegroundWindow(), &identifier);
		len = GetModuleFileNameExW(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, identifier), NULL, currentTitle, 128);
		//if (len == 0 || std::strstr(currentTitle, "LockApp")) {
		//	continue;
		//}
		for (int i = len; i < sizeof(currentTitle) / 2 + 27; i++) { // apparently currentTitle is given an additional 27-character sequence at the end
			currentTitle[i] = L' ';
		}
		addToFile(fileOverwriteA_exe, fileAppendA_exe, std::wstring(currentTitle));

		// get window title name
		len = GetWindowTextW(GetForegroundWindow(), currentTitle, 128);
		for (int i = len; i < sizeof(currentTitle) / 2 + 27; i++) {
			currentTitle[i] = L' ';
		}
		addToFile(fileOverwriteA_title, fileAppendA_title, std::wstring(currentTitle));

		fileAppendD_exe.close();
		fileOverwriteD_exe.close();
		fileAppendD_title.close();
		fileOverwriteD_title.close();
		fileAppendA_exe.close();
		fileOverwriteA_exe.close();
		fileAppendA_title.close();
		fileOverwriteA_title.close();
		testfile.close();
	}

	// todo: implement unicode
	// todo: use functions to implement daily
	// todo: remove lockapp.exe? (if string contains LockApp)
	// todo: test accuracy of not using system time
	// todo: get exe to run in system tray
}
