#include <windows.h>
#include <commctrl.h>
#include <vector>
#include "PhonebookLargeCore/Phonebook.h"

#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MY_COMMAND (WM_COMMAND + 4)
#define MY_NOTIFY (WM_NOTIFY + 4)
#define IDC_FILTER_EDIT 1000
#define IDC_BUTTON_FIND 2000
#define IDC_BUTTON_REFRESH 2001

CONST WCHAR MainClassName[] = TEXT("MainClass");
CONST WCHAR MainWindowName[] = TEXT("Window");

HWND hMainWindow, hListView;
LVHITTESTINFO htInfo;
std::vector<PhonebookRecord*> phoneBook;
PhonebookRecord searchRecord;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListViewProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK FilterEditProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK DynamicEditProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
HWND CreateListView(HWND hWndParent, std::vector<std::wstring> columsName);
VOID ClearListView(HWND hListView);
VOID InsertListViewItems(HWND hListView, std::vector<PhonebookRecord*> items);
VOID CreateFilter(HWND hWndParent, std::vector<std::wstring> filterName);
std::wstring GetText(HWND hEdit);

LRESULT CALLBACK DynamicEditProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_RETURN) {
			ChangeData(phoneBook[htInfo.iItem], htInfo.iSubItem, GetText(hEdit));
			DestroyWindow(hEdit);
		}
		break;
	}
	case WM_KILLFOCUS:
		DestroyWindow(hEdit);
		break;
	}
	return DefSubclassProc(hEdit, uMsg, wParam, lParam);
}

LRESULT CALLBACK FilterEditProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static WPARAM previousWparam;

	switch (uMsg) {
	case WM_KILLFOCUS:
	{
		std::wstring text = GetText(hEdit);
		switch (GetWindowLong(hEdit, GWL_ID)) {
		case IDC_FILTER_EDIT + 1:
			ZeroMemory(searchRecord.telephone, sizeof(searchRecord.telephone));
			std::copy(std::begin(text), std::end(text), std::begin(searchRecord.telephone));
			break;
		case IDC_FILTER_EDIT + 2:
			ZeroMemory(searchRecord.lastName, sizeof(searchRecord.lastName));
			std::copy(std::begin(text), std::end(text), std::begin(searchRecord.lastName));
			break;
		case IDC_FILTER_EDIT + 3:
			ZeroMemory(searchRecord.firstName, sizeof(searchRecord.firstName));
			std::copy(std::begin(text), std::end(text), std::begin(searchRecord.firstName));
			break;
		case IDC_FILTER_EDIT + 4:
			ZeroMemory(searchRecord.patronymic, sizeof(searchRecord.patronymic));
			std::copy(std::begin(text), std::end(text), std::begin(searchRecord.patronymic));
			break;
		case IDC_FILTER_EDIT + 5:
			ZeroMemory(searchRecord.streetName, sizeof(searchRecord.streetName));
			std::copy(std::begin(text), std::end(text), std::begin(searchRecord.streetName));
			break;
		case IDC_FILTER_EDIT + 6:
			try
			{
				searchRecord.houseNumber = std::stoi(text != TEXT("") ? text : TEXT("0"));
			}
			catch (...)
			{
				SetWindowText(hEdit, std::to_wstring(searchRecord.houseNumber).c_str());
			}
			break;
		case IDC_FILTER_EDIT + 7:
			try
			{
				searchRecord.housingNumber = std::stoi(text != TEXT("") ? text : TEXT("0"));
			}
			catch (...)
			{
				SetWindowText(hEdit, std::to_wstring(searchRecord.housingNumber).c_str());
			}
			break;
		case IDC_FILTER_EDIT + 8:
			try
			{
				searchRecord.apartamentNumber = std::stoi(text != TEXT("") ? text : TEXT("0"));
			}
			catch (...)
			{
				SetWindowText(hEdit, std::to_wstring(searchRecord.apartamentNumber).c_str());
			}
			break;
		}
	}
	}
	return DefSubclassProc(hEdit, uMsg, wParam, lParam);
}

LRESULT CALLBACK ListViewProc(HWND hListView, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	NMLVDISPINFO* plvdi;
	static std::wstring temp;

	switch (uMsg) {
	case MY_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case NM_DBLCLK:
		{
			HWND hEdit;
			RECT rcItem;

			htInfo.pt = ((LPNMITEMACTIVATE)lParam)->ptAction;
			ListView_SubItemHitTest(hListView, &htInfo);
			if (htInfo.iItem != -1) {
				ListView_GetSubItemRect(hListView, htInfo.iItem, htInfo.iSubItem, LVIR_LABEL, &rcItem);
				hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE, rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, hListView, NULL, GetModuleHandle(NULL), NULL);
				SetWindowSubclass(hEdit, DynamicEditProc, 2, 0);
				SetFocus(hEdit);
			}
			break;
		}
		case LVN_GETDISPINFO:
		{
			plvdi = (NMLVDISPINFO*)lParam;
			PhonebookRecord* addedItem = phoneBook[plvdi->item.iItem];
			switch (plvdi->item.iSubItem) {
			case 0:
				plvdi->item.pszText = const_cast<LPWSTR>(addedItem->telephone);
				break;
			case 1:
				plvdi->item.pszText = const_cast<LPWSTR>(addedItem->lastName);
				break;
			case 2:
				plvdi->item.pszText = const_cast<LPWSTR>(addedItem->firstName);
				break;
			case 3:
				plvdi->item.pszText = const_cast<LPWSTR>(addedItem->patronymic);
				break;
			case 4:
				plvdi->item.pszText = const_cast<LPWSTR>(addedItem->streetName);
				break;
			case 5:
				temp = std::to_wstring(addedItem->houseNumber);
				plvdi->item.pszText = const_cast<LPWSTR>(temp.c_str());
				break;
			case 6:
				temp = std::to_wstring(addedItem->housingNumber);
				plvdi->item.pszText = const_cast<LPWSTR>(temp.c_str());
				break;
			case 7:
				temp = std::to_wstring(addedItem->apartamentNumber);
				plvdi->item.pszText = const_cast<LPWSTR>(temp.c_str());
				break;
			}
			break;
		}
		break;
		}
	}
	return DefSubclassProc(hListView, uMsg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMLVDISPINFO* plvdi;

	if (uMsg == WM_CREATE) {
		hListView = CreateListView(hWnd, columnsName);
		InsertListViewItems(hListView, GetPhonebook());
		CreateFilter(hWnd, columnsName);
		ShowWindow(hListView, SW_SHOWDEFAULT);
	}

	switch (uMsg) {
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_BUTTON_FIND:
			{
				std::vector<PhonebookRecord*> search = Search(searchRecord);
				if (search != phoneBook) {
					ClearListView(hListView);
					InsertListViewItems(hListView, search);
				}
				break;
			}
			case IDC_BUTTON_REFRESH:
				ClearListView(hListView);
				InsertListViewItems(hListView, phoneBook);
				break;
			}
			break;
		default:
			SendMessage((HWND)lParam, MY_COMMAND, wParam, lParam);
			break;
		}
		break;
	case WM_NOTIFY:
		SendMessage(((NMHDR*)lParam)->hwndFrom, MY_NOTIFY, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return NULL;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MainClassName;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;

	RegisterClassEx(&wc);
	hMainWindow = CreateWindowEx(NULL, MainClassName, MainWindowName, WS_OVERLAPPED | WS_SYSMENU, 400, 100, 806, 600, NULL, NULL, hInstance, NULL);

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

HWND CreateListView(HWND hWndParent, std::vector<std::wstring> columsName)
{
	HWND hWndListView;
	RECT rcClient, rcListView;
	LVCOLUMN lvc;

	GetClientRect(hWndParent, &rcClient);
	hListView = CreateWindowEx(NULL, WC_LISTVIEW, TEXT(""), WS_CHILD | LVS_REPORT, 0, 0, rcClient.right - rcClient.left - 150, rcClient.bottom - rcClient.top, hWndParent, NULL, GetModuleHandle(NULL), NULL);
	SetWindowSubclass(hListView, ListViewProc, 0, 0);

	GetClientRect(hListView, &rcListView);
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = (rcListView.right - rcListView.left) / columnsName.size();
	for (int i = 0; i < columnsName.size(); i++) {
		lvc.pszText = const_cast<LPWSTR>(columnsName[i].c_str());
		ListView_InsertColumn(hListView, i, &lvc);
	}

	return hListView;
}

VOID ClearListView(HWND hListView)
{
	ListView_DeleteAllItems(hListView);
}

VOID InsertListViewItems(HWND hListView, std::vector<PhonebookRecord*> items)
{
	LVITEM lvI;

	lvI.mask = LVIF_TEXT;
	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.iSubItem = 0;

	phoneBook = items;
	for (int i = 0; i < phoneBook.size(); i++) {
		lvI.iItem = i;
		ListView_InsertItem(hListView, &lvI);
	}
}

VOID CreateFilter(HWND hWndParent, std::vector<std::wstring> filterName)
{
	HWND hEdit, hButton;
	RECT rcClient;
	GetClientRect(hWndParent, &rcClient);

	for (int i = 0; i < filterName.size(); i++) {
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE, rcClient.right - 130, 40 * (i + 1), 110, 20, hWndParent, (HMENU)(IDC_FILTER_EDIT + i + 1), GetModuleHandle(NULL), NULL);
		SetWindowSubclass(hEdit, FilterEditProc, 1, 0);
		Edit_SetCueBannerText(hEdit, filterName[i].c_str());
	}
	CreateWindowEx(WS_EX_CLIENTEDGE, WC_BUTTON, TEXT("Find"), WS_CHILD | WS_VISIBLE, rcClient.right - 130, 40 * (filterName.size() + 1), 110, 20, hWndParent, (HMENU)IDC_BUTTON_FIND, GetModuleHandle(NULL), NULL);
	CreateWindowEx(WS_EX_CLIENTEDGE, WC_BUTTON, TEXT("Refresh"), WS_CHILD | WS_VISIBLE, rcClient.right - 130, 40 * (filterName.size() + 1) + 30, 110, 20, hWndParent, (HMENU)IDC_BUTTON_REFRESH, GetModuleHandle(NULL), NULL);
}

std::wstring GetText(HWND hEdit)
{
	WCHAR buffer[512];
	GetWindowText(hEdit, buffer, sizeof(buffer));
	return buffer;
}