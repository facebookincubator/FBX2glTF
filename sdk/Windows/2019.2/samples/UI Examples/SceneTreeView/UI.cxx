/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

// UI.cxx : Defines the entry point for the application.
#include "stdafx.h"
#include <commctrl.h> // for treeview control

#include "UI.h"
#include "SDK_Utility.h"

#define MAX_LOADSTRING 100

#define READ_FROM_BUTTON        1000
#define READ_FROM_EDITBOX       1001
#define FBX_TREEVIEW            1002

// Global Variables:
HINSTANCE hInst;                        // current instance
HWND      ghWnd = NULL;                 // main window

TCHAR szTitle[MAX_LOADSTRING];          // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];    // the main window class name

char gszInputFile[_MAX_PATH];            // File name to import

extern FbxManager *gSdkManager;     // access to the global SdkManager object
extern FbxScene      *gScene;          // access to the global scene object

// Forward declarations of functions included in this code module:
ATOM                UIRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void CreateUIControls(HWND hWndParent);
void GetInputFileName(HWND hWndParent);

//-----------------------------------------------------------
// treview utility
// used to add a new treeview item
HTREEITEM InsertTreeViewItem(const HWND hTv, const char *txt, HTREEITEM htiParent);

// used to expand all treeview items
void Expand_All();
void Expand_All_Recurse(HWND htv, HTREEITEM htvi);

//-----------------------------------------------------------
// scene nodes utility
// used to display the nodes hierarchy
void DisplayHierarchy(const HWND hTv);
void DisplayHierarchy_Recurse(const FbxNode* pNode, HWND hTv, HTREEITEM htiParent);

// used to add FbxNode attributes parameters
void Add_TreeViewItem_KFbxNode_Parameters(const FbxNode* pNode, HWND hTv, HTREEITEM htiParent);

// used to fill the treeview with the FBX scene content
void Fill_TreeView(const HWND mainHwnd);

static bool gAutoQuit = false;

//----------------------------------------------------------
// entry point for the application
int APIENTRY _tWinMain(
                       HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow
                       )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    if( FbxString(lpCmdLine) == "-test" ) gAutoQuit = true;

    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_UI, szWindowClass, MAX_LOADSTRING);
    UIRegisterClass(hInstance);

    // empty our global file name buffer
    ZeroMemory(gszInputFile,  sizeof(gszInputFile)  ); 

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UI));

    // Main message loop:
    while( GetMessage(&msg, NULL, 0, 0) && !gAutoQuit )
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: UIRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM UIRegisterClass(
                     HINSTANCE hInstance
                     )
{
    WNDCLASSEX wcex;

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UI));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush( GetSysColor(COLOR_3DFACE) );
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_UI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_UI)); 

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//   In this function, we save the instance handle in a global variable,
//   create and display the main program window.
//
BOOL InitInstance(
                  HINSTANCE hInstance, 
                  int nCmdShow
                  )
{
    hInst = hInstance; // Store instance handle in our global variable

    ghWnd = CreateWindow(
        szWindowClass,                          // LPCTSTR lpClassName
        "FBX SDK SceneTreeView sample program", // LPCTSTR lpWindowName 
        WS_OVERLAPPED | WS_SYSMENU,             // DWORD dwStyle
        400,                                    // int x
        200,                                    // int Y
        700,                                    // int nWidth
        800,                                    // int nHeight
        NULL,                                   // HWND hWndParent
        NULL,                                   // HMENU hMenu
        hInstance,                              // HINSTANCE hInstance
        NULL                                    // LPVOID lpParam
        );

    if (!ghWnd)
    {
        return FALSE;
    }

    ShowWindow(ghWnd, nCmdShow);
    UpdateWindow(ghWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(
                         HWND hWnd, 
                         UINT message, 
                         WPARAM wParam, 
                         LPARAM lParam
                         )
{
    int wmId, wmEvent;

    switch (message)
    {

    case WM_CREATE:
        {
            CreateUIControls(hWnd);

			InitializeSdkManagerAndScene();
        }
        break;

    case WM_COMMAND:

        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case READ_FROM_BUTTON:
            GetInputFileName(hWnd); // ask user for a FBX file path
            Fill_TreeView(hWnd);    // fill the treeview with the scene content
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:

		DestroySdkObjects(gSdkManager, true);

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(
                       HWND hDlg, 
                       UINT message, 
                       WPARAM wParam, 
                       LPARAM lParam
                       )
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        {
            // set the FBX app. icon
	        HINSTANCE hinst = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);

            HICON hIconFBX = (HICON) LoadImage( hinst, MAKEINTRESOURCE(IDI_UI), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE); 
            ::SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIconFBX); 
        }

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Create the UI children controls
void CreateUIControls(
                      HWND hWndParent
                      )
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE;

    // create the <Read from> button
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Read from ...",                // control caption
        dwStyle,                        // DWORD dwStyle
        10,                             // int x
        10,                             // int y
        130,                            // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) READ_FROM_BUTTON,       // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // create the <Read from> edit box
    CreateWindowEx( 
        WS_EX_STATICEDGE,               // DWORD dwExStyle,
        "EDIT",                         // LPCTSTR lpClassName
        " <- select a file to read",    // control caption
        dwStyle|ES_AUTOHSCROLL,         // DWORD dwStyle
        150,                            // int x
        15,                             // int y
        534,                            // int nWidth
        20,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) READ_FROM_EDITBOX,      // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // create a small font 
    HFONT hf = CreateFont(
        14,                 // height of font
        6,                  // average character width
        0,                  // angle of escapement
        0,                  // base-line orientation angle
        40,                 // font weight
        0,                  // italic attribute option
        0,                  // underline attribute option
        0,                  // strikeout attribute option
        ANSI_CHARSET,       // character set identifier
        OUT_DEFAULT_PRECIS, // output precision
        CLIP_DEFAULT_PRECIS,// clipping precision
        0,					// output quality
        0,                  // pitch and family
        "Arial"             // typeface name
        );

    // set the font for the READ_FROM_EDITBOX
    SendMessage(GetDlgItem(hWndParent, READ_FROM_EDITBOX), WM_SETFONT, (WPARAM) hf, (LPARAM) false );  


    // create the <Tree view> control
    dwStyle = dwStyle | TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS;

    CreateWindowEx( 
        WS_EX_STATICEDGE,               // DWORD dwExStyle,
        WC_TREEVIEW,                    // LPCTSTR lpClassName
        "",                             // control caption
        dwStyle,                        // DWORD dwStyle
        10,                             // int x
        50,                             // int y
        674,                            // int nWidth
        686,                            // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) FBX_TREEVIEW,           // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );
}

// show the <Open file> dialog
void GetInputFileName(
                      HWND hWndParent
                      )
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char szFile[_MAX_PATH];  // buffer for file name
    ZeroMemory(szFile, sizeof(szFile));

    // Initialize OPENFILENAME
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hWndParent;
    ofn.lpstrFile       = szFile;
    ofn.nMaxFile        = sizeof(szFile);
    ofn.nFilterIndex    = 1;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle      = "Select the file to read ... (use the file type filter)";
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // get a description of all readers registered in the FBX SDK
    const char *filter  = GetReaderOFNFilters(); 
    ofn.lpstrFilter     = filter;

    // Display the Open dialog box. 
    if(GetOpenFileName(&ofn) == false)
    {
        // user cancel
        delete filter;
        return;
    }

    delete filter;

    // show the file name selected
    SetWindowText( GetDlgItem(hWndParent, READ_FROM_EDITBOX),  szFile );

    // Keep a copy of the file name
    FBXSDK_strcpy(gszInputFile, _MAX_PATH, szFile);
}

// used to add a new treeview item
HTREEITEM InsertTreeViewItem(
                             const HWND hTv, 
                             const char *txt, 
                             HTREEITEM htiParent
                             )
{
    TVITEM tvi      = {0};
    tvi.mask        = TVIF_TEXT|TVIF_PARAM;           // text + param only
    tvi.pszText     = (LPSTR)txt;                     // caption
    tvi.cchTextMax  = static_cast<int>(strlen(txt));  // length of item label

    TVINSERTSTRUCT tvis = {0};
    tvis.item           = tvi;
    tvis.hInsertAfter   = 0;
    tvis.hParent        = htiParent;                  // parent item of item to be inserted

    return reinterpret_cast<HTREEITEM>( SendMessage(hTv, TVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&tvis)) );
}

// used to expand all treeview nodes
void Expand_All()
{
    // get the handle of the treeview
    HWND htv = GetDlgItem(ghWnd, FBX_TREEVIEW);
    if(htv == NULL) return;

    Expand_All_Recurse(htv, TreeView_GetRoot(htv));

    // force the root node visible on expand
    TreeView_SelectSetFirstVisible(htv, TreeView_GetRoot(htv) );
}

// used to expand all treeview items recursively
void Expand_All_Recurse(
                        HWND htv, 
                        HTREEITEM htvi
                        )
{
    if(htvi == NULL) return;

    TreeView_Expand(htv, htvi, TVE_EXPAND);

    while(htvi)
    {
        // expand all children
        htvi = TreeView_GetChild(htv, htvi);
        Expand_All_Recurse(htv, htvi);

        // expand all siblings
        while(htvi)
        {
            htvi = TreeView_GetNextSibling(htv, htvi);
            Expand_All_Recurse(htv, htvi);
        }
    }
}

// used to add the rootNode name and start to add children nodes
void DisplayHierarchy(
                      const HWND hTv
                      )
{
    HTREEITEM htvi = InsertTreeViewItem(hTv, GetRootNodeName(), TVI_ROOT);

    for(int i = 0; i < GetRootNode()->GetChildCount(); i++)
    {
        DisplayHierarchy_Recurse(GetRootNode()->GetChild(i), hTv, htvi);
    }
}

// used to recursively add children nodes
void DisplayHierarchy_Recurse(
                              const FbxNode* pNode, 
                              const HWND hTv, 
                              HTREEITEM htiParent
                              )
{
    // create a new Treeview item with node name and Attribute type name
    HTREEITEM htvi = InsertTreeViewItem(hTv, GetNodeNameAndAttributeTypeName(pNode).Buffer(), htiParent);

    // show some FbxNode parameters
    Add_TreeViewItem_KFbxNode_Parameters(pNode, hTv, htvi);

    for(int i = 0; i < pNode->GetChildCount(); i++)
    {
        // recursively call this
        DisplayHierarchy_Recurse(pNode->GetChild(i), hTv, htvi);
    }
}

// used to add FbxNode parameters
void Add_TreeViewItem_KFbxNode_Parameters( 
    const FbxNode* pNode,
    HWND hTv, 
    HTREEITEM htiParent 
    )
{
    if(pNode == NULL) return;

    // show node default translation
    InsertTreeViewItem(hTv, GetDefaultTranslationInfo(pNode).Buffer(),  htiParent);

    // show node visibility
    InsertTreeViewItem(hTv, GetNodeVisibility(pNode).Buffer(),  htiParent);
}


// fill the treeview with the FBX scene content
void Fill_TreeView(
                   const HWND mainHwnd
                   )
{
    if(strlen(gszInputFile) == 0) return;

    // get the handle of the treeview
    HWND htv = GetDlgItem(mainHwnd, FBX_TREEVIEW);
    if(htv == NULL) return;

    // clear the treeview content
    TreeView_DeleteAllItems(htv);

    // show a wait cursor
    HCURSOR oldCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );

    // load the FBX scene
    if(LoadFBXScene(gszInputFile) == false)
    {
        // reset to default cursor
        SetCursor(oldCursor);

        return;
    }

    // display scene hierarchy
    DisplayHierarchy(htv);

    // expand all items of the treeview
    Expand_All();

    // force the root node visible
    TreeView_SelectSetFirstVisible(htv,  TreeView_GetRoot(htv) );

    // reset to default cursor
    SetCursor(oldCursor);
}
