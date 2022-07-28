/*
Program to calculate simple statistics such as mean and standard deviation
*/

#include <windows.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

#define FILE_MENU_NEW 1
#define FILE_MENU_OPEN 2
#define FILE_MENU_CHANGETITLE 3
#define FILE_MENU_SUBMENU_ITEM 4
#define HELP_MENU 69
#define EXIT_MENU 70
#define BUTTON_POP 100
#define BUTTON_SAM 101

#if defined _DEBUG
boolean debug = true;
#else
boolean debug = false;
#endif

// give these a broad scope.  Global is the quick and dirty way to do it
HDC hdcMem = NULL;
HBITMAP hBmp = NULL;
HBITMAP hBmpOld = NULL;

struct data_sum { //summary of data
    double sum = 0;
    double sum2 = 0;
    double mean = 0;
};

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND);
void AddControls(HWND);

std::wstring convertToWString(std::ostringstream&);
struct data_sum findDataSum(double data[], int);

void DoSomething(HWND, HWND);
void PopStat(HWND, HWND);

void LoadPic(LPCWSTR);
void FreePic();

//Declaring handlers for our window objects
HMENU hMenu;
HWND hEdit;   //(type Edit) window for input
HWND hOutput; //(type Edit) window for output

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR args, _In_ int nCmdShow)
{
    WNDCLASSW wc = { 0 };

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;    //background color of window
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   //cursor type
    wc.hInstance = hInst;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"myWindowClass", L"My Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 625, 900, NULL, NULL, NULL, NULL);
    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (wp)
        {
        case FILE_MENU_NEW:
            MessageBeep(MB_ABORTRETRYIGNORE);
            break;
        case FILE_MENU_OPEN:
            MessageBeep(MB_RIGHT);
            break;
        case FILE_MENU_CHANGETITLE:
            wchar_t text[100];
            GetWindowTextW(hEdit, text, 100);
            SetWindowTextW(hWnd, text);        //changes title of window
            break;
        case FILE_MENU_SUBMENU_ITEM:
            break;
        case HELP_MENU:
            MessageBeep(MB_HELP);
            break;
        case EXIT_MENU:
            DestroyWindow(hWnd);
            break;
        case BUTTON_POP:
            LoadPic(L"./images/image1.bmp");  // call our LoadPic function to load the image
            InvalidateRect(hWnd, NULL, true);  // tell the window it needs to redraw (sends a WM_PAINT message)
            PopStat(hEdit, hOutput);
            break;
        case BUTTON_SAM:
            //DoSomething(hEdit, hOutput);
            LoadPic(NULL);  // call our LoadPic function to load the image
            InvalidateRect(hWnd, NULL, true);  // tell the window it needs to redraw (sends a WM_PAINT message)
            break;
        }
        break;
    case WM_CREATE:
        AddMenus(hWnd);
        AddControls(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC screen = BeginPaint(hWnd, &ps);
            StretchBlt(     /* fill in your coords and junk here */
                screen,         // destination DC
                650,         // x upper left
                50,          // y upper left
                200,//384,      // destination width
                83, //288,      // destination height
                hdcMem,      // you just created this above
                0,
                0,                      // x and y upper left
                400,                    // source bitmap width
                166,                    // source bitmap height
                SRCCOPY);       // raster operation
            EndPaint(hWnd, &ps);
        }
        break;
    default:
        return DefWindowProcW(hWnd, msg, wp, lp);
    }
}

void AddMenus(HWND hWnd)
{
    hMenu = CreateMenu();            //creates top-level menu
    HMENU hFileMenu = CreateMenu();  //sub-menu for file
    HMENU hSubMenu = CreateMenu();

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_NEW, L"New");
    AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_OPEN, L"Open");
    AppendMenu(hFileMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Submenu");
    AppendMenu(hSubMenu, MF_STRING, FILE_MENU_SUBMENU_ITEM, L"Submenu Item");
    AppendMenu(hFileMenu, MF_STRING, FILE_MENU_CHANGETITLE, L"Change Title");

    AppendMenu(hMenu, MF_STRING, HELP_MENU, L"Help");
    AppendMenu(hMenu, MF_STRING, EXIT_MENU, L"Exit");


    SetMenu(hWnd, hMenu);            //assigns menu to window handler
}

void AddControls(HWND hWnd)
{
    const wchar_t* sampleText = L"123,69. 420 666 88 HelloWorld8787.87870 .0101 111";

    CreateWindowW(L"Static", L"Input :", WS_VISIBLE | WS_CHILD | SS_CENTER, 25, 10, 550, 50, hWnd, NULL, NULL, NULL);
    hEdit = CreateWindowW(L"Edit", sampleText, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 25, 45, 550, 200, hWnd, NULL, NULL, NULL);

    CreateWindowW(L"Static", L"Calculate for:", WS_VISIBLE | WS_CHILD | SS_CENTER, 25, 260, 550, 50, hWnd, NULL, NULL, NULL);
    CreateWindowW(L"Button", L"Population", WS_VISIBLE | WS_CHILD | BS_DIBPATTERN8X8, 25, 290, 270, 50, hWnd, (HMENU)BUTTON_POP, NULL, NULL);
    CreateWindowW(L"Button", L"Sample", WS_VISIBLE | WS_CHILD | BS_DIBPATTERN8X8, 305, 290, 270, 50, hWnd, (HMENU)BUTTON_SAM, NULL, NULL);

    hOutput = CreateWindowW(L"Edit", L"Output", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL, 25, 360, 550, 200, hWnd, NULL, NULL, NULL);
}

std::wstring convertToWString(std::ostringstream& from)
{
    std::wstring to;
    std::string stdString = from.str();
    return to.assign(stdString.begin(), stdString.end());
}

struct data_sum findDataSum(double data[], int data_size) { //find data summary
    struct data_sum dt;

    //finding Σx and Σx_square
    for (int i = 0; i < data_size; i++) {
        dt.sum += data[i];
        dt.sum2 += data[i] * data[i];
    }

    dt.mean = dt.sum / (double)data_size;

    return dt;
}

void DoSomething(HWND hEdit, HWND hOutput) {
    wchar_t text_input[5000];
    GetWindowTextW(hEdit, text_input, 5000);

    std::wstring str_output(text_input);            //convert LPCWSTR (aka const wchar_t) to std::string
    
    ///Do stuff

    const wchar_t* text_output = str_output.c_str();  //convert std::string back to LPCWSTR

    SetWindowTextW(hOutput, text_output);     //changes the contents of Output window
}

void PopStat(HWND hEdit, HWND hOutput) {
    wchar_t * text_input = new wchar_t[5000];
    GetWindowTextW(hEdit, text_input, 5000);

    std::wstring str_output(text_input);        //convert LPCWSTR (aka const wchar_t) to std::string

    ///----------------------DO STUFF----------------------
    //first step: determine n and put data into an array
    wchar_t* prev_char = new wchar_t;
    wchar_t* curr_char = new wchar_t;
    *prev_char = 'X'; *curr_char = 'X';   //initialization 
    str_output += L"-end-";                   //padding str_output by one so that the end of the final data entry can be detected in the case the string ends with a digit
    int num_entries = 0; int max_dplaces = 1; int dplace = -1;

    ///FOR LOOP NO. 1 - finding out the number of entries
    for (int i = 0; i < str_output.length(); i++) {  
        *prev_char = *curr_char;
        *curr_char = str_output[i];

        if (iswdigit(*curr_char) == 0       // if curr_char is not a digit
            && iswdigit(*prev_char) != 0) { // and prev_char is a digit 
            if (*curr_char != '.') {            
                num_entries += 1;               //this denotes the end of a data entry, so num_entries +1
            }
            else if (iswdigit(str_output[i+1]) == 0) {    //check for forward 1 character, if its not a digit,
                str_output[i] = 'O';
                num_entries += 1;               //this denotes the end of a data entry, so num_entries +1
            }
        }
    }

    double* entries = new double[num_entries] {0};
    *prev_char = 'X'; *curr_char = 'X';   //re-initialization to clean-up leftover data from previous loop

    ///FOR LOOP NO. 2 - putting data entries into entries[] array
    for (int i = 0, n = 0; i < str_output.length() && n < num_entries; i++) {  
        *prev_char = *curr_char;
        *curr_char = str_output[i];

        if (iswdigit(*curr_char) == 0 && *curr_char != '.') {       // if curr_char is not a digit
            if (iswdigit(*prev_char) != 0) {                        // and prev_char is a digit 
                    n += 1; //this denotes the end of a data entry, so num_entries +1
                    dplace = 0;
                }
        }
        else {
            if (*curr_char == '.') {
                dplace = 1;
            }
            if (iswdigit(*prev_char) == 0 && *prev_char != '.' && *curr_char != '.') {
                entries[n] = (double)*curr_char - 48; //minus 48 is because of shift in ASCII table lookup when typecasting
            }
            else {
                if (dplace <= 0) {
                    entries[n] *= 10;
                    entries[n] += (double)*curr_char - 48; //minus 48 is because of shift in ASCII table lookup when typecasting
                }
                else {
                    if (*curr_char != '.') {
                        entries[n] += ((double)*curr_char - 48) / (pow(10, dplace)); //minus 48 is because of shift in ASCII table lookup when typecasting
                        dplace += 1;
                        if (dplace > max_dplaces) {
                            max_dplaces = dplace;
                        }
                    }
                }
            }
        }
    }

    /// Creating a Stream Object
    std::ostringstream* streamObj = new std::ostringstream;
    * streamObj << std::fixed;
    * streamObj << std::setprecision(max_dplaces-1);
    if (!debug) {
        str_output = L"";
    }
    str_output += L"\r\nprinting out contents of entry array: ";
    for (int i = 0; i < num_entries; i++) {
        //reset stream object
        (* streamObj).str("");
        * streamObj << entries[i];
        str_output += convertToWString(* streamObj);
        str_output += L",";
    }

    str_output += L"\r\n\r\nNumber of entries (n): ";
    str_output += std::to_wstring(num_entries);

    struct data_sum dt = findDataSum(entries, num_entries);

    str_output += L"\r\n\r\nMean: ";
    str_output += std::to_wstring(dt.mean);

    ///----------------------END OF DO STUFF SECTION----------------------

    const wchar_t * text_output = str_output.c_str();  //convert std::string back to LPCWSTR
    SetWindowTextW(hOutput, text_output);     //changes the contents of Output window

    delete streamObj;
    delete[] text_input;
    delete[] entries;
    delete prev_char, curr_char;
}

// call this function once in your WM_COMMAND handler, or wherever you want the image to be loaded
void LoadPic(LPCWSTR image_path)
{
    //if (hdcMem)
        //return;  // already loaded, no need to load it again
    if (hdcMem)
    {
        SelectObject(hdcMem, hBmpOld);
        DeleteObject(hBmp);
    }

      // note:  here you must put the file name in a TEXT() macro.  DO NOT CAST IT TO LPCSTR
    hBmp = (HBITMAP)LoadImage(NULL, image_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!hBmp)  // the load failed (couldn't find file?  Invalid file?)
        return;

    hdcMem = CreateCompatibleDC(NULL);
    hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmp);
}

// call this function when the program is done (shutting down)
void FreePic()
{
    if (hdcMem)
    {
        SelectObject(hdcMem, hBmpOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
    }
}
