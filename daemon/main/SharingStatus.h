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
	std::string m_myName;
	std::string m_statusUrl;
	std::string m_tempDir;
	std::string m_currentUser;
	std::string readUrl(std::string url);

	bool m_enabled;
	bool m_pollResume;
	time_t m_lastPoll;
	double p_pollInterval;
	bool m_remoteClientMode;
	
	void UpdateCurrentUser();
	bool Pause();
	bool TryResume();
public:
	SharingStatus(bool enabled, char* myName, char* statusUrl, char* tempDir, int pollInterval, bool remoteClientMode);
	~SharingStatus();
	bool ChangePauseState(bool currentPauseState, bool wantedPauseState);
	bool CheckPauseState(bool currentlyPaused, bool hasJob);
	const char* GetCurrentSharingUser();
};

#endif
