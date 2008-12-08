/*****************************************************************************
 * interface_widgets.hpp : Custom widgets for the main interface
 ****************************************************************************
 * Copyright (C) 2006 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *          Rafaël Carré <funman@videolanorg>
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

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_interface.h>

#include "qt4.hpp"
#include "main_interface.hpp"
#include "input_manager.hpp"

#include <QWidget>
#include <QFrame>
#include <QToolButton>

class QPixmap;
class QLabel;
class QGridLayout;

class InputSlider;
class QAbstractSlider;

class QAbstractButton;

class VolumeClickHandler;
class QSignalMapper;

class QTimer;

typedef enum buttonType_e
{
    PLAY_BUTTON,
    PAUSE_BUTTON,
    STOP_BUTTON,
    OPEN_BUTTON,
    PREVIOUS_BUTTON,
    NEXT_BUTTON,
    SLOWER_BUTTON,
    FASTER_BUTTON,
    FULLSCREEN_BUTTON,
    DEFULLSCREEN_BUTTON,
    EXTENDED_BUTTON,
    PLAYLIST_BUTTON,
    SNAPSHOT_BUTTON,
    RECORD_BUTTON,
    ATOB_BUTTON,
    FRAME_BUTTON,
    INPUT_SLIDER,
    VOLUME_SLIDER,
    MENU_BUTTONS,
    TELETEXT_BUTTONS,
    VOLUME,
    WIDGET_SPACER,
    WIDGET_SPACER_EXTEND,
    TIME_LABEL,
    SPLITTER,
    ADVANCED_CONTROLLER,
    REVERSE_BUTTON,
} buttonType_e;

typedef enum actionType_e
{
    PLAY_ACTION,
    PAUSE_ACTION,
    STOP_ACTION,
    PREVIOUS_ACTION,
    NEXT_ACTION,
    SLOWER_ACTION,
    FASTER_ACTION,
    FULLSCREEN_ACTION,
    EXTENDED_ACTION,
    PLAYLIST_ACTION,
    SNAPSHOT_ACTION,
    RECORD_ACTION,
    FRAME_ACTION,
    ATOB_ACTION,
    REVERSE_ACTION,
} actionType_e;

enum
{
   WIDGET_NORMAL = 0x0,
   WIDGET_FLAT   = 0x1,
   WIDGET_BIG    = 0x2,
   WIDGET_SHINY  = 0x4,
};

class AbstractController : public QFrame
{
    Q_OBJECT
public:
    AbstractController( intf_thread_t  *_p_i );
    int getWidth() { return controlLayout->columnCount(); }

protected:
    intf_thread_t       *p_intf;

    QSignalMapper       *toolbarActionsMapper;
    QGridLayout         *controlLayout;

    AdvControlsWidget   *advControls;

    void parseAndCreateLine( QString config, int i_line );

private:
    QWidget *createWidget( buttonType_e, int *i_size,
                           int options = WIDGET_NORMAL );
    void setupButton( QAbstractButton * );
    QWidget *discFrame();
    QWidget *telexFrame();

private slots:
    void doAction( int );

protected slots:
    void play();
    void stop();
    void prev();
    void next();
    void fullscreen();
    void extSettings();
    void faster();
    void slower();
    void reverse();
    void playlist();
    void snapshot();
    void record();
    void frame();

    virtual void setStatus( int );

signals:
    void inputExists( bool ); /// This might be usefull in the IM ?
    void inputPlaying( bool ); /// This might be usefull in the IM ?
    void inputIsRecordable( bool ); /// same ?
};

/**
 * SPECIAL Widgets that are a bit more than just a ToolButton
 * and have an icon/behaviour that changes depending on the context:
 * - playButton
 * - A->B Button
 * - Teletext group buttons
 * - Sound Widget group
 **/
class PlayButton : public QToolButton
{
    Q_OBJECT
private slots:
    void updateButton( bool );
};

class AtoB_Button : public QToolButton
{
    Q_OBJECT
private slots:
    void setIcons( bool, bool );
};

class TeletextController : public QWidget
{
    Q_OBJECT
    friend class AbstractController;
private:
    QToolButton         *telexTransparent, *telexOn;
    QSpinBox            *telexPage;

private slots:
    void enableTeletextButtons( bool );
    void toggleTeletextTransparency( bool );
};

class SoundWidget : public QWidget
{
    Q_OBJECT
    friend class VolumeClickHandler;

public:
    SoundWidget( intf_thread_t  *_p_i, bool );

private:
    intf_thread_t       *p_intf;
    QLabel              *volMuteLabel;
    QAbstractSlider     *volumeSlider;
    VolumeClickHandler  *hVolLabel;
    bool                 b_my_volume;

protected slots:
    void updateVolume( int );
    void updateVolume( void );
};

/* Advanced Button Bar */
class AdvControlsWidget : public AbstractController
{
    Q_OBJECT
public:
    AdvControlsWidget( intf_thread_t * );
};

/* Slider Bar */
class InputControlsWidget : public AbstractController
{
    Q_OBJECT
public:
    InputControlsWidget( intf_thread_t * );
};

/* Button Bar */
class ControlsWidget : public AbstractController
{
    Q_OBJECT
public:
    /* p_intf, advanced control visible or not, blingbling or not */
    ControlsWidget( intf_thread_t *_p_i, bool b_advControls );
    virtual ~ControlsWidget();

protected:
    friend class MainInterface;

    bool                 b_advancedVisible;

protected slots:
    void toggleAdvanced();

signals:
    void advancedControlsToggled( bool );
};

/* on WIN32 hide() for fullscreen controller doesnt work, so it have to be
   done by trick with setting the opacity of window */
#ifdef WIN32
    #define WIN32TRICK
#endif

/* to trying transparency with fullscreen controller on windows enable that */
/* it can be enabled on-non windows systems,
   but it will be transparent only with composite manager */
#ifndef WIN32
    #define HAVE_TRANSPARENCY 1
#else
    #define HAVE_TRANSPARENCY 0
#endif

/* Default value of opacity for FS controller */
#define DEFAULT_OPACITY 0.75

/***********************************
 * Fullscreen controller
 ***********************************/
class FullscreenControllerWidget : public AbstractController
{
    Q_OBJECT
public:
    FullscreenControllerWidget( intf_thread_t * );
    virtual ~FullscreenControllerWidget();

    /* Vout */
    vout_thread_t *p_vout;
    void attachVout( vout_thread_t *p_vout );
    void detachVout();
    void fullscreenChanged( vout_thread_t *, bool b_fs, int i_timeout );

    int i_mouse_last_move_x;
    int i_mouse_last_move_y;

protected:
    friend class MainInterface;

    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void enterEvent( QEvent *event );
    virtual void leaveEvent( QEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );

private slots:
    void showFSC();
    void planHideFSC();
    void hideFSC();
    void slowHideFSC();

private:
    virtual void customEvent( QEvent *event );

    QTimer *p_hideTimer;
#if HAVE_TRANSPARENCY
    QTimer *p_slowHideTimer;
    bool b_slow_hide_begin;
    int  i_slow_hide_timeout;
#endif

    int i_mouse_last_x, i_mouse_last_y;
    bool b_mouse_over;
    int i_screennumber;

#ifdef WIN32TRICK
    bool b_fscHidden;
#endif

    /* Shared variable between FSC and VLC (protected by a lock) */
    vlc_mutex_t lock;
    bool        b_fullscreen;
    int         i_hide_timeout;  /* FSC hiding timeout, same as mouse hiding timeout */
};

#endif