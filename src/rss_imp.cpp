/*
 * Bittorrent Client using Qt4 and libtorrent.
 * Copyright (C) 2006  Christophe Dumez, Arnaud Demaiziere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Contact : chris@qbittorrent.org arnaud@qbittorrent.org
 */

#include "rss_imp.h"
#include <QDesktopServices>
#include <QInputDialog>
#include <QMenu>
#include <QStandardItemModel>

    // display a right-click menu
    void RSSImp::displayFinishedListMenu(const QPoint& pos){
      QMenu myFinishedListMenu(this);
      QListWidgetItem* item = listStreams->itemAt(pos);
      if(item!=NULL) {
	myFinishedListMenu.addAction(actionDelete);
	myFinishedListMenu.addAction(actionRename);
	myFinishedListMenu.addAction(actionRefresh);
      }
      myFinishedListMenu.addAction(actionCreate);
      myFinishedListMenu.addAction(actionRefreshAll);
      myFinishedListMenu.exec(mapToGlobal(pos)+QPoint(10,33));
    }

    // add a stream by a button
    void RSSImp::on_addStream_button_clicked() {
      createStream();
    }

    // delete a stream by a button
    void RSSImp::on_delStream_button_clicked() {
      if(listStreams->currentRow()<0 || rssmanager.getNbStream()==0) {
	qDebug("no stream selected");
	return;
      }else {
	textBrowser->clear();
	listNews->clear();
	rssmanager.removeStream(rssmanager.getStream(listStreams->currentRow()));
	refreshStreamList();
      }
    }

    // refresh all streams by a button
    void RSSImp::on_refreshAll_button_clicked() {
      refreshAllStreams();
    }

    // display the news of a stream when click on it
    void RSSImp::on_listStreams_clicked() {
      rssmanager.getStream(listStreams->currentRow())->setRead();
      //streamNeedRefresh(listStreams->currentRow());
      listStreams->item(listStreams->currentRow())->setData(Qt::BackgroundRole, QVariant(QColor("white")));
      refreshNewsList();
    }

    // display the content of a new when clicked on it
    void RSSImp::on_listNews_clicked() {
      listNews->item(listNews->currentRow())->setData(Qt::ForegroundRole, QVariant(QColor("grey")));
      refreshTextBrowser();
    }

    // open the url of the news in a browser
    void RSSImp::on_listNews_doubleClicked() {
      if(listStreams->currentRow()>=0 && listNews->currentRow()>=0 && rssmanager.getStream(listStreams->currentRow())->getListSize()>0) {
	RssItem* currentItem =  rssmanager.getStream(listStreams->currentRow())->getItem(listNews->currentRow());
	if(currentItem->getLink()!=NULL && currentItem->getLink().length()>5)
	 QDesktopServices::openUrl(QUrl(currentItem->getLink()));
      }
    }

    //right-clik on stream : delete it
    void RSSImp::deleteStream() {
      if(rssmanager.getNbStream()==0) {
	qDebug("no stream selected");
	return;
      }else {
	textBrowser->clear();
	listNews->clear();
	rssmanager.removeStream(rssmanager.getStream(listStreams->currentRow()));
	refreshStreamList();
      }
    }

    //right-clik on stream : give him an alias
    void RSSImp::renameStream() {
      if(rssmanager.getNbStream()==0) {
	qDebug("no stream selected");
	return;
      }else {
	bool ok;
	short index = listStreams->currentRow();
	QString newAlias = QInputDialog::getText(this, tr("Please choose a new name for this stream"), tr("New stream name:"), QLineEdit::Normal, rssmanager.getStream(index)->getAlias(), &ok);
	if(ok) {
	  rssmanager.setAlias(index, newAlias);
	  updateStreamName(index);
	}
      }

    }

    //right-clik on stream : refresh it
    void RSSImp::refreshStream() {
      short index = listStreams->currentRow();
      if(rssmanager.getNbStream()>0) {
	textBrowser->clear();
	listNews->clear();
	listStreams->item(index)->setData(Qt::DecorationRole, QVariant(QIcon(":/Icons/refresh.png")));
	rssmanager.refresh(index);
      }
    }

    //right-click somewhere, refresh all the streams
    void RSSImp::refreshAllStreams() {
      textBrowser->clear();
      listNews->clear();
      unsigned short nbstream = rssmanager.getNbStream();
      for(unsigned short i=0; i<nbstream; i++)
        listStreams->item(i)->setData(Qt::DecorationRole, QVariant(QIcon(":/Icons/refresh.png")));
      rssmanager.refreshAll();
    }

    //right-click, register a new stream
    void RSSImp::createStream() {
      bool ok;
      QString newUrl = QInputDialog::getText(this, tr("Please type a rss stream url"), tr("Stream URL:"), QLineEdit::Normal, "http://", &ok);
      if(ok) {
        newUrl = newUrl.trimmed();
        if(!newUrl.isEmpty() && newUrl != "http://"){
          rssmanager.addStream(newUrl);
          refreshStreamList();
        }
      }
    }

    // fills the streamList
    void RSSImp::refreshStreamList() {
      short currentStream = listStreams->currentRow();
      unsigned short nbstream = rssmanager.getNbStream();
      listStreams->clear();
      for(unsigned short i=0; i<nbstream; i++) {
	new QListWidgetItem(rssmanager.getStream(i)->getAlias()+" ("+QString::number(rssmanager.getStream(i)->getListSize(),10).toUtf8()+")", listStreams);
      }
      if(currentStream>=0 && currentStream<nbstream) {
	listStreams->setCurrentRow(currentStream);
	listNews->clear();
        refreshNewsList();
      }
      //updateAllStreamsName();
    }

    // fills the newsList
    void RSSImp::refreshNewsList() {
      if(rssmanager.getNbStream()>0) {
	RssStream* currentstream = rssmanager.getStream(listStreams->currentRow());
	listNews->clear();
        unsigned short currentStreamSize = currentstream->getListSize();
	for(unsigned short i=0; i<currentStreamSize; ++i) {
	  new QListWidgetItem(currentstream->getItem(i)->getTitle(), listNews);
	  if(currentstream->getItem(i)->isRead())
	     listNews->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("grey")));
	  if(i%2==0)
	    listNews->item(i)->setData(Qt::BackgroundRole, QVariant(QColor(0, 255, 255, 20)));
	}
      }
    }

    // display a news
    void RSSImp::refreshTextBrowser() {
      if(listStreams->currentRow()>=0 && listNews->currentRow()>=0) {
	RssItem* currentitem = rssmanager.getStream(listStreams->currentRow())->getItem(listNews->currentRow());
	textBrowser->setHtml(currentitem->getTitle()+" : \n"+currentitem->getDescription());
	currentitem->setRead();
      }
    }

    // show the number of news for a stream, his status and an icon
    void RSSImp::updateStreamName(const unsigned short& i) {
      unsigned short nbitem = rssmanager.getStream(i)->getListSize();      
      listStreams->item(i)->setText(rssmanager.getStream(i)->getAlias()+" ("+QString::number(nbitem,10).toUtf8()+")");
      if(nbitem==0)
	listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("red")));
      else if(rssmanager.getStream(i)->getLastRefreshElapsed()>REFRESH_MAX_LATENCY)
	listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("orange")));
      else
	listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("green")));
      if(!rssmanager.getStream(i)->isRead())
	listStreams->item(i)->setData(Qt::BackgroundRole, QVariant(QColor(0, 255, 0, 20)));
      listStreams->item(i)->setData(Qt::DecorationRole, QVariant(QIcon(":/Icons/rss.png")));
      if(listStreams->currentRow()==i) {
	listNews->clear();
	refreshNewsList();
      }
    }

    // show the number of news for each stream
    /*void RSSImp::updateAllStreamsName() {
      unsigned short nbstream = rssmanager.getNbStream();
      for(unsigned short i=0; i<nbstream; i++) {
	unsigned short nbitem = rssmanager.getStream(i)->getListSize();
	listStreams->item(i)->setText(rssmanager.getStream(i)->getAlias()+" ("+QString::number(nbitem,10).toUtf8()+")");
	if(nbitem==0)
	  listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("red")));
	else if(rssmanager.getStream(i)->getLastRefreshElapsed()>REFRESH_MAX_LATENCY)
	  listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("orange")));
	else
	  listStreams->item(i)->setData(Qt::ForegroundRole, QVariant(QColor("green")));
 	if(!rssmanager.getStream(i)->isRead())
	  listStreams->item(i)->setData(Qt::BackgroundRole, QVariant(QColor(0, 255, 0, 20)));
      }
      int currentStream = listStreams->currentRow();
      if(currentStream>=0 && currentStream<nbstream) {
	listStreams->setCurrentRow(currentStream);
	listNews->clear();
	refreshNewsList();
      }
    }*/

    RSSImp::RSSImp() : QWidget(){
      setupUi(this);
      addStream_button->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/add.png")));
      delStream_button->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/remove.png")));
      refreshAll_button->setIcon(QIcon(QString::fromUtf8(":/Icons/refresh.png")));
      connect(listStreams, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayFinishedListMenu(const QPoint&)));
      connect(actionDelete, SIGNAL(triggered()), this, SLOT(deleteStream()));
      connect(actionRename, SIGNAL(triggered()), this, SLOT(renameStream()));
      connect(actionRefresh, SIGNAL(triggered()), this, SLOT(refreshStream()));
      connect(actionCreate, SIGNAL(triggered()), this, SLOT(createStream()));
      connect(actionRefreshAll, SIGNAL(triggered()), this, SLOT(refreshAllStreams()));
      connect(&rssmanager, SIGNAL(streamNeedRefresh(const unsigned short&)), this, SLOT(updateStreamName(const unsigned short&)));
      refreshStreamList();
      refreshTextBrowser();
      // force the first alias-refresh
      //QTimer::singleShot(10000, this, SLOT(updateAllStreamsName()));
    }

    RSSImp::~RSSImp(){
    }



