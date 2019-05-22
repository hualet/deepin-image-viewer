/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "slideeffectplayer.h"
#include "application.h"
#include "controller/configsetter.h"
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QTimerEvent>

namespace {

const QString DURATION_SETTING_GROUP = "SLIDESHOWDURATION";
const QString DURATION_SETTING_KEY = "Duration";
const int ANIMATION_DURATION  = 1000;

} // namespace

SlideEffectPlayer::SlideEffectPlayer(QObject *parent)
    : QObject(parent)
{

}

void SlideEffectPlayer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_tid || m_pausing)
        return;
    if (m_effect)
        m_effect->deleteLater();
    if (! startNext()) {
        stop();
    }
}

int SlideEffectPlayer::duration() const
{
    return dApp->setter->value(DURATION_SETTING_GROUP,
                               DURATION_SETTING_KEY).toInt() * 1000;
}

void SlideEffectPlayer::setFrameSize(int width, int height)
{
    m_w = width;
    m_h = height;
}

void SlideEffectPlayer::setImagePaths(const QStringList& paths)
{
    m_paths = paths;
    m_current = m_paths.constBegin();
}

void SlideEffectPlayer::setCurrentImage(const QString &path)
{
    m_current = std::find(m_paths.cbegin(), m_paths.cend(),  path);
    if (m_current == m_paths.constEnd())
        m_current = m_paths.constBegin();
}

QString SlideEffectPlayer::currentImagePath() const
{
    if (m_current == m_paths.constEnd())
        return *m_paths.constBegin();
    return *m_current;
}

bool SlideEffectPlayer::isRunning() const
{
    return m_running;
}

void SlideEffectPlayer::start()
{
    if (m_paths.isEmpty())
        return;

    cacheNext();
    m_running = true;
    m_tid = startTimer(duration());
}

void SlideEffectPlayer::pause() {
    m_pausing = !m_pausing;
    m_effect->pause();
}

bool SlideEffectPlayer::startNext()
{
    if (m_paths.isEmpty())
        return false;
    QSize fSize(m_w, m_h);
    if (! fSize.isValid()) {
        qWarning() << "Invalid frame size!";
        return false;
    }

    const QString oldPath = currentImagePath();

    if (m_paths.length() > 1) {
        m_current ++;
        if (m_current == m_paths.constEnd()) {
            m_current = m_paths.constBegin();
            if (!QFileInfo(*m_current).exists()) {
                m_current = m_paths.constBegin() + 1;
            }
        }

        // Cache next
        cacheNext();
    }

    QString newPath = currentImagePath();
    QImage image = QImage(newPath);

    emit frameReady(image);

    return true;
}

void SlideEffectPlayer::cacheNext()
{
    QStringList::ConstIterator current = m_current;
    current ++;
    if (current == m_paths.constEnd()) {
        current = m_paths.constBegin();
        if (!QFileInfo(*current).exists()) {
            current = m_paths.constBegin() + 1;
        }
    }
    QString path;
    if (current == m_paths.constEnd())
        path = *m_paths.constBegin();
    else
        path = *current;

    CacheThread *t = new CacheThread(path);
    connect(t, &CacheThread::cached,
            this, [=] (const QString path, const QImage img) {
        m_cacheImages.insert(path, img);
    });
    connect(t, &CacheThread::finished, t, &CacheThread::deleteLater);
    t->start();
}

void SlideEffectPlayer::stop()
{
    if (!isRunning())
        return;

    killTimer(m_tid);
    m_tid = 0;
    m_running = false;
    m_cacheImages.clear();
    Q_EMIT finished();
}
