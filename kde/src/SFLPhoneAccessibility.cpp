/***************************************************************************
 *   Copyright (C) 2009-2012 by Savoir-Faire Linux                         *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 **************************************************************************/
#include "SFLPhoneAccessibility.h"
#include "KSpeechInterfaceSingleton.h"
#include <KSpeech>
#include <KDebug>
#include <KLocale>
#include "SFLPhone.h"

SFLPhoneAccessibility* SFLPhoneAccessibility::m_pInstance=0;

SFLPhoneAccessibility::SFLPhoneAccessibility() : QObject(0),QList<KAction*>()
{
   KAction* action = new KAction(0);
   action->setObjectName("listCall");
   action->setShortcut( Qt::CTRL + Qt::Key_L );
   action->setText("List all current calls");
   action->setIcon(KIcon("text-speak"));
   *this << action;
   connect(action,SIGNAL(triggered(bool)),this,SLOT(listCall()));

   action = new KAction(0);
   action->setObjectName("currentCallDetails");
   action->setShortcut( Qt::CTRL + Qt::Key_I );
   action->setText("Get current call details");
   action->setIcon(KIcon("text-speak"));
   *this << action;
   connect(action,SIGNAL(triggered(bool)),this,SLOT(currentCallDetails()));
}

SFLPhoneAccessibility* SFLPhoneAccessibility::getInstance()
{
   if (not m_pInstance) {
      m_pInstance = new SFLPhoneAccessibility();
   }
   return m_pInstance;
}

void SFLPhoneAccessibility::listCall()
{
   if (SFLPhone::model()->getCallList().size()>0) {
      KSpeechInterfaceSingleton::getInstance()->say(i18n("You currently have %1 calls").arg(QString::number(SFLPhone::model()->getCallList().size())), KSpeech::soPlainText);
      foreach (Call* call,SFLPhone::model()->getCallList()) {
         KSpeechInterfaceSingleton::getInstance()->say(i18n("Call from %1, number %2").arg(call->getPeerName()).arg(numberToDigit((!call->getPeerPhoneNumber().isEmpty())?call->getPeerPhoneNumber():call->getCallNumber())), KSpeech::soPlainText);
      }
   }
   else {
      KSpeechInterfaceSingleton::getInstance()->say("You currently have no call", KSpeech::soPlainText);
   }
}

QString SFLPhoneAccessibility::numberToDigit(QString number)
{
   QString toReturn;
   for(int i=0;i<number.count();i++) {
      if (i+1 < number.count() && (number[i] >= 0x30 && number[i] <= 0x39) && (number[i+1] >= 0x30 && number[i+1] <= 0x39))
         toReturn += number[i]+" ";
      else
         toReturn += number[i];
   }
   return toReturn;
}

void SFLPhoneAccessibility::currentCallDetails()
{
   foreach (Call* call,SFLPhone::model()->getCallList()) {
      if (call->isSelected()) {
         QString toSay = i18n("The current call is %1").arg( i18n(call->toHumanStateName().toAscii() ));
         if (!call->getPeerName().trimmed().isEmpty())
            toSay += i18n(",Your peer is %1").arg( numberToDigit(call->getPeerName())           );
         if (!call->getPeerPhoneNumber().isEmpty())
            toSay += i18n(", the peer phone number is %1 ").arg( numberToDigit(call->getPeerPhoneNumber())    );
         else if (!call->getCallNumber().isEmpty())
            toSay += i18n(", the phone number is %1 ").arg( numberToDigit(call->getCallNumber())    );
         
         int nSec = QDateTime::fromTime_t(call->getStartTimeStamp().toInt()).time().secsTo( QTime::currentTime() );
         if (nSec>0)
            toSay += i18n(" and you have been talking since %1 seconds").arg( nSec );

         KSpeechInterfaceSingleton::getInstance()->say(toSay, KSpeech::soPlainText);
      }
   }
}

void SFLPhoneAccessibility::say(QString message)
{
   KSpeechInterfaceSingleton::getInstance()->say(message, KSpeech::soPlainText);
}