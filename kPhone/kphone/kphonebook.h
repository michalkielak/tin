/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 * Copyright (c) 2006 Kphone Team  <kphone-devel@lists.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */
#ifndef KSIPUSERINFO_H_INCLUDED
#define KSIPUSERINFO_H_INCLUDED

#include <qdatetime.h>
#include <qdialog.h>
#include <qptrdict.h>
#include <qxml.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "../dissipate2/sipcall.h"
#include "menulistview.h"

class QListView;
class QListViewItem;
class QString;
class QPushButton;


/**
* @short PhoneBook Implementation
*/
class IncomingCall {
public:
	IncomingCall( QString s, QDateTime time );
	~IncomingCall( void );
	QString getContact( void ) const { return contact; }
	QString getRejected( void ) const { return rejected; }
	QString getScreened( void ) const { return screened; }
	QDateTime getDateTime( void ) const { return dt; }
private:
	QString contact;
	QString rejected;
	QString screened;
	QDateTime dt;
};

/**
* @short PhoneBook Implementation
*/
class PhoneBookAddIdentity : public QDialog
{
	Q_OBJECT
public:
	PhoneBookAddIdentity( QWidget *parent = 0, const char *name = 0 );
	void clean( void );
	void setPerson( QString );
	void setIdentity( QString );
	void setDescription( QString );
	void setContact( bool );
	void setRejected( bool );
	void setScreened( bool );
	QString getIdentity( void );
	QString getDescription( void );
	QString getPerson( void );
	QString getContact( void );
	QString getRejected( void );
	QString getScreened( void );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *identity;
	QLineEdit *description;
	QLineEdit *person;
	QCheckBox *contact;
	QCheckBox *rejected;
	QCheckBox *screened;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

/**
* @short PhoneBook Implementation
*/
class PhoneBookEditIdentity : public QDialog
{
	Q_OBJECT
public:
	PhoneBookEditIdentity( QWidget *parent = 0, const char *name = 0 );
	~PhoneBookEditIdentity( void );
	QString getIdentity( void );
	QString getDescription( void );
	QString getContact( void );
	QString getRejected( void );
	QString getScreened( void );
	void setIdentity( QString );
	void setDescription( QString );
	void setContact( QString );
	void setRejected( QString );
	void setScreened( QString );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *identity;
	QLineEdit *description;
	QCheckBox *contact;
	QCheckBox *rejected;
	QCheckBox *screened;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

/**
* @short PhoneBook Implementation
*/
class PhoneBookEditPerson : public QDialog
{
	Q_OBJECT
public:
	PhoneBookEditPerson( QWidget *parent = 0, const char *name = 0 );
	~PhoneBookEditPerson( void );
	QString getPerson( void );
	void setPerson( QString );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *person;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

/**
* @short PhoneBook Implementation
*/
class PhoneBookParser : public QXmlDefaultHandler
{
public:
	PhoneBookParser( QListView * t, bool CallRegister = true );
	bool startDocument();
	bool startElement( const QString&, const QString&, const QString&, const QXmlAttributes& );
	bool endElement( const QString&, const QString&, const QString& );
	QListViewItem *getListPhoneBook( void ) { return listPhoneBook; }
	QListViewItem *getListReceivedCalls( void ) { return listReceivedCalls; }
	QListViewItem *getListMissedCalls( void ) { return listMissedCalls; }
	QListViewItem *getListDialledCalls( void ) { return listDialledCalls; }
private:
	QPtrStack<QListViewItem> stack;
	QListViewItem *rejected;
	QListViewItem *listPhoneBook;
	QListViewItem *listReceivedCalls;
	QListViewItem *listMissedCalls;
	QListViewItem *listDialledCalls;
};

/**
* @short The PhoneBook
*
* Our phonebook. The data are stored in XML records on a file.This
* class serves as the interface between KPhone and that file

*/
class PhoneBook : public QDialog
{
	Q_OBJECT
public:
	PhoneBook( QString fileUserInfo, QWidget *parent, const char *name ,
		QList<IncomingCall> &receivedCalls, QList<IncomingCall> &missedCalls,QList<IncomingCall> &dialledCalls);
	~PhoneBook( void );
	void addContact( SipCallMember *member, bool reject = false );
private slots:
	bool setIdentity( QListViewItem * );
	void setIdentityAndDone( QListViewItem * );
	void clickAddButton(void );
	void clickEditButton( void );
	void clickRemoveButton( void );
	void clickHideButton( void );
public slots:
	QString getUri();
private:
	void save( void );
	QString clean( QString s );
	MenuListView *phoneBook;
	QPushButton *add_button;
	QPushButton *edit_button;
	QPushButton *remove_button;
	QPushButton *save_button;
	QString remote;
	PhoneBookAddIdentity *addIdentity;
	PhoneBookEditIdentity *editIdentity;
	PhoneBookEditPerson *editPerson;
	QString filePhoneBook;
	PhoneBookParser *parser;
	QString caption;
};

#endif // KSIPUSERINFO_H_INCLUDED
