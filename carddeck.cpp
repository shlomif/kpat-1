/*
 * Copyright (C) 1995 Paul Olav Tvete <paul@troll.no>
 * Copyright (C) 2000-2009 Stephan Kulow <coolo@kde.org>
 * Copyright (C) 2009 Parker Coates <parker.coates@gmail.com>
 *
 * License of original code:
 * -------------------------------------------------------------------------
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 *   This file is provided AS IS with no warranties of any kind.  The author
 *   shall have no liability with respect to the infringement of copyrights,
 *   trade secrets or any patents by this file or any part thereof.  In no
 *   event will the author be liable for any lost revenue or profits or
 *   other special, indirect and consequential damages.
 * -------------------------------------------------------------------------
 *
 * License of modifications/additions made after 2009-01-01:
 * -------------------------------------------------------------------------
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------------
 */

#include "carddeck.h"

#include "dealer.h"
#include "version.h"

#include <KCardDeckInfo>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KSharedConfig>

class CardDeckPrivate
{
public:
    CardDeck instance;
};

K_GLOBAL_STATIC( CardDeckPrivate, cdp )


CardDeck::CardDeck()
  : m_originalCardSize(1, 1),
    m_currentCardSize(0, 0)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup cs(config, settings_group );
    updateTheme(cs);
    setCardWidth( cs.readEntry( "CardWidth", 100 ) );
}


CardDeck::~CardDeck()
{
    clear();
}


CardDeck * CardDeck::self()
{
    return &(cdp->instance);
}


void CardDeck::setDeckType( int copies, QList<Card::Suit> suits, QList<Card::Rank> ranks )
{
    Q_ASSERT( copies >= 1 );
    Q_ASSERT( suits.size() >= 1 );
    Q_ASSERT( ranks.size() >= 1 );

    // Delete current cards
    clear();

    // Note the order the cards are created in can't be changed as doing so
    // will mess up the game numbering.
    for ( int i = 0; i < copies; ++i )
        foreach ( Card::Rank r, ranks )
            foreach ( Card::Suit s, suits )
                m_allCards << new Card( r, s );

    m_undealtCards = m_allCards;
}


QList< Card* > CardDeck::cards() const
{
    return m_allCards;
}


bool CardDeck::hasUndealtCards()
{
    return !m_undealtCards.isEmpty();
}


Card* CardDeck::takeCard()
{
    if (m_undealtCards.isEmpty())
        return 0;

    return m_undealtCards.takeLast();
}


void CardDeck::takeAllCards( Pile * p )
{
    while (!m_undealtCards.isEmpty())
        p->add( m_undealtCards.takeFirst(), true);
}


void CardDeck::returnCard( Card * c )
{
    c->setTakenDown(false);
    if (c->source())
        c->source()->remove(c);
    if (c->scene())
        c->scene()->removeItem(c);
    m_undealtCards.append(c);
}


void CardDeck::returnAllCards()
{
    m_undealtCards.clear();
    foreach ( Card * c, m_allCards )
        returnCard( c );
}


void CardDeck::clear()
{
    returnAllCards();
    qDeleteAll( m_allCards );
    m_allCards.clear();
    m_undealtCards.clear();
}


// Shuffle all undealt cards
void CardDeck::shuffle( int gameNumber )
{
    Q_ASSERT(gameNumber >= 0);
    m_pseudoRandomSeed = gameNumber;

    Card* t;
    int z;
    int left = m_undealtCards.size();
    for (int i = 0; i < m_undealtCards.size(); i++)
    {
        z = pseudoRandom() % left;
        t = m_undealtCards[z];
        m_undealtCards[z] = m_undealtCards[left-1];
        m_undealtCards[left-1] = t;
        left--;
    }
}


void CardDeck::setCardWidth( int width )
{
    if ( width > 200 || width < 20 )
        return;

    int height = width * m_originalCardSize.height() / m_originalCardSize.width();
    QSize newSize( width, height );

    if ( newSize != m_currentCardSize )
    {
        KConfigGroup cs( KGlobal::config(), settings_group );
        cs.writeEntry( "CardWidth", m_currentCardSize.width() );

        m_currentCardSize = newSize;
        m_cache.setSize( newSize );
        foreach (Card * c, m_allCards)
            c->updatePixmap();

        QTimer::singleShot( 200, this, SLOT(loadInBackground()) );;
    }
}


int CardDeck::cardWidth() const
{
    return m_currentCardSize.width();
}


void CardDeck::setCardHeight( int height )
{
    setCardWidth( height * m_originalCardSize.width() / m_originalCardSize.height() );
}


int CardDeck::cardHeight() const
{
    return m_currentCardSize.height();
}


QSize CardDeck::cardSize() const
{
    return m_currentCardSize;
}


QPixmap CardDeck::frontsidePixmap( AbstractCard::Rank r, AbstractCard::Suit s )
{
    KCardInfo::Suit suit;
    switch ( s )
    {
        case AbstractCard::Clubs :    suit = KCardInfo::Club;    break;
        case AbstractCard::Spades :   suit = KCardInfo::Spade;   break;
        case AbstractCard::Diamonds : suit = KCardInfo::Diamond; break;
        case AbstractCard::Hearts :   suit = KCardInfo::Heart;   break;
    }

    KCardInfo::Card rank;
    switch ( r )
    {
        case AbstractCard::Ace :   rank = KCardInfo::Ace;   break;
        case AbstractCard::Two :   rank = KCardInfo::Two;   break;
        case AbstractCard::Three : rank = KCardInfo::Three; break;
        case AbstractCard::Four :  rank = KCardInfo::Four;  break;
        case AbstractCard::Five :  rank = KCardInfo::Five;  break;
        case AbstractCard::Six :   rank = KCardInfo::Six;   break;
        case AbstractCard::Seven : rank = KCardInfo::Seven; break;
        case AbstractCard::Eight : rank = KCardInfo::Eight; break;
        case AbstractCard::Nine :  rank = KCardInfo::Nine;  break;
        case AbstractCard::Ten :   rank = KCardInfo::Ten;   break;
        case AbstractCard::Jack :  rank = KCardInfo::Jack;  break;
        case AbstractCard::Queen : rank = KCardInfo::Queen; break;
        case AbstractCard::King :  rank = KCardInfo::King;  break;
    }

    return m_cache.frontside( KCardInfo( suit, rank ) );
}


QPixmap CardDeck::backsidePixmap( int variant )
{
    return m_cache.backside( variant );
}


void CardDeck::updateTheme(const KConfigGroup &cs)
{
    QString fronttheme = CardDeckInfo::frontTheme( cs );
    QString backtheme = CardDeckInfo::backTheme( cs );
    Q_ASSERT ( !backtheme.isEmpty() );
    Q_ASSERT ( !fronttheme.isEmpty() );

    m_cache.setFrontTheme( fronttheme );
    m_cache.setBackTheme( backtheme );

    m_originalCardSize = m_cache.defaultFrontSize( KCardInfo( KCardInfo::Spade, KCardInfo::Ace ) );
    Q_ASSERT( !m_originalCardSize.isNull() );
    m_currentCardSize = m_originalCardSize.toSize();
}


void CardDeck::loadInBackground()
{
    m_cache.loadTheme( KCardCache::LoadFrontSide | KCardCache::Load52Cards );
}


// KPat uses the same pseudorandom number generation algorithm as Windows
// Freecell, so that game numbers are the same between the two applications.
// For more inforation, see 
// http://support.microsoft.com/default.aspx?scid=kb;EN-US;Q28150
int CardDeck::pseudoRandom() {
    m_pseudoRandomSeed = 214013 * m_pseudoRandomSeed + 2531011;
    return (m_pseudoRandomSeed >> 16) & 0x7fff;
}

#include "carddeck.moc"