
#include "library.h"

#define FTP_DOWNLOAD	1
#define FTP_UPLOAD		2
#define FTP_RENAME		3
#define FTP_DELETE		4
#define FTP_CREATE_DIR	5
#define FTP_LIST		6

class CDirectory : public CObject
{
	DECLARE_SERIAL(CDirectory)

	CDirectory();
	CDirectory(CString,CString);
	virtual ~CDirectory();

public:
	virtual void Serialize(CArchive&);
	CDirectory (const CDirectory &dir);				// copy-constructor
	CDirectory &operator=(const CDirectory &dir);	// =-operator

	CString m_strDir;
	CString m_strAlias;
	BOOL m_bAllowDownload;
	BOOL m_bAllowUpload;
	BOOL m_bAllowRename;
	BOOL m_bAllowDelete;
	BOOL m_bAllowCreateDirectory;
	BOOL m_bIsHomeDir; 
};


class CUser : public CObject
{
	DECLARE_SERIAL(CUser)

	CUser();
	CUser(CString,CString);
	virtual ~CUser();

public:
	virtual void Serialize(CArchive&);
	CUser ( CUser &user);				// copy-constructor
	CUser &operator=( CUser &user);	// =-operator

	CString m_strName;
	CString m_strPassword;

	CArray <CDirectory, CDirectory&> m_DirectoryArray;

	BOOL m_bAccountDisabled;
};



class CUserManager  
{
public:
	void GetUserList(CArray<CUser, CUser&>&array);
	void UpdateUserList(CArray<CUser, CUser&>&array);
	CUserManager();
	virtual ~CUserManager();
	
	CArray<CUser, CUser&> m_UserArray;

	BOOL Serialize(BOOL bStoring);
	BOOL GetUser(LPCTSTR lpszUser, CUser &user);
	BOOL ConvertPathToLocal(LPCTSTR lpszUser, CString &strDirectoryIn, CString &strDirectoryOut);
	int CheckDirectory(LPCTSTR lpszUser, CString dirname, CString strCurrentdir, int nOption, CString &strResult);
	int CheckFileName(LPCTSTR lpszUser, CString filename, CString strCurrentdir, int nOption, CString &strResult);
	int ChangeDirectory(LPCTSTR lpszUser, CString &currentdir, CString &dir);
	int GetDirectoryList(LPCTSTR lpszUser, LPCTSTR lpszDirectory, CString &strResult);
	BOOL FileExists(LPCTSTR lpszFileName, BOOL bIsDirCheck = TRUE);
protected:
	BOOL CheckAccessRights(LPCTSTR lpszUser, LPCTSTR lpszDirectory, int nOption);
	CString GetFileDate(CFileFind &find);
	CString m_strFilename;
	CCriticalSection m_CriticalSection;
};

