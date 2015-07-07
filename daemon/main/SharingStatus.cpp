#include "SharingStatus.h"

SharingStatus::SharingStatus(bool bEnabled, char* szMyName, char* szStatusUrl, char* szTempDir, int iPollInterval, bool bRemoteClientMode) {
	m_bEnabled = bEnabled;
	m_szMyName.assign(szMyName);
	m_szStatusUrl.assign(szStatusUrl);
	m_szTempDir.assign(szTempDir);
	m_dPollInterval = (double) iPollInterval;
	m_bRemoteClientMode = bRemoteClientMode;

	// defaults
	m_bPollResume = true;
	m_tLastPoll = 0;
	m_szCurrentUser = "none";
}

SharingStatus::~SharingStatus() {
	if (!m_bRemoteClientMode)
	{
		info("SharingStatus: Calling destructor - pausing");
		Pause();
	}
}

std::string SharingStatus::readUrl(std::string url)
{
	// generate temp file name
	char szTempFileName[1024];
	int iNum = 1;
	while (iNum == 1 || Util::FileExists(szTempFileName))
	{
		snprintf(szTempFileName, 1024, "%statusurl-%i.tmp", m_szTempDir.c_str(), iNum);
		szTempFileName[1024-1] = '\0';
		iNum++;
	}

	// url to query
	char szInfoName[] = "StatusURL";
	info("Downloading URL: %s", url.c_str());

	WebDownloader* pDownloader = new WebDownloader();
	pDownloader->SetURL(url.c_str());
	pDownloader->SetForce(true);
	pDownloader->SetRetry(false);
	pDownloader->SetOutputFilename(szTempFileName);
	pDownloader->SetInfoName(szInfoName);

	// do sync download
	WebDownloader::EStatus eStatus = pDownloader->Download();
	bool bOK = eStatus == WebDownloader::adFinished;

	delete pDownloader;

	std::string output;
	if (bOK)
	{
		char* szFileContent = NULL;
		int iFileContentLen = 0;
		Util::LoadFileIntoBuffer(szTempFileName, &szFileContent, &iFileContentLen);
		output = std::string(szFileContent);
		free(szFileContent);
	}
	else
	{
		warn("Querying %s failed", szInfoName);
	}

	remove(szTempFileName);
	return(output);
}

void SharingStatus::UpdateCurrentUser()
{
	std::string url = m_szStatusUrl + "status.txt";
	m_szCurrentUser = readUrl(url);
}

bool SharingStatus::Pause()
{
	// Return if not enabled
	if (!m_bEnabled)
		return true;

	// Tell webserver we've paused.  No response given.
	std::string url = m_szStatusUrl + "share.php?person=" + m_szMyName + "&action=stop";
	readUrl(url);

	// Belt and braces - this should always set user to "none".
	UpdateCurrentUser();
	return true;
}

bool SharingStatus::TryResume()
{
	// Always allow resume if sharing not enabled
	if (!m_bEnabled)
		return true;

	// Query webserver to see if resume allowed
	std::string url = m_szStatusUrl + "share.php?person=" + m_szMyName + "&action=nzbadd";
	std::string reply = readUrl(url);

	// m_bPollResume = true;
	if (reply == "ok")
	{
		info("SharingStatus: resume allowed");
		m_szCurrentUser = m_szMyName;
		return true;
	}
	else
	{
		UpdateCurrentUser();
		std::string message = "SharingStatus: In use by " + m_szCurrentUser +
			", resume not allowed";
		info(message.c_str());
		return false;
	}
}

bool SharingStatus::ChangePauseState(bool bCurrentPauseState, bool bWantedPauseState)
{
	// This is only called by user and not polled
	// so no need to add any time delay

	if (bCurrentPauseState == bWantedPauseState)
	{
		// if we are in the correct state then all is well
		return bCurrentPauseState;
	}

	if (bWantedPauseState == true)
	{
		// if we want to pause then no polling
		m_bPollResume = false;
		return Pause();
	}
	else
	{
		// if we want to resume now then we want to resume in the future too
		m_bPollResume = true;
		return !TryResume();
	}
}

bool SharingStatus::CheckPauseState(bool bCurrentlyPaused, bool bHasJob)
{
	// work out if anything else to download
	DownloadQueue *pDownloadQueue = DownloadQueue::Lock();
	bHasJob = bHasJob || !pDownloadQueue->GetQueue()->empty();
	DownloadQueue::Unlock();

	// If we are running with a job or paused with no job then do nothing
	if (!bCurrentlyPaused == bHasJob)
	{
		return bCurrentlyPaused;
	}

	// If we are paused but pollResume is false then do nothing 
	// (irrespective of whether we have a job)
	if (bCurrentlyPaused && !m_bPollResume)
	{
		return bCurrentlyPaused;
	}

	// check if a long enough period has elapsed since tried to
	// update status
	double interval = difftime(time(NULL), m_tLastPoll);
	if (interval < m_dPollInterval)
	{
		return bCurrentlyPaused;
	}

	m_tLastPoll = time(NULL);

	// Otherwise we want to try and set paused state to !bHasJob
	bool bResult = ChangePauseState(bCurrentlyPaused, !bHasJob);

	// But remember that ChangePauseState could set m_bPollResume = false
	m_bPollResume = true;

	return bResult;
}

const char* SharingStatus::GetCurrentSharingUser()
{
	return m_szCurrentUser.c_str();
}
