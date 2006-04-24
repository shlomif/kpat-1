#include "gamestatsimpl.h"
#include "dealer.h"
#include "version.h"

#include <qcombobox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QList>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

GameStatsImpl::GameStatsImpl(QWidget* aParent)
	: QDialog(aParent)
{
	setupUi(this);
	QStringList list;
	QList<DealerInfo*>::ConstIterator it;
	for (it = DealerInfoList::self()->games().begin();
			it != DealerInfoList::self()->games().end(); ++it)
	{
		// while we develop, it may happen that some lower
		// indices do not exist
		int index = (*it)->gameindex;
		for (int i = 0; i <= index; i++)
			if (list.count() <= i)
				list.append("unknown");
		list[index] = i18n((*it)->name);
		list[index].replace('&',"");
	}
	GameType->addItems(list);
	showGameType(0);
	connect(buttonOk, SIGNAL(clicked(bool)), SLOT(accept()));
}

void GameStatsImpl::showGameType(int id)
{
	GameType->setCurrentIndex(id);
	setGameType(id);
}

void GameStatsImpl::setGameType(int id)
{
	KConfigGroup cg(KGlobal::config(), scores_group);
	unsigned int t = cg.readEntry(QString("total%1").arg(id),0);
	Played->setText(Played->text().arg(t));
	unsigned int w = cg.readEntry(QString("won%1").arg(id),0);
	Won->setText(i18n("%1 (%2%%)", w));
	if (t)
		WonPerc->setText(i18n("%1 (%2%%)", w, w*100/t));
	else
		WonPerc->setText(i18n("%1", w));
	WinStreak->setText(
		i18n("%1", cg.readEntry(QString("maxwinstreak%1").arg(id), 0)));
	LooseStreak->setText(
		i18n("%1", cg.readEntry(QString("maxloosestreak%1").arg(id), 0)));
}
