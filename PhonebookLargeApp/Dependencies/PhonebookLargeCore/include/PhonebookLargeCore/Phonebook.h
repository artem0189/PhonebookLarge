#pragma once

#include <string>
#include <vector>

#ifdef PHONEBOOKCORE_EXPORTS
#define PHONEBOOKCORE_API __declspec(dllexport)
#else
#define PHONEBOOKCORE_API __declspec(dllimport)
#endif

const std::vector<std::wstring> columnsName{ TEXT("Telephone"), TEXT("LastName"), TEXT("FirstName"), TEXT("Patronymic"), TEXT("Street"), TEXT("House"), TEXT("Housing"), TEXT("Apartament") };

struct PhonebookRecord
{
	wchar_t telephone[20];
	wchar_t lastName[20];
	wchar_t firstName[20];
	wchar_t patronymic[20];
	wchar_t streetName[20];
	unsigned int houseNumber;
	unsigned int housingNumber;
	unsigned int apartamentNumber;
};

PHONEBOOKCORE_API std::vector<PhonebookRecord*> __cdecl GetPhonebook();

PHONEBOOKCORE_API std::vector<PhonebookRecord*> __cdecl Search(PhonebookRecord searchParam);

PHONEBOOKCORE_API VOID __cdecl ChangeData(PhonebookRecord* changedItem, int subItem, std::wstring changedData);
