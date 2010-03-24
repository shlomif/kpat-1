/*
 *  Copyright (C) 2009-2010 Parker Coates <parker.coates@kdemail.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KABSTRACTCARDDECK_P_H
#define KABSTRACTCARDDECK_P_H

#include "kabstractcarddeck.h"

#include "kcardtheme.h"

class KPixmapCache;

#include <QtCore/QHash>
class QImage;
#include <QtCore/QMutex>
#include <QtCore/QSet>
#include <QtCore/QSizeF>
#include <QtCore/QStringList>
#include <QtCore/QThread>
class QSvgRenderer;


typedef QHash<QString,QPair<QPixmap,QList<KCard*> > > CardPixmapHash;


class RenderingThread : public QThread
{
    Q_OBJECT

public:
    RenderingThread( KAbstractCardDeckPrivate * d, QSize size, const QStringList & elements );
    void run();
    void halt();

signals:
    void renderingDone( const QString & key, const QImage & image );

private:
    KAbstractCardDeckPrivate * const d;
    const QSize m_size;
    const QStringList m_elementsToRender;
    bool m_haltFlag;
    QMutex m_haltMutex;
};


class KAbstractCardDeckPrivate : public QObject
{
    Q_OBJECT

public:
    KAbstractCardDeckPrivate( KAbstractCardDeck * q );
    ~KAbstractCardDeckPrivate();

    QSvgRenderer * renderer();
    QPixmap renderCard( const QString & element );
    QSizeF unscaledCardSize();
    QPixmap requestPixmap( QString elementId, bool immediate );
    void updateCardSize( const QSize & size );
    void deleteThread();

public slots:
    void submitRendering( const QString & key, const QImage & image );
    void cardStartedAnimation( KCard * card );
    void cardStoppedAnimation( KCard * card );


public:
    KAbstractCardDeck * q;

    QSizeF originalCardSize;
    QSize currentCardSize;

    QList<KCard*> cards;
    QSet<KCard*> cardsWaitedFor;

    KCardTheme theme;
    KPixmapCache * cache;
    QSvgRenderer * svgRenderer;
    QMutex rendererMutex;
    RenderingThread * thread;

    CardPixmapHash elementIdMapping;
};

#endif