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

#ifndef KABSTRACTCARDDECK_H
#define KABSTRACTCARDDECK_H

#include "libkcardgame_export.h"
class KCard;
class KCardPile;
#include "kcardtheme.h"

#include <QtCore/QObject>
class QSize;
class QPainter;


class LIBKCARDGAME_EXPORT KAbstractCardDeck : public QObject
{
    Q_OBJECT

public:
    explicit KAbstractCardDeck( const KCardTheme & theme = KCardTheme(), QObject * parent = 0 );
    virtual ~KAbstractCardDeck();

    void setDeckContents( QList<quint32> ids );
    QList<KCard*> cards() const;

    void setCardWidth( int width );
    int cardWidth() const;
    void setCardHeight( int height );
    int cardHeight() const;
    QSize cardSize() const;

    void setTheme( const KCardTheme & theme );
    KCardTheme theme() const;

    bool hasAnimatedCards() const;

Q_SIGNALS:
    void cardAnimationDone();

protected:
    virtual QString elementName( quint32 id, bool faceUp = true ) const = 0;

private:
    virtual void paintCard( QPainter * painter, quint32 id, bool faceUp, qreal highlightedness );

private:
    class KAbstractCardDeckPrivate * const d;

    friend class KCard;
    friend class KAbstractCardDeckPrivate;
};

#endif