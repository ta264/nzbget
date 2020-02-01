#include "SharingStatus.h"

SharingStatus::SharingStatus(bool e, const char* name, const char* url, const char* temp, int interval, bool remote) {
	enabled = e;
	myName.assign(name);
	statusUrl.assign(url);
	tempDir.assign(temp);
	pollInterval = (double) interval;
	remoteClientMode = remote;

	// defaults
	pollResume = true;
	lastPoll = 0;
	currentUser = "none";
}

SharingStatus::~SharingStatus() {
        if (!remoteClientMode && currentUser == myName)
	{
		info("SharingStatus: In use and calling destructor - pausing");
		Pause();
	}
}

std::string SharingStatus::readUrl(std::string url)
{
	// generate temp file name
	BString<1024> tempFileName;
	int num = 1;
	while (num == 1 || FileSystem::FileExists(tempFileName))
	{
		tempFileName.Format("%s%cstatusurl-%i.tmp", tempDir.c_str(), PATH_SEPARATOR, num);
		num++;
	}

	// url to query
	char szInfoName[] = "StatusURL";
	info("Downloading URL: %s", url.c_str());

	WebDownloader downloader;
	downloader.SetUrl(url.c_str());
	downloader.SetForce(true);
	downloader.SetRetry(false);
	downloader.SetOutputFilename(tempFileName);
	downloader.SetInfoName(szInfoName);

	// do sync download
	WebDownloader::EStatus eStatus = downloader.Download();
	bool bOK = eStatus == WebDownloader::adFinished;

	std::string output;
	if (bOK)
	{
		CharBuffer fileContent;
		FileSystem::LoadFileIntoBuffer(tempFileName, fileContent, true);
		output = std::string(fileContent);
	}
	else
	{
		warn("Querying %s failed", szInfoName);
	}

	return(output);
}

void SharingStatus::UpdateCurrentUser()
{
	std::string url = statusUrl + "status.txt";
	currentUser = readUrl(url);
}

bool SharingStatus::Pause()
{
	// Return if not enabled
	if (!enabled)
		return true;

	// Tell webserver we've paused.	 No response given.
	std::string url = statusUrl + "share.php?person=" + myName + "&action=stop";
	readUrl(url);

	// Belt and braces - this should always set user to "none".
	UpdateCurrentUser();
	return true;
}

bool SharingStatus::TryResume()
{
	// Always allow resume if sharing not enabled
	if (!enabled)
		return true;

        detail("Try resume - checking if allowed");

	// Query webserver to see if resume allowed
	std::string url = statusUrl + "share.php?person=" + myName + "&action=nzbadd";
	std::string reply = readUrl(url);

	// pollResume = true;
	if (reply == "ok")
	{
		info("SharingStatus: resume allowed");
		currentUser = myName;
		return true;
	}
	else
	{
		UpdateCurrentUser();
		info("SharingStatus: In use by %s, resume not allowed", currentUser.c_str());
		return false;
	}
}

bool SharingStatus::ChangePauseState(bool currentPauseState, bool wantedPauseState)
{
	// This is only called by user and not polled
	// so no need to add any time delay

	if (currentPauseState == wantedPauseState)
	{
		// if we are in the correct state then all is well
		return currentPauseState;
	}

	if (wantedPauseState == true)
	{
		// if we want to pause then no polling
		pollResume = false;
		return Pause();
	}
	else
	{
		// if we want to resume now then we want to resume in the future too
		pollResume = true;
		return !TryResume();
	}
}

bool SharingStatus::CheckPauseState(bool currentlyPaused, bool hasJob)
{
	// work out if anything else to download
	GuardedDownloadQueue downloadQueue = DownloadQueue::Guard();
	hasJob = hasJob || !downloadQueue->GetQueue()->empty();

	// If we are running with a job or paused with no job then do nothing
	if (!currentlyPaused == hasJob)
	{
		return currentlyPaused;
	}

	// If we are paused but pollResume is false then do nothing 
	// (irrespective of whether we have a job)
	if (currentlyPaused && !pollResume)
	{
		return currentlyPaused;
	}

	// check if a long enough period has elapsed since tried to
	// update status
	if (currentlyPaused && difftime(time(NULL), lastPoll) < pollInterval)
	{
		return currentlyPaused;
	}

	lastPoll = time(NULL);

	// Otherwise we want to try and set paused state to !hasJob
	bool bResult = ChangePauseState(currentlyPaused, !hasJob);

	// But remember that ChangePauseState could set pollResume = false
	pollResume = true;

	return bResult;
}

const char* SharingStatus::GetCurrentSharingUser()
{
	return currentUser.c_str();
}

bool SharingStatus::GetPollResume()
{
        return pollResume;
}
