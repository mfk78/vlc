/*****************************************************************************
 * ToolbarEdit.hpp : ToolbarEdit dialogs
 ****************************************************************************
 * Copyright (C) 2007 the VideoLAN team
 * $Id$
 *
 * Authors: Jean-Baptiste Kempf <jb (at) videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef _TOOLBAREDIT_DIALOG_H_
#define _TOOLBAREDIT_DIALOG_H_

#include "util/qvlcframe.hpp"
#include "components/controller.hpp"

#include <QRubberBand>
#include <QListWidget>
#include <QCheckBox>

#define PROFILE_NAME_1 "Modern Style"
#define VALUE_1 "0|64;39;64;38;65;|0-2;64;3;1;4;64;7;10;9;64-4;37;65;35-4;|12;11;13;14;|5-1;33;6-1;|0-2;64;3;1;4;64;37;64;38;64;8;65;35-4;34;"
#define PROFILE_NAME_2 "Classic Style"
#define VALUE_2 "1|64;39-1;64;38;13-1;|2-1;32-4;0-5;1-5;32-5;3-5;5-5;6-5;4-5;32-5;10-1;64-1;35-1;65;|12-1;11-1;13-1;14-1;|33;37-4;|0-5;1-5;32-1;1-5;5-1;6-1;4-5;32-1;12-5;11-1;65;34-4;35-1;"
#define PROFILE_NAME_3 "Minimalist Style"
#define VALUE_3 "0|64;65;|0-7;64;3-1;1-5;4-1;64;12-5;64-5;37-4;38;64-4;10-1;65;36-4;|11;13;13;|5-1;33;6-1;|0-5;64;3-5;1-5;4-5;64;12-5;65;34-4;35-1;"
#define PROFILE_NAME_4 "One-Liner Style"
#define VALUE_4 "0|64;38;65;|0-4;64;3;1;4;64;7;10;9;64-4;39;64-4;37;65;36-4;|12;11;14;13;|5-1;33;6-1;|0-2;64;3;1;4;64;37;64;38;64;8;65;35-4;34;"

class ToolbarEditDialog;
class DroppingController;
class QCheckBox;
class QComboBox;

class WidgetListing : public QListWidget
{
    Q_OBJECT;
public:
    WidgetListing( intf_thread_t *, QWidget *_parent = 0 );
protected:
    virtual void startDrag( Qt::DropActions /*supportedActions*/ );
private:
    ToolbarEditDialog *parent;
};

class ToolbarEditDialog : public QVLCFrame
{
    Q_OBJECT;
public:
    static ToolbarEditDialog * getInstance( intf_thread_t *p_intf )
    {
        if( !instance)
            instance = new ToolbarEditDialog( p_intf );
        return instance;
    }
    static void killInstance()
    { if( instance ) delete instance; instance = NULL;}
    virtual ~ToolbarEditDialog();
    int getOptions() { return flatBox->isChecked() * WIDGET_FLAT +
                        bigBox->isChecked() * WIDGET_BIG +
                        !shinyBox->isChecked() * WIDGET_SHINY; }
private:
    ToolbarEditDialog( intf_thread_t * );
    static ToolbarEditDialog *instance;

    QCheckBox *flatBox, *bigBox, *shinyBox;
    QComboBox *positionCombo, *profileCombo;

    WidgetListing *widgetListing;
    DroppingController *controller1, *controller2, *controllerA;
    DroppingController *controllerFSC, *controller;

private slots:
    void newProfile();
    void deleteProfile();
    void changeProfile( int );
    void cancel();
    void close();
};

class DroppingController: public AbstractController
{
    Q_OBJECT;
public:
    DroppingController( intf_thread_t *, QString line, QWidget *parent = 0 );
    QString getValue();
    virtual ~DroppingController();

    void resetLine( QString );
protected:
    virtual void createAndAddWidget( QBoxLayout *controlLayout, int i_index,
            buttonType_e i_type, int i_option );
    virtual void dragEnterEvent ( QDragEnterEvent * event );
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent ( QDropEvent * event );
    virtual void dragLeaveEvent ( QDragLeaveEvent * event );

    virtual void doAction( int );

    bool eventFilter( QObject *, QEvent * );
private:
    struct doubleInt
    {
        int i_type;
        int i_option;
    };
    QRubberBand *rubberband;
    QList <doubleInt *> widgetList;

    int getParentPosInLayout( QPoint point);

    bool b_draging;

};

#endif
