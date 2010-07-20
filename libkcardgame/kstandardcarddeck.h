/*
 *  Copyright (C) 2010 Parker Coates <parker.coates@kdemail.org>
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

#ifndef KSTANDARDCARDDECK_H
#define KSTANDARDCARDDECK_H

#include "kabstractcarddeck.h"
#include "libkcardgame_export.h"


class LIBKCARDGAME_EXPORT KStandardCardDeck : public KAbstractCardDeck
{
public:
    enum Suit
    {
        NoSuit = -1,
        Clubs = 0,
        Diamonds = 1,
        Hearts = 2,
        Spades = 3
    };

    enum Rank
    {
        NoRank = 0,
        Ace = 1,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Jack,
        Queen,
        King
    };

    static QList<Suit> standardSuits();
    static QList<Rank> standardRanks();

    explicit KStandardCardDeck( const KCardTheme & theme = KCardTheme(), QObject * parent = 0 );
    virtual ~KStandardCardDeck();

    void setDeckContents( int copies = 1,
                          QList<Suit> suits = standardSuits(),
                          QList<Rank> ranks = standardRanks() );

protected:
    virtual QString elementName( quint32 id, bool faceUp = true ) const;

private:
    class KStandardCardDeckPrivate * const d;
};

LIBKCARDGAME_EXPORT KStandardCardDeck::Suit getSuit( const KCard * card );
LIBKCARDGAME_EXPORT KStandardCardDeck::Rank getRank( const KCard * card );
LIBKCARDGAME_EXPORT bool getIsRed( const KCard * card );

#endif