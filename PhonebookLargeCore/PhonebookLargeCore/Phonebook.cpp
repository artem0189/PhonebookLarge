// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include <vector>
#include <stack>
#include "Phonebook.h"

#define MEMORY_NAME L"PhonebookLarge"

static HANDLE hMainFile = NULL, hSearchFile = NULL;
static HANDLE hMainMapObject = NULL, hSearchMapObject = NULL;
static LPVOID lpvMem = NULL;
static DWORD dwFileSize = 0;
static SYSTEM_INFO sysinfo = { 0 };
static BOOL isSearch = FALSE;

static std::vector<PhonebookRecord> phoneBook;
static std::stack<DWORD> offsets{ {0} };

std::wstring GetDirectory();
PhonebookRecord ParseLine(std::wstring line);
BYTE GetByte(DWORD offset);
VOID WriteSymbol(HANDLE hFile, wchar_t symbol);
wchar_t ReadSymbol(DWORD* offset);
BOOL ReadLine(DWORD* offset, std::wstring* result);
std::vector<PhonebookRecord> ReadPointer();
LPVOID SetFilePointer(HANDLE hMap, LPVOID prevFilePointer, DWORD offset);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        GetSystemInfo(&sysinfo);

        hSearchFile = CreateFile((GetDirectory() + L"/temp.txt").c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        hSearchMapObject = CreateFileMapping(hSearchFile, NULL, PAGE_READONLY, 0, 0, NULL);

        hMainFile = CreateFile((GetDirectory() + L"/db.txt").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        dwFileSize = GetFileSize(hMainFile, NULL);

        hMainMapObject = CreateFileMapping(hMainFile, NULL, PAGE_READONLY, 0, 0, MEMORY_NAME);
        lpvMem = SetFilePointer(hMainMapObject, NULL, 0);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        UnmapViewOfFile(lpvMem);
        CloseHandle(hSearchMapObject);
        CloseHandle(hMainMapObject);
        CloseHandle(hSearchFile);
        CloseHandle(hMainFile);
        break;
    }
    return TRUE;
}

PHONEBOOKCORE_API std::vector<PhonebookRecord> __cdecl GetPhonebook()
{
    phoneBook = ReadPointer();
    return phoneBook;
}

PHONEBOOKCORE_API BOOL Update(Direction direction, std::vector<PhonebookRecord>* result)
{
    switch (direction) {
    case PREV:
        if (offsets.size() > 2) {
            offsets.pop();
            offsets.pop();
        }
        else {
            return FALSE;
        }
        break;
    case NEXT:
        if (offsets.top() >= dwFileSize) {
            return FALSE;
        }
        break;
    }

    *result = ReadPointer();
    return TRUE;
}

PHONEBOOKCORE_API BOOL Search(PhonebookRecord searchParam, std::vector<PhonebookRecord>* result)
{
    dwFileSize = GetFileSize(hMainFile, NULL);
    while (offsets.top() != 0) {
        offsets.pop();
    }
    lpvMem = SetFilePointer(hMainMapObject, lpvMem, 0);

    char zeroBuffer[sizeof(PhonebookRecord)];
    ZeroMemory(zeroBuffer, sizeof(zeroBuffer));
    if (memcmp(&searchParam, zeroBuffer, sizeof(searchParam)) == 0) {
        if (isSearch) {
            isSearch = FALSE;
            *result = ReadPointer();
            return TRUE;
        }
        return FALSE;
    }

    CloseHandle(hSearchMapObject);
    CloseHandle(hSearchFile);
    hSearchFile = CreateFile((GetDirectory() + L"/temp.txt").c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    isSearch = FALSE;
    DWORD offset = 0;
    std::wstring line;
    while (offset < dwFileSize) {
        line.clear();
        if (ReadLine(&offset, &line)) {
            PhonebookRecord temp = ParseLine(line);
            if (!((std::wstring)searchParam.telephone).empty()) {
                if ((std::wstring)temp.telephone != (std::wstring)searchParam.telephone) {
                    continue;
                }
            }
            if (!((std::wstring)searchParam.lastName).empty()) {
                if ((std::wstring)temp.lastName != (std::wstring)searchParam.lastName) {
                    continue;
                }
            }
            if (!((std::wstring)searchParam.firstName).empty()) {
                if ((std::wstring)temp.firstName != (std::wstring)searchParam.firstName) {
                    continue;
                }
            }
            if (!((std::wstring)searchParam.patronymic).empty()) {
                if ((std::wstring)temp.patronymic != (std::wstring)searchParam.patronymic) {
                    continue;
                }
            }
            if (!((std::wstring)searchParam.streetName).empty()) {
                if ((std::wstring)temp.streetName != (std::wstring)searchParam.streetName) {
                    continue;
                }
            }
            if (searchParam.houseNumber != 0) {
                if (temp.houseNumber != searchParam.houseNumber) {
                    continue;
                }
            }
            if (searchParam.housingNumber != 0) {
                if (temp.housingNumber != searchParam.housingNumber) {
                    continue;
                }
            }
            if (searchParam.apartamentNumber != 0) {
                if (temp.apartamentNumber != searchParam.apartamentNumber) {
                    continue;
                }
            }
            line += L'\n';
            for (int i = 0; i < line.size(); i++) {
                WriteSymbol(hSearchFile, line[i]);
            }
        }
    }
    isSearch = TRUE;

    dwFileSize = GetFileSize(hSearchFile, NULL);
    if (dwFileSize != 0) {
        hSearchMapObject = CreateFileMapping(hSearchFile, NULL, PAGE_READONLY, 0, 0, NULL);
        lpvMem = SetFilePointer(hSearchMapObject, lpvMem, 0);
        *result = ReadPointer();
    }
    else {
        *result = {};
    }
    return TRUE;
}

std::wstring GetDirectory()
{
    TCHAR currentDirectory[256];
    GetCurrentDirectory(sizeof(currentDirectory), currentDirectory);
    return currentDirectory;
}

std::wstring GetFileName()
{
    TCHAR buffer[256], dir[256];
    GetTempPath(sizeof(dir), dir);
    GetTempFileName(dir, NULL, 0, buffer);
    return buffer;
}

PhonebookRecord ParseLine(std::wstring line)
{
    PhonebookRecord newRecord;
    ZeroMemory(&newRecord, sizeof(newRecord));
    std::vector<std::wstring> tokens;

    int pos = 0;
    std::wstring token;
    while ((pos = line.find(L"|")) != std::string::npos) {
        tokens.push_back((token = line.substr(0, pos)) != L"" ? token : NULL);
        line.erase(0, pos + 1);
    }
    std::copy(std::begin(tokens[0]), std::end(tokens[0]), std::begin(newRecord.telephone));
    std::copy(std::begin(tokens[1]), std::end(tokens[1]), std::begin(newRecord.lastName));
    std::copy(std::begin(tokens[2]), std::end(tokens[2]), std::begin(newRecord.firstName));
    std::copy(std::begin(tokens[3]), std::end(tokens[3]), std::begin(newRecord.patronymic));
    std::copy(std::begin(tokens[4]), std::end(tokens[4]), std::begin(newRecord.streetName));
    newRecord.houseNumber = (unsigned int)std::stoi(tokens[5]);
    newRecord.housingNumber = (unsigned int)std::stoi(tokens[6]);
    newRecord.apartamentNumber = (unsigned int)std::stoi(tokens[7]);

    return newRecord;
}

BYTE GetByte(DWORD offset)
{
    if (offset % 65536 == 0) {
        if (isSearch) {
            lpvMem = SetFilePointer(hSearchMapObject, lpvMem, offset);
        }
        else {
            lpvMem = SetFilePointer(hMainMapObject, lpvMem, offset);
        }
    }
    return *(BYTE*)((DWORD)lpvMem + (offset % 65536));
}

VOID WriteSymbol(HANDLE hFile, wchar_t symbol)
{
    if ((symbol >> 8) != 0) {
        WriteFile(hFile, &symbol, 2, NULL, NULL);
    }
    else {
        WriteFile(hFile, &symbol, 1, NULL, NULL);
    }
}

wchar_t ReadSymbol(DWORD* offset)
{
    short first, second;

    first = GetByte(*offset);
    *offset += 1;
    if ((first >> 6) == 3) {
        second = GetByte(*offset);
        *offset += 1;
        if ((second >> 6) == 2) {
            return ((first & 0x003F) << 6) + (second & 0x003F);
        }
    }
    return first;
}

BOOL ReadLine(DWORD* offset, std::wstring* result)
{
    wchar_t symbol;

    symbol = ReadSymbol(offset);
    while (symbol != L'\n') {
        *result += symbol;
        symbol = ReadSymbol(offset);
    }

    return TRUE; //
}

std::vector<PhonebookRecord> ReadPointer()
{
    DWORD offset = offsets.top();
    std::wstring line;

    std::vector<PhonebookRecord> result;
    for (int i = 0; i < 25; i++) {
        if (offset < dwFileSize)
        {
            if (ReadLine(&offset, &line)) {
                result.push_back(ParseLine(line));
            }
            line.clear();
        }
        else {
            break;
        }
    }
    offsets.push(offset);
    return result;
}

LPVOID SetFilePointer(HANDLE hMap, LPVOID prevFilePointer, DWORD offset)
{
    LPVOID result;
    if (prevFilePointer != NULL) {
        UnmapViewOfFile(prevFilePointer);
    }

    result = MapViewOfFile(hMap, FILE_MAP_READ, 0, offset, sysinfo.dwAllocationGranularity);
    if (result == NULL) {
        result = MapViewOfFile(hMap, FILE_MAP_READ, 0, offset, 0);
    }
    return result;
}
