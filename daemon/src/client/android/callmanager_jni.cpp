/*
 *  Copyright (C) 2004-2013 Savoir-Faire Linux Inc.
 *  Author: Pierre-Luc Beaudoin <pierre-luc.beaudoin@savoirfairelinux.com>
 *  Author: Alexandre Bourget <alexandre.bourget@savoirfairelinux.com>
 *  Author: Emeric Vigier <emeric.vigier@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include "client/callmanager.h"
#include "jni_callbacks.h"

CallManager::CallManager()
{}

void CallManager::callStateChanged(const std::string& callID, const std::string& state)
{
    on_call_state_changed_wrapper(callID, state);
}

void CallManager::transferFailed()
{

}

void CallManager::transferSucceeded()
{

}

void CallManager::recordPlaybackStopped(const std::string& path)
{

}

void CallManager::voiceMailNotify(const std::string& callID, const std::string& nd_msg)
{

}

void CallManager::incomingMessage(const std::string& ID, const std::string& from, const std::string& msg)
{
    on_incoming_message_wrapper(ID, from, msg);
}

void CallManager::incomingCall(const std::string& accountID, const std::string& callID, const std::string& from)
{
    on_incoming_call_wrapper(accountID, callID, from);
}

void CallManager::recordPlaybackFilepath(const std::string& id, const std::string& filename)
{
    on_record_playback_filepath_wrapper(id, filename);
}

void CallManager::conferenceCreated(const std::string& confID)
{
    on_conference_created_wrapper(confID);
}

void CallManager::conferenceChanged(const std::string& confID,const std::string& state)
{
    on_conference_state_changed_wrapper(confID, state);
}

void CallManager::conferenceRemoved(const std::string& confID)
{
    on_conference_removed_wrapper(confID);
}

void CallManager::newCallCreated(const std::string& accountID, const std::string& callID, const std::string& to)
{
    on_new_call_created_wrapper(accountID, callID, to);
}

void CallManager::sipCallStateChanged(const std::string& accoundID, const std::string& state, const int32_t& code)
{

}

void CallManager::recordingStateChanged(const std::string& callID, const bool& state )
{

}

void CallManager::updatePlaybackScale(const std::string&, const int32_t&, const int32_t&)
{

}

void CallManager::secureSdesOn(std::string const&)
{}

void CallManager::secureSdesOff(std::string const&)
{}