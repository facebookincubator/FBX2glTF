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

#define SAVE_TO_BUTTON          1000
#define SAVE_TO_EDITBOX         1001
#define ADD_CUBE_BUTTON         1002
#define REMOVE_CUBES_BUTTON     1003
#define FBX_TREEVIEW            1004
#define TEXTURE_CHECKBOX        1005
#define ANIMATION_CHECKBOX      1006

// Global Variables:
HINSTANCE hInst;                        // current instance
HWND      ghWnd = NULL;                 // main window

TCHAR szTitle[MAX_LOADSTRING];          // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];    // the main window class name

char gszOutputFile[_MAX_PATH];           // File name to export
int  gWriteFileFormat = -1;             // Write file format

extern FbxManager *gSdkManager;     // access to the global SdkManager object
extern FbxScene      *gScene;          // access to the global scene object

extern FbxString* gAppPath;                // application path

// Forward declarations of functions included in this code module:
ATOM                UIRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void CreateUIControls(HWND hWndParent);
void GetOutputFileName(HWND hWndParent);

// used to check if in the filepath the file extention exist
bool ExtExist(const char * filepath, const char * ext);

// used to get the directory part of a filepath
FbxString GetDirectoryFromFilePath(const char *pFilePath);

// to save the file selected
void ExportFile();

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
void DisplayHierarchyRecurse(const FbxNode* pNode, HWND hTv, HTREEITEM htiParent);

// used to add FbxNode attributes parameters
void AddTreeViewItemKFbxNodeParameters(const FbxNode* pNode, HWND hTv, HTREEITEM htiParent);

// used to fill the treeview with the FBX scene content
void FillTreeView(const HWND mainHwnd);

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
    ZeroMemory(gszOutputFile,  sizeof(gszOutputFile)  ); 

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
        "FBX SDK CubeCreator sample program",   // LPCTSTR lpWindowName 
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

    // Get the path of the application (.exe module)
    char szFilePath[_MAX_PATH];
    if( GetModuleFileName(NULL, szFilePath, _MAX_PATH) )
    {
        // extract the directory only
		gAppPath = new FbxString();
        *gAppPath = GetDirectoryFromFilePath(szFilePath);
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
            // Create GUI controls
            CreateUIControls(hWnd);
        }
        break;

    case WM_SHOWWINDOW :
        {
            if(gSdkManager == NULL)
            {
                // create a basic scene
                CreateScene();

                // show the scene content in the treeview
                FillTreeView( hWnd );
            }
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

        case SAVE_TO_BUTTON:
            GetOutputFileName(hWnd);            // ask for file name
            ExportFile();                       // save the file
            break;

        case ADD_CUBE_BUTTON:
            {
                bool lWithTexture   = (1 == IsDlgButtonChecked( hWnd, TEXTURE_CHECKBOX    ));
                bool lWithAnimation = (1 == IsDlgButtonChecked( hWnd, ANIMATION_CHECKBOX  ));

                // create a new cube with option selected
                CreateCube(lWithTexture, lWithAnimation);

                // show scene content
                FillTreeView( hWnd );
            }
            break;

        case REMOVE_CUBES_BUTTON:

            // remove all cubes
            RemoveCubes();

            // show scene content
            FillTreeView( hWnd );
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:

        // dont forget to delete the SdkManager 
        // and all objects created by the SDK manager
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

    // create the <Save to> button
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Save to ...",                  // control caption
        dwStyle,                        // DWORD dwStyle
        10,                             // int x
        10,                             // int y
        130,                            // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) SAVE_TO_BUTTON,         // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // create the <Add cube> button
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Add cube",                     // control caption
        dwStyle,                        // DWORD dwStyle
        10,                             // int x
        710,                            // int y
        130,                            // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) ADD_CUBE_BUTTON,        // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // create the <Texture> checkbox
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Texture",                      // control caption
        dwStyle|BS_AUTOCHECKBOX,        // DWORD dwStyle
        150,                            // int x
        710,                            // int y
        80,                             // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) TEXTURE_CHECKBOX,       // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // set <Texture> checked by default
    CheckDlgButton(hWndParent, TEXTURE_CHECKBOX, 1);

    // create the <Animation> checkbox
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Animation",                    // control caption
        dwStyle|BS_AUTOCHECKBOX,        // DWORD dwStyle
        230,                            // int x
        710,                            // int y
        90,                             // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) ANIMATION_CHECKBOX,     // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // set <Animation> checked by default
    CheckDlgButton(hWndParent, ANIMATION_CHECKBOX, 1);


    // create the <Remove cube> button
    CreateWindowEx( 
        0,                              // DWORD dwExStyle,
        "BUTTON",                       // LPCTSTR lpClassName
        "Remove cubes",                 // control caption
        dwStyle,                        // DWORD dwStyle
        555,                            // int x
        710,                            // int y
        130,                            // int nWidth
        30,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) REMOVE_CUBES_BUTTON,    // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );

    // create the <Save to> edit box
    CreateWindowEx( 
        WS_EX_STATICEDGE,               // DWORD dwExStyle,
        "EDIT",                         // LPCTSTR lpClassName
        " <- select a file to save",    // control caption
        dwStyle|ES_AUTOHSCROLL,         // DWORD dwStyle
        150,                            // int x
        15,                             // int y
        534,                            // int nWidth
        20,                             // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) SAVE_TO_EDITBOX,        // HMENU hMenu or control's ID for WM_COMMAND 
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
        0,  		    // output quality
        0,                  // pitch and family
        "Arial"             // typeface name
        );

    // set the font for the SAVE_TO_EDITBOX
    SendMessage(GetDlgItem(hWndParent, SAVE_TO_EDITBOX), WM_SETFONT, (WPARAM) hf, (LPARAM) false );  

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
        646,                            // int nHeight    
        hWndParent,                     // HWND hWndParent
        (HMENU) FBX_TREEVIEW,           // HMENU hMenu or control's ID for WM_COMMAND 
        hInst,                          // HINSTANCE hInstance
        NULL                            // LPVOID lpParam
        );
}

// show the <Save file> dialog
void GetOutputFileName(
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
    ofn.nMaxFile        = sizeof(szFile)/ sizeof(*szFile); 
    ofn.nFilterIndex    = 1;      // *.fbx binairy by default
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle      = "Select the file to export to ... (use the file type filter)";
    ofn.Flags           = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

    // get a description of all writers registered in the FBX SDK
    const char *filter  = GetWriterSFNFilters();
    ofn.lpstrFilter     = filter;

    // Display the save as dialog box. 
    if(GetSaveFileName(&ofn) == false)
    {
        ZeroMemory(gszOutputFile,  sizeof(gszOutputFile)  ); 

        // User cancel ...
        delete filter;
        return;
    }

    delete filter;

    // keep the selected file format writer
    // ofn.nFilterIndex is not 0 based but start at 1, the FBX SDK file format enum start at 0
    gWriteFileFormat = ofn.nFilterIndex - 1; 

    // get the extention string from the file format selected by the user
    const char * ext = GetFileFormatExt( gWriteFileFormat );

    // check for file extention
    if(ExtExist(szFile, ext) == false)
    {
        // add the selected file extention
        FBXSDK_strcat(szFile, _MAX_PATH, ext);
    }

    delete ext;

    // show the file name selected with the extention
    SetWindowText( GetDlgItem(hWndParent, SAVE_TO_EDITBOX), szFile);

    // Keep a copy of the file name
    FBXSDK_strcpy(gszOutputFile, _MAX_PATH, szFile);
}

// to save the file selected
void ExportFile()
{
    if(strlen(gszOutputFile) == 0) return;

    Export(gszOutputFile, gWriteFileFormat);
}

// check if in the filepath the file extention exist
bool ExtExist(
              const char * filepath, 
              const char * ext
              )
{
    int iExtLen = (int)strlen(ext);
    int ifpLen  = (int)strlen(filepath);

    if( ifpLen < iExtLen) return false;

    int x = ifpLen - iExtLen;

    for(int i=0; i < iExtLen; i++)
    {
        if(filepath[x] != ext[i] ) return false;
        x++;
    }

    return true;
}

// extract the directory from a file path
FbxString GetDirectoryFromFilePath( const char *pFilePath )
{
    char   PathName[1024];
    char*  PathNamePtr;

    FBX_ASSERT( pFilePath );
    FBX_ASSERT( strlen( pFilePath ) < 1024 );

    FBXSDK_strcpy(PathName,1024,pFilePath);
    PathNamePtr=strrchr(PathName,'/');

    if (!PathNamePtr) {
        PathNamePtr=strrchr(PathName,'\\');
    }

    if (PathNamePtr) {
        (*PathNamePtr) = '\0';
    }
    else
    {
        PathName[0] = 0;
    }

    return FbxString(PathName);
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
        DisplayHierarchyRecurse(GetRootNode()->GetChild(i), hTv, htvi);
    }
}

// used to recursively add children nodes
void DisplayHierarchyRecurse(
                             const FbxNode* pNode, 
                             const HWND hTv, 
                             HTREEITEM htiParent
                             )
{
    // create a new Treeview item with node name and Attribute type name
    HTREEITEM htvi = InsertTreeViewItem(hTv, GetNodeNameAndAttributeTypeName(pNode).Buffer(), htiParent);

    // show some FbxNode parameters
    AddTreeViewItemKFbxNodeParameters(pNode, hTv, htvi);

    for(int i = 0; i < pNode->GetChildCount(); i++)
    {
        // recursively call this
        DisplayHierarchyRecurse(pNode->GetChild(i), hTv, htvi);
    }
}

// used to add FbxNode parameters
void AddTreeViewItemKFbxNodeParameters( 
                                       const FbxNode* pNode,
                                       HWND hTv, 
                                       HTREEITEM htiParent 
                                       )
{
    if(pNode == NULL) return;

    // show node default translation
    InsertTreeViewItem(hTv, GetDefaultTranslationInfo(pNode).Buffer(),  htiParent);

    // show if a material, texture or animation is in the node
    InsertTreeViewItem(hTv, GetNodeInfo(pNode).Buffer(),  htiParent);
}


// fill the treeview with the FBX scene content
void FillTreeView(
                  const HWND mainHwnd
                  )
{
    // get the handle of the treeview
    HWND htv = GetDlgItem(mainHwnd, FBX_TREEVIEW);
    if(htv == NULL) return;

    // clear the treeview content
    TreeView_DeleteAllItems(htv);

    // display scene hierarchy
    DisplayHierarchy(htv);

    // expand all items of the treeview
    Expand_All();

    // force the root node visible
    TreeView_SelectSetFirstVisible(htv,  TreeView_GetRoot(htv) );
}
