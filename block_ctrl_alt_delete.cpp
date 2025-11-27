#include <windows.h>
#include <stdio.h>
#include <tchar.h> // _TCHAR, _tprintf 사용을 위해 포함

// 전역 훅 핸들
HHOOK g_keyboardHook = NULL;

// 훅 프로시저 (콜백 함수)
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // 1. nCode < 0 이면 이벤트를 처리하지 않고 다음 훅으로 전달 (필수 최적화)
    if (nCode < 0)
    {
        return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
    }

    // 키를 누르는 이벤트(WM_KEYDOWN 또는 WM_SYSKEYDOWN)만 처리
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
    {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

        // --- 차단할 키 조합 목록 ---
        
        // 1. Windows 키 (Win Key) 차단
        if (pKey->vkCode == VK_LWIN || pKey->vkCode == VK_RWIN)
        {
            printf("Windows Key Blocked! (VK: %lx)\n", pKey->vkCode);
            return 1; // 이벤트 차단
        }

        // 2. Alt+Tab, Alt+Esc 차단 (시스템 전환 키)
        // Alt 키(VK_MENU)가 눌린 상태에서 Tab/Esc가 눌렸는지 확인
        if (pKey->vkCode == VK_TAB && (GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            printf("Alt+Tab Blocked! (VK: %lx)\n", pKey->vkCode);
            return 1; // 이벤트 차단
        }

        if (pKey->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_MENU) & 0x8000))
        {
            printf("Alt+Esc Blocked! (VK: %lx)\n", pKey->vkCode);
            return 1; // 이벤트 차단
        }

        // 3. Ctrl+Esc (시작 메뉴) 차단
        // Ctrl 키(VK_CONTROL)가 눌린 상태에서 Esc가 눌렸는지 확인
        if (pKey->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
        {
            printf("Ctrl+Esc Blocked! (VK: %lx)\n", pKey->vkCode);
            return 1; // 이벤트 차단
        }
    }

    // 처리하지 않은 모든 이벤트는 다음 훅으로 전달
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

int main()
{
    // 콘솔 제목 설정 (프로그램 식별 용이)
    SetConsoleTitle(_T("Enhanced Keyboard Hook - Croc100"));

    // --- 훅 설정 ---
    g_keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL, // 로우 레벨 키보드 훅
        KeyboardProc,   // 콜백 함수
        NULL,           // LL 훅은 DLL 인스턴스가 필요 없음
        0               // 모든 스레드에 적용 (시스템 전체 훅)
    );

    if (g_keyboardHook == NULL)
    {
        DWORD dwError = GetLastError();
        // 에러 로깅: 훅 설정 실패 시 에러 코드 출력
        fprintf(stderr, "SetWindowsHookEx failed! Error Code: %lu\n", dwError);
        MessageBox(NULL, _T("Failed to set Keyboard Hook."), _T("Error"), MB_ICONERROR);
        return 1;
    }

    printf("--- Enhanced Keyboard Hook Active ---\n");
    printf("Monitoring and Blocking: Win Key, Alt+Tab, Alt+Esc, Ctrl+Esc.\n");
    printf("Press Ctrl+C in this console or close the window to stop.\n\n");
    
    // --- 메시지 루프 (프로그램 실행 상태 유지) ---
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // --- 훅 해제 (프로그램 종료 시 필수) ---
    UnhookWindowsHookEx(g_keyboardHook);
    printf("Keyboard Hook Successfully Unhooked.\n");

    return 0;
}

// 참고: 이 코드를 실행 파일로 만들려면 Visual Studio 등의 C++ 컴파일러가 필요합니다.
