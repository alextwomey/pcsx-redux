/***************************************************************************
 *   Copyright (C) 2022 PCSX-Redux authors                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "cdrom/iso9660-reader.h"

#include "cdrom/iso9660-lowlevel.h"
#include "cdrom/file.h"

PCSX::ISO9660Reader::ISO9660Reader(std::shared_ptr<CDRiso> iso) : m_iso(iso) {
    IO<File> pvdFile(new CDRIsoFile(iso, 16, 2048));
    if (pvdFile->failed()) {
        m_failed = true;
        return;
    }

    ISO9660LowLevel::PVD pvd;
    pvd.deserialize(pvdFile);
}
