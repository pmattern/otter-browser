/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 Piotr Wójcik <chocimier@tlen.pl>
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

#ifndef OTTER_PREFERENCESDIALOG_H
#define OTTER_PREFERENCESDIALOG_H

#include <QtCore/QVariantMap>
#include <QtWidgets/QDialog>

namespace Otter
{

struct ShortcutsProfile;
class ItemViewWidget;

namespace Ui
{
	class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PreferencesDialog(const QLatin1String &section, QWidget *parent = NULL);
	~PreferencesDialog();

protected:
	void changeEvent(QEvent *event);
	QString createProfileIdentifier(ItemViewWidget *view, QString identifier = QString());

protected slots:
	void currentTabChanged(int tab);
	void useCurrentAsHomePage();
	void restoreHomePage();
	void currentFontChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void fontChanged(QWidget *editor);
	void currentColorChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void colorChanged(QWidget *editor);
	void setupClearHistory();
	void manageAcceptLanguage();
	void addSearch();
	void editSearch();
	void updateSearchActions();
	void manageUserAgents();
	void proxyModeChanged(int index);
	void addCipher(QAction *action);
	void removeCipher();
	void updateCiphersActions();
	void addKeyboardProfile();
	void editKeyboardProfile();
	void cloneKeyboardProfile();
	void removeKeyboardProfile();
	void updateKeyboardProfleActions();
	void updateJavaScriptOptions();
	void openConfigurationManager();
	void markModified();
	void save();

private:
	QString m_defaultSearch;
	QStringList m_clearSettings;
	QStringList m_removedShortcutsProfiles;
	QVector<bool> m_loadedTabs;
	QVariantMap m_javaScriptOptions;
	QHash<QString, ShortcutsProfile> m_shortcutsProfiles;
	bool m_userAgentsModified;
	Ui::PreferencesDialog *m_ui;
};

}

#endif
