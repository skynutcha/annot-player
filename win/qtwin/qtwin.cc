// qtwin.cc
// 7/21/2011

#include "qtwin.h"
#include <QtCore>

#ifndef Q_WS_WIN
  #error "This file is for windows only."
#endif // Q_WS_WIN

#include <qt_windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
//#include <strsafe.h>
#include <memory>
#include <string>
#include <cstdlib>

#ifdef _MSC_VER
  #pragma warning (disable: 4996)     // C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#endif // _MSC_VER

// - Helpers -

#define MAKEDWORD(_a, _b)       ((DWORD)(((WORD)(((DWORD_PTR)(_a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(_b)) & 0xffff))) << 16))

namespace { // anonymous

  // Conversions between qt and win32 data structures

  inline QPoint POINT2QPoint(const POINT &pt)
  { return QPoint((int)pt.x, (int)pt.y); }

  inline POINT QPoint2POINT(const QPoint &pos)
  { POINT ret = { pos.x(), pos.y() }; return ret; }

  inline QRect RECT2QRect(const RECT &rect)
  { return QRect((int)rect.left, (int)rect.top, int(rect.right - rect.left), int(rect.bottom - rect.top)); } // QRect(x, y, w, h)

  inline RECT QRect2RECT(const QRect &r)
  { RECT ret = { r.x(), r.y(), r.right(), r.bottom() }; return ret; } // RECT { x1, y1, x2, y2 }

  // Return ached OS version info
  // See: http://msdn.microsoft.com/en-us/library/ms724451(v=VS.85).aspx
  LPOSVERSIONINFO lposvi_()
  {
    static std::auto_ptr<OSVERSIONINFO> autoReleasePool;
    LPOSVERSIONINFO ret = autoReleasePool.get();
    if (!ret) {
      ret = new OSVERSIONINFO;
      ::ZeroMemory(ret, sizeof(*ret));
      ret->dwOSVersionInfoSize = sizeof(*ret);
      ::GetVersionEx(ret);

      autoReleasePool.reset(ret);
    }
    Q_ASSERT(ret);
    return ret;
  }

} // anonymous namespace

// - Maintence -

void
QtWin::warmUp()
{
  // Load cached functions
  isWindowsXpOrLater();
  isWindowsVistaOrLater();
}


// - Threads and processes -

bool
QtWin::isProcessActiveWithId(DWORD dwProcessId)
{
  bool ret = false;
  HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
  if (hProcess) {
    DWORD dwExitCode;
    ret = ::GetExitCodeProcess(hProcess, &dwExitCode) && (dwExitCode == STILL_ACTIVE);
    ::CloseHandle(hProcess);
  }
  return ret;
}

// See: http://hackdiy.com/I-11408248.html
// Requiring Psapi.lib.
QList<DWORD>
QtWin::getThreadIdsByProcessId(DWORD dwOwnerPID)
{
  QList<DWORD> ret;

  // Take a snapshot of all threads currently in the system.
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  THREADENTRY32 te32 = { };
  te32.dwSize = sizeof(THREADENTRY32); // Fill in the size of the structure before using it.

  // Walk   the   hread snapshot to find all threads of the process.
  // If the thread belongs to the process, add its information
  // to the display list.
  if (::Thread32First(hSnapshot, &te32)) {
    do {
      if (te32.th32OwnerProcessID == dwOwnerPID)
        // te32.th32ThreadID          TID
        // te32.th32OwnerProcessID    Owner PID
        // te32.tpDeltaPri            Delta Priority
        // te32.tpBasePri             Base Priority
        ret.push_back(te32.th32ThreadID);
     } while (::Thread32Next(hSnapshot, &te32));
  } else // could not walk the list of threads
    // Do not forget to clean up the snapshot object.
    ::CloseHandle(hSnapshot);
  return ret;
}

QList<DWORD>
QtWin::getProcessIdsByParentProcessId(DWORD dwOwnerPID)
{
  QList<DWORD> ret;
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  PROCESSENTRY32 pe = { };
  pe.dwSize = sizeof(pe); // Fill in the size of the structure before using it.

  if (::Process32First(hSnapshot, &pe)) {
    do {
      HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
      if(hProcess && pe.th32ParentProcessID == dwOwnerPID)
         ret.push_back(pe.th32ProcessID);
      ::CloseHandle(hProcess);
    } while(::Process32Next(hSnapshot, &pe));
  } else
    ::CloseHandle(hSnapshot);
  return ret;
}

QList<ulong>
QtWin::getProcessIdsByName(const QString &processName)
{
  QList<ulong> ret;
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  PROCESSENTRY32 pe = { };
  pe.dwSize = sizeof(pe); // Fill in the size of the structure before using it.

  if (::Process32First(hSnapshot, &pe)) {
    do {
      HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
      if (hProcess &&
          processName == QString::fromWCharArray(pe.szExeFile))
        ret.append(pe.th32ProcessID);

      ::CloseHandle(hProcess);
    } while(::Process32Next(hSnapshot, &pe));
  } else
    ::CloseHandle(hSnapshot);
  return ret;
}

ulong
QtWin::getProcessIdByName(const QString &processName)
{
  DWORD ret = 0; // either dwProcessId or 0
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  PROCESSENTRY32 pe = { };
  pe.dwSize = sizeof(pe); // Fill in the size of the structure before using it.

  if (::Process32First(hSnapshot, &pe)) {
    do {
      HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
      if (hProcess &&
          processName == QString::fromWCharArray(pe.szExeFile))
        ret = pe.th32ProcessID;

      ::CloseHandle(hProcess);
    } while(!ret && ::Process32Next(hSnapshot, &pe));
  } else
    ::CloseHandle(hSnapshot);
  return ret;
}

QList<QtWin::ProcessInfo>
QtWin::getProcessesInfo()
{
  QList<ProcessInfo> ret;
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  PROCESSENTRY32 pe = { };
  pe.dwSize = sizeof(pe); // Fill in the size of the structure before using it.

  if (::Process32First(hSnapshot, &pe)) {
    do {
      HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
      if (hProcess) {
        ProcessInfo pi;
        pi.id = pe.th32ProcessID;
        pi.name = QString::fromWCharArray(pe.szExeFile);
        ret.push_back(pi);
      }
      ::CloseHandle(hProcess);
    } while(::Process32Next(hSnapshot, &pe));
  } else
    ::CloseHandle(hSnapshot);
  return ret;
}

QString
QtWin::getProcessPathById(DWORD dwProcessId)
{
  QString ret;
  TCHAR szFilePath[MAX_PATH];
  HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId) ;
  if (hProcess) {
    int len = ::GetModuleFileNameEx(hProcess, NULL, szFilePath, MAX_PATH);
    if (len)
      ret = QString::fromWCharArray(szFilePath, len);
    ::CloseHandle(hProcess);
  }
  return ret;
}

QList<QtWin::ModuleInfo>
QtWin::getModulesInfo()
{
  QList<ModuleInfo> ret;
  HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return ret;

  MODULEENTRY32 me = { };
  me.dwSize = sizeof(me); // Fill in the size of the structure before using it.

  if (::Module32First(hSnapshot, &me)) {
    do {
      ModuleInfo mi;
      mi.moduleId = me.th32ModuleID;
      mi.processId = me.th32ProcessID;
      mi.moduleName = QString::fromWCharArray(me.szModule);
      mi.modulePath = QString::fromWCharArray(me.szExePath);
      ret.push_back(mi);
    } while(::Module32Next(hSnapshot, &me));
  } else
    ::CloseHandle(hSnapshot);
  return ret;
}

bool
QtWin::createProcessWithExecutablePath(const QString &filePath)
{
  std::wstring wszExePath = filePath.toStdWString();

  QString fileDir = QFileInfo(filePath).absolutePath();
  std::wstring wszExeDir = fileDir.toStdWString();

  STARTUPINFOW siStartupInfo;
  ::memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  PROCESS_INFORMATION piProcessInfo;
  ::memset(&piProcessInfo, 0, sizeof(piProcessInfo));

  LPCWSTR lpAppName = wszExePath.c_str();
  LPWSTR pwszParam = 0;
  LPVOID lpEnvironment = 0;
  LPCWSTR lpCurrentDirectory = wszExeDir.c_str();
  // See: http://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
  //
  // BOOL WINAPI CreateProcess(
  //   __in_opt     LPCTSTR lpApplicationName,
  //   __inout_opt  LPTSTR lpCommandLine,
  //   __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
  //   __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
  //   __in         BOOL bInheritHandles,
  //   __in         DWORD dwCreationFlags,
  //   __in_opt     LPVOID lpEnvironment,
  //   __in_opt     LPCTSTR lpCurrentDirectory,
  //   __in         LPSTARTUPINFO lpStartupInfo,
  //   __out        LPPROCESS_INFORMATION lpProcessInformation
  // );
  //
  BOOL bResult = ::CreateProcessW(
    lpAppName,          // app path
    pwszParam,          // app param
    0, 0,               // security attributes
    false,              // inherited
    CREATE_DEFAULT_ERROR_MODE, // creation flags
    lpEnvironment,
    lpCurrentDirectory,
    &siStartupInfo,
    &piProcessInfo
  );

  return bResult;
}

// - Windows -

bool
QtWin::setFocus(QWidget *w)
{
  if (!w)
    return false;

  bool succeed = ::SetFocus(w->winId());
  w->setFocus();
  return succeed;
}

bool
QtWin::setFocus(HWND hwnd)
{ return hwnd && ::SetFocus(hwnd); }

HWND
QtWin::getChildWindow(HWND hwnd)
{
  if (!hwnd)
    return 0;
  HWND child = ::GetWindow(hwnd, GW_CHILD);
  return child && ::IsWindow(child) ? child : 0;
}

QList<HWND>
QtWin::getChildWindows(HWND hwnd)
{
  QList<HWND> ret;
  HWND h = hwnd;
  if (h)
    while (h = QtWin::getChildWindow(h))
      ret.append(h);
  return ret;
}

HWND
QtWin::getWindowAtPos(const QPoint &globalPos)
{ return ::WindowFromPoint(::QPoint2POINT(globalPos)); }

HWND
QtWin::getChildWindowAtPos(const QPoint &globalPos, HWND parent)
{ return ::ChildWindowFromPoint(parent, ::QPoint2POINT(globalPos)); }

QRect
QtWin::getWindowRect(HWND hwnd)
{
  QRect ret;
  RECT rect;
  if (::GetWindowRect(hwnd, &rect))
    ret = ::RECT2QRect(rect);
  return ret;
}

bool
QtWin::isValidWindow(HWND hwnd)
{ return ::IsWindow(hwnd); }

DWORD
QtWin::getWindowThreadId(HWND hwnd)
{ return ::GetWindowThreadProcessId(hwnd, 0); }

DWORD
QtWin::getWindowProcessId(HWND hwnd)
{
  DWORD dwProcessId;
  ::GetWindowThreadProcessId(hwnd, &dwProcessId);
  return dwProcessId;
}

// - Mouse and keyboard -

// See mouse_event: http://msdn.microsoft.com/en-us/library/ms646260(v=vs.85).aspx
void
QtWin::sendMouseClick(const QPoint& globalPos, Qt::MouseButton button)
{
  switch (button) {
  case Qt::LeftButton:
    ::mouse_event(MOUSEEVENTF_LEFTDOWN,       globalPos.x(), globalPos.y(), 0, 0);
    ::mouse_event(MOUSEEVENTF_LEFTUP,         globalPos.x(), globalPos.y(), 0, 0);
    break;
  case Qt::RightButton:
    ::mouse_event(MOUSEEVENTF_RIGHTDOWN,      globalPos.x(), globalPos.y(), 0, 0);
    ::mouse_event(MOUSEEVENTF_RIGHTUP,        globalPos.x(), globalPos.y(), 0, 0);
    break;
  case Qt::MiddleButton:
    ::mouse_event(MOUSEEVENTF_MIDDLEDOWN,     globalPos.x(), globalPos.y(), 0, 0);
    ::mouse_event(MOUSEEVENTF_MIDDLEUP,       globalPos.x(), globalPos.y(), 0, 0);
    break;
  case Qt::XButton1:
  case Qt::XButton2:
    ::mouse_event(MOUSEEVENTF_XDOWN,          globalPos.x(), globalPos.y(), 0, 0);
    ::mouse_event(MOUSEEVENTF_XUP,            globalPos.x(), globalPos.y(), 0, 0);
    break;
  }
}

// - Environments -

QString
QtWin::getWinDirPath()
{ return ::getenv("windir"); }

QString
QtWin::getAppDataPath()
{ return ::getenv("AppData"); }

// See: http://msdn.microsoft.com/en-us/library/ms724451(v=VS.85).aspx
// See: http://msdn.microsoft.com/en-us/library/ms724833(VS.85).aspx
bool
QtWin::isWindowsXpOrLater()
{
  static int ret = -1;
  if (ret < 0) {
    LPOSVERSIONINFO lp = ::lposvi_();
    ret = (lp->dwMajorVersion > 5 ||
           lp->dwMajorVersion == 5 && lp->dwMinorVersion >= 1) // dwMinorVersion == 0 => Windows 2000
        ? 1 : 0;
  }
  return ret;
}

bool
QtWin::isWindowsVistaOrLater()
{
  static int ret = -1;
  if (ret < 0)
    ret = ::lposvi_()->dwMajorVersion >= 6 ? 1 : 0;
  return ret;
}

// - Files -

// Requiring Ole32.dll.
bool
QtWin::createLink(const QString &lnkPath, const QString &targetPath, const QString &description)
{
  if (lnkPath.isEmpty() || targetPath.isEmpty())
    return false;

  std::wstring wszLnkPath = lnkPath.toStdWString();
  std::wstring wszTargetPath = targetPath.toStdWString();
  std::wstring wszDescription = description.toStdWString();

  // See: http://msdn.microsoft.com/en-us/library/bb776891(VS.85).aspx
  //
  // CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces
  //              to create and store a shortcut to the specified object.
  //
  // Returns the result of calling the member functions of the interfaces.
  //
  // Parameters:
  // lpszPathObj  - Address of a buffer that contains the path of the object,
  //                including the file name.
  // lpszPathLink - Address of a buffer that contains the path where the
  //                Shell link is to be stored, including the file name.
  // lpszDesc     - Address of a buffer that contains a description of the
  //                Shell link, stored in the Comment field of the link
  //                properties.
  LPCWSTR lpszPathLink = wszLnkPath.c_str();
  LPCWSTR lpszPathObj = wszTargetPath.c_str();
  LPCWSTR lpszDesc = wszDescription.empty() ? 0 : wszDescription.c_str();

  HRESULT hres;
  IShellLink* psl;

  // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
  // has already been called.
  hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
  if (SUCCEEDED(hres))  {
    IPersistFile* ppf;
    // Set the path to the shortcut target and add the description.
    psl->SetPath(lpszPathObj);
    if (lpszDesc)
      psl->SetDescription(lpszDesc);

    // Query IShellLink for the IPersistFile interface, used for saving the
    // shortcut in persistent storage.
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
    if (SUCCEEDED(hres))  {

      // Ensure that the string is Unicode.
      //WCHAR wsz[MAX_PATH];
      //::MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

      // Add code here to check return value from MultiByteWideChar
      // for success.

      // Save the link by calling IPersistFile::Save.
      hres = ppf->Save(lpszPathLink, TRUE);
      ppf->Release();
    }
    psl->Release();
  }
  return SUCCEEDED(hres);
}

// Requiring Ole32.dll.
QString
QtWin::resolveLink(const QString &lnkPath, WId winId)
{
  QString ret;
  if (lnkPath.isEmpty())
    return ret;

  std::wstring wszLnkPath = lnkPath.toStdWString();
  //WCHAR wszRet[MAX_PATH];

  // ResolveIt - Uses the Shell's IShellLink and IPersistFile interfaces
  //             to retrieve the path and description from an existing shortcut.
  //
  // Returns the result of calling the member functions of the interfaces.
  //
  // Parameters:
  // hwnd         - A handle to the parent window. The Shell uses this window to
  //                display a dialog box if it needs to prompt the user for more
  //                information while resolving the link.
  // lpszLinkFile - Address of a buffer that contains the path of the link,
  //                including the file name.
  // lpszPath     - Address of a buffer that receives the path of the link
  //                target, including the file name.
  // lpszDesc     - Address of a buffer that receives the description of the
  //                Shell link, stored in the Comment field of the link
  //                properties.
  LPCWSTR lpszLinkFile = wszLnkPath.c_str();
  //LPWSTR lpszPath = wszRet;
  //int iPathBufferSize = sizeof(wszRet) / sizeof(*wszRet);

  HWND hwnd = (HWND)winId;

  HRESULT hres;
  IShellLink *psl;
  WCHAR szGotPath[MAX_PATH];
  //WCHAR szDescription[MAX_PATH];
  WIN32_FIND_DATA wfd;

  szGotPath[0] = 0;
  //*lpszPath = 0; // Assume failure

  // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
  // has already been called.
  hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
  if (SUCCEEDED(hres))  {
    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface.
    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

    if (SUCCEEDED(hres))  {

      // Ensure that the string is Unicode.
      //WCHAR wsz[MAX_PATH];
      //::MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);

     // Add code here to check return value from MultiByteWideChar
     // for success.

     // Load the shortcut.
     hres = ppf->Load(lpszLinkFile, STGM_READ);
     if (SUCCEEDED(hres))  {
       // Resolve the link.
       if (hwnd)
         hres = psl->Resolve((HWND)hwnd, 0);
       if (SUCCEEDED(hres))  {
         // Get the path to the link target.
         hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);
         /*
         if (SUCCEEDED(hres))  {
           // Get the description of the target.
           hres = psl->GetDescription(szDescription, MAX_PATH);

           if (SUCCEEDED(hres))  {
             hres = ::StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
             if (SUCCEEDED(hres)) {
                 // Handle success
              } else {
                // Handle the error
              }
            }
          }
          */
        }
      }

      // Release the pointer to the IPersistFile interface.
      ppf->Release();
    }

    // Release the pointer to the IShellLink interface.
    psl->Release();
  }

  if (SUCCEEDED(hres))
    ret = QString::fromWCharArray(szGotPath);
  return ret;
}

// - Devices -

// See: http://www.codeguru.com/forum/archive/index.php/t-252784.html
QStringList
QtWin::getLogicalDrives()
{
  QStringList ret;

  enum { BufSize = 3 * 26 }; // At most 26 drives, each take 3 chars such as "C:\"
  char szBuffer[BufSize];

  DWORD dwReturnSize = ::GetLogicalDriveStringsA(BufSize, szBuffer);
  if (dwReturnSize) {
    char *pch = szBuffer;
    while (*pch) {
      ret.append(pch);
      pch += ::strlen(pch) + 1;
    }
  }

  return ret;
}

QtWin::DriveType
QtWin::getDriveType(const QString &driveLetter)
{
  return (DriveType)
      ::GetDriveTypeA(driveLetter.toAscii());
}

QStringList
QtWin::getLogicalDrivesWithType(DriveType type)
{
  QStringList ret;

  enum { BufSize = 3 * 26 }; // At most 26 drives, each take 3 chars such as "C:\"
  char szBuffer[BufSize];

  DWORD dwReturnSize = ::GetLogicalDriveStringsA(BufSize, szBuffer);
  if (dwReturnSize) {
    char *pch = szBuffer;
    while (*pch) {
      if (::GetDriveTypeA(pch) == type)
        ret.append(pch);
      pch += ::strlen(pch) + 1;
    }
  }

  return ret;
}

QString
QtWin::guessDeviceFileName(const QString &hint)
{
  if (hint.isEmpty())
    return QString();

  QString normalized = hint.trimmed().toUpper();
  if (normalized.isEmpty())
    return QString();

  else if (normalized.contains(QRegExp("^\\\\\\\\\\.\\\\[A-Z]:$")))
    return normalized;
  else if (normalized.contains(QRegExp("^[A-Z]:$")))
    return "\\\\.\\" + normalized;
  else if (normalized.contains(QRegExp("^[A-Z]:\\\\$"))) {
    normalized.chop(1);
    return "\\\\.\\" + normalized;
  }
  else
    return QString();
}

bool
QtWin::isValidDeviceFileName(const QString &fileName)
{ return fileName.contains(QRegExp("^\\\\\\\\\\.\\\\[A-Z]:$", Qt::CaseInsensitive)); }

#if 0
// Require Winmm.lib.
bool
QtWin::setWaveVolume(qreal percentage)
{
  enum { MAX_VOLUME = 0xff };
  if (percentage < 0)
    percentage = 0;
  else if (percentage > 1)
    percentage = 1;

  WORD w = MAX_VOLUME * percentage;
  DWORD dw = MAKEDWORD(w, w);

  return ::waveOutSetVolume(0, dw) == MMSYSERR_NOERROR;
}

// Require Winmm.lib.
qreal
QtWin::getWaveVolume()
{
  enum { MAX_VOLUME = 0xff };

  qreal ret = -1;
  DWORD dwVolume;
  if (::waveOutGetVolume(0, &dwVolume) == MMSYSERR_NOERROR)
    ret = (LOWORD(dwVolume) + HIWORD(dwVolume)) / (2.0 * MAX_VOLUME);

  return ret;
}

#endif // 0

// EOF

/*

// See: http://msdn.microsoft.com/en-us/library/bb776891(VS.85).aspx
//
// CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces
//              to create and store a shortcut to the specified object.
//
// Returns the result of calling the member functions of the interfaces.
//
// Parameters:
// lpszPathObj  - Address of a buffer that contains the path of the object,
//                including the file name.
// lpszPathLink - Address of a buffer that contains the path where the
//                Shell link is to be stored, including the file name.
// lpszDesc     - Address of a buffer that contains a description of the
//                Shell link, stored in the Comment field of the link
//                properties.

#include "stdafx.h"
#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"

HRESULT CreateLink(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description.
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the
        // shortcut in persistent storage.
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode.
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Add code here to check return value from MultiByteWideChar
            // for success.

            // Save the link by calling IPersistFile::Save.
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}


// See: http://msdn.microsoft.com/en-us/library/bb776891(VS.85).aspx
//
// ResolveIt - Uses the Shell's IShellLink and IPersistFile interfaces
//             to retrieve the path and description from an existing shortcut.
//
// Returns the result of calling the member functions of the interfaces.
//
// Parameters:
// hwnd         - A handle to the parent window. The Shell uses this window to
//                display a dialog box if it needs to prompt the user for more
//                information while resolving the link.
// lpszLinkFile - Address of a buffer that contains the path of the link,
//                including the file name.
// lpszPath     - Address of a buffer that receives the path of the link
                  target, including the file name.
// lpszDesc     - Address of a buffer that receives the description of the
//                Shell link, stored in the Comment field of the link
//                properties.

#include "stdafx.h"
#include "windows.h"
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"

HRESULT ResolveIt(HWND hwnd, LPCSTR lpszLinkFile, LPWSTR lpszPath, int iPathBufferSize)
{
    HRESULT hres;
    IShellLink* psl;
    WCHAR szGotPath[MAX_PATH];
    WCHAR szDescription[MAX_PATH];
    WIN32_FIND_DATA wfd;

    *lpszPath = 0; // Assume failure

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Get a pointer to the IPersistFile interface.
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode.
            MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);

            // Add code here to check return value from MultiByteWideChar
            // for success.

            // Load the shortcut.
            hres = ppf->Load(wsz, STGM_READ);

            if (SUCCEEDED(hres))
            {
                // Resolve the link.
                hres = psl->Resolve(hwnd, 0);

                if (SUCCEEDED(hres))
                {
                    // Get the path to the link target.
                    hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

                    if (SUCCEEDED(hres))
                    {
                        // Get the description of the target.
                        hres = psl->GetDescription(szDescription, MAX_PATH);

                        if (SUCCEEDED(hres))
                        {
                            hres = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
                            if (SUCCEEDED(hres))
                            {
                                // Handle success
                            }
                            else
                            {
                                // Handle the error
                            }
                        }
                    }
                }
            }

            // Release the pointer to the IPersistFile interface.
            ppf->Release();
        }

        // Release the pointer to the IShellLink interface.
        psl->Release();
    }
    return hres;
}

*/
