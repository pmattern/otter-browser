/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2014 - 2015 Piotr Wójcik <chocimier@tlen.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "SidebarWidget.h"
#include "MainWindow.h"
#include "../core/SettingsManager.h"
#include "../core/Utils.h"
#include "../modules/windows/bookmarks/BookmarksContentsWidget.h"
#include "../modules/windows/cache/CacheContentsWidget.h"
#include "../modules/windows/configuration/ConfigurationContentsWidget.h"
#include "../modules/windows/cookies/CookiesContentsWidget.h"
#include "../modules/windows/history/HistoryContentsWidget.h"
#include "../modules/windows/transfers/TransfersContentsWidget.h"
#include "../modules/windows/web/WebContentsWidget.h"

#include "ui_SidebarWidget.h"

#include <QtGui/QIcon>
#include <QtWidgets/QInputDialog>

namespace Otter
{

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent),
	m_currentWidget(NULL),
	m_ui(new Ui::SidebarWidget)
{
	m_ui->setupUi(this);

	m_ui->panelsChooseButton->setPopupMode(QToolButton::InstantPopup);
	m_ui->panelsChooseButton->setIcon(Utils::getIcon(QLatin1String("configure")));

	optionChanged(QLatin1String("Sidebar/CurrentPanel"), SettingsManager::getValue(QLatin1String("Sidebar/CurrentPanel")));
	optionChanged(QLatin1String("Sidebar/Panels"), SettingsManager::getValue(QLatin1String("Sidebar/Panels")));

	connect(SettingsManager::getInstance(), SIGNAL(valueChanged(QString,QVariant)), this, SLOT(optionChanged(QString,QVariant)));
}

SidebarWidget::~SidebarWidget()
{
	delete m_ui;
}

void SidebarWidget::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);

			for (QHash<QString, QToolButton*>::iterator iterator = m_buttons.begin(); iterator != m_buttons.end(); ++iterator)
			{
				iterator.value()->setToolTip(translate(iterator.key()));
			}

			if (m_ui->panelsChooseButton->menu())
			{
				QList<QAction*> actions = m_ui->panelsChooseButton->menu()->actions();

				for (int i = 0; i < actions.count(); ++i)
				{
					if (!actions[i]->data().toString().isEmpty() && !actions[i]->data().toString().startsWith(QLatin1String("web:")))
					{
						actions[i]->setText(translate(actions[i]->data().toString()));
					}
				}
			}

			break;
		default:
			break;
	}
}

void SidebarWidget::optionChanged(const QString &option, const QVariant &value)
{
	if (option == QLatin1String("Sidebar/CurrentPanel"))
	{
		openPanel(value.toString());
	}
	else if (option == QLatin1String("Sidebar/Panels"))
	{
		qDeleteAll(m_buttons.begin(), m_buttons.end());

		m_buttons.clear();

		const QStringList panels = value.toStringList();

		for (int i = 0; i < panels.count(); ++i)
		{
			registerPanel(panels.at(i));
		}

		updatePanelsMenu();
	}
}

void SidebarWidget::addWebPanel()
{
	WindowsManager *manager = SessionsManager::getWindowsManager();
	QString url;

	if (manager)
	{
		url = manager->getUrl().toString(QUrl::RemovePassword);
	}

	url = QInputDialog::getText(this, tr("Add web panel"), tr("Input address of web page to show in panel:"), QLineEdit::Normal, url);

	if (!url.isEmpty())
	{
		url = QLatin1String("web:") + url;

		SettingsManager::setValue(QLatin1String("Sidebar/Panels"), SettingsManager::getValue(QLatin1String("Sidebar/Panels")).toStringList() << url);
	}
}

void SidebarWidget::choosePanel(bool checked)
{
	QAction *action = qobject_cast<QAction*>(sender());

	if (!action)
	{
		return;
	}

	QStringList chosenPanels = SettingsManager::getValue(QLatin1String("Sidebar/Panels")).toStringList();

	if (checked)
	{
		chosenPanels.append(action->data().toString());
	}
	else
	{
		chosenPanels.removeAll(action->data().toString());
	}

	SettingsManager::setValue(QLatin1String("Sidebar/Panels"), chosenPanels);
}

void SidebarWidget::openPanel()
{
	QAction *action = qobject_cast<QAction*>(sender());

	if (action)
	{
		SettingsManager::setValue(QLatin1String("Sidebar/CurrentPanel"), ((action->data().toString() == m_currentPanel) ? QString() : action->data().toString()));
	}
}

void SidebarWidget::openPanel(const QString &identifier)
{
	QWidget *widget = NULL;

	if (identifier.isEmpty())
	{
		widget = NULL;
	}
	else if (identifier == QLatin1String("bookmarks"))
	{
		widget = new BookmarksContentsWidget(NULL);
	}
	else if (identifier == QLatin1String("cache"))
	{
		widget = new CacheContentsWidget(NULL);
	}
	else if (identifier == QLatin1String("config"))
	{
		widget = new ConfigurationContentsWidget(NULL);
	}
	else if (identifier == QLatin1String("cookies"))
	{
		widget = new CookiesContentsWidget(NULL);
	}
	else if (identifier == QLatin1String("history"))
	{
		widget = new HistoryContentsWidget(NULL);
	}
	else if (identifier == QLatin1String("transfers"))
	{
		widget = new TransfersContentsWidget(NULL);
	}
	else if (identifier.startsWith(QLatin1String("web:")))
	{
		WebContentsWidget *webWidget = new WebContentsWidget(true, NULL, NULL);
		webWidget->setUrl(identifier.section(QLatin1Char(':'), 1, -1), false);

		widget = webWidget;
	}

	if (qobject_cast<ContentsWidget*>(widget))
	{
		connect(widget, SIGNAL(requestedOpenUrl(QUrl,OpenHints)), this, SLOT(openUrl(QUrl,OpenHints)));
	}

	if (m_currentWidget)
	{
		QWidget *currentWidget = m_currentWidget;

		m_currentWidget = NULL;

		layout()->removeWidget(currentWidget);

		currentWidget->deleteLater();
	}

	if (m_buttons.contains(m_currentPanel) && m_buttons[m_currentPanel])
	{
		m_buttons[m_currentPanel]->setChecked(false);
	}

	if (widget)
	{
		layout()->addWidget(widget);

		if (m_buttons.contains(identifier) && m_buttons[identifier])
		{
			m_buttons[identifier]->setChecked(true);
		}
	}

	m_currentPanel = identifier;
	m_currentWidget = widget;
}

void SidebarWidget::openUrl(const QUrl &url, OpenHints hints)
{
	MainWindow *window = MainWindow::findMainWindow(parent());

	if (window)
	{
		window->getWindowsManager()->open(url, hints);
	}
}

void SidebarWidget::registerPanel(const QString &identifier)
{
	QString title;
	QIcon icon;

	if (identifier == QLatin1String("bookmarks"))
	{
		icon = Utils::getIcon(QLatin1String("bookmarks"));
	}
	else if (identifier == QLatin1String("cache"))
	{
		icon  = Utils::getIcon(QLatin1String("cache"));
	}
	else if (identifier == QLatin1String("config"))
	{
		icon  = Utils::getIcon(QLatin1String("configuration"));
	}
	else if (identifier == QLatin1String("cookies"))
	{
		icon  = Utils::getIcon(QLatin1String("cookies"));
	}
	else if (identifier == QLatin1String("history"))
	{
		icon  = Utils::getIcon(QLatin1String("view-history"));
	}
	else if (identifier == QLatin1String("transfers"))
	{
		icon  = Utils::getIcon(QLatin1String("transfers"));
	}
	else
	{
		icon  = Utils::getIcon(QLatin1String("text-html"));
	}

	QToolButton *button = new QToolButton(this);
	button->setIcon(icon);
	button->setToolTip(translate(identifier));
	button->setCheckable(true);
	button->setAutoRaise(true);

	QAction *action = new QAction(button);
	action->setData(identifier);

	m_ui->buttonsLayout->insertWidget(qMax(0, (m_ui->buttonsLayout->count() - 2)), button);

	m_buttons.insert(identifier, button);

	connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
	connect(action, SIGNAL(triggered()), this, SLOT(openPanel()));
}

void SidebarWidget::setButtonsEdge(Qt::Edge edge)
{
	qobject_cast<QBoxLayout*>(layout())->setDirection((edge == Qt::RightEdge) ?  QBoxLayout::RightToLeft : QBoxLayout::LeftToRight);
}

void SidebarWidget::updatePanelsMenu()
{
	QMenu *menu = new QMenu(m_ui->panelsChooseButton);
	const QStringList chosenPanels = SettingsManager::getValue(QLatin1String("Sidebar/Panels")).toStringList();
	QStringList allPanels;
	allPanels << QLatin1String("bookmarks") << QLatin1String("history") << QLatin1String("transfers") << QLatin1String("cache") << QLatin1String("cookies") << QLatin1String("config");

	for (int i = 0; i < allPanels.count(); ++i)
	{
		QAction *action = new QAction(menu);
		action->setCheckable(true);
		action->setChecked(chosenPanels.contains(allPanels[i]));
		action->setData(allPanels[i]);
		action->setText(translate(allPanels[i]));

		connect(action, SIGNAL(toggled(bool)), this, SLOT(choosePanel(bool)));

		menu->addAction(action);
	}

	menu->addSeparator();

	for (int i = 0; i < chosenPanels.count(); ++i)
	{
		if (chosenPanels[i].startsWith(QLatin1String("web:")))
		{
			QAction *action = new QAction(menu);
			action->setCheckable(true);
			action->setChecked(true);
			action->setData(chosenPanels[i]);
			action->setText(chosenPanels[i].section(QLatin1Char(':'), 1, -1));

			connect(action, SIGNAL(toggled(bool)), this, SLOT(choosePanel(bool)));

			menu->addAction(action);
		}
	}


	QAction *addPanelAction = new QAction(menu);
	addPanelAction->setText(tr("Add web panel"));

	connect(addPanelAction, SIGNAL(triggered()), this, SLOT(addWebPanel()));

	menu->addSeparator();
	menu->addAction(addPanelAction);

	if (m_ui->panelsChooseButton->menu())
	{
		m_ui->panelsChooseButton->menu()->deleteLater();
	}

	m_ui->panelsChooseButton->setMenu(menu);
}

QSize SidebarWidget::sizeHint() const
{
	if (m_currentWidget)
	{
		return QSize(SettingsManager::getValue(QLatin1String("Sidebar/Width")).toInt(), m_ui->buttonsLayout->sizeHint().height());
	}
	else
	{
		return m_ui->buttonsLayout->sizeHint();
	}
}

QString SidebarWidget::translate(const QString &identifier)
{
	if (identifier == QLatin1String("bookmarks"))
	{
		return tr("Bookmarks");
	}
	else if (identifier == QLatin1String("history"))
	{
		return tr("History");
	}
	else if (identifier == QLatin1String("transfers"))
	{
		return tr("Transfers");
	}
	else if (identifier == QLatin1String("cache"))
	{
		return tr("Cache");
	}
	else if (identifier == QLatin1String("cookies"))
	{
		return tr("Cookies");
	}
	else if (identifier == QLatin1String("config"))
	{
		return tr("Configuration");
	}
	else
	{
		return identifier;
	}
}

}
