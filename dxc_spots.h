/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           https://gqrx.dk/
 *
 * Copyright 2020 Oliver Grossmann.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef DXC_SPOTS_H
#define DXC_SPOTS_H

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QColor>
#include <chrono>

struct DXCSpotInfo
{
    qint64  frequency;
    QString name;
    std::chrono::time_point<std::chrono::steady_clock> time;
    QColor color;

    DXCSpotInfo()
    {
        this->frequency = 0;
        this->time = std::chrono::steady_clock::now();
        this->color = Qt::lightGray;
    }

    bool operator<(const DXCSpotInfo &other) const
    {
        return frequency < other.frequency;
    }

    bool operator==(const DXCSpotInfo &other) const
    {
        // we check only the name because frequency can change a bit
        // not good for multi-operator with the case callsign
        return name == other.name;
    }

    const QColor GetColor() const;
};

class DXCSpots : public QObject
{
    Q_OBJECT
public:
    // This is a Singleton Class now because you can not send qt-signals from static functions.
    static void create();
    static DXCSpots& Get();

    void add(DXCSpotInfo& info);
    void setSpotTimeout(int i) { m_DXCSpotTimeout = std::chrono::seconds(i * 60); }
    DXCSpotInfo& getDXCSpot(int i) { return m_DXCSpotList[i]; }
    QList<DXCSpotInfo> getDXCSpotsInRange(qint64 low, qint64 high);

private:
    DXCSpots(); // Singleton Constructor is private.
    QList<DXCSpotInfo> m_DXCSpotList;
    std::chrono::seconds m_DXCSpotTimeout;
    static DXCSpots* m_pThis;

private slots:
    void checkSpotTimeout(void);

signals:
    void dxcSpotsUpdated(void);
};

#endif // DXC_SPOTS_H
