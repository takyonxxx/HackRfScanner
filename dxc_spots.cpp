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
#include <Qt>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QString>
#include <QSet>
#include <QTimer>
#include <algorithm>
#include "dxc_spots.h"
#include <stdio.h>
#include <wchar.h>

DXCSpots* DXCSpots::m_pThis = 0;

DXCSpots::DXCSpots()
{
}

void DXCSpots::create()
{
    m_pThis = new DXCSpots;
}

DXCSpots& DXCSpots::Get()
{
    return *m_pThis;
}

void DXCSpots::add(DXCSpotInfo &info)
{
    info.time = std::chrono::steady_clock::now();
    // check only callsign, so if present remove and re-append
    // if check also frequency we can only change the time
    if (m_DXCSpotList.contains(info))
        m_DXCSpotList.removeAt(m_DXCSpotList.indexOf(info));
    m_DXCSpotList.append(info);
    std::stable_sort(m_DXCSpotList.begin(),m_DXCSpotList.end());
    emit( dxcSpotsUpdated() );
    QTimer::singleShot(m_DXCSpotTimeout, this, SLOT(checkSpotTimeout()));
}

void DXCSpots::checkSpotTimeout()
{
    auto now = std::chrono::steady_clock::now();
    for (int i = 0; i < m_DXCSpotList.size(); i++)
    {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - m_DXCSpotList[i].time);
        if ( m_DXCSpotTimeout <= diff)
        {
            m_DXCSpotList.removeAt(i);
        }
    }
    std::stable_sort(m_DXCSpotList.begin(),m_DXCSpotList.end());
    emit( dxcSpotsUpdated() );
}

QList<DXCSpotInfo> DXCSpots::getDXCSpotsInRange(qint64 low, qint64 high)
{
    DXCSpotInfo info;
    info.frequency=low;
    QList<DXCSpotInfo>::const_iterator lb = std::lower_bound(m_DXCSpotList.begin(), m_DXCSpotList.end(), info);
    info.frequency=high;
    QList<DXCSpotInfo>::const_iterator ub = std::upper_bound(m_DXCSpotList.begin(), m_DXCSpotList.end(), info);

    QList<DXCSpotInfo> found;

    while (lb != ub)
    {
        const DXCSpotInfo& info = *lb;
        found.append(info);
        lb++;
    }

    return found;
}

const QColor DXCSpotInfo::GetColor() const
{
    return DXCSpotInfo::color;
}
