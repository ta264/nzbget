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
	std::string myName;
	std::string statusUrl;
	std::string tempDir;
	std::string currentUser;
	std::string readUrl(std::string url);

	bool enabled;
	bool pollResume;
	time_t lastPoll;
	double pollInterval;
	bool remoteClientMode;
	
	void UpdateCurrentUser();
	bool Pause();
	bool TryResume();
public:
	SharingStatus(bool enabled, const char* myName, const char* statusUrl, const char* tempDir, int pollInterval, bool remoteClientMode);
	~SharingStatus();
	bool ChangePauseState(bool currentPauseState, bool wantedPauseState);
	bool CheckPauseState(bool currentlyPaused, bool hasJob);
	const char* GetCurrentSharingUser();
        bool GetPollResume();
};

#endif
