/*---------------------------------------------------------------------------

  mod3.cpp  implements a patience card game

     Copyright (C) 1997  Rodolfo Borges

 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.

---------------------------------------------------------------------------*/

#include <qdialog.h>
#include "mod3.h"
#include <klocale.h>
#include <kmessagebox.h>
#include "deck.h"
#include "pile.h"
#include <kdebug.h>
#include <kmainwindow.h>
#include <kaction.h>

//-------------------------------------------------------------------------//

Mod3::Mod3( KMainWindow* parent, const char* _name)
        : Dealer( parent, _name )
{
    deck = new Deck( 0, this, 2);
    deck->hide();

    aces = new Pile(40, this);
    aces->hide();
    aces->setTarget(true);

    for( int r = 0; r < 4; r++ ) {
        for( int c = 0; c < 8; c++ ) {
            stack[ r ][ c ] = new Pile ( r + 1, this );
            stack[r][c]->move( 8 + 80 * c, 8 + 105 * r + 32 * ( r == 3 ));
            if( r < 3 ) {
                stack[r][c]->setCheckIndex( 0 );
                stack[r][c]->setTarget(true);
            } else {
                stack[r][c]->setAddFlags( Pile::addSpread );
                stack[r][c]->setCheckIndex( 1 );
            }
        }
    }

    setTakeTargetForHints(true);
    setActions(Dealer::Hint | Dealer::Redeal | Dealer::Demo );
}

//-------------------------------------------------------------------------//

bool Mod3::checkAdd( int checkIndex, const Pile *c1, const CardList& cl) const
{
    if (checkIndex == 0) {
        Card *c2 = cl.first();

        if (c1->isEmpty())
            return (c2->value() == (c1->index()+1));

        if (c1->top()->suit() != c2->suit())
            return false;

        if (c2->value() != (c1->top()->value()+3))
            return false;

        if (c1->cardsLeft() == 1)
            return (c1->top()->value() == (c1->index()+1));

        return true;
    } else if (checkIndex == 1) {
        return c1->isEmpty();
    } else return false;
}

//-------------------------------------------------------------------------//

void Mod3::restart()
{
    deck->collectAndShuffle();
    deal();
}

//-------------------------------------------------------------------------//

void Mod3::dealRow(int row)
{
    if (deck->isEmpty())
        return;

    for (int c = 0; c < 8; c++)
    {
        Card *card;

        bool foundone = false;

        do {
            card = deck->nextCard();
            if (card->value() != Card::Ace)
                foundone = true;
            else
                aces->add(card);
        } while (!foundone);

        stack[row][c]->add (card, false, true);
    }
}

void Mod3::redeal()
{
    unmarkAll();

    if (deck->isEmpty()) {
        KMessageBox::information(this, i18n("No more cards"));
        return;
    }
    dealRow(3);
    aredeal->setEnabled( !deck->isEmpty() );
    takeState();
}

void Mod3::setGameState( QDataStream & )
{
    aredeal->setEnabled( !deck->isEmpty() );
}

//-------------------------------------------------------------------------//

void Mod3::deal()
{
    unmarkAll();

    for (int r = 0; r < 4; r++)
        dealRow(r);

    aredeal->setEnabled(true);
}

Card *Mod3::demoNewCards()
{
   if (deck->isEmpty())
       return 0;
   redeal();
   return stack[3][0]->top();
}

static class LocalDealerInfo5 : public DealerInfo
{
public:
    LocalDealerInfo5() : DealerInfo(I18N_NOOP("M&od3"), 5) {}
    virtual Dealer *createGame(KMainWindow *parent) { return new Mod3(parent); }
} ldi5;

//-------------------------------------------------------------------------//

#include"mod3.moc"

//-------------------------------------------------------------------------//

