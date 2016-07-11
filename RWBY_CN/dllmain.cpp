#include "CPatch.h"
#include <windows.h>
#include <stdio.h>
#include <vector>

#define Max_Length 666

FILE *TheText = NULL;
int mono = 0;
char path[MAX_PATH];
int titleset = 0;
std::vector<char *> en;
std::vector<int> en_len;
std::vector<char *> zh;
std::vector<int> zh_len;
BOOL isNyaDebug = false;


void ChangeTextProcess(char **Text, int *size)
{
	if (!titleset && !strcmp(*Text, "English (United States)"))
	{
		SetWindowTextW(FindWindow(NULL, "RWBY: Grimm Eclipse"), L"RWBY: Â¾ÊÞÖ®Ê´");
		titleset = 1;
	}
	if (titleset)
	{
		int langt;
		for (unsigned int i = 0;i < en_len.size();i++)
		{
			if (*size == en_len[i])
			{
				langt = *size;
				if (langt > 16)
					langt = 16;
				if (!memcmp(*Text, en[i], langt))
				{
					*Text = zh[i];
					*size = zh_len[i];
					if (isNyaDebug)
						MessageBox(0, en[i], "Get!", 0);
					break;
				}
			}
		}
	}
}

void ChangeText(int ebp)
{
	ChangeTextProcess((char**)(ebp + 8), (int*)(ebp + 0xC));
}

__declspec(naked) void HOOK_DEBUG_1000135D()
{
	__asm
	{
		push ebp
		call ChangeText
		push ecx
		push ebx
		push esi
		mov esi, [ebp + 0x18]
		push mono
		retn
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (strstr(GetCommandLine(), " -NyaDebug"))
			{
				isNyaDebug = true;
				MessageBox(0,"Ready for Debug!","NyaPlus!",0);
			}
			if (GetModuleFileName(0, path, MAX_PATH))
			{
				for (int i = strlen(path) - 1; i >= 0; i--)
				{
					if (path[i] == '\\' || path[i] == '/')
					{
						path[i+1] = 0;
						strcat_s(path, "RWBY_TEXT.txt");
						if (fopen_s(&TheText, path, "r+"))
							break;
						fseek(TheText, 3, SEEK_SET);
						char *temptext = new char[Max_Length];
						char *temp2;
						int leng;
						BOOL zhongwen = false;
						while (!feof(TheText))
						{
							fgets(temptext, Max_Length, TheText);
							if (isNyaDebug)
								MessageBox(0, temptext, "From txt", 0);
							leng = strlen(temptext) - 1;
							if (temptext[leng] == '\n')
								temptext[leng] = 0;
							else
								leng++;
							temp2 = (char *)malloc(leng + 1);
							memcpy_s(temp2, leng + 1, temptext, leng + 1);
							if (zhongwen)
							{
								zh.push_back(temp2);
								zh_len.push_back(leng);
								zhongwen = false;
							}
							else
							{
								en.push_back(temp2);
								en_len.push_back(leng);
								zhongwen = true;
							}
						}
						delete[]temptext;
						fclose(TheText);
						if (!zhongwen)
						{
							mono = (int)GetModuleHandle("mono.dll");
							CPatch::RedirectJump(mono + 0x135D, &HOOK_DEBUG_1000135D);
							mono += 0x1363;
						}
						break;
					}
				}
			}
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			for (int i = 0;i <= zh.size();i++)
				free(zh[i]);
			zh.clear();
			zh_len.clear();
			for (int i = 0;i <= en.size();i++)
				free(en[i]);
			en.clear();
			en_len.clear();
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	
	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}

extern "C" void __stdcall RWBY(void)
{
	return;
}
