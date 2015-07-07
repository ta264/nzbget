#ifndef SHARINGSTATUS_H
#define SHARINGSTATUS_H

#ifdef WIN32
#include "win32.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>

#include "nzbget.h"
#include "DownloadInfo.h"
#include "WebDownloader.h"
#include "Util.h"
#include "Log.h"


class SharingStatus
{
private:
	std::string m_szMyName;
	std::string m_szStatusUrl;
	std::string m_szTempDir;
	std::string m_szCurrentUser;
	std::string readUrl(std::string szUrl);

	bool m_bEnabled;
	bool m_bPollResume;
	time_t m_tLastPoll;
	double m_dPollInterval;
	bool m_bRemoteClientMode;
	
	void UpdateCurrentUser();
	bool Pause();
	bool TryResume();
public:
	SharingStatus(bool bEnabled, char* szMyName, char* szStatusUrl, char* szTempDir, int iPollInterval, bool bRemoteClientMode);
	~SharingStatus();
	bool ChangePauseState(bool bCurrentPauseState, bool bWantedPauseState);
	bool CheckPauseState(bool bCurrentlyPaused, bool bHasJob);
	const char* GetCurrentSharingUser();
};

#endif
