#pragma once

#include <string>
#include <vector>

#define PHONEBOOKCORE_API __declspec(dllexport)

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

enum Direction
{
	PREV,
	NEXT
};

PHONEBOOKCORE_API std::vector<PhonebookRecord> __cdecl GetPhonebook();

PHONEBOOKCORE_API BOOL Update(Direction direction, std::vector<PhonebookRecord>* result);

PHONEBOOKCORE_API BOOL Search(PhonebookRecord searchParam, std::vector<PhonebookRecord>* result);
