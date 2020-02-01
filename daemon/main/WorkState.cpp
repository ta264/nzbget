/*
 *  This file is part of nzbget. See <http://nzbget.net>.
 *
 *  Copyright (C) 2019 Andrey Prygunkov <hugbug@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "nzbget.h"
#include "WorkState.h"

WorkState::~WorkState()
{
        delete m_SharingStatus;
}

void WorkState::Changed()
{
	Notify(nullptr);
}

void WorkState::InitSharingStatus(bool enabled, const char* name, const char* url, const char* tempDir, int pollInterval, bool remoteClientMode)
{
        m_SharingStatus = new SharingStatus(enabled, name, url, tempDir, pollInterval, remoteClientMode);
}

void WorkState::SetPauseDownload(bool pauseDownload)
{
	m_pauseDownload = m_SharingStatus->ChangePauseState(m_pauseDownload, pauseDownload);
        Changed();
}

void WorkState::CheckPauseDownload(bool hasJob)
{
	m_pauseDownload = m_SharingStatus->CheckPauseState(m_pauseDownload, hasJob);
        Changed();
}

const char* WorkState::GetCurrentSharingUser()
{
	return m_SharingStatus->GetCurrentSharingUser();
}

bool WorkState::GetSharingPollResume()
{
        return m_SharingStatus->GetPollResume();
}
