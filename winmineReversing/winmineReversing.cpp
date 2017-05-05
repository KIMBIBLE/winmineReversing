#include <stdio.h>
#include <Windows.h>
#include <tlHelp32.h>
#include <windows.h>
#include <locale.h>
#include <wchar.h>

#define BUF_SIZE 1024
#define MAP_SIZE 0x360

const char *ProcessName = "winmine.exe";
const unsigned int MineMemoryAddress = 0x01005361;
const unsigned int XBoxMaxMemoryAddress = 0x01005334;
const unsigned int YBoxMaxMemoryAddress = 0x01005338;
const unsigned int MineMaxCntMemoryAddress = 0x010056A4;

const wchar_t * NO_MINE_SYMBOL = L"□";
const wchar_t * MINE_SYMBOL = L"♣";
const wchar_t * MINE_FLAG_SUCCESS_SYMBOL = L"●";
const wchar_t * MINE_FLAG_FAIL_SYMBOL = L"¶";
const wchar_t * MINE_ON_QUESTION_SYMBOL = L"★";
const wchar_t * NOT_MINE_ON_QUESTION_SYMBOL = L"☆";

const BYTE NO_MINE = 0x0F;
const BYTE MINE = 0x8F;
const BYTE MINE_FLAG_SUCCESS = 0x8E;
const BYTE MINE_FLAG_FAIL = 0x0E;
const BYTE MINE_ON_QUESTION = 0x8D;
const BYTE NOT_MINE_ON_QUESTION = 0x0D;

typedef struct {
	BYTE XBoxMax;
	BYTE YBoxMax;
	BYTE MineMaxCnt;
}MineMapInfo;


DWORD findProcessPID(LPCSTR processName);
void hackWinMine(const BYTE *map, const MineMapInfo pMapInfo);
void errorHandling(const char *message);
void ReadMapSubInfo(HANDLE pHandle, MineMapInfo *pMapInfo);

int main()
{
	DWORD PID;
	HANDLE pHandle;

	MineMapInfo mapInfo = { NULL, };
	BYTE map[MAP_SIZE] = { NULL, };

	PID = findProcessPID(ProcessName);
	if (!(pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)))
	{
		errorHandling("OpenProcess()");
	}

	while (1)
	{
		ReadMapSubInfo(pHandle, &mapInfo);
		if (!ReadProcessMemory(pHandle, (LPCVOID)MineMemoryAddress, map, MAP_SIZE, NULL))
		{
			errorHandling("ReadProcessMemory()");
		}
		printf("< 지뢰 찾기 정보>\n");
		printf("지뢰 맵(x좌표) : %d\n", mapInfo.XBoxMax);
		printf("지뢰 맵(y좌표) : %d\n", mapInfo.YBoxMax);
		printf("지뢰 갯수 : %d\n", mapInfo.MineMaxCnt);
		hackWinMine(map, mapInfo);
		Sleep(500);
		system("cls");
	}

}


DWORD findProcessPID(LPCSTR processName)
{
	BOOL bGet = FALSE;
	char buf[BUF_SIZE] = "";
	HANDLE hSnapshot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ppe; //엔트리
	DWORD dwPid = 0xFFFFFFFF;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // 시스템 프로세서
	ppe.dwSize = sizeof(PROCESSENTRY32);	// 엔트리 사이즈 지정

	bGet = Process32First(hSnapshot, &ppe);	// 엔트리 중 자료 가져옴.
	while (bGet)	//엔트리에 남아있는 자료가 없을 경우 break
	{
		WideCharToMultiByte(CP_ACP, 0, ppe.szExeFile, 260, buf, 260, NULL, NULL);

		if (!strcmp(processName, buf))
		{
			dwPid = ppe.th32ProcessID;
			break;
		}
		bGet = Process32Next(hSnapshot, &ppe);
	}
	CloseHandle(hSnapshot);

	return dwPid;
}

void hackWinMine(const BYTE *map, const MineMapInfo pMapInfo)
{
	int i;
	int cnt = 0;
	for (i = 0; i < MAP_SIZE; i++)
	{
		setlocale(LC_ALL, "");
		fwide(stdout, 1);


		if (map[i] == NO_MINE)
		{
			wprintf(L"%ls", NO_MINE_SYMBOL);
			cnt++;

		}
		else if (map[i] == MINE)
		{
			wprintf(L"%ls", MINE_SYMBOL);
			cnt++;

		}
		else if (map[i] == MINE_FLAG_SUCCESS) {
			wprintf(L"%ls", MINE_FLAG_SUCCESS_SYMBOL);
			cnt++;
		}
		else if (0x40 <= map[i] && map[i] <= 0x48)
		{
			printf("%2d", map[i] - 0x40);
			cnt++;
		}
		else if (map[i] == MINE_FLAG_FAIL)
		{
			wprintf(L"%ls", MINE_FLAG_FAIL_SYMBOL);
			cnt++;
		}
		else if (map[i] == MINE_ON_QUESTION)
		{
			wprintf(L"%ls", MINE_ON_QUESTION_SYMBOL);
			cnt++;
		}
		else if (map[i] == NOT_MINE_ON_QUESTION)
		{
			wprintf(L"%ls", NOT_MINE_ON_QUESTION_SYMBOL);
			cnt++;
		}
		else if (map[i] == 0x10)
		{
			while (map[++i] != 0x10) { ; }
			continue;
		}
		else { continue; }

		if (cnt % pMapInfo.XBoxMax == 0)
		{
			puts("");
		}

		if (cnt == pMapInfo.XBoxMax * pMapInfo.YBoxMax) { break; }


	}
}

void ReadMapSubInfo(HANDLE pHandle, MineMapInfo *pMapInfo)
{
	if (!ReadProcessMemory(pHandle, (LPCVOID)XBoxMaxMemoryAddress, &pMapInfo->XBoxMax, 4, NULL))
	{
		errorHandling("ReadProcessMemory() : XBoxMaxMemoryAddress");
	}

	if (!ReadProcessMemory(pHandle, (LPCVOID)YBoxMaxMemoryAddress, &pMapInfo->YBoxMax, 4, NULL))
	{
		errorHandling("ReadProcessMemory() : YBoxMaxMemoryAddress");
	}

	if (!ReadProcessMemory(pHandle, (LPCVOID)MineMaxCntMemoryAddress, &pMapInfo->MineMaxCnt, 4, NULL))
	{
		errorHandling("ReadProcessMemory() : MineMaxCntMemoryAddress");
	}
}

void errorHandling(const char *message)
{
	fprintf(stdout, "error occured in %s\n", message);
	exit(1);
}

